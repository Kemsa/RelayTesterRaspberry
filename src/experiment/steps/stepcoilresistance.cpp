#include "stepcoilresistance.h"

#include <QJsonObject>

StepCoilResistance::StepCoilResistance(QString name) : name(name) {}

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
