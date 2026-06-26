#include "ADC24.h"
#include "ADCBase.h"
#include "GPIOHandler.h"
#include "mainWindow.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QRegularExpression>
#include <QStringList>

static QString readTextFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}

static QString normalizeColorKey(QString key) {
    key = key.trimmed().toLower();
    key.replace('_', '-');
    return key;
}

static QHash<QString, QString> loadColors(const QString& colorsCsvPath) {
    QHash<QString, QString> colors;
    const QString content = readTextFile(colorsCsvPath);
    const QStringList lines = content.split('\n');

    for (QString line : lines) {
        line = line.trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        const QStringList parts = line.split(';');
        if (parts.size() < 2) {
            continue;
        }

        const QString key = normalizeColorKey(parts.at(0));
        const QString value = parts.mid(1).join(";").trimmed();
        if (!key.isEmpty() && !value.isEmpty()) {
            colors.insert(key, value);
        }
    }

    return colors;
}

static QString applyColorTokens(const QString& styleSheet,
                                const QHash<QString, QString>& colors) {
    static const QRegularExpression tokenPattern("@([A-Za-z0-9_-]+)");

    QString result;
    int lastPos = 0;
    QRegularExpressionMatchIterator it = tokenPattern.globalMatch(styleSheet);

    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        const int start = match.capturedStart(0);
        const int end = match.capturedEnd(0);
        QString key = normalizeColorKey(match.captured(1));

        result += styleSheet.mid(lastPos, start - lastPos);

        QString replacement = colors.value(key);

        result += replacement.isEmpty() ? match.captured(0) : replacement;
        lastPos = end;
    }

    result += styleSheet.mid(lastPos);
    return result;
}

static QString loadResolvedStyleSheet(const QString& styleSheetPath,
                                      const QString& colorsCsvPath) {
    const QString rawStyleSheet = readTextFile(styleSheetPath);
    if (rawStyleSheet.isEmpty()) {
        qWarning() << "Could not read stylesheet:" << styleSheetPath;
        return QString();
    }

    const QHash<QString, QString> colors = loadColors(colorsCsvPath);
    if (colors.isEmpty()) {
        qWarning() << "Could not read colors or no valid color entries found in:" << colorsCsvPath;
        return rawStyleSheet;
    }

    return applyColorTokens(rawStyleSheet, colors);
}

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    const QString styleSheet = loadResolvedStyleSheet("styles.css", "colors.csv");
    if (!styleSheet.isEmpty()) {
        a.setStyleSheet(styleSheet);
        qDebug() << "Stylesheet applied successfully: <<" << styleSheet;
    }

    ADC24 adc;
    GPIOHandler* gpioHandler = GPIOHandler::setupInstance(false);

    MainWindow w;
    w.show();

    // return a.exec();
    return QApplication::exec();
}
