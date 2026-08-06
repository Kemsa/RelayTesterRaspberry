#include "stepselftestresistance.h"

#include <QJsonObject>

StepSelfTestResistance::StepSelfTestResistance(QString name) : GenericStep(name) {}

static int intValueOrDefault(const QJsonObject& object, const QString& key, int defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isDouble() ? value.toInt(defaultValue) : defaultValue;
}

static double doubleValueOrDefault(const QJsonObject& object, const QString& key, double defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isDouble() ? value.toDouble(defaultValue) : defaultValue;
}

static QString stringValueOrDefault(const QJsonObject& object, const QString& key, const QString& defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isString() ? value.toString() : defaultValue;
}

void StepSelfTestResistance::fromJSON(const QJsonObject& object) {
    contact = intValueOrDefault(object, QStringLiteral("contact"), contact);
    subContact = stringValueOrDefault(object, QStringLiteral("subContact"), subContact);
    nMeasures = intValueOrDefault(object, QStringLiteral("nMeasures"), nMeasures);

    const QJsonObject successObject = object.value(QStringLiteral("successValues")).toObject();
    successValues.targetResistance_ohm = doubleValueOrDefault(successObject, QStringLiteral("targetResistance_ohm"), successValues.targetResistance_ohm);
    successValues.targetdeviation_ohm = doubleValueOrDefault(successObject, QStringLiteral("targetdeviation_ohm"), successValues.targetdeviation_ohm);
}

QString StepSelfTestResistance::getName() const {
    return name;
}

QString StepSelfTestResistance::getDescription() const {
    return QString();
}

QString StepSelfTestResistance::getResultSummary() const {
    return QString();
}

GenericStep::ResultStatus StepSelfTestResistance::runMeasureAsync(const std::atomic<bool>& stopToken) {
    // Implementation for running the measurement asynchronously

    return ResultFailure; // Placeholder return value, replace with actual result
}