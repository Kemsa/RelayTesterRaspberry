#include "stepcontactresistance.h"

#include <QJsonObject>

StepContactResistance::StepContactResistance(QString name) : name(name) {}

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

    const QJsonObject successObject = object.value(QStringLiteral("successValues")).toObject();
    successValues.maxResistanceClosed_mOhm = intValueOrDefault(successObject, QStringLiteral("maxResistanceClosed_mOhm"), successValues.maxResistanceClosed_mOhm);
    successValues.minResistanceOpened_kOhm = intValueOrDefault(successObject, QStringLiteral("minResistanceOpened_kOhm"), successValues.minResistanceOpened_kOhm);
}

QString StepContactResistance::getName() const {
    return name;
}

QString StepContactResistance::getDescription() const {
    return QString();
}

QString StepContactResistance::getResultSummary() const {
    return QString();
}
