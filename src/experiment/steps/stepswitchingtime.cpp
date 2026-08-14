#include "stepswitchingtime.h"

#include <QJsonObject>
#include <QThread>

#include "contactselector.h"
#include "currentadjuster.h"
#include "dynamicreadings.h"
#include "dynamicswitch.h"
#include "powerSupply.h"
#include "powercontrol.h"

#include <math.h>

#define SWITCH_WAIT_TIME_MS 50 // ms

StepSwitchingTime::StepSwitchingTime(QString name) : GenericStep(name) {}

static int intValueOrDefault(const QJsonObject& object, const QString& key, int defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isDouble() ? value.toInt(defaultValue) : defaultValue;
}

static QString stringValueOrDefault(const QJsonObject& object, const QString& key, const QString& defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isString() ? value.toString() : defaultValue;
}

void StepSwitchingTime::fromJSON(const QJsonObject& object) {
    coilToPowerOn = intValueOrDefault(object, QStringLiteral("coilToPowerOn"), coilToPowerOn);
    coilToPowerOff = intValueOrDefault(object, QStringLiteral("coilToPowerOff"), coilToPowerOff);
    nContacts = intValueOrDefault(object, QStringLiteral("nContacts"), nContacts);
    switchCount = intValueOrDefault(object, QStringLiteral("switchCount"), switchCount);
    supplyVoltage_cV = intValueOrDefault(object, QStringLiteral("supplyVoltage_cV"), supplyVoltage_cV);
    maxCurrent_mA = intValueOrDefault(object, QStringLiteral("maxCurrent_mA"), maxCurrent_mA);

    const QJsonObject successObject = object.value(QStringLiteral("successValues")).toObject();
    successValues.maxWorkTime_ms = intValueOrDefault(successObject, QStringLiteral("maxWorkTime_ms"), successValues.maxWorkTime_ms);
    successValues.maxWorkTimeRebound_ms = intValueOrDefault(successObject, QStringLiteral("maxWorkTimeRebound_ms"), successValues.maxWorkTimeRebound_ms);
    successValues.maxCutTime_ms = intValueOrDefault(successObject, QStringLiteral("maxCutTime_ms"), successValues.maxCutTime_ms);
    successValues.maxCutTimeRebound_ms = intValueOrDefault(successObject, QStringLiteral("maxCutTimeRebound_ms"), successValues.maxCutTimeRebound_ms);
}

QString StepSwitchingTime::getName() const {
    return name;
}

QString StepSwitchingTime::getDescription() const {
    QString str = QString::fromUtf8(R"(Mesure du temps de commutation d'un relais: bascule du relais et mesure des temps pour chaque contact.
        bobine "On": %1
        bobine "Off": %2
        Nombre de contacts: %3
        Nombre de commutations: %4
        Tension de bobine (V): %5
        Courant max (mA): %6

    )")
                      .arg(coilToPowerOn)
                      .arg(coilToPowerOff)
                      .arg(nContacts)
                      .arg(switchCount)
                      .arg(supplyVoltage_cV / 100.0, 0, 'f', 2)
                      .arg(maxCurrent_mA);
    return str;
}

