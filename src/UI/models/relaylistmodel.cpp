#include "relaylistmodel.h"
#include "experiment/jsonvalidator.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace {
QString defaultRelayRootPath() {
    return QDir(QCoreApplication::applicationDirPath()).filePath("JSON_relays");
}
} // namespace

relayListModel::relayListModel(QObject* parent)
    : relayListModel(defaultRelayRootPath(), parent) {}

relayListModel::relayListModel(const QString& rootPath, QObject* parent)
    : QAbstractItemModel(parent),
      m_rootPath(rootPath),
      m_rootNode(std::make_unique<Node>()) {
    m_rootNode->name = QStringLiteral("Relays");
    m_rootNode->path = rootPath;
    reload();
}

int relayListModel::Node::row() const {
    if (parent == nullptr) {
        return 0;
    }

    for (int index = 0; index < static_cast<int>(parent->children.size()); ++index) {
        if (parent->children.at(index).get() == this) {
            return index;
        }
    }

    return 0;
}

QVariant relayListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole || section != 0) {
        return QVariant();
    }

    return QStringLiteral("Relays");
}

QModelIndex relayListModel::index(int row, int column, const QModelIndex& parent) const {
    if (column != 0 || row < 0) {
        return QModelIndex();
    }

    Node* parentNode = nodeFromIndex(parent);
    if (parentNode == nullptr || row >= static_cast<int>(parentNode->children.size())) {
        return QModelIndex();
    }

    return createIndex(row, column, parentNode->children.at(row).get());
}

QModelIndex relayListModel::parent(const QModelIndex& index) const {
    Node* node = nodeFromIndex(index);
    if (node == nullptr || node->parent == nullptr || node->parent == m_rootNode.get()) {
        return QModelIndex();
    }

    return createIndex(node->parent->row(), 0, node->parent);
}

int relayListModel::rowCount(const QModelIndex& parent) const {
    if (parent.column() > 0) {
        return 0;
    }

    Node* parentNode = nodeFromIndex(parent);
    if (parentNode == nullptr) {
        return 0;
    }

    return static_cast<int>(parentNode->children.size());
}

int relayListModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return 1;
}

QVariant relayListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    Node* node = nodeFromIndex(index);
    if (node == nullptr) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return node->name;
    }

    return QVariant();
}

Qt::ItemFlags relayListModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool relayListModel::reload() {
    beginResetModel();

    m_rootNode = std::make_unique<Node>();
    m_rootNode->name = QStringLiteral("Relays");
    m_rootNode->path = m_rootPath;

    m_schema = QJsonObject();
    m_schemaAvailable = false;

    QString schemaError;
    const QString schemaPath = QDir(m_rootPath).filePath(QStringLiteral("relay_schema.json"));
    if (experiment::JsonValidator::loadJsonObject(schemaPath, &m_schema, &schemaError)) {
        m_schemaAvailable = true;
    } else {
        qWarning().noquote() << QStringLiteral("Unable to load relay schema \"%1\": %2").arg(schemaPath, schemaError);
    }

    loadChildren(m_rootNode.get());

    endResetModel();
    return QDir(m_rootPath).exists();
}

QString relayListModel::filePath(const QModelIndex& index) const {
    Node* node = nodeFromIndex(index);
    if (node == nullptr || node->type != NodeType::File) {
        return QString();
    }

    return node->path;
}

bool relayListModel::isFile(const QModelIndex& index) const {
    Node* node = nodeFromIndex(index);
    return node != nullptr && node->type == NodeType::File;
}

void relayListModel::loadChildren(Node* parentNode) {
    QDir directory(parentNode->path);
    const QFileInfoList entries = directory.entryInfoList(
        QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot,
        QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

    for (const QFileInfo& entry : entries) {
        if (entry.isDir()) {
            auto folderNode = std::make_unique<Node>();
            folderNode->name = entry.fileName();
            folderNode->path = entry.absoluteFilePath();
            folderNode->type = NodeType::Folder;
            folderNode->parent = parentNode;

            loadChildren(folderNode.get());

            if (!folderNode->children.empty()) {
                parentNode->children.push_back(std::move(folderNode));
            }
            continue;
        }

        if (!entry.isFile() || entry.suffix().compare(QStringLiteral("json"), Qt::CaseInsensitive) != 0) {
            continue;
        }

        if (entry.fileName().compare(QStringLiteral("relay_schema.json"), Qt::CaseInsensitive) == 0) {
            continue;
        }

        if (!m_schemaAvailable) {
            qWarning().noquote() << QStringLiteral("Skipping relay JSON \"%1\": schema is unavailable").arg(entry.absoluteFilePath());
            continue;
        }

        QJsonObject relayJson;
        QString validationError;
        const bool relayLoaded = experiment::JsonValidator::loadJsonObject(entry.absoluteFilePath(), &relayJson, &validationError);
        if (!relayLoaded || !experiment::JsonValidator::validateValue(relayJson, m_schema, m_schema, QStringLiteral("$"), &validationError)) {
            qWarning().noquote() << QStringLiteral("Skipping invalid relay JSON \"%1\": %2")
                                        .arg(entry.absoluteFilePath(), validationError);
            continue;
        }

        auto fileNode = std::make_unique<Node>();
        fileNode->name = entry.completeBaseName();
        fileNode->path = entry.absoluteFilePath();
        fileNode->type = NodeType::File;
        fileNode->parent = parentNode;
        parentNode->children.push_back(std::move(fileNode));
    }
}

relayListModel::Node* relayListModel::nodeFromIndex(const QModelIndex& index) const {
    if (!index.isValid()) {
        return m_rootNode.get();
    }

    return static_cast<Node*>(index.internalPointer());
}
