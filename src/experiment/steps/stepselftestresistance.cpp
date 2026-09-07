#include "stepselftestresistance.h"

#include <QJsonObject>

StepSelfTestResistance::StepSelfTestResistance(QString name) : GenericStep(name) {}

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