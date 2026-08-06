#include "stepcutvoltage.h"

#include <QJsonObject>

StepCutVoltage::StepCutVoltage(QString name) : name(name) {}

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
    successValues.maxContactOnVoltage_mV = intValueOrDefault(successObject, QStringLiteral("maxContactOnVoltage_mV"), successValues.maxContactOnVoltage_mV);
    successValues.minContactOffVoltage_mV = intValueOrDefault(successObject, QStringLiteral("minContactOffVoltage_mV"), successValues.minContactOffVoltage_mV);
    successValues.minCutVoltage_cV = intValueOrDefault(successObject, QStringLiteral("minCutVoltage_cV"), successValues.minCutVoltage_cV);
}

QString StepCutVoltage::getName() const {
    return name;
}

QString StepCutVoltage::getDescription() const {
    return QString();
}

QString StepCutVoltage::getResultSummary() const {
    return QString();
}
