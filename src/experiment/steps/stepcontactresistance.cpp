#include "stepcontactresistance.h"

#include <QJsonObject>
#include <QThread>

#include "contactselector.h"
#include "currentadjuster.h"
#include "dynamicreadings.h"
#include "powerSupply.h"
#include "powercontrol.h"
#include "staticreadings.h"

StepContactResistance::StepContactResistance(QString name) : GenericStep(name) {}

static int intValueOrDefault(const QJsonObject& object, const QString& key, int defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isDouble() ? value.toInt(defaultValue) : defaultValue;
}

static QString stringValueOrDefault(const QJsonObject& object, const QString& key, const QString& defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isString() ? value.toString() : defaultValue;
}

void StepContactResistance::fromJSON(const QJsonObject& object) {
    coilToPowerOn = intValueOrDefault(object, QStringLiteral("coilToPowerOn"), coilToPowerOn);
    coilToPowerOff = intValueOrDefault(object, QStringLiteral("coilToPowerOff"), coilToPowerOff);
    contactDirection = stringValueOrDefault(object, QStringLiteral("contactDirection"), contactDirection);
    nContacts = intValueOrDefault(object, QStringLiteral("nContacts"), nContacts);
    supplyVoltage_cV = intValueOrDefault(object, QStringLiteral("supplyVoltage_cV"), supplyVoltage_cV);
    maxCurrent_mA = intValueOrDefault(object, QStringLiteral("maxCurrent_mA"), maxCurrent_mA);
    nMeasures = intValueOrDefault(object, QStringLiteral("nMeasures"), nMeasures);
    nCycles = intValueOrDefault(object, QStringLiteral("nCycles"), nMeasures);

    const QJsonObject successObject = object.value(QStringLiteral("successValues")).toObject();
    successValues.maxResistanceClosed_mOhm = intValueOrDefault(successObject, QStringLiteral("maxResistanceClosed_mOhm"), successValues.maxResistanceClosed_mOhm);
    successValues.minResistanceOpened_kOhm = intValueOrDefault(successObject, QStringLiteral("minResistanceOpened_kOhm"), successValues.minResistanceOpened_kOhm);
}

QString StepContactResistance::getName() const {
    return name;
}

QString StepContactResistance::getDescription() const {
    QString str = QString::fromUtf8(R"(Mesure de la résistance de la contact sur %1 contacts. La résistance est mesurée dans les deux directions avec un courant de 10mA et une tension de 6V.
    avec:
	bobine "ON": %2
	bobine "OFF": %3
	tension de bobine (V): %4
	courant de bobine maximum (mA): %6 mA
	nombre de mesures: %7
)")
                      .arg(nContacts)
                      .arg(coilToPowerOn)
                      .arg(coilToPowerOff)
                      .arg(supplyVoltage_cV / 100.0f, 0, 'f', 2)
                      .arg(maxCurrent_mA)
                      .arg(nMeasures);
    return str;
}

QString StepContactResistance::getResultSummary() const {
    return QString();
}

