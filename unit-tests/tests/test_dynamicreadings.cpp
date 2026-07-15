#include <QtTest>

#include "GPIOHandler.h"
#include "GPIOMockup.h"
#include "dynamicreadings.h"
#include "dynamicswitch.h"

#define COIL1_PIN 1
#define COIL2_PIN 2
#define CONTACT1_PIN 3
#define CONTACT2_PIN 4

#define COIL1_BCM 10
#define COIL2_BCM 11
#define CONTACT1_BCM 12
#define CONTACT2_BCM 13

class DynamicReadingsTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testCleanInterrupt();
    void testComplexInterruptSequence();
    void testTotalFailure();
    void testPartialContactFailure();
    void testTotalContactFailure();
};

void DynamicReadingsTest::initTestCase() {
    GPIOHandler::setupInstance(true);
    DynamicReadings::initialize(COIL1_PIN, COIL2_PIN, CONTACT1_PIN, CONTACT2_PIN); // Use mock GPIO pins for testing
}

void DynamicReadingsTest::cleanupTestCase() {
    GPIOHandler::cleanupInstance();
}

void DynamicReadingsTest::testCleanInterrupt() {
    DynamicReadings* dynamicReadings = DynamicReadings::getInstance();
    QVERIFY(dynamicReadings != nullptr);

    GPIOMockup* mock = static_cast<GPIOMockup*>(GPIOHandler::instance());

    dynamicReadings->clearInterrupts();

    int coilTime = 2000;
    int contact1Time = 10000;
    int contact2Time = 11000;

    // Simulate an interrupt for COIL1
    mock->mockupInterrupt(COIL1_PIN, GPIOHandler::InterruptStatus{1, COIL1_BCM, INT_EDGE_RISING, coilTime});
    mock->mockupInterrupt(CONTACT1_PIN, GPIOHandler::InterruptStatus{1, CONTACT1_BCM, INT_EDGE_FALLING, contact1Time});
    mock->mockupInterrupt(CONTACT2_PIN, GPIOHandler::InterruptStatus{1, CONTACT2_BCM, INT_EDGE_RISING, contact2Time});

    auto res = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL1, 100).get(); // Assuming this triggers the interrupt handler

    QVERIFY(res != nullptr);
    QCOMPARE(res->getCoilStatus().pinBCM, COIL1_BCM);
    QCOMPARE(res->getContact1Status().pinBCM, CONTACT1_BCM);
    QCOMPARE(res->getContact2Status().pinBCM, CONTACT2_BCM);

    QCOMPARE(res->getCoilStatus().statusOK, 1);
    QCOMPARE(res->getContact1Status().statusOK, 1);
    QCOMPARE(res->getContact2Status().statusOK, 1);

    QVERIFY(res->isValid());

    QCOMPARE(res->getContact1SwitchTime(), contact1Time - coilTime);
    QCOMPARE(res->getContact2SwitchTime(), contact2Time - coilTime);

    QCOMPARE(res->getContact1TransistionType(), INT_EDGE_FALLING);
    QCOMPARE(res->getContact2TransistionType(), INT_EDGE_RISING);
}