QString StepSwitchingTime::getFormattedResults() const {

    QString str;
    str.append(QString("Temps de travail max (ms): %1\n")
                   .arg(successValues.maxWorkTime_ms));
    str.append(QString("Temps de travail rebond max (ms): %1\n")
                   .arg(successValues.maxWorkTimeRebound_ms));
    str.append(QString("Temps de coupure max (ms): %1\n")
                   .arg(successValues.maxCutTime_ms));
    str.append(QString("Temps de coupure rebond max (ms): %1\n")
                   .arg(successValues.maxCutTimeRebound_ms));

    str.append("résultats formattés:\n");
    str.append("temps x/y: temps de travail (ms)/temps de rebond travail(ms)/temps de coupure (ms)/temps de rebond de coupure (ms)\n");
    str.append("deviation x/y: temps de travail (ms)/temps de rebond travail(ms)/temps de coupure (ms)/temps de rebond de coupure (ms)\n");

    for (int c = 0; c < nContacts; ++c) {
        str += QString("Contact %1/A:\n").arg(c + 1);

        double avrgWorkTimeA_ms = 0, avrgWorkTimeReboundA_ms = 0, avrgCutTimeA_ms = 0, avrgCutTimeReboundA_ms = 0;
        double avrgWorkTimeB_ms = 0, avrgWorkTimeReboundB_ms = 0, avrgCutTimeB_ms = 0, avrgCutTimeReboundB_ms = 0;
        for (int i = 0; i < switchCount; ++i) {
            avrgWorkTimeA_ms += measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTime][i] / 1000.0;
            avrgWorkTimeReboundA_ms += measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkReboundTime][i] / 1000.0;
            avrgCutTimeA_ms += measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTime][i] / 1000.0;
            avrgCutTimeReboundA_ms += measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] / 1000.0;

            avrgWorkTimeB_ms += measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkTime][i] / 1000.0;
            avrgWorkTimeReboundB_ms += measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkReboundTime][i] / 1000.0;
            avrgCutTimeB_ms += measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTime][i] / 1000.0;
            avrgCutTimeReboundB_ms += measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] / 1000.0;
        }

        avrgWorkTimeA_ms /= switchCount;
        avrgWorkTimeReboundA_ms /= switchCount;
        avrgCutTimeA_ms /= switchCount;
        avrgCutTimeReboundA_ms /= switchCount;

        avrgWorkTimeB_ms /= switchCount;
        avrgWorkTimeReboundB_ms /= switchCount;
        avrgCutTimeB_ms /= switchCount;
        avrgCutTimeReboundB_ms /= switchCount;

        double signmaWorkTimeA_ms = 0, signmaWorkTimeReboundA_ms = 0, signmaCutTimeA_ms = 0, signmaCutTimeReboundA_ms = 0;
        double signmaWorkTimeB_ms = 0, signmaWorkTimeReboundB_ms = 0, signmaCutTimeB_ms = 0, signmaCutTimeReboundB_ms = 0;
        for (int i = 0; i < switchCount; ++i) {
            signmaWorkTimeA_ms += std::pow(measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTime][i] / 1000.0 - avrgWorkTimeA_ms, 2);
            signmaWorkTimeReboundA_ms += std::pow(measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkReboundTime][i] / 1000.0 - avrgWorkTimeReboundA_ms, 2);
            signmaCutTimeA_ms += std::pow(measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTime][i] / 1000.0 - avrgCutTimeA_ms, 2);
            signmaCutTimeReboundA_ms += std::pow(measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] / 1000.0 - avrgCutTimeReboundA_ms, 2);

            signmaWorkTimeB_ms += std::pow(measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkTime][i] / 1000.0 - avrgWorkTimeB_ms, 2);
            signmaWorkTimeReboundB_ms += std::pow(measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkReboundTime][i] / 1000.0 - avrgWorkTimeReboundB_ms, 2);
            signmaCutTimeB_ms += std::pow(measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTime][i] / 1000.0 - avrgCutTimeB_ms, 2);
            signmaCutTimeReboundB_ms += std::pow(measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] / 1000.0 - avrgCutTimeReboundB_ms, 2);
        }

        signmaWorkTimeA_ms = std::sqrt(signmaWorkTimeA_ms / switchCount);
        signmaWorkTimeReboundA_ms = std::sqrt(signmaWorkTimeReboundA_ms / switchCount);
        signmaCutTimeA_ms = std::sqrt(signmaCutTimeA_ms / switchCount);
        signmaCutTimeReboundA_ms = std::sqrt(signmaCutTimeReboundA_ms / switchCount);

        signmaWorkTimeB_ms = std::sqrt(signmaWorkTimeB_ms / switchCount);
        signmaWorkTimeReboundB_ms = std::sqrt(signmaWorkTimeReboundB_ms / switchCount);
        signmaCutTimeB_ms = std::sqrt(signmaCutTimeB_ms / switchCount);
        signmaCutTimeReboundB_ms = std::sqrt(signmaCutTimeReboundB_ms / switchCount);

        str.append(QString("temps %1/A: %2/A: %3, %4, %5\n")
                       .arg(c + 1)
                       .arg(avrgWorkTimeA_ms, 0, 'f', 2)
                       .arg(avrgWorkTimeReboundA_ms, 0, 'f', 2)
                       .arg(avrgCutTimeA_ms, 0, 'f', 2)
                       .arg(avrgCutTimeReboundA_ms, 0, 'f', 2));
        str.append(QString("deviation %1/A: %2/A: %3, %4, %5\n")
                       .arg(c + 1)
                       .arg(signmaWorkTimeA_ms, 0, 'f', 2)
                       .arg(signmaWorkTimeReboundA_ms, 0, 'f', 2)
                       .arg(signmaCutTimeA_ms, 0, 'f', 2)
                       .arg(signmaCutTimeReboundA_ms, 0, 'f', 2));
        str.append(QString("temps %1/B: %2/B: %3, %4, %5\n")
                       .arg(c + 1)
                       .arg(avrgWorkTimeB_ms, 0, 'f', 2)
                       .arg(avrgWorkTimeReboundB_ms, 0, 'f', 2)
                       .arg(avrgCutTimeB_ms, 0, 'f', 2)
                       .arg(avrgCutTimeReboundB_ms, 0, 'f', 2));
        str.append(QString("deviation %1/B: %2/B: %3, %4, %5\n")
                       .arg(c + 1)
                       .arg(signmaWorkTimeB_ms, 0, 'f', 2)
                       .arg(signmaWorkTimeReboundB_ms, 0, 'f', 2)
                       .arg(signmaCutTimeB_ms, 0, 'f', 2)
                       .arg(signmaCutTimeReboundB_ms, 0, 'f', 2));
        str.append("\n");
    }
    return str;
}

