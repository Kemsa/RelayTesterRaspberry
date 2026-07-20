#include <QSignalSpy>
#include <QtTest>

#include "GPIOHandler.h"
#include "contactselector.h"

namespace {
constexpr int kS0 = 10;
constexpr int kS1 = 11;
constexpr int kS2 = 12;
constexpr int kEnable = 13;
constexpr int kHBridge1 = 14;
constexpr int kHBridge2 = 15;
constexpr int kHBridge3 = 16;
} // namespace

class ContactSelectorTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void selectContact_updatesGpioStateAndEmitsSignal();
    void selectContact_sameValueDoesNotEmitSignal();
    void selectHBridge_updatesGpioStateAndEmitsSignal();
    void selectHBridge_sameValueDoesNotEmitSignal();
};

void ContactSelectorTest::initTestCase() {
    GPIOHandler::setupInstance(true);
    qRegisterMetaType<ContactSelector::HBridge_options>("HBridge_options");
    ContactSelector::initialize(kS0, kS1, kS2, kEnable, kHBridge1, kHBridge2, kHBridge3);
}

void ContactSelectorTest::cleanupTestCase() {
    GPIOHandler::cleanupInstance();
}

void ContactSelectorTest::selectContact_updatesGpioStateAndEmitsSignal() {
    ContactSelector& selector = ContactSelector::instance();
    QSignalSpy spy(&selector, &ContactSelector::contactSelected);

    selector.selectContact(3);

    QCOMPARE(selector.currentContact(), 3);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 3);

    GPIOHandler* gpio = GPIOHandler::instance();
    QVERIFY(gpio != nullptr);

    QCOMPARE(gpio->pinRead(kEnable), GPIOHandler::WPI_HIGH);
    QCOMPARE(gpio->pinRead(kS0), GPIOHandler::WPI_LOW);
    QCOMPARE(gpio->pinRead(kS1), GPIOHandler::WPI_HIGH);
    QCOMPARE(gpio->pinRead(kS2), GPIOHandler::WPI_LOW);
}

void ContactSelectorTest::selectContact_sameValueDoesNotEmitSignal() {
    ContactSelector& selector = ContactSelector::instance();
    selector.selectContact(4);
    QSignalSpy spy(&selector, &ContactSelector::contactSelected);

    selector.selectContact(4);

    QCOMPARE(spy.count(), 0);
}

void ContactSelectorTest::selectHBridge_updatesGpioStateAndEmitsSignal() {
    ContactSelector& selector = ContactSelector::instance();
    QSignalSpy spy(&selector, &ContactSelector::hBridgeOptionSelected);

    selector.selectHBridge(ContactSelector::HBridge_reverse_p1);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), static_cast<int>(ContactSelector::HBridge_reverse_p1));

    GPIOHandler* gpio = GPIOHandler::instance();
    QVERIFY(gpio != nullptr);

    QCOMPARE(gpio->pinRead(kHBridge1), GPIOHandler::WPI_HIGH);
    QCOMPARE(gpio->pinRead(kHBridge2), GPIOHandler::WPI_LOW);
    QCOMPARE(gpio->pinRead(kHBridge3), GPIOHandler::WPI_HIGH);
}

void ContactSelectorTest::selectHBridge_sameValueDoesNotEmitSignal() {
    ContactSelector& selector = ContactSelector::instance();
    selector.selectHBridge(ContactSelector::HBridge_reverse_p1);
    QSignalSpy spy(&selector, &ContactSelector::hBridgeOptionSelected);

    selector.selectHBridge(ContactSelector::HBridge_reverse_p1);

    QCOMPARE(spy.count(), 0);
}

int runContactSelectorTest(int argc, char** argv) {
    ContactSelectorTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "test_contactselector.moc"