GenericStep::ResultStatus StepContactResistance::runMeasureAsync(const std::atomic<bool>& stopToken) {
    // Implementation for running the measurement asynchronously

    STEP_CHECK_STOP_TOKEN();

    auto powerControl = PowerControl::getInstance();
    auto dynamicReadings = DynamicReadings::getInstance();
    auto powerSupply = powerSupply::instance();
    auto contactSelector = ContactSelector::instance();
    auto staticReadings = StaticReadings::getInstance();
    auto currentAdjuster = CurrentAdjuster::instance();

    if (!powerControl || !staticReadings || !powerSupply || !dynamicReadings || !contactSelector) {
        qCritical() << "One or more required instances are not available. Aborting measurement.";
        return ResultFailure;
    }
    if (!powerControl->checkSafetyStatus()) {
        qCritical() << "Safety status check failed. Aborting measurement.";
        return ResultFailure;
    }

    contactSelector->selectContact(0); // disable contact to start with a known state

    powerSupply->setMaxValues(supplyVoltage_cV / 100.0, maxCurrent_mA / 1000.0);
    powerSupply->setVoltage(supplyVoltage_cV / 100.0);
    powerSupply->enableOutput();

    QThread::msleep(100); // Wait for the coil to stabilize

    measurementValues.averageResistanceContactA_Ohm.clear();
    measurementValues.averageResistanceContactB_Ohm.clear();
    measurementValues.averageResistanceContactA_Ohm.reserve(nCycles);
    measurementValues.averageResistanceContactB_Ohm.reserve(nCycles);
    for (int i = 0; i < nCycles; ++i) {
        measurementValues.averageResistanceContactA_Ohm.emplace_back(nContacts, 0.0);
        measurementValues.averageResistanceContactB_Ohm.emplace_back(nContacts, 0.0);
    }

    for (int i = 0; i < nCycles; ++i) {
        STEP_CHECK_STOP_TOKEN();
        qDebug() << "Starting cycle" << (i + 1) << "of" << nCycles;

        // Measure with relay "OFF"
        if (coilToPowerOff != 0) {
            powerControl->enableCoil(static_cast<PowerControl::Coil>(coilToPowerOff));
        } else {
            powerControl->disableCoils();
        }
        QThread::msleep(10);

        // Measure "normally closed" contact
        for (int c = 1; c <= nContacts; c++) {
            STEP_CHECK_STOP_TOKEN();
            qDebug() << "Measuring contact" << c << "with relay OFF";

            contactSelector->selectHBridge(ContactSelector::HBridge_forward_all);
            contactSelector->selectContact(c);
            QThread::msleep(10);
            // currentAdjuster->adjustCurrentToTarget(10, 0.8, false);

            double meanResistanceForward_Ohm = getResistanceForContact(StaticReadings::ReadingFlags::contactAVoltage, nMeasures, stopToken);
            qDebug() << "Contact" << c << "mean resistance forward(Ohm):" << meanResistanceForward_Ohm;

            contactSelector->selectHBridge(ContactSelector::HBridge_reverse_all);
            QThread::msleep(10);
            double meanResistanceReverse_Ohm = getResistanceForContact(StaticReadings::ReadingFlags::contactAVoltage, nMeasures, stopToken);
            qDebug() << "Contact" << c << "mean resistance reverse(Ohm):" << meanResistanceReverse_Ohm;

            measurementValues.averageResistanceContactA_Ohm[i][c - 1] = (meanResistanceForward_Ohm + meanResistanceReverse_Ohm) / 2.0;
        }
        contactSelector->selectContact(0); // disable contact between measures
        QThread::msleep(10);

        // Measure with relay "ON"
        if (coilToPowerOn != 0) {
            powerControl->enableCoil(static_cast<PowerControl::Coil>(coilToPowerOn));
        } else {
            powerControl->disableCoils();
        }

        // Measure "normally open" contact
        for (int c = 1; c <= nContacts; c++) {
            STEP_CHECK_STOP_TOKEN();
            qDebug() << "Measuring contact" << c << "with relay ON";

            contactSelector->selectHBridge(ContactSelector::HBridge_forward_all);
            contactSelector->selectContact(c);
            QThread::msleep(10);
            // currentAdjuster->adjustCurrentToTarget(10, 0.8, false);

            double meanResistanceForward_Ohm = getResistanceForContact(StaticReadings::ReadingFlags::contactBVoltage, nMeasures, stopToken);
            qDebug() << "Contact" << c << "mean resistance forward(Ohm):" << meanResistanceForward_Ohm;
            measurementValues.averageResistanceContactB_Ohm[i][c - 1] = meanResistanceForward_Ohm;

            contactSelector->selectHBridge(ContactSelector::HBridge_reverse_all);
            QThread::msleep(10);
            double meanResistanceReverse_Ohm = getResistanceForContact(StaticReadings::ReadingFlags::contactBVoltage, nMeasures, stopToken);
            qDebug() << "Contact" << c << "mean resistance reverse(Ohm):" << meanResistanceReverse_Ohm;

            measurementValues.averageResistanceContactB_Ohm[i][c - 1] = (meanResistanceForward_Ohm + meanResistanceReverse_Ohm) / 2.0;
        }
        contactSelector->selectContact(0); // disable contact between measures
        QThread::msleep(10);
    }

    powerControl->disableCoils();

    bool success = true;
    // check results against success criteria
    for (int cycle = 0; cycle < nCycles; cycle++) {
        for (int c = 1; c <= nContacts; c++) {
            double resistanceContactA_Ohm = measurementValues.averageResistanceContactA_Ohm[cycle][c - 1];
            double resistanceContactB_Ohm = measurementValues.averageResistanceContactB_Ohm[cycle][c - 1];

            if (resistanceContactA_Ohm > successValues.maxResistanceClosed_mOhm / 1000.0) {
                qWarning() << "Cycle" << (cycle + 1) << "Contact" << c << "closed resistance A" << resistanceContactA_Ohm << "Ohm exceeds maximum allowed" << successValues.maxResistanceClosed_mOhm / 1000.0 << "Ohm";
                success = false;
            }
            if (resistanceContactB_Ohm > successValues.maxResistanceClosed_mOhm / 1000.0) {
                qWarning() << "Cycle" << (cycle + 1) << "Contact" << c << "closed resistance B" << resistanceContactB_Ohm << "Ohm exceeds maximum allowed" << successValues.maxResistanceClosed_mOhm / 1000.0 << "Ohm";
                success = false;
            }
        }
    }

    return success ? ResultSuccess : ResultFailure;
}

double StepContactResistance::getResistanceForContact(StaticReadings::ReadingFlags contactFlag, int nMeasures, const std::atomic<bool>& stopToken) {

    auto staticReadings = StaticReadings::getInstance();

    ADCValue voltages[nMeasures];
    ADCValue currents[nMeasures];

    staticReadings->getNReadings(contactFlag, nMeasures, voltages, ADCBase::Caliber_Auto);
    staticReadings->getNReadings(StaticReadings::ReadingFlags::contactCurrent, nMeasures, currents, ADCBase::Caliber_Auto);

    double meanVoltage = 0.0;
    double meanCurrent = 0.0;

    for (int j = 0; j < nMeasures; ++j) {
        meanVoltage += staticReadings->toContactVoltage_mV(voltages[j]);
        meanCurrent += staticReadings->toContactCurrent_mA(currents[j]);
        qDebug() << "Measure" << j + 1 << ": Voltage (mV):" << staticReadings->toContactVoltage_mV(voltages[j]) << ", Current (mA):" << staticReadings->toContactCurrent_mA(currents[j]);
    }
    meanVoltage /= nMeasures;
    meanCurrent /= nMeasures;

    qDebug() << "Mean voltage (mV):" << meanVoltage << ", Mean current (mA):" << meanCurrent;

    double meanResistance_Ohm = (meanCurrent != 0.0) ? (meanVoltage / meanCurrent) : std::numeric_limits<double>::infinity();
    return meanResistance_Ohm; // Return in ohms
}