QString StepSwitchingTime::getResultSummary() const {
    switch (resultStatus) {
    case ResultSuccess:
        return QString::fromUtf8(R"(SUCCES
            %1)")
            .arg(getFormattedResults());
    case ResultFailure:
        return QString::fromUtf8(R"(ECHEC
            %1)")
            .arg(getFormattedResults());
    default:
        return GenericStep::getResultSummary();
    }
}

GenericStep::ResultStatus StepSwitchingTime::runMeasureAsync(const std::atomic<bool>& stopToken) {
    // Implementation for running the measurement asynchronously

    STEP_CHECK_STOP_TOKEN();

    auto powerControl = PowerControl::getInstance();
    auto dynamicReadings = DynamicReadings::getInstance();
    auto powerSupply = powerSupply::instance();
    auto contactSelector = ContactSelector::instance();
    auto currentAdjuster = CurrentAdjuster::instance();

    if (!powerControl || !powerSupply || !dynamicReadings || !contactSelector) {
        qCritical() << "One or more required instances are not available. Aborting measurement.";
        return ResultCantMeasure;
    }
    if (!powerControl->checkSafetyStatus()) {
        qCritical() << "Safety status check failed. Aborting measurement.";
        return ResultCantMeasure;
    }

    contactSelector->selectContact(0); // disable contact to start with a known state

    powerSupply->setMaxValues(supplyVoltage_cV / 100.0, maxCurrent_mA / 1000.0);
    powerSupply->setVoltage(supplyVoltage_cV / 100.0);
    powerSupply->enableOutput();
    QThread::msleep(100); // Wait for the power supply to stabilize

    // prepare results storage
    measurementValues.contactASwitchTimes_us.clear();
    measurementValues.contactBSwitchTimes_us.clear();

    measurementValues.contactASwitchTimes_us.reserve(nContacts);
    measurementValues.contactBSwitchTimes_us.reserve(nContacts);
    for (int c = 0; c < nContacts; ++c) {
        measurementValues.contactASwitchTimes_us.emplace_back();
        measurementValues.contactBSwitchTimes_us.emplace_back();

        for (int i = 0; i < switchCount; ++i) {
            measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTime].emplace_back(0);
            measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTimeStable].emplace_back(0);
            measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkReboundTime].emplace_back(0);
            measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTime].emplace_back(0);
            measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTimeStable].emplace_back(0);
            measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseReboundTime].emplace_back(0);

            measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkTime].emplace_back(0);
            measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkTimeStable].emplace_back(0);
            measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkReboundTime].emplace_back(0);
            measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTime].emplace_back(0);
            measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTimeStable].emplace_back(0);
            measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseReboundTime].emplace_back(0);
        }
    }

    std::future<std::shared_ptr<DynamicSwitch>> switchFuture;
    std::shared_ptr<DynamicSwitch> switchResult;

    contactSelector->selectContact(0);                                   // disable contact to start with a known state
    contactSelector->selectHBridge(ContactSelector::HBridge_forward_p1); // Measuring contact A only first
    QThread::msleep(1);                                                  // Wait for the contact to settle

    for (int c = 1; c <= nContacts; ++c) {
        contactSelector->selectContact(c);
        QThread::msleep(1); // Wait for the contact to settle
        // Perform switching time measurement for contact c
        // This would involve enabling the coil, measuring the time it takes for the contact to switch, and recording the results

        for (int i = 0; i < switchCount; ++i) {
            STEP_CHECK_STOP_TOKEN();

            // Enable coil to power on
            if (coilToPowerOn == 1) {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL1, SWITCH_WAIT_TIME_MS);
                powerControl->enableCoil(static_cast<PowerControl::Coil>(coilToPowerOn));
            } else if (coilToPowerOn == 2) {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL2, SWITCH_WAIT_TIME_MS);
                powerControl->enableCoil(static_cast<PowerControl::Coil>(coilToPowerOn));
            } else {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COILS_OFF, SWITCH_WAIT_TIME_MS);
                powerControl->disableCoils();
            }
            // Measure work time, stable time, and rebound time for contact
            // Store results in measurementValues.contactASwitchTimes_ms
            switchResult = switchFuture.get(); // Wait for the switch to complete

            if (switchResult && switchResult->isValid() && switchResult->getContactATransistionType() == INT_EDGE_RISING) { // contact open measures high
                measurementValues.contactASwitchTimes_us[c - 1][switchTimeType::WorkTime][i] = switchResult->getContactAWorkSwitchTime_us();
                measurementValues.contactASwitchTimes_us[c - 1][switchTimeType::WorkTimeStable][i] = switchResult->getContactAStableSwitchTime_us();
                measurementValues.contactASwitchTimes_us[c - 1][switchTimeType::WorkReboundTime][i] = switchResult->getContactAReboundTime_us();
            } else {
                qWarning() << "Switch result is invalid for contact" << c << "on iteration" << i;

                measurementValues.contactASwitchTimes_us[c - 1][switchTimeType::WorkTime][i] = -1;
                measurementValues.contactASwitchTimes_us[c - 1][switchTimeType::WorkTimeStable][i] = -1;
                measurementValues.contactASwitchTimes_us[c - 1][switchTimeType::WorkReboundTime][i] = -1;
            }

            // Enable coil to power off
            if (coilToPowerOff == 1) {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL1, SWITCH_WAIT_TIME_MS);
                powerControl->enableCoil(static_cast<PowerControl::Coil>(coilToPowerOff));
            } else if (coilToPowerOff == 2) {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL2, SWITCH_WAIT_TIME_MS);
                powerControl->enableCoil(static_cast<PowerControl::Coil>(coilToPowerOff));
            } else {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COILS_OFF, SWITCH_WAIT_TIME_MS);
                powerControl->disableCoils();
            }

            // Measure release time, stable time, and rebound time for contact A
            // Store results in measurementValues.contactASwitchTimes_ms
            switchResult = switchFuture.get(); // Wait for the switch to complete

            if (switchResult && switchResult->isValid() && switchResult->getContactATransistionType() == INT_EDGE_FALLING) { // contact closed measures low
                measurementValues.contactASwitchTimes_us[c - 1][switchTimeType::ReleaseTime][i] = switchResult->getContactAWorkSwitchTime_us();
                measurementValues.contactASwitchTimes_us[c - 1][switchTimeType::ReleaseTimeStable][i] = switchResult->getContactAStableSwitchTime_us();
                measurementValues.contactASwitchTimes_us[c - 1][switchTimeType::ReleaseReboundTime][i] = switchResult->getContactAReboundTime_us();
            } else {
                qWarning() << "Switch result is invalid for contact" << c << "on iteration" << i;

                measurementValues.contactASwitchTimes_us[c - 1][switchTimeType::ReleaseTime][i] = -1;
                measurementValues.contactASwitchTimes_us[c - 1][switchTimeType::ReleaseTimeStable][i] = -1;
                measurementValues.contactASwitchTimes_us[c - 1][switchTimeType::ReleaseReboundTime][i] = -1;
            }
        }

        contactSelector->selectContact(0);
        QThread::msleep(1); // Wait for the contact to settle
    }

    // Now measure contact B switching times
    contactSelector->selectHBridge(ContactSelector::HBridge_forward_p2); // Measuring contact A only first
    QThread::msleep(1);                                                  // Wait for the contact to settle

    for (int c = 1; c <= nContacts; ++c) {
        contactSelector->selectContact(c);
        QThread::msleep(1); // Wait for the contact to settle
        // Perform switching time measurement for contact c
        // This would involve enabling the coil, measuring the time it takes for the contact to switch, and recording the results

        for (int i = 0; i < switchCount; ++i) {
            STEP_CHECK_STOP_TOKEN();

            // Enable coil to power on
            if (coilToPowerOn == 1) {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL1, SWITCH_WAIT_TIME_MS);
                powerControl->enableCoil(static_cast<PowerControl::Coil>(coilToPowerOn));
            } else if (coilToPowerOn == 2) {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL2, SWITCH_WAIT_TIME_MS);
                powerControl->enableCoil(static_cast<PowerControl::Coil>(coilToPowerOn));
            } else {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COILS_OFF, SWITCH_WAIT_TIME_MS);
                powerControl->disableCoils();
            }
            // Measure work time, stable time, and rebound time for contact
            // Store results in measurementValues.contactASwitchTimes_ms
            switchResult = switchFuture.get(); // Wait for the switch to complete

            if (switchResult && switchResult->isValid() && switchResult->getContactBTransistionType() == INT_EDGE_FALLING) { // contact open measures high
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::WorkTime][i] = switchResult->getContactAWorkSwitchTime_us();
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::WorkTimeStable][i] = switchResult->getContactAStableSwitchTime_us();
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::WorkReboundTime][i] = switchResult->getContactAReboundTime_us();
            } else {
                qWarning() << "Switch result is invalid for contact" << c << "on iteration" << i;

                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::WorkTime][i] = -1;
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::WorkTimeStable][i] = -1;
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::WorkReboundTime][i] = -1;
            }

            // Enable coil to power off
            if (coilToPowerOff == 1) {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL1, SWITCH_WAIT_TIME_MS);
                powerControl->enableCoil(static_cast<PowerControl::Coil>(coilToPowerOff));
            } else if (coilToPowerOff == 2) {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL2, SWITCH_WAIT_TIME_MS);
                powerControl->enableCoil(static_cast<PowerControl::Coil>(coilToPowerOff));
            } else {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COILS_OFF, SWITCH_WAIT_TIME_MS);
                powerControl->disableCoils();
            }

            // Measure release time, stable time, and rebound time for contact A
            // Store results in measurementValues.contactASwitchTimes_ms
            switchResult = switchFuture.get(); // Wait for the switch to complete

            if (switchResult && switchResult->isValid() && switchResult->getContactBTransistionType() == INT_EDGE_RISING) { // contact closed measures low
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::ReleaseTime][i] = switchResult->getContactAWorkSwitchTime_us();
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::ReleaseTimeStable][i] = switchResult->getContactAStableSwitchTime_us();
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::ReleaseReboundTime][i] = switchResult->getContactAReboundTime_us();
            } else {
                qWarning() << "Switch result is invalid for contact" << c << "on iteration" << i;

                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::ReleaseTime][i] = -1;
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::ReleaseTimeStable][i] = -1;
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::ReleaseReboundTime][i] = -1;
            }
        }
    }

    powerControl->disableCoils();
    contactSelector->selectContact(0);
    powerSupply->disableOutput();
    QThread::msleep(100); // Wait for the contact to settle

    // Handle results and check against success criteria

    bool success = true;
    for (int c = 0; c < nContacts; ++c) {
        for (int i = 0; i < switchCount; ++i) {
            if (measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTime][i] > successValues.maxWorkTime_ms * 1000 ||
                measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTimeStable][i] > successValues.maxWorkTime_ms * 1000 ||
                measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkReboundTime][i] > successValues.maxWorkTimeRebound_ms * 1000 ||
                measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTime][i] > successValues.maxCutTime_ms * 1000 ||
                measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTimeStable][i] > successValues.maxCutTime_ms * 1000 ||
                measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] > successValues.maxCutTimeRebound_ms * 1000) {
                success = false;
                qDebug() << "Contact" << c + 1 << ".A switching time exceeded success criteria on iteration" << i + 1;
                qDebug() << "WorkTime:" << measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTime][i] << "us, Max allowed:" << successValues.maxWorkTime_ms * 1000 << "us";
                qDebug() << "WorkTimeStable:" << measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTimeStable][i] << "us, Max allowed:" << successValues.maxWorkTime_ms * 1000 << "us";
                qDebug() << "WorkTimeRebound:" << measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkReboundTime][i] << "us, Max allowed:" << successValues.maxWorkTimeRebound_ms * 1000 << "us";
                qDebug() << "ReleaseTime:" << measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTime][i] << "us, Max allowed:" << successValues.maxCutTime_ms * 1000 << "us";
                qDebug() << "ReleaseTimeStable:" << measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTimeStable][i] << "us, Max allowed:" << successValues.maxCutTime_ms * 1000 << "us";
                qDebug() << "ReleaseTimeRebound:" << measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] << "us, Max allowed:" << successValues.maxCutTimeRebound_ms * 1000 << "us";
                // break;
            }
        }
        // if (!success) {
        //     break;
        // }
    }

    for (int c = 0; c < nContacts; ++c) {
        for (int i = 0; i < switchCount; ++i) {
            if (measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkTime][i] > successValues.maxWorkTime_ms * 1000 ||
                measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkTimeStable][i] > successValues.maxWorkTime_ms * 1000 ||
                measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkReboundTime][i] > successValues.maxWorkTimeRebound_ms * 1000 ||
                measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTime][i] > successValues.maxCutTime_ms * 1000 ||
                measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTimeStable][i] > successValues.maxCutTime_ms * 1000 ||
                measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] > successValues.maxCutTimeRebound_ms * 1000) {
                success = false;
                qDebug() << "Contact" << c + 1 << "switching time exceeded success criteria on iteration" << i + 1;
                success = false;
                qDebug() << "Contact" << c + 1 << ".B switching time exceeded success criteria on iteration" << i + 1;
                qDebug() << "WorkTimeStable:" << measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkTimeStable][i] << "us, Max allowed:" << successValues.maxWorkTime_ms * 1000 << "us";
                qDebug() << "WorkTimeRebound:" << measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkReboundTime][i] << "us, Max allowed:" << successValues.maxWorkTimeRebound_ms * 1000 << "us";
                qDebug() << "ReleaseTime:" << measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTime][i] << "us, Max allowed:" << successValues.maxCutTime_ms * 1000 << "us";
                qDebug() << "ReleaseTimeStable:" << measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTimeStable][i] << "us, Max allowed:" << successValues.maxCutTime_ms * 1000 << "us";
                qDebug() << "ReleaseTimeRebound:" << measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] << "us, Max allowed:" << successValues.maxCutTimeRebound_ms * 1000 << "us";
                // break;
            }
        }
        // if (!success) {
        //     break;
        // }
    }

    return success ? ResultSuccess : ResultFailure; // Return actual result based on success
}