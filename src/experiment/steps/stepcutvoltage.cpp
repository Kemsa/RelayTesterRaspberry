#include "stepcutvoltage.h"

#include <QJsonObject>
#include <QThread>

#include "contactselector.h"
#include "dynamicreadings.h"
#include "powerSupply.h"
#include "powercontrol.h"
#include "staticreadings.h"

StepCutVoltage::StepCutVoltage(QString name) : GenericStep(name) {}

static int intValueOrDefault(const QJsonObject& object, const QString& key, int defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isDouble() ? value.toInt(defaultValue) : defaultValue;
}

void StepCutVoltage::fromJSON(const QJsonObject& object) {
    coilToMeasure = intValueOrDefault(object, QStringLiteral("coilToMeasure"), coilToMeasure);
    nContacts = intValueOrDefault(object, QStringLiteral("nContacts"), nContacts);
    startVoltage_cV = intValueOrDefault(object, QStringLiteral("startVoltage_cV"), startVoltage_cV);
    stopVoltage_cV = intValueOrDefault(object, QStringLiteral("stopVoltage_cV"), stopVoltage_cV);
    voltageStep_cV = intValueOrDefault(object, QStringLiteral("voltageStep_cV"), voltageStep_cV);
    maxCurrent_mA = intValueOrDefault(object, QStringLiteral("maxCurrent_mA"), maxCurrent_mA);
    nMeasures = intValueOrDefault(object, QStringLiteral("nMeasures"), nMeasures);

    const QJsonObject successObject = object.value(QStringLiteral("successValues")).toObject();
    successValues.minCutVoltage_cV = intValueOrDefault(successObject, QStringLiteral("minCutVoltage_cV"), successValues.minCutVoltage_cV);
}

QString StepCutVoltage::getName() const {
    return name;
}

QString StepCutVoltage::getDescription() const {
    QString str = QString::fromUtf8(R"(Mesure de la tension de relachement du relais #%1 avec:
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

QString StepCutVoltage::getResultSummary() const {
    switch (resultStatus) {
    case ResultSuccess:
        return QString::fromUtf8(R"(SUCCES
            Tension de commutation: %1 V (min autorisée: %2 V))")
            .arg(measurementValues.switchingVoltage_V, 0, 'f', 2)
            .arg(successValues.minCutVoltage_cV / 100.0f, 0, 'f', 2);

    case ResultFailure:
        return QString::fromUtf8(R"(ECHEC
        Contacts commutés: %1
        Contacts non commutés: %2)")
            .arg([this]() {
                QStringList switchedContacts;
                for (int i = 0; i < nContacts; ++i) {
                    if (measurementValues.switchedContacts[i]) {
                        switchedContacts.append(QString::number(i + 1));
                    }
                }
                return switchedContacts.join(", ");
            }())
            .arg([this]() {
                QStringList notSwitchedContacts;
                for (int i = 0; i < nContacts; ++i) {
                    if (!measurementValues.switchedContacts[i]) {
                        notSwitchedContacts.append(QString::number(i + 1));
                    }
                }
                return notSwitchedContacts.join(", ");
            }())
            .arg(measurementValues.switchingVoltage_V, 0, 'f', 2);
    default:
        return GenericStep::getResultSummary();
    }
}

GenericStep::ResultStatus StepCutVoltage::runMeasureAsync(const std::atomic<bool>& stopToken) {
    // Implementation for running the measurement asynchronously

    STEP_CHECK_STOP_TOKEN();

    auto powerControl = PowerControl::getInstance();
    auto dynamicReadings = DynamicReadings::getInstance();
    auto powerSupply = powerSupply::instance();
    auto contactSelector = ContactSelector::instance();
    auto staticReadings = StaticReadings::getInstance();

    if (!powerControl || !staticReadings || !powerSupply || !dynamicReadings || !contactSelector) {
        qCritical() << "One or more required instances are not available. Aborting measurement.";
        return ResultCantMeasure;
    }
    if (!powerControl->checkSafetyStatus()) {
        qCritical() << "Safety status check failed. Aborting measurement.";
        return ResultCantMeasure;
    }

    measurementValues.switchingVoltage_V = -1.0; // Initialize to an invalid value
    for (int i = 0; i < 8; ++i) {
        measurementValues.switchedContacts[i] = false; // Initialize all contacts as not switched
    }

    powerControl->disableCoils();
    powerSupply->setMaxValues(stopVoltage_cV / 100.0, maxCurrent_mA / 1000.0);
    powerSupply->setVoltage(startVoltage_cV / 100.0);
    powerSupply->enableOutput();
    powerControl->enableCoil(static_cast<PowerControl::Coil>(coilToMeasure));
    QThread::msleep(100); // Wait for the coil to stabilize

    StaticReadings::ReadingFlags coilToMeasureFlag = (coilToMeasure == 1) ? StaticReadings::ReadingFlags::coil1Voltage : StaticReadings::ReadingFlags::coil2Voltage;

    for (int voltage_cV = startVoltage_cV; voltage_cV >= stopVoltage_cV; voltage_cV -= voltageStep_cV) {
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
            contactSelector->selectHBridge(ContactSelector::HBridge_forward_p2);
            QThread::msleep(1); // Wait for the contact to settle

            bool isContactBClosed = dynamicReadings->isContactClosed(DynamicReadings::ContactType::CONTACT_B);

            contactSelector->selectHBridge(ContactSelector::HBridge_forward_p1);
            QThread::msleep(1); // Wait for the contact to settle

            bool isContactAClosed = dynamicReadings->isContactClosed(DynamicReadings::ContactType::CONTACT_A);

            if (isContactAClosed && !isContactBClosed) {
                measurementValues.switchedContacts[contactIndex - 1] = true;
            } else {
                nowSwitched = false;
            }

            qDebug() << "Contact" << contactIndex << "status: A closed:" << isContactAClosed << ", B closed:" << isContactBClosed;
        }
        qDebug() << "nowSwitched:" << nowSwitched;
        if (isAllSwitched() && measurementValues.switchingVoltage_V < 0.0) {
            measurementValues.switchingVoltage_V = measuredVoltage;
        } else if(!isAllSwitched()) {
            measurementValues.switchingVoltage_V = -1.0;
        }
    }

    powerControl->disableCoils();
    powerSupply->disableOutput();

    if (isAllSwitched() && measurementValues.switchingVoltage_V >= successValues.minCutVoltage_cV / 100.0) {
        qDebug() << "Cut voltage measurement successful. Cut voltage:" << measurementValues.switchingVoltage_V << "V";
        return ResultSuccess;
    } else {
        qDebug() << "Cut voltage measurement failed. ";
        return ResultFailure;
    }
}

bool StepCutVoltage::isAllSwitched() const {
    for (int i = 0; i < nContacts; ++i) {
        if (!measurementValues.switchedContacts[i]) {
            return false;
        }
    }
    return true;
}