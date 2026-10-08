#include "genericstep.h"
#include <QMetaType>
#include <QThread>
#include <memory>

#include "config.h"

#include "powerSupply.h"
#include "powercontrol.h"
#include "staticreadings.h"

GenericStep::GenericStep(QString name, QObject* parent)
    : QObject(parent), name(name) {
    qRegisterMetaType<GenericStep::ResultStatus>("ResultStatus");
}

int GenericStep::intValueOrDefault(const QJsonObject& object, const QString& key, int defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isDouble() ? value.toInt(defaultValue) : defaultValue;
}

double GenericStep::doubleValueOrDefault(const QJsonObject& object, const QString& key, double defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isDouble() ? value.toDouble(defaultValue) : defaultValue;
}

QString GenericStep::stringValueOrDefault(const QJsonObject& object, const QString& key, const QString& defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isString() ? value.toString() : defaultValue;
}

void GenericStep::setResultStatus(ResultStatus status) {
    if (resultStatus != status) {
        resultStatus = status;
        emit measureStatusChanged(resultStatus);
    }
}

GenericStep::ResultStatus GenericStep::getResultStatus() const {
    return resultStatus;
}

QString GenericStep::getName() const {
    return name;
}

std::future<GenericStep::ResultStatus> GenericStep::measureAsync() {

    // Measure is a blocking function, so we need to run the measureAsync in a separate thread to avoid blocking the main thread.

    setResultStatus(GenericStep::ResultNotStarted);
    stopRequested.store(false, std::memory_order_relaxed);

    auto promise = std::make_shared<std::promise<GenericStep::ResultStatus>>();
    std::thread([this, promise]() {
        qDebug() << "Starting measure for step:" << name;
        setResultStatus(GenericStep::ResultMeasuring);

        if (prerun_checks() == false) {
            promise->set_value(ResultPreMeasure);
            setResultStatus(ResultPreMeasure);
            return;
        }

        GenericStep::ResultStatus result = runMeasureAsync(stopRequested);
        promise->set_value(result);
        setResultStatus(result);
    }).detach();

    return promise->get_future();
}

void GenericStep::stopMeasure() {
    setResultStatus(GenericStep::ResultStopPending);
    stopRequested.store(true, std::memory_order_relaxed);
}

QString GenericStep::getResultSummary() const {
    switch (resultStatus) {
    case ResultSuccess:
        return QString::fromUtf8(R"(Résultat: Succès
 )");
    case ResultFailure:
        return QString::fromUtf8(R"(Résultat: Échec
 )");
    case ResultStopped:
        return QString::fromUtf8(R"(Résultat: Mesure arrêtée
 )");
    case ResultStopPending:
        return QString::fromUtf8(R"(Résultat: Arrêt de la mesure en cours...
 )");
    case ResultMeasuring:
        return QString::fromUtf8(R"(Résultat: Mesure en cours... )");
    case ResultNotStarted:
        return QString::fromUtf8(R"(Résultat: Mesure non démarrée )");
    case ResultCantMeasure:
        return QString::fromUtf8(R"(Résultat: Impossible de mesurer, veuillez vérifier les connexions et l'état de sécurité avant de relancer la mesure.)");
    default:
        return QString::fromUtf8(R"(Résultat: Inconnu, veuillez faire une mesure pour obtenir un résultat.
 )");
    }
}

bool GenericStep::prerun_checks() {
    // Implement any necessary checks before running the measure

    auto powerControl = PowerControl::getInstance();
    auto staticReadings = StaticReadings::getInstance();
    auto powerSupply = powerSupply::instance();

    if (!powerControl || !staticReadings || !powerSupply || !staticReadings->checkOpen() || !powerSupply->isConnected()) {
        qCritical() << "One or more required instances are not available. Aborting measurement.";
        return false;
    }
    if (!powerControl->checkSafetyStatus()) {
        qCritical() << "Safety status check failed. Aborting measurement.";
        return false;
    }

    // Check coil resistances: if <10ohm, stop immediately, risk of burning the board!

    powerSupply->setMaxValues(5 , MAX_CURRENT_mA / 1000.0f); // Convert mA to A
    powerSupply->setVoltage(5);                             // Convert cV to V
    powerSupply->enableOutput();

    // coil 1
    powerControl->enableCoilTop(static_cast<PowerControl::Coil>(static_cast<PowerControl::Coil>(1)));    
    powerControl->enableCoilBottom(static_cast<PowerControl::Coil>(static_cast<PowerControl::Coil>(1)));
    QThread::msleep(100); // Wait for the coil to stabilize

    auto voltage1 = std::make_shared<ADCValue>();
    staticReadings->getReading(StaticReadings::ReadingFlags::coil1Voltage, voltage1);

    auto current1 = std::make_shared<ADCValue>();
    staticReadings->getReading(StaticReadings::ReadingFlags::coil1Current, current1);

    auto resistance1 = abs(StaticReadings::toCoilVoltage_V(*voltage1) / StaticReadings::toCoilCurrent_mA(*current1)*1000);

    powerControl->disableCoilsTop();
    powerControl->disableCoilsBottom();

    // coil2
    powerControl->enableCoilTop(static_cast<PowerControl::Coil>(2));
    powerControl->enableCoilBottom(static_cast<PowerControl::Coil>(2));
    QThread::msleep(100); // Wait for the coil to stabilize


    auto voltage2 = std::make_shared<ADCValue>();
    staticReadings->getReading(StaticReadings::ReadingFlags::coil2Voltage, voltage2, ADCBase::Caliber_2500mV);

    auto current2 = std::make_shared<ADCValue>();
    staticReadings->getReading(StaticReadings::ReadingFlags::coil2Current, current2, ADCBase::Caliber_2500mV);

    auto resistance2 = abs(StaticReadings::toCoilVoltage_V(*voltage2) / StaticReadings::toCoilCurrent_mA(*current2)*1000);

    powerControl->disableCoilsBottom();
    powerControl->disableCoilsTop();
    powerControl->disableContactPower();
    powerSupply->disableOutput();

    if (resistance1 < 10 && StaticReadings::toCoilVoltage_V(*voltage1) > 1)
    {
        qCritical() << "Coil1 resistance is too low:" << resistance1 << "Ohm, Voltage:" << StaticReadings::toCoilVoltage_V(*voltage1) << "V. Aborting measurement.";
        return false;
    } else if(resistance2 < 10 && StaticReadings::toCoilVoltage_V(*voltage2) > 1) {
        qCritical() << "Coil2 resistance is too low:" << resistance2 << "Ohm, Voltage:" << StaticReadings::toCoilVoltage_V(*voltage2) << "V. Aborting measurement.";
        return false;
    }else if(StaticReadings::toCoilVoltage_V(*voltage1) < 1) {
        qCritical() << "Coil1 voltage is too low:" << StaticReadings::toCoilVoltage_V(*voltage1) << "V. Aborting measurement.";
        return false;
    }else if(StaticReadings::toCoilVoltage_V(*voltage2) < 1) {
        qCritical() << "Coil2 voltage is too low:" << StaticReadings::toCoilVoltage_V(*voltage2) << "V. Aborting measurement.";
        return false;
    }

    return true;
}