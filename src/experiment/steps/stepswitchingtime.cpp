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

void StepSwitchingTime::fromJSON(const QJsonObject& object) {
    coilToPowerOn = intValueOrDefault(object, QStringLiteral("coilToPowerOn"), coilToPowerOn);
    coilToPowerOff = intValueOrDefault(object, QStringLiteral("coilToPowerOff"), coilToPowerOff);
    nContacts = intValueOrDefault(object, QStringLiteral("nContacts"), nContacts);
    switchCount = intValueOrDefault(object, QStringLiteral("switchCount"), switchCount);
    supplyVoltage_cV = intValueOrDefault(object, QStringLiteral("supplyVoltage_cV"), supplyVoltage_cV);
    maxCurrent_mA = intValueOrDefault(object, QStringLiteral("maxCurrent_mA"), maxCurrent_mA);

    const QJsonObject successObject = object.value(QStringLiteral("successValues")).toObject();
    successValues.maxWorkTime_no_ms = doubleValueOrDefault(successObject, QStringLiteral("maxWorkTime_no_ms"), successValues.maxWorkTime_no_ms);
    successValues.maxWorkTimeRebound_no_ms = doubleValueOrDefault(successObject, QStringLiteral("maxWorkTimeRebound_no_ms"), successValues.maxWorkTimeRebound_no_ms);
    successValues.maxCutTime_no_ms = doubleValueOrDefault(successObject, QStringLiteral("maxCutTime_no_ms"), successValues.maxCutTime_no_ms);
    successValues.maxCutTimeRebound_no_ms = doubleValueOrDefault(successObject, QStringLiteral("maxCutTimeRebound_no_ms"), successValues.maxCutTimeRebound_no_ms);
    successValues.maxWorkTime_nc_ms = doubleValueOrDefault(successObject, QStringLiteral("maxWorkTime_nc_ms"), successValues.maxWorkTime_nc_ms);
    successValues.maxWorkTimeRebound_nc_ms = doubleValueOrDefault(successObject, QStringLiteral("maxWorkTimeRebound_nc_ms"), successValues.maxWorkTimeRebound_nc_ms);
    successValues.maxCutTime_nc_ms = doubleValueOrDefault(successObject, QStringLiteral("maxCutTime_nc_ms"), successValues.maxCutTime_nc_ms);
    successValues.maxCutTimeRebound_nc_ms = doubleValueOrDefault(successObject, QStringLiteral("maxCutTimeRebound_nc_ms"), successValues.maxCutTimeRebound_nc_ms);
    successValues.minWorkTime_no_ms = doubleValueOrDefault(successObject, QStringLiteral("minWorkTime_no_ms"), successValues.minWorkTime_no_ms);
    successValues.minWorkTimeRebound_no_ms = doubleValueOrDefault(successObject, QStringLiteral("minWorkTimeRebound_no_ms"), successValues.minWorkTimeRebound_no_ms);
    successValues.minCutTime_no_ms = doubleValueOrDefault(successObject, QStringLiteral("minCutTime_no_ms"), successValues.minCutTime_no_ms);
    successValues.minCutTimeRebound_no_ms = doubleValueOrDefault(successObject, QStringLiteral("minCutTimeRebound_no_ms"), successValues.minCutTimeRebound_no_ms);
    successValues.minWorkTime_nc_ms = doubleValueOrDefault(successObject, QStringLiteral("minWorkTime_nc_ms"), successValues.minWorkTime_nc_ms);
    successValues.minWorkTimeRebound_nc_ms = doubleValueOrDefault(successObject, QStringLiteral("minWorkTimeRebound_nc_ms"), successValues.minWorkTimeRebound_nc_ms);
    successValues.minCutTime_nc_ms = doubleValueOrDefault(successObject, QStringLiteral("minCutTime_nc_ms"), successValues.minCutTime_nc_ms);
    successValues.minCutTimeRebound_nc_ms = doubleValueOrDefault(successObject, QStringLiteral("minCutTimeRebound_nc_ms"), successValues.minCutTimeRebound_nc_ms);
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
        Courant max (mA): %6)")
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
    str.append(QString("Temps de travail min/max normalement ouvert (ms): %1/%2\n")
                   .arg(successValues.minWorkTime_no_ms)
                   .arg(successValues.maxWorkTime_no_ms));
    if (successValues.maxWorkTimeRebound_no_ms >= 0 || successValues.minWorkTimeRebound_no_ms >= 0) {
        str.append(QString("Temps de travail rebond min/max normalement ouvert (ms): %1/%2\n")
                       .arg(successValues.minWorkTimeRebound_no_ms)
                       .arg(successValues.maxWorkTimeRebound_no_ms));
    }
    if (successValues.minCutTime_no_ms >= 0 || successValues.maxCutTime_no_ms >= 0) {
        str.append(QString("Temps de coupure min/max normalement ouvert (ms): %1/%2\n")
                       .arg(successValues.minCutTime_no_ms)
                       .arg(successValues.maxCutTime_no_ms));
    }
    if (successValues.maxCutTimeRebound_no_ms >= 0 || successValues.minCutTimeRebound_no_ms >= 0) {
        str.append(QString("Temps de coupure rebond min/max normalement ouvert (ms): %1/%2\n")
                       .arg(successValues.minCutTimeRebound_no_ms)
                       .arg(successValues.maxCutTimeRebound_no_ms));
    }
    str.append(QString("Temps de travail min/max normalement fermé (ms): %1/%2\n")
                   .arg(successValues.minWorkTime_nc_ms)
                   .arg(successValues.maxWorkTime_nc_ms));
    if (successValues.maxWorkTimeRebound_nc_ms >= 0 || successValues.minWorkTimeRebound_nc_ms >= 0) {
        str.append(QString("Temps de travail rebond min/max normalement fermé (ms): %1/%2\n")
                       .arg(successValues.minWorkTimeRebound_nc_ms)
                       .arg(successValues.maxWorkTimeRebound_nc_ms));
    }
    str.append(QString("Temps de coupure min/max normalement fermé (ms): %1/%2\n")
                   .arg(successValues.minCutTime_nc_ms)
                   .arg(successValues.maxCutTime_nc_ms));
    if (successValues.maxCutTimeRebound_nc_ms >= 0 || successValues.minCutTimeRebound_nc_ms >= 0) {
        str.append(QString("Temps de coupure rebond min/max normalement fermé (ms): %1/%2\n")
                       .arg(successValues.minCutTimeRebound_nc_ms)
                       .arg(successValues.maxCutTimeRebound_nc_ms));
    }

    str.append("\nrésultats formattés:\n");
    str.append("pour chaque ligne: temps de travail (ms)/temps de rebond travail(ms)/temps de coupure (ms)/temps de rebond de coupure (ms)\n");
    str.append("pour chaque contact, lignes dans l'ordre: moyenne, minimum, maximum, sigma\n");
    str.append("\n");

    for (int c = 0; c < nContacts; ++c) {

        double avrgWorkTimeA_ms = 0, avrgWorkTimeReboundA_ms = 0, avrgCutTimeA_ms = 0, avrgCutTimeReboundA_ms = 0;
        double avrgWorkTimeB_ms = 0, avrgWorkTimeReboundB_ms = 0, avrgCutTimeB_ms = 0, avrgCutTimeReboundB_ms = 0;
        double minWorkTimeA_ms = std::numeric_limits<double>::max(), minWorkTimeReboundA_ms = std::numeric_limits<double>::max(), minCutTimeA_ms = std::numeric_limits<double>::max(), minCutTimeReboundA_ms = std::numeric_limits<double>::max();
        double minWorkTimeB_ms = std::numeric_limits<double>::max(), minWorkTimeReboundB_ms = std::numeric_limits<double>::max(), minCutTimeB_ms = std::numeric_limits<double>::max(), minCutTimeReboundB_ms = std::numeric_limits<double>::max();
        double maxWorkTimeA_ms = 0, maxWorkTimeReboundA_ms = 0, maxCutTimeA_ms = 0, maxCutTimeReboundA_ms = 0;
        double maxWorkTimeB_ms = 0, maxWorkTimeReboundB_ms = 0, maxCutTimeB_ms = 0, maxCutTimeReboundB_ms = 0;
        for (int i = 0; i < switchCount; ++i) {
            avrgWorkTimeA_ms += measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTime][i] / 1000.0;
            avrgWorkTimeReboundA_ms += measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkReboundTime][i] / 1000.0;
            avrgCutTimeA_ms += measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTime][i] / 1000.0;
            avrgCutTimeReboundA_ms += measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] / 1000.0;

            minWorkTimeA_ms = std::min(minWorkTimeA_ms, measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTime][i] / 1000.0);
            minWorkTimeReboundA_ms = std::min(minWorkTimeReboundA_ms, measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkReboundTime][i] / 1000.0);
            minCutTimeA_ms = std::min(minCutTimeA_ms, measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTime][i] / 1000.0);
            minCutTimeReboundA_ms = std::min(minCutTimeReboundA_ms, measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] / 1000.0);

            maxWorkTimeA_ms = std::max(maxWorkTimeA_ms, measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTime][i] / 1000.0);
            maxWorkTimeReboundA_ms = std::max(maxWorkTimeReboundA_ms, measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkReboundTime][i] / 1000.0);
            maxCutTimeA_ms = std::max(maxCutTimeA_ms, measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTime][i] / 1000.0);
            maxCutTimeReboundA_ms = std::max(maxCutTimeReboundA_ms, measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] / 1000.0);

            avrgWorkTimeB_ms += measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkTime][i] / 1000.0;
            avrgWorkTimeReboundB_ms += measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkReboundTime][i] / 1000.0;
            avrgCutTimeB_ms += measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTime][i] / 1000.0;
            avrgCutTimeReboundB_ms += measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] / 1000.0;

            minWorkTimeB_ms = std::min(minWorkTimeB_ms, measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkTime][i] / 1000.0);
            minWorkTimeReboundB_ms = std::min(minWorkTimeReboundB_ms, measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkReboundTime][i] / 1000.0);
            minCutTimeB_ms = std::min(minCutTimeB_ms, measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTime][i] / 1000.0);
            minCutTimeReboundB_ms = std::min(minCutTimeReboundB_ms, measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] / 1000.0);

            maxWorkTimeB_ms = std::max(maxWorkTimeB_ms, measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkTime][i] / 1000.0);
            maxWorkTimeReboundB_ms = std::max(maxWorkTimeReboundB_ms, measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkReboundTime][i] / 1000.0);
            maxCutTimeB_ms = std::max(maxCutTimeB_ms, measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTime][i] / 1000.0);
            maxCutTimeReboundB_ms = std::max(maxCutTimeReboundB_ms, measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] / 1000.0);
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

        str += QString("Contact %1, A normalement fermé, B normalement ouvert:\n").arg(c + 1);
        str.append("-------\n");
        str.append(QString("moyen %1/A: %2: %3, %4, %5\n")
                       .arg(c + 1)
                       .arg(avrgWorkTimeA_ms, 0, 'f', 2)
                       .arg(avrgWorkTimeReboundA_ms, 0, 'f', 2)
                       .arg(avrgCutTimeA_ms, 0, 'f', 2)
                       .arg(avrgCutTimeReboundA_ms, 0, 'f', 2));
        str.append(QString("min %1/A: %2: %3, %4, %5\n")
                       .arg(c + 1)
                       .arg(minWorkTimeA_ms, 0, 'f', 2)
                       .arg(minWorkTimeReboundA_ms, 0, 'f', 2)
                       .arg(minCutTimeA_ms, 0, 'f', 2)
                       .arg(minCutTimeReboundA_ms, 0, 'f', 2));
        str.append(QString("max %1/A: %2: %3, %4, %5\n")
                       .arg(c + 1)
                       .arg(maxWorkTimeA_ms, 0, 'f', 2)
                       .arg(maxWorkTimeReboundA_ms, 0, 'f', 2)
                       .arg(maxCutTimeA_ms, 0, 'f', 2)
                       .arg(maxCutTimeReboundA_ms, 0, 'f', 2));
        str.append(QString("deviation %1/A: %2: %3, %4, %5\n")
                       .arg(c + 1)
                       .arg(signmaWorkTimeA_ms, 0, 'f', 2)
                       .arg(signmaWorkTimeReboundA_ms, 0, 'f', 2)
                       .arg(signmaCutTimeA_ms, 0, 'f', 2)
                       .arg(signmaCutTimeReboundA_ms, 0, 'f', 2));
        str.append("-------\n");
        str.append(QString("temps %1/B: %2: %3, %4, %5\n")
                       .arg(c + 1)
                       .arg(avrgWorkTimeB_ms, 0, 'f', 2)
                       .arg(avrgWorkTimeReboundB_ms, 0, 'f', 2)
                       .arg(avrgCutTimeB_ms, 0, 'f', 2)
                       .arg(avrgCutTimeReboundB_ms, 0, 'f', 2));
        str.append(QString("min %1/B: %2: %3, %4, %5\n")
                       .arg(c + 1)
                       .arg(minWorkTimeB_ms, 0, 'f', 2)
                       .arg(minWorkTimeReboundB_ms, 0, 'f', 2)
                       .arg(minCutTimeB_ms, 0, 'f', 2)
                       .arg(minCutTimeReboundB_ms, 0, 'f', 2));
        str.append(QString("max %1/B: %2: %3, %4, %5\n")
                       .arg(c + 1)
                       .arg(maxWorkTimeB_ms, 0, 'f', 2)
                       .arg(maxWorkTimeReboundB_ms, 0, 'f', 2)
                       .arg(maxCutTimeB_ms, 0, 'f', 2)
                       .arg(maxCutTimeReboundB_ms, 0, 'f', 2));
        str.append(QString("deviation %1/B: %2: %3, %4, %5\n")
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

    if (!powerControl || !powerSupply || !dynamicReadings || !contactSelector || !currentAdjuster || !dynamicReadings->isReady() || !powerSupply->isConnected()) {
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
    powerControl->enableContactPower();
    powerControl->enableCoilTop(static_cast<PowerControl::Coil>(coilToPowerOn));
    powerControl->enableCoilTop(static_cast<PowerControl::Coil>(coilToPowerOff));
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
                powerControl->enableCoilBottom(static_cast<PowerControl::Coil>(coilToPowerOn));
            } else if (coilToPowerOn == 2) {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL2, SWITCH_WAIT_TIME_MS);
                powerControl->enableCoilBottom(static_cast<PowerControl::Coil>(coilToPowerOn));
            } else {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COILS_OFF, SWITCH_WAIT_TIME_MS);
                powerControl->disableCoilsBottom();
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
                powerControl->enableCoilBottom(static_cast<PowerControl::Coil>(coilToPowerOff));
            } else if (coilToPowerOff == 2) {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL2, SWITCH_WAIT_TIME_MS);
                powerControl->enableCoilBottom(static_cast<PowerControl::Coil>(coilToPowerOff));
            } else {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COILS_OFF, SWITCH_WAIT_TIME_MS);
                powerControl->disableCoilsBottom();
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
                powerControl->enableCoilBottom(static_cast<PowerControl::Coil>(coilToPowerOn));
            } else if (coilToPowerOn == 2) {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL2, SWITCH_WAIT_TIME_MS);
                powerControl->enableCoilBottom(static_cast<PowerControl::Coil>(coilToPowerOn));
            } else {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COILS_OFF, SWITCH_WAIT_TIME_MS);
                powerControl->disableCoilsBottom();
            }
            // Measure work time, stable time, and rebound time for contact
            // Store results in measurementValues.contactASwitchTimes_ms
            switchResult = switchFuture.get(); // Wait for the switch to complete

            if (switchResult && switchResult->isValid() && switchResult->getContactBTransistionType() == INT_EDGE_FALLING) { // contact open measures high
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::WorkTime][i] = switchResult->getContactBWorkSwitchTime_us();
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::WorkTimeStable][i] = switchResult->getContactBStableSwitchTime_us();
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::WorkReboundTime][i] = switchResult->getContactBReboundTime_us();
            } else {
                qWarning() << "Switch result is invalid for contact" << c << "on iteration" << i;

                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::WorkTime][i] = -1;
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::WorkTimeStable][i] = -1;
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::WorkReboundTime][i] = -1;
            }

            // Enable coil to power off
            if (coilToPowerOff == 1) {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL1, SWITCH_WAIT_TIME_MS);
                powerControl->enableCoilBottom(static_cast<PowerControl::Coil>(coilToPowerOff));
            } else if (coilToPowerOff == 2) {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COIL2, SWITCH_WAIT_TIME_MS);
                powerControl->enableCoilBottom(static_cast<PowerControl::Coil>(coilToPowerOff));
            } else {
                switchFuture = dynamicReadings->waitAndProcessOneSwitch(DynamicReadings::ContactType::COILS_OFF, SWITCH_WAIT_TIME_MS);
                powerControl->disableCoilsBottom();
            }

            // Measure release time, stable time, and rebound time for contact A
            // Store results in measurementValues.contactASwitchTimes_ms
            switchResult = switchFuture.get(); // Wait for the switch to complete

            if (switchResult && switchResult->isValid() && switchResult->getContactBTransistionType() == INT_EDGE_RISING) { // contact closed measures low
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::ReleaseTime][i] = switchResult->getContactBWorkSwitchTime_us();
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::ReleaseTimeStable][i] = switchResult->getContactBStableSwitchTime_us();
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::ReleaseReboundTime][i] = switchResult->getContactBReboundTime_us();
            } else {
                qWarning() << "Switch result is invalid for contact" << c << "on iteration" << i;

                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::ReleaseTime][i] = -1;
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::ReleaseTimeStable][i] = -1;
                measurementValues.contactBSwitchTimes_us[c - 1][switchTimeType::ReleaseReboundTime][i] = -1;
            }
        }
    }

    powerControl->disableContactPower();
    powerControl->disableCoilsBottom();
    contactSelector->selectContact(0);
    powerControl->disableCoilsTop();
    powerSupply->disableOutput();
    QThread::msleep(100); // Wait for the contact to settle

    // Handle results and check against success criteria

    bool success = true;
    for (int c = 0; c < nContacts; ++c) {
        for (int i = 0; i < switchCount; ++i) {
            if (measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTime][i] > successValues.maxWorkTime_no_ms * 1000 ||
                measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTimeStable][i] > successValues.maxWorkTime_no_ms * 1000 ||
                measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTime][i] > successValues.maxCutTime_no_ms * 1000 ||
                measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTimeStable][i] > successValues.maxCutTime_no_ms * 1000 ||
                measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTime][i] < successValues.minWorkTime_no_ms * 1000 ||
                measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTimeStable][i] < successValues.minWorkTime_no_ms * 1000 ||
                measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTime][i] < successValues.minCutTime_no_ms * 1000 ||
                measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTimeStable][i] < successValues.minCutTime_no_ms * 1000 ||
                (successValues.maxWorkTimeRebound_no_ms >= 0 && measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkReboundTime][i] > successValues.maxWorkTimeRebound_no_ms * 1000) ||
                (successValues.maxCutTimeRebound_no_ms >= 0 && measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] > successValues.maxCutTimeRebound_no_ms * 1000)) {
                success = false;
                qDebug() << "Contact" << c + 1 << ".A switching time exceeded success criteria on iteration" << i + 1;
                qDebug() << "WorkTime:" << measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTime][i] << "us, Min/Max allowed:" << successValues.minWorkTime_no_ms * 1000 << "/" << successValues.maxWorkTime_no_ms * 1000 << "us";
                qDebug() << "WorkTimeStable:" << measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkTimeStable][i] << "us, Min/Max allowed:" << successValues.maxWorkTime_no_ms * 1000 << "us";
                qDebug() << "WorkTimeRebound:" << measurementValues.contactASwitchTimes_us[c][switchTimeType::WorkReboundTime][i] << "us, Min/Max allowed:" << successValues.minWorkTimeRebound_no_ms * 1000 << "/" << successValues.maxWorkTimeRebound_no_ms * 1000 << "us";
                qDebug() << "ReleaseTime:" << measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTime][i] << "us, Min/Max allowed:" << successValues.minCutTime_no_ms * 1000 << "/" << successValues.maxCutTime_no_ms * 1000 << "us";
                qDebug() << "ReleaseTimeStable:" << measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseTimeStable][i] << "us, Min/Max allowed:" << successValues.minCutTime_no_ms * 1000 << "/" << successValues.maxCutTime_no_ms * 1000 << "us";
                qDebug() << "ReleaseTimeRebound:" << measurementValues.contactASwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] << "us, Min/Max allowed:" << successValues.minCutTimeRebound_no_ms * 1000 << "/" << successValues.maxCutTimeRebound_no_ms * 1000 << "us";
                // break;
            }
        }
        // if (!success) {
        //     break;
        // }
    }

    for (int c = 0; c < nContacts; ++c) {
        for (int i = 0; i < switchCount; ++i) {
            if (measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkTime][i] > successValues.maxWorkTime_nc_ms * 1000 ||
                measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkTimeStable][i] > successValues.maxWorkTime_nc_ms * 1000 ||
                measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTime][i] > successValues.maxCutTime_nc_ms * 1000 ||
                measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTimeStable][i] > successValues.maxCutTime_nc_ms * 1000 ||
                measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkTime][i] < successValues.minWorkTime_nc_ms * 1000 ||
                measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkTimeStable][i] < successValues.minWorkTime_nc_ms * 1000 ||
                measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTime][i] < successValues.minCutTime_nc_ms * 1000 ||
                measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTimeStable][i] < successValues.minCutTime_nc_ms * 1000 ||
                (successValues.maxWorkTimeRebound_nc_ms >= 0 && measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkReboundTime][i] > successValues.maxWorkTimeRebound_nc_ms * 1000) ||
                (successValues.maxCutTimeRebound_nc_ms >= 0 && measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] > successValues.maxCutTimeRebound_nc_ms * 1000)) {
                success = false;
                qDebug() << "Contact" << c + 1 << "switching time exceeded success criteria on iteration" << i + 1;
                success = false;
                qDebug() << "Contact" << c + 1 << ".B switching time exceeded success criteria on iteration" << i + 1;
                qDebug() << "WorkTimeStable:" << measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkTimeStable][i] << "us, Min/Max allowed:" << successValues.minWorkTime_nc_ms * 1000 << "/" << successValues.maxWorkTime_nc_ms * 1000 << "us";
                qDebug() << "WorkTimeRebound:" << measurementValues.contactBSwitchTimes_us[c][switchTimeType::WorkReboundTime][i] << "us, Min/Max allowed:" << successValues.minWorkTimeRebound_nc_ms * 1000 << "/" << successValues.maxWorkTimeRebound_nc_ms * 1000 << "us";
                qDebug() << "ReleaseTime:" << measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTime][i] << "us, Min/Max allowed:" << successValues.minCutTime_nc_ms * 1000 << "/" << successValues.maxCutTime_nc_ms * 1000 << "us";
                qDebug() << "ReleaseTimeStable:" << measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseTimeStable][i] << "us, Min/Max allowed:" << successValues.minCutTime_nc_ms * 1000 << "/" << successValues.maxCutTime_nc_ms * 1000 << "us";
                qDebug() << "ReleaseTimeRebound:" << measurementValues.contactBSwitchTimes_us[c][switchTimeType::ReleaseReboundTime][i] << "us, Min/Max allowed:" << successValues.minCutTimeRebound_nc_ms * 1000 << "/" << successValues.maxCutTimeRebound_nc_ms * 1000 << "us";
                // break;
            }
        }
        // if (!success) {
        //     break;
        // }
    }

    return success ? ResultSuccess : ResultFailure; // Return actual result based on success
}