#include "stepswitchingvoltage.h"

#include <QJsonObject>

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
    successValues.maxContactOnVoltage_mV = intValueOrDefault(successObject, QStringLiteral("maxContactOnVoltage_mV"), successValues.maxContactOnVoltage_mV);
    successValues.minContactOffVoltage_mV = intValueOrDefault(successObject, QStringLiteral("minContactOffVoltage_mV"), successValues.minContactOffVoltage_mV);
    successValues.maxSwitchingVoltage_cV = intValueOrDefault(successObject, QStringLiteral("maxSwitchingVoltage_cV"), successValues.maxSwitchingVoltage_cV);
}

QString StepSwitchingVoltage::getName() const {
    return name;
}

QString StepSwitchingVoltage::getDescription() const {
    return QString();
}

QString StepSwitchingVoltage::getResultSummary() const {
    return QString();
}

GenericStep::ResultStatus StepSwitchingVoltage::runMeasureAsync(const std::atomic<bool>& stopToken) {
    // Implementation for running the measurement asynchronously

    return ResultFailure; // Placeholder return value, replace with actual result
}