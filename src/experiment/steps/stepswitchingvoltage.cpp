#include "stepswitchingvoltage.h"

#include "contactselector.h"
#include "dynamicreadings.h"
#include "powerSupply.h"
#include "powercontrol.h"
#include "staticreadings.h"
#include <QJsonObject>
#include <QThread>

StepSwitchingVoltage::StepSwitchingVoltage(QString name) : GenericStep(name) {}

static int intValueOrDefault(const QJsonObject& object, const QString& key, int defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isDouble() ? value.toInt(defaultValue) : defaultValue;
}

void StepSwitchingVoltage::fromJSON(const QJsonObject& object) {
    coilToMeasure = intValueOrDefault(object, QStringLiteral("coilToMeasure"), coilToMeasure);
    nContacts = intValueOrDefault(object, QStringLiteral("nContacts"), nContacts);
    startVoltage_cV = intValueOrDefault(object, QStringLiteral("startVoltage_cV"), startVoltage_cV);
    stopVoltage_cV = intValueOrDefault(object, QStringLiteral("stopVoltage_cV"), stopVoltage_cV);
    voltageStep_cV = intValueOrDefault(object, QStringLiteral("voltageStep_cV"), voltageStep_cV);
    maxCurrent_mA = intValueOrDefault(object, QStringLiteral("maxCurrent_mA"), maxCurrent_mA);
    nMeasures = intValueOrDefault(object, QStringLiteral("nMeasures"), nMeasures);

    const QJsonObject successObject = object.value(QStringLiteral("successValues")).toObject();
    successValues.maxSwitchingVoltage_cV = intValueOrDefault(successObject, QStringLiteral("maxSwitchingVoltage_cV"), successValues.maxSwitchingVoltage_cV);
}

QString StepSwitchingVoltage::getName() const {
    return name;
}

QString StepSwitchingVoltage::getDescription() const {
    QString str = QString::fromUtf8(R"(Mesure de la tension de commutation du relais #%1 avec:
    tension de départ: %2 V
    tension de fin: %3 V
    pas de tension: %4 V
)")
                      .arg(coilToMeasure)
                      .arg(startVoltage_cV / 100.0f, 0, 'f', 2)
                      .arg(stopVoltage_cV / 100.0f, 0, 'f', 2)
                      .arg(voltageStep_cV / 100.0f, 0, 'f', 2);
    return str;
}

QString StepSwitchingVoltage::getResultSummary() const {
    return QString();
}

GenericStep::ResultStatus StepSwitchingVoltage::runMeasureAsync(const std::atomic<bool>& stopToken) {
    // Implementation for running the measurement asynchronously

    STEP_CHECK_STOP_TOKEN();

    auto powerControl = PowerControl::getInstance();
    auto dynamicReadings = DynamicReadings::getInstance();
    auto powerSupply = powerSupply::instance();
    auto contactSelector = ContactSelector::instance();
    auto staticReadings = StaticReadings::getInstance();

    if (!powerControl || !staticReadings || !powerSupply || !dynamicReadings || !contactSelector) {
        qCritical() << "One or more required instances are not available. Aborting measurement.";
        return ResultFailure;
    }
    if (!powerControl->checkSafetyStatus()) {
        qCritical() << "Safety status check failed. Aborting measurement.";
        return ResultFailure;
    }

    measurementValues.allSwitched = false;
    measurementValues.switchingVoltage_V = 0.0;

    powerControl->disableCoils();
    powerSupply->setMaxValues(stopVoltage_cV / 100.0, maxCurrent_mA / 1000.0);
    powerSupply->setVoltage(startVoltage_cV / 100.0);
    powerSupply->enableOutput();
    powerControl->enableCoil(static_cast<PowerControl::Coil>(coilToMeasure));
    QThread::msleep(100); // Wait for the coil to stabilize

    StaticReadings::ReadingFlags coilToMeasureFlag = (coilToMeasure == 1) ? StaticReadings::ReadingFlags::coil1Voltage : StaticReadings::ReadingFlags::coil2Voltage;

    for (int voltage_cV = startVoltage_cV; voltage_cV <= stopVoltage_cV; voltage_cV += voltageStep_cV) {
        STEP_CHECK_STOP_TOKEN();

        powerSupply->setVoltage(voltage_cV / 100.0);
        QThread::msleep(100); // Wait for the coil to stabilize

        std::shared_ptr<ADCValue> reading = std::make_shared<ADCValue>();
        staticReadings->getReading(coilToMeasureFlag, reading);
        double measuredVoltage = StaticReadings::toCoilVoltage_V(*reading.get());

        qDebug() << "Measured voltage at" << voltage_cV / 100.0 << "V:" << measuredVoltage << "V";

        bool nowSwitched = true;
        for (int contactIndex = 1; contactIndex <= nContacts; ++contactIndex) {
            STEP_CHECK_STOP_TOKEN();

            contactSelector->selectContact(contactIndex);
            contactSelector->selectHBridge(ContactSelector::HBridge_forward_p1);
            QThread::msleep(1); // Wait for the contact to settle

            bool isContactAClosed = dynamicReadings->isContactClosed(DynamicReadings::ContactType::CONTACT_A);

            contactSelector->selectHBridge(ContactSelector::HBridge_forward_p2);
            QThread::msleep(1); // Wait for the contact to settle

            bool isContactBClosed = dynamicReadings->isContactClosed(DynamicReadings::ContactType::CONTACT_B);

            if (isContactAClosed || !isContactBClosed) {
                nowSwitched = false;
            }

            qDebug() << "Contact" << contactIndex << "status: A closed:" << isContactAClosed << ", B closed:" << isContactBClosed;
        }
        qDebug() << "nowSwitched:" << nowSwitched;
        if (measurementValues.allSwitched == false && nowSwitched == true) {
            measurementValues.allSwitched = nowSwitched;
            measurementValues.switchingVoltage_V = measuredVoltage;
            qDebug() << "All contacts switched at voltage:" << measurementValues.switchingVoltage_V << "V";
        } else if (measurementValues.allSwitched == true && nowSwitched == false) {
            measurementValues.allSwitched = nowSwitched;
            measurementValues.switchingVoltage_V = 0.0;
            qDebug() << "Contacts no longer switched at voltage:" << measuredVoltage << "V";
        }
    }

    powerControl->disableCoils();
    powerSupply->disableOutput();

    if (measurementValues.allSwitched && measurementValues.switchingVoltage_V <= successValues.maxSwitchingVoltage_cV / 100.0) {
        qDebug() << "Switching voltage measurement successful. Switching voltage:" << measurementValues.switchingVoltage_V << "V";
        return ResultSuccess;
    } else {
        qDebug() << "Switching voltage measurement failed. ";
        return ResultFailure;
    }
}