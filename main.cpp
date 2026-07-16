#include "GPIOHandler.h"
#include "UI/styling.h"
#include "config.h"
#include "contactselector.h"
#include "dynamicreadings.h"
#include "logbus.h"
#include "mainWindow.h"
#include "powerSupply.h"
#include "powercontrol.h"
#include "staticreadings.h"
#include <QApplication>
#include <QDebug>
#include <QFont>

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
    a.setFont(QFont("Arial"));

    LogBus::instance().installQtMessageHandler();

    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] %{message}");
    prevLoggerHandler = qInstallMessageHandler(logToFile);

    // initialize components
    GPIOHandler* gpioHandler = GPIOHandler::setupInstance(false);
    PowerControl::initialize(COIL1_ENABLE, COIL2_ENABLE, CONTACT_POWER_ENABLE, REED_PIN, BOARD_CLOSED_PIN);
    StaticReadings::initialize();
    DynamicReadings::initialize(COIL1_DETECT, COIL2_DETECT, CONTACT_TRIGGER1, CONTACT_TRIGGER2);
    ContactSelector::initialize(CONTACT_SELECT_S0, CONTACT_SELECT_S1, CONTACT_SELECT_S2, CONTACT_SELECT_EN,
                                H_BRIDGE1, H_BRIDGE2, H_BRIDGE3);
    powerSupply::initialize(powerSupply::SupplyType::DMP86xx, SERIAL_PORT_NAME);
    powerSupply::instance()->connect();

    // Connect securities
    QObject::connect(PowerControl::getInstance(), &PowerControl::safetyStatusChanged,
                     nullptr, [](bool isSafe) {
                         if (!isSafe) {
                             qDebug() << "Safety status changed: Unsafe condition detected. Disabling all contacts.";
                             ContactSelector::instance().selectContact(0); // Deselect contact
                             powerSupply::instance()->disableOutput();     // Disable power supply output
                         }
                     });

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
