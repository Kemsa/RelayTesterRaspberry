#include "stepswitchingtime.h"

#include <QJsonObject>

StepSwitchingTime::StepSwitchingTime(QString name) : name(name) {}

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
    contactDirection = stringValueOrDefault(object, QStringLiteral("contactDirection"), contactDirection);
    nContacts = intValueOrDefault(object, QStringLiteral("nContacts"), nContacts);
    contactMaxVoltageSwitch_mV = intValueOrDefault(object, QStringLiteral("contactMaxVoltageSwitch_mV"), contactMaxVoltageSwitch_mV);
    contactMinVoltageCut_mV = intValueOrDefault(object, QStringLiteral("contactMinVoltageCut_mV"), contactMinVoltageCut_mV);
    switchCount = intValueOrDefault(object, QStringLiteral("switchCount"), switchCount);
    supplyVoltage_cV = intValueOrDefault(object, QStringLiteral("supplyVoltage_cV"), supplyVoltage_cV);
    maxCurrent_mA = intValueOrDefault(object, QStringLiteral("maxCurrent_mA"), maxCurrent_mA);
    nMeasurePointsBackward = intValueOrDefault(object, QStringLiteral("nMeasurePointsBackward"), nMeasurePointsBackward);

    const QJsonObject successObject = object.value(QStringLiteral("successValues")).toObject();
    successValues.maxSwitchTimeOn_ms = intValueOrDefault(successObject, QStringLiteral("maxSwitchTimeOn_ms"), successValues.maxSwitchTimeOn_ms);
    successValues.maxSwitchTimeOff_ms = intValueOrDefault(successObject, QStringLiteral("maxSwitchTimeOff_ms"), successValues.maxSwitchTimeOff_ms);
    successValues.maxCutTimeOn_ms = intValueOrDefault(successObject, QStringLiteral("maxCutTimeOn_ms"), successValues.maxCutTimeOn_ms);
    successValues.maxCutTimeOff_ms = intValueOrDefault(successObject, QStringLiteral("maxCutTimeOff_ms"), successValues.maxCutTimeOff_ms);
}

QString StepSwitchingTime::getName() const {
    return name;
}

QString StepSwitchingTime::getDescription() const {
    return QString();
}

QString StepSwitchingTime::getResultSummary() const {
    return QString();
}
