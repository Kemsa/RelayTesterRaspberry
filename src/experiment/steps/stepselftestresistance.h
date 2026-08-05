#ifndef STEPSELFTESTRESISTANCE_H
#define STEPSELFTESTRESISTANCE_H

#include "genericstep.h"
#include <QString>

class QJsonObject;

class StepSelfTestResistance : public GenericStep {
public:
    StepSelfTestResistance(QString name);
    void fromJSON(const QJsonObject& object) override;
    void startMeasure() override;
    void stopMeasure() override;
    QString getName() const override;
    QString getDescription() const override;
    QString getResultSummary() const override;

private:
    QString name;
    QString measureType = QStringLiteral("selfTestResistance");
    int contact = 1;
    QString subContact;
    int nMeasures = 16;

    struct SuccessValues {
        double targetResistance_ohm = 0.0;
        double targetdeviation_ohm = 0.0;
    } successValues;
};

#endif // STEPSELFTESTRESISTANCE_H
