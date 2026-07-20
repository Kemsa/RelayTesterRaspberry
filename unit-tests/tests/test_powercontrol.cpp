#include <QSignalSpy>
#include <QtTest>

#include "GPIOHandler.h"
#include "GPIOMockup.h"
#include "powercontrol.h"

namespace {
constexpr int kCoil1Pin = 21;
constexpr int kCoil2Pin = 22;
constexpr int kContactPowerEnablePin = 23;
constexpr int kReedPin = 24;
constexpr int kBoardPin = 25;
} // namespace

class PowerControlTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void enableCoil_whenSafe_setsExpectedGpioLevels();
    void enableCoil_whenUnsafe_failsAndKeepsCoilsDisabled();
    void forceCheckSafetyStatus_tracksPinStateAndEmitsSignal();
};

void PowerControlTest::initTestCase() {
    GPIOHandler::setupInstance(true);
    auto* mock = static_cast<GPIOMockup*>(GPIOHandler::instance());
    QVERIFY(mock != nullptr);

    mock->mockupPinWrite(kReedPin, GPIOHandler::WPI_HIGH);
    mock->mockupPinWrite(kBoardPin, GPIOHandler::WPI_HIGH);

    PowerControl::initialize(kCoil1Pin, kCoil2Pin, kContactPowerEnablePin, kReedPin, kBoardPin);
}

void PowerControlTest::cleanupTestCase() {
    GPIOHandler::cleanupInstance();
}

void PowerControlTest::enableCoil_whenSafe_setsExpectedGpioLevels() {
    PowerControl* powerControl = PowerControl::getInstance();
    QVERIFY(powerControl != nullptr);

    auto* mock = static_cast<GPIOMockup*>(GPIOHandler::instance());
    QVERIFY(mock != nullptr);

    mock->mockupPinWrite(kReedPin, GPIOHandler::WPI_HIGH);
    mock->mockupPinWrite(kBoardPin, GPIOHandler::WPI_HIGH);

    QVERIFY(powerControl->checkSafetyStatus());
    QVERIFY(powerControl->enableCoil(PowerControl::COIL1));

    QVERIFY(mock != nullptr);
    QCOMPARE(mock->pinRead(kCoil1Pin), GPIOHandler::WPI_HIGH);
    QCOMPARE(mock->pinRead(kCoil2Pin), GPIOHandler::WPI_LOW);
}

void PowerControlTest::enableCoil_whenUnsafe_failsAndKeepsCoilsDisabled() {
    PowerControl* powerControl = PowerControl::getInstance();
    QVERIFY(powerControl != nullptr);

    auto* mock = static_cast<GPIOMockup*>(GPIOHandler::instance());
    QVERIFY(mock != nullptr);

    // Check with only board disabled
    mock->mockupPinWrite(kReedPin, GPIOHandler::WPI_HIGH);
    mock->mockupPinWrite(kBoardPin, GPIOHandler::WPI_LOW);

    QVERIFY(!powerControl->enableCoil(PowerControl::COIL2));
    QVERIFY(powerControl->checkSafetyStatus() == false);

    QCOMPARE(mock->pinRead(kCoil1Pin), GPIOHandler::WPI_LOW);
    QCOMPARE(mock->pinRead(kCoil2Pin), GPIOHandler::WPI_LOW);

    // Check again with both reed and board disabled
    mock->mockupPinWrite(kReedPin, GPIOHandler::WPI_LOW);
    mock->mockupPinWrite(kBoardPin, GPIOHandler::WPI_LOW);

    QVERIFY(!powerControl->enableCoil(PowerControl::COIL2));
    QVERIFY(powerControl->checkSafetyStatus() == false);

    QCOMPARE(mock->pinRead(kCoil1Pin), GPIOHandler::WPI_LOW);
    QCOMPARE(mock->pinRead(kCoil2Pin), GPIOHandler::WPI_LOW);
}

void PowerControlTest::forceCheckSafetyStatus_tracksPinStateAndEmitsSignal() {
    PowerControl* powerControl = PowerControl::getInstance();
    QVERIFY(powerControl != nullptr);

    auto* mock = static_cast<GPIOMockup*>(GPIOHandler::instance());
    QVERIFY(mock != nullptr);

    QSignalSpy safetySpy(powerControl, &PowerControl::safetyStatusChanged);

    mock->mockupPinWrite(kReedPin, GPIOHandler::WPI_HIGH);
    mock->mockupPinWrite(kBoardPin, GPIOHandler::WPI_HIGH);
    QVERIFY(powerControl->forceCheckSafetyStatus());

    mock->mockupPinWrite(kBoardPin, GPIOHandler::WPI_LOW);
    QVERIFY(!powerControl->forceCheckSafetyStatus());

    QVERIFY(safetySpy.count() >= 2);
    QCOMPARE(safetySpy.at(0).at(0).toBool(), true);
    QCOMPARE(safetySpy.at(safetySpy.count() - 1).at(0).toBool(), false);
}

int runPowerControlTest(int argc, char** argv) {
    PowerControlTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_powercontrol.moc"