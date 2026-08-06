#include "stepcoilresistance.h"

#include <QJsonObject>
#include <QString>
#include <QThread>

#include "powerSupply.h"
#include "powercontrol.h"
#include "staticreadings.h"

#include "qdebug.h"

StepCoilResistance::StepCoilResistance(QString name) : GenericStep(name) {
}

static int intValueOrDefault(const QJsonObject& object, const QString& key, int defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isDouble() ? value.toInt(defaultValue) : defaultValue;
}

void StepCoilResistance::fromJSON(const QJsonObject& object) {
    coilToMeasure = intValueOrDefault(object, QStringLiteral("coilToMeasure"), coilToMeasure);
    supplyVoltage_cV = intValueOrDefault(object, QStringLiteral("supplyVoltage_cV"), supplyVoltage_cV);
    maxCurrent_mA = intValueOrDefault(object, QStringLiteral("maxCurrent_mA"), maxCurrent_mA);
    nMeasures = intValueOrDefault(object, QStringLiteral("nMeasures"), nMeasures);

    const QJsonObject successObject = object.value(QStringLiteral("successValues")).toObject();
    successValues.minResistance_ohm = intValueOrDefault(successObject, QStringLiteral("minResistance_ohm"), successValues.minResistance_ohm);
    successValues.maxResistance_ohm = intValueOrDefault(successObject, QStringLiteral("maxResistance_ohm"), successValues.maxResistance_ohm);
}

QString StepCoilResistance::getName() const {
    return name;
}

QString StepCoilResistance::getDescription() const {
    QString str = QString::fromUtf8(R"(Mesure de la résistance de la bobine #%1 avec:
	tension d'alimentation: %2 cV
	courant maximum: %3 mA
	nombre de mesures: %4
)")
                      .arg(coilToMeasure)
                      .arg(supplyVoltage_cV)
                      .arg(maxCurrent_mA)
                      .arg(nMeasures);
    return str;
}

QString StepCoilResistance::getResultSummary() const {
    return QString();
}

GenericStep::ResultStatus StepCoilResistance::runMeasureAsync(const std::atomic<bool>& stopToken) {
    // Implementation for running the measurement asynchronously

    STEP_CHECK_STOP_TOKEN();

    auto powerControl = PowerControl::getInstance();
    auto staticReadings = StaticReadings::getInstance();
    auto powerSupply = powerSupply::instance();

    if (!powerControl || !staticReadings || !powerSupply) {
        qCritical() << "One or more required instances are not available. Aborting measurement.";
        return ResultFailure;
    }
    if (!powerControl->checkSafetyStatus()) {
        qCritical() << "Safety status check failed. Aborting measurement.";
        return ResultFailure;
    }

    // Set the supply voltage and max current for the coil measurement

    powerSupply->setVoltage(supplyVoltage_cV);
    powerSupply->setCurrent(maxCurrent_mA);
    powerSupply->enableOutput();

    STEP_CHECK_STOP_TOKEN();
    powerControl->enableCoil(static_cast<PowerControl::Coil>(coilToMeasure));
    QThread::msleep(100); // Wait for the coil to stabilize

    QMap<StaticReadings::ReadingFlags, std::vector<ADCValue>> readings;
    if (coilToMeasure == 1) {
        readings = staticReadings->getMultipleReadings(static_cast<uint8_t>(StaticReadings::ReadingFlags::coil1Current) | static_cast<uint8_t>(StaticReadings::ReadingFlags::coil1Voltage), nMeasures);
    } else if (coilToMeasure == 2) {
        readings = staticReadings->getMultipleReadings(static_cast<uint8_t>(StaticReadings::ReadingFlags::coil2Current) | static_cast<uint8_t>(StaticReadings::ReadingFlags::coil2Voltage), nMeasures);
    }

    powerControl->disableCoils();
    powerSupply->disableOutput();

    STEP_CHECK_STOP_TOKEN();

    // Now compute results and check against success criteria
    double currentAverage = 0.0;
    double voltageAverage = 0.0;

    for (int i = 0; i < nMeasures; ++i) {
        if (coilToMeasure == 1) {
            currentAverage += StaticReadings::toCoilCurrent_mA(readings[StaticReadings::ReadingFlags::coil1Current][i]);
            voltageAverage += StaticReadings::toCoilVoltage_V(readings[StaticReadings::ReadingFlags::coil1Voltage][i]);
        } else if (coilToMeasure == 2) {
            currentAverage += StaticReadings::toCoilCurrent_mA(readings[StaticReadings::ReadingFlags::coil2Current][i]);
            voltageAverage += StaticReadings::toCoilVoltage_V(readings[StaticReadings::ReadingFlags::coil2Voltage][i]);
        }
    }
    currentAverage /= nMeasures;
    voltageAverage /= nMeasures;

    double resistance = abs((voltageAverage / (currentAverage * 1e-3))); // Convert mA to A for Ohm's law

    measurementValues.averageCurrent_mA = currentAverage;
    measurementValues.averageVoltage_V = voltageAverage;
    measurementValues.averageResistance_ohm = resistance;

    if (resistance < successValues.minResistance_ohm || resistance > successValues.maxResistance_ohm) {
        return ResultFailure;
    }

    return ResultSuccess;
}