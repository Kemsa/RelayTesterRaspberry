#ifndef RELAYLISTMODEL_H
#define RELAYLISTMODEL_H

#include <QAbstractItemModel>
#include <QJsonObject>

#include <memory>
#include <vector>

class QString;

class relayListModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit relayListModel(QObject* parent = nullptr);
    explicit relayListModel(const QString& rootPath, QObject* parent = nullptr);

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool reload();
    QString filePath(const QModelIndex& index) const;
    bool isFile(const QModelIndex& index) const;

private:
    enum class NodeType {
        Root,
        Folder,
        File
    };

    struct Node {
        QString name;
        QString path;
        NodeType type = NodeType::Root;
        Node* parent = nullptr;
        std::vector<std::unique_ptr<Node>> children;

        int row() const;
    };

    void loadChildren(Node* parentNode);
    Node* nodeFromIndex(const QModelIndex& index) const;

    QString m_rootPath;
    std::unique_ptr<Node> m_rootNode;
    QJsonObject m_schema;
    bool m_schemaAvailable = false;
};

#endif // RELAYLISTMODEL_H
