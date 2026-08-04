#include "relaymeasurescreen.h"
#include "experiment/jsonvalidator.h"
#include "navigator.h"
#include "ui_relaymeasurescreen.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QShowEvent>
#include <memory>

namespace {

QString findSchemaPath(const QString& relayFilePath) {
    QFileInfo fileInfo(relayFilePath);
    QDir directory = fileInfo.absoluteDir();

    while (true) {
        const QString schemaCandidate = directory.filePath(QStringLiteral("relay_schema.json"));
        if (QFile::exists(schemaCandidate)) {
            return schemaCandidate;
        }
        if (!directory.cdUp()) {
            break;
        }
    }

    return QString();
}

} // namespace

RelayMeasureScreen::RelayMeasureScreen(QWidget* parent)
    : QWidget(parent), ui(new Ui::RelayMeasureScreen) {
    ui->setupUi(this);
}

RelayMeasureScreen::~RelayMeasureScreen() {
    delete ui;
}

void RelayMeasureScreen::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);

    // Check navigator exchange data for this screen
    auto data = Navigator::instance().getScreenExchangeData(Navigator::RelayMeasure_screen);
    if (data) {
        auto strptr = std::static_pointer_cast<QString>(data);
        if (strptr) {
            emit navigatedTo(*strptr);
            onNavigatedTo(*strptr);
            return;
        }
    }

    emit navigatedTo(QString());
    onNavigatedTo(QString());
}

void RelayMeasureScreen::onNavigatedTo(const QString& path) {
    qDebug() << "RelayMeasureScreen navigated to path:" << path;

    m_relayMeasure = std::make_unique<RelayMeasure>();

    if (path.isEmpty()) {
        qDebug() << "RelayMeasureScreen: no path provided";
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "RelayMeasureScreen: failed to open relay JSON file:" << path;
        return;
    }

    const QString jsonContent = QString::fromUtf8(file.readAll());
    file.close();

    const QString schemaPath = findSchemaPath(path);
    if (schemaPath.isEmpty()) {
        qWarning() << "RelayMeasureScreen: relay schema not found for" << path;
        return;
    }

    QJsonObject schemaObject;
    QString schemaError;
    if (!experiment::JsonValidator::loadJsonObject(schemaPath, &schemaObject, &schemaError)) {
        qWarning() << "RelayMeasureScreen: unable to load schema" << schemaPath << ":" << schemaError;
        return;
    }

    QJsonObject relayObject;
    QString relayError;
    if (!experiment::JsonValidator::loadJsonObject(path, &relayObject, &relayError)) {
        qWarning() << "RelayMeasureScreen: invalid relay JSON" << path << ":" << relayError;
        return;
    }

    QString validationError;
    if (!experiment::JsonValidator::validateValue(relayObject, schemaObject, schemaObject, QStringLiteral("$"), &validationError)) {
        qWarning() << "RelayMeasureScreen: relay JSON does not validate against schema" << path << ":" << validationError;
        return;
    }

    m_relayMeasure->fromJSON(jsonContent);
}
