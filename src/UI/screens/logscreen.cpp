#include "logscreen.h"
#include "ui_logscreen.h"

#include <QFile>
#include <QRegularExpression>

namespace {
QtMsgType parseQtMsgType(const QString& levelText) {
    const QString normalized = levelText.trimmed().toLower();
    if (normalized == "debug")
        return QtDebugMsg;
    if (normalized == "warning")
        return QtWarningMsg;
    if (normalized == "critical")
        return QtCriticalMsg;
    if (normalized == "fatal")
        return QtFatalMsg;
    if (normalized == "info")
        return QtInfoMsg;
    return QtInfoMsg;
}

QString qtMsgTypeToText(QtMsgType type) {
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("debug");
    case QtWarningMsg:
        return QStringLiteral("warning");
    case QtCriticalMsg:
        return QStringLiteral("critical");
    case QtFatalMsg:
        return QStringLiteral("fatal");
    case QtInfoMsg:
    default:
        return QStringLiteral("info");
    }
}

QString qtMsgTypeColor(QtMsgType type) {
    switch (type) {
    case QtDebugMsg:
        return QStringLiteral("#D0D0D0");
    case QtInfoMsg:
        return QStringLiteral("Green");
    case QtWarningMsg:
        return QStringLiteral("GoldenRod");
    case QtCriticalMsg:
    case QtFatalMsg:
        return QStringLiteral("Red");
    default:
        return QStringLiteral("WhiteSmoke");
    }
}
} // namespace

LogScreen::LogScreen(QWidget* parent)
    : QWidget(parent), ui(new Ui::LogScreen) {
    ui->setupUi(this);
    ui->logText_TB->setReadOnly(true);

    connect(ui->critical_CB, &QCheckBox::toggled, this, &LogScreen::refreshLogDisplay);
    connect(ui->warning_CB, &QCheckBox::toggled, this, &LogScreen::refreshLogDisplay);
    connect(ui->info_CB, &QCheckBox::toggled, this, &LogScreen::refreshLogDisplay);
    connect(ui->debug_CB, &QCheckBox::toggled, this, &LogScreen::refreshLogDisplay);
}

LogScreen::~LogScreen() {
    delete ui;
}

void LogScreen::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);

    QFile logFile("log.txt");
    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_allLogLines.clear();
        ui->logText_TB->clear();
        return;
    }

    const QStringList rawLines = QString::fromUtf8(logFile.readAll()).split('\n', Qt::SkipEmptyParts);
    logFile.close();

    m_allLogLines.clear();
    static const QRegularExpression logPattern(R"(^\[(.*?)] \[(.*?)\] (.*)$)");

    for (const QString& rawLine : rawLines) {
        QRegularExpressionMatch match = logPattern.match(rawLine.trimmed());
        if (!match.hasMatch()) {
            continue;
        }

        Log logEntry;
        logEntry.date = match.captured(1);
        logEntry.type = parseQtMsgType(match.captured(2));
        logEntry.content = match.captured(3);
        m_allLogLines.append(logEntry);
    }

    refreshLogDisplay();
}

void LogScreen::refreshLogDisplay() {
    ui->logText_TB->clear();

    if (m_allLogLines.isEmpty()) {
        return;
    }

    QVector<QtMsgType> selectedLevels;
    if (ui->critical_CB->isChecked())
        selectedLevels << QtCriticalMsg;
    if (ui->warning_CB->isChecked())
        selectedLevels << QtWarningMsg;
    if (ui->info_CB->isChecked())
        selectedLevels << QtInfoMsg;
    if (ui->debug_CB->isChecked())
        selectedLevels << QtDebugMsg;

    if (selectedLevels.isEmpty()) {
        return;
    }

    QStringList filteredLines;
    filteredLines.reserve(m_allLogLines.size());
    for (int i = m_allLogLines.size() - 1; i >= 0; --i) {
        const Log& logEntry = m_allLogLines.at(i);
        if (!selectedLevels.contains(logEntry.type)) {
            continue;
        }

        const QString color = qtMsgTypeColor(logEntry.type);
        filteredLines << QString("<span style=\"color:%1;\">%2</span> %3")
                             .arg(color, logEntry.date, logEntry.content);
    }

    ui->logText_TB->setHtml(filteredLines.join("<br>"));
}
