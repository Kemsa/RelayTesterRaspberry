#include "logscreen.h"
#include "ui_logscreen.h"

#include <QDateTime>
#include <QFile>
#include <QMessageBox>
#include <QRegularExpression>
#include <QTextStream>

namespace {
QDateTime parseLogDateTime(const QString& dateText) {
    const QString cleaned = dateText.trimmed();
    const QStringList formats = {
        QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz"),
        QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"),
        QStringLiteral("yyyy-MM-dd hh:mm:ss"),
        QStringLiteral("yyyy-MM-dd HH:mm:ss"),
        QStringLiteral("yyyy-MM-dd")};

    for (const QString& format : formats) {
        const QDateTime parsed = QDateTime::fromString(cleaned, format);
        if (parsed.isValid())
            return parsed;
    }

    return QDateTime();
}

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
    populateLogDurationOptions();

    connect(ui->critical_CB, &QCheckBox::toggled, this, &LogScreen::refreshLogDisplay);
    connect(ui->warning_CB, &QCheckBox::toggled, this, &LogScreen::refreshLogDisplay);
    connect(ui->info_CB, &QCheckBox::toggled, this, &LogScreen::refreshLogDisplay);
    connect(ui->debug_CB, &QCheckBox::toggled, this, &LogScreen::refreshLogDisplay);
    connect(ui->clearLogs_PB, &QPushButton::clicked, this, &LogScreen::clearOldLogs);
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

void LogScreen::populateLogDurationOptions() {
    ui->logDate_CB->addItem(QStringLiteral("Un an"), QVariant(QStringLiteral("1year")));
    ui->logDate_CB->addItem(QStringLiteral("Un mois"), QVariant(QStringLiteral("1month")));
    ui->logDate_CB->addItem(QStringLiteral("Une semaine"), QVariant(QStringLiteral("1week")));
    ui->logDate_CB->addItem(QStringLiteral("Un jour"), QVariant(QStringLiteral("1day")));
    ui->logDate_CB->setCurrentIndex(1);
}

QDateTime LogScreen::getThresholdForSelectedDuration() const {
    const QString selectedValue = ui->logDate_CB->currentData().toString();
    const QDateTime now = QDateTime::currentDateTime();

    if (selectedValue == "1year")
        return now.addYears(-1);
    if (selectedValue == "1month")
        return now.addMonths(-1);
    if (selectedValue == "1week")
        return now.addDays(-7);
    return now.addDays(-1);
}

void LogScreen::clearOldLogs() {
    const QDateTime threshold = getThresholdForSelectedDuration();
    const QString selectedLabel = ui->logDate_CB->currentText();

    const int answer = QMessageBox::question(this,
                                             QStringLiteral("Confirmation"),
                                             QString("Voulez-vous vraiment supprimer les journaux plus anciens que %1 ?")
                                                 .arg(selectedLabel),
                                             QMessageBox::Yes | QMessageBox::No,
                                             QMessageBox::No);

    if (answer != QMessageBox::Yes)
        return;

    QFile logFile("log.txt");
    if (!logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    const QStringList rawLines = QString::fromUtf8(logFile.readAll()).split('\n', Qt::SkipEmptyParts);
    logFile.close();

    QStringList preservedLines;
    preservedLines.reserve(rawLines.size());
    static const QRegularExpression logPattern(R"(^\[(.*?)] \[(.*?)\] (.*)$)");

    for (const QString& rawLine : rawLines) {
        const QString trimmed = rawLine.trimmed();
        if (trimmed.isEmpty())
            continue;

        QRegularExpressionMatch match = logPattern.match(trimmed);
        if (!match.hasMatch()) {
            continue;
        }

        const QString dateText = match.captured(1);
        const QDateTime logDate = parseLogDateTime(dateText);
        if (logDate.isValid() && logDate >= threshold) {
            preservedLines << rawLine;
        }
    }

    if (preservedLines.isEmpty()) {
        logFile.resize(0);
        logFile.close();
        m_allLogLines.clear();
        refreshLogDisplay();
        return;
    }

    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }

    QTextStream out(&logFile);
    out << preservedLines.join('\n');
    out << '\n';
    logFile.close();

    m_allLogLines.clear();
    for (const QString& rawLine : preservedLines) {
        const QString trimmed = rawLine.trimmed();
        if (trimmed.isEmpty())
            continue;

        QRegularExpressionMatch match = logPattern.match(trimmed);
        if (!match.hasMatch())
            continue;

        Log logEntry;
        logEntry.date = match.captured(1);
        logEntry.type = parseQtMsgType(match.captured(2));
        logEntry.content = match.captured(3);
        m_allLogLines.append(logEntry);
    }

    refreshLogDisplay();
}