void DynamicReadingsTest::testComplexInterruptSequence() {
    DynamicReadings* dynamicReadings = DynamicReadings::getInstance();
    QVERIFY(dynamicReadings != nullptr);

    GPIOMockup* mock = static_cast<GPIOMockup*>(GPIOHandler::instance());

    dynamicReadings->clearInterrupts();

    int coilTime = 2000;
    int contact1Time = 10000;
    int contact2Time = 11000;

    // Simulate a complex interrupt sequence
    mock->mockupInterrupt(COIL1_PIN, GPIOHandler::InterruptStatus{1, COIL1_BCM, INT_EDGE_RISING, coilTime});
    mock->mockupInterrupt(COIL1_PIN, GPIOHandler::InterruptStatus{1, COIL1_BCM, INT_EDGE_FALLING, coilTime + 50});
    mock->mockupInterrupt(COIL1_PIN, GPIOHandler::InterruptStatus{1, COIL1_BCM, INT_EDGE_RISING, coilTime + 100});

    mock->mockupInterrupt(CONTACT1_PIN, GPIOHandler::InterruptStatus{1, CONTACT1_BCM, INT_EDGE_FALLING, contact1Time - 100});
    mock->mockupInterrupt(CONTACT1_PIN, GPIOHandler::InterruptStatus{1, CONTACT1_BCM, INT_EDGE_RISING, contact1Time - 50});
    mock->mockupInterrupt(CONTACT1_PIN, GPIOHandler::InterruptStatus{1, CONTACT1_BCM, INT_EDGE_FALLING, contact1Time});

    mock->mockupInterrupt(CONTACT2_PIN, GPIOHandler::InterruptStatus{1, CONTACT2_BCM, INT_EDGE_RISING, contact2Time - 100});
    mock->mockupInterrupt(CONTACT2_PIN, GPIOHandler::InterruptStatus{1, CONTACT2_BCM, INT_EDGE_FALLING, contact2Time - 50});
    mock->mockupInterrupt(CONTACT2_PIN, GPIOHandler::InterruptStatus{1, CONTACT2_BCM, INT_EDGE_RISING, contact2Time});

    auto res = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL1, 100).get(); // Assuming this triggers the interrupt handler

    QVERIFY(res != nullptr);
    QCOMPARE(res->getCoilStatus().pinBCM, COIL1_BCM);
    QCOMPARE(res->getContact1Status().pinBCM, CONTACT1_BCM);
    QCOMPARE(res->getContact2Status().pinBCM, CONTACT2_BCM);

    QCOMPARE(res->getCoilStatus().statusOK, 1);
    QCOMPARE(res->getContact1Status().statusOK, 1);
    QCOMPARE(res->getContact2Status().statusOK, 1);

    QVERIFY(res->isValid());

    QCOMPARE(res->getContact1SwitchTime(), contact1Time - coilTime);
    QCOMPARE(res->getContact2SwitchTime(), contact2Time - coilTime);

    QCOMPARE(res->getContact1TransistionType(), INT_EDGE_FALLING);
    QCOMPARE(res->getContact2TransistionType(), INT_EDGE_RISING);
}

void DynamicReadingsTest::testTotalFailure() {
    DynamicReadings* dynamicReadings = DynamicReadings::getInstance();
    QVERIFY(dynamicReadings != nullptr);

    GPIOMockup* mock = static_cast<GPIOMockup*>(GPIOHandler::instance());

    dynamicReadings->clearInterrupts();

    int coilTime = 2000;

    auto res = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL1, 100).get(); // Assuming this triggers the interrupt handler

    QVERIFY(res != nullptr);
    QCOMPARE(res->getCoilStatus().statusOK, -1);
    QCOMPARE(res->getContact1Status().statusOK, -1);
    QCOMPARE(res->getContact2Status().statusOK, -1);

    QVERIFY(!res->isValid());
}

void DynamicReadingsTest::testPartialContactFailure() {
    DynamicReadings* dynamicReadings = DynamicReadings::getInstance();
    QVERIFY(dynamicReadings != nullptr);

    GPIOMockup* mock = static_cast<GPIOMockup*>(GPIOHandler::instance());

    dynamicReadings->clearInterrupts();

    int coilTime = 2000;
    int contact1Time = 10000;

    // Simulate an interrupt for COIL1 and CONTACT1, but not CONTACT2
    mock->mockupInterrupt(COIL1_PIN, GPIOHandler::InterruptStatus{1, COIL1_BCM, INT_EDGE_RISING, coilTime});
    mock->mockupInterrupt(CONTACT1_PIN, GPIOHandler::InterruptStatus{1, CONTACT1_BCM, INT_EDGE_FALLING, contact1Time});

    auto res = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL1, 100).get(); // Assuming this triggers the interrupt handler

    QVERIFY(res != nullptr);
    QCOMPARE(res->getCoilStatus().statusOK, 1);
    QCOMPARE(res->getContact1Status().statusOK, 1);
    QCOMPARE(res->getContact2Status().statusOK, -1);

    QVERIFY(res->isValid());
}

void DynamicReadingsTest::testTotalContactFailure() {
    DynamicReadings* dynamicReadings = DynamicReadings::getInstance();
    QVERIFY(dynamicReadings != nullptr);

    GPIOMockup* mock = static_cast<GPIOMockup*>(GPIOHandler::instance());

    dynamicReadings->clearInterrupts();

    int coilTime = 2000;

    // Simulate an interrupt for COIL1, but not for CONTACT1 or CONTACT2
    mock->mockupInterrupt(COIL1_PIN, GPIOHandler::InterruptStatus{1, COIL1_BCM, INT_EDGE_RISING, coilTime});

    auto res = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL1, 100).get(); // Assuming this triggers the interrupt handler

    QVERIFY(res != nullptr);
    QCOMPARE(res->getCoilStatus().statusOK, 1);
    QCOMPARE(res->getContact1Status().statusOK, -1);
    QCOMPARE(res->getContact2Status().statusOK, -1);

    QVERIFY(!res->isValid());
}

int runDynamicReadingsTest(int argc, char** argv) {
    DynamicReadingsTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_dynamicreadings.moc"