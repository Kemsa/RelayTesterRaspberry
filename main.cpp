#include "GPIOHandler.h"
#include "UI/styling.h"
#include "config.h"
#include "contactselector.h"
#include "logbus.h"
#include "mainWindow.h"
#include "powerSupply.h"
#include "readings.h"
#include <QApplication>
#include <QDebug>

QtMessageHandler prevLoggerHandler = nullptr;

void logToFile(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    QString message = qFormatLogMessage(type, context, msg);
    static FILE* f = fopen("log.txt", "a");
    fprintf(f, "%s\n", qPrintable(message));
    fflush(f);

    if (prevLoggerHandler)
        prevLoggerHandler(type, context, msg);
}

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    LogBus::instance().installQtMessageHandler();

    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] %{message}");
    prevLoggerHandler = qInstallMessageHandler(logToFile);

    // initialize components
    Readings::initialize();
    GPIOHandler* gpioHandler = GPIOHandler::setupInstance(false);
    ContactSelector::initialize(CONTACT_SELECT_S0, CONTACT_SELECT_S1, CONTACT_SELECT_S2, CONTACT_SELECT_EN);
    powerSupply::initialize(powerSupply::SupplyType::DMP86xx, SERIAL_PORT_NAME);
    powerSupply::instance()->connect();

    const QString styleSheet = loadResolvedStyleSheet("UI/styles.css", "UI/colors.csv");
    if (!styleSheet.isEmpty()) {
        a.setStyleSheet(styleSheet);
        qDebug() << "Stylesheet applied successfully.";
    }

    MainWindow w;
    w.show();

    qInfo() << "RelayTester application started";

    // return a.exec();
    const int appExitCode = QApplication::exec();
    return appExitCode;
}
