#include "relaymeasurescreen.h"
#include "experiment/jsonvalidator.h"
#include "navigator.h"
#include "ui_relaymeasurescreen.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>
#include <QShowEvent>
#include <QSize>
#include <QStyle>
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

static QIcon paintStatusIcon(const QIcon& sourceIcon, const QSize& size) {
    const QSize iconSize(size.width(), size.height());
    QPixmap pixmap(iconSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    const QColor backgroundColor(225, 225, 225); // light-grey from colors.csv
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor);
    painter.drawEllipse(QRectF(0, 0, iconSize.width(), iconSize.height()));

    const QSize innerSize(qRound(iconSize.width() * 0.6), qRound(iconSize.height() * 0.6));
    const QPixmap sourcePixmap = sourceIcon.pixmap(innerSize, QIcon::Normal, QIcon::On);
    const QPoint center((iconSize.width() - sourcePixmap.width()) / 2,
                        (iconSize.height() - sourcePixmap.height()) / 2);
    painter.drawPixmap(center, sourcePixmap);

    return QIcon(pixmap);
}

RelayMeasureScreen::RelayMeasureScreen(QWidget* parent)
    : QWidget(parent), ui(new Ui::RelayMeasureScreen) {
    ui->setupUi(this);
    ui->measureSteps_LW->setIconSize(QSize(24, 24));

    // Update step description when the selected step changes
    connect(ui->measureSteps_LW, &QListWidget::currentRowChanged, this, [this](int row) {
        if (m_relayMeasure) {
            ui->stepDescription_TB->setText(m_relayMeasure->getStepDescription(row));
        } else {
            ui->stepDescription_TB->clear();
        }
    });
}

RelayMeasureScreen::~RelayMeasureScreen() {
    delete ui;
}

QIcon RelayMeasureScreen::iconForResultStatus(GenericStep::ResultStatus status) const {
    const QIcon baseIcon = [&] {
        switch (status) {
        case GenericStep::ResultSuccess:
            return QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton);
        case GenericStep::ResultFailure:
            return QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
        case GenericStep::ResultMeasuring:
            return QApplication::style()->standardIcon(QStyle::SP_BrowserReload);
        case GenericStep::ResultNotStarted:
            return QApplication::style()->standardIcon(QStyle::SP_TitleBarContextHelpButton);
        case GenericStep::ResultUnknown:
        default:
            return QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion);
        }
    }();

    return paintStatusIcon(baseIcon, QSize(24, 24));
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

    m_relayMeasure = std::make_unique<RelayMeasure>(schemaObject);
    m_relayMeasure->fromJSON(jsonContent);

    ui->RelayName_lbl->setText(m_relayMeasure->getModel());

    ui->measureSteps_LW->clear();
    const QMap<int, QString> steps = m_relayMeasure->getSteps();
    for (auto it = steps.constBegin(); it != steps.constEnd(); ++it) {
        const int index = it.key();
        const QString name = it.value();
        auto* stepItem = new QListWidgetItem(name);
        // choose an icon based on the step result status
        stepItem->setIcon(iconForResultStatus(m_relayMeasure->getStepResultStatus(index)));
        ui->measureSteps_LW->addItem(stepItem);
    }

    if (ui->measureSteps_LW->count() > 0) {
        ui->measureSteps_LW->setCurrentRow(0);
    } else {
        ui->stepDescription_TB->clear();
    }
}
