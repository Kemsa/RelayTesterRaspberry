#ifndef STEPCUTVOLTAGE_H
#define STEPCUTVOLTAGE_H

#include "genericstep.h"
#include <QString>

class QJsonObject;

class StepCutVoltage : public GenericStep {
public:
    StepCutVoltage(QString name);
    void fromJSON(const QJsonObject& object) override;
    void startMeasure() override;
    void stopMeasure() override;
    QString getName() const override;
    QString getDescription() const override;
    QString getResultSummary() const override;

private:
    QString name;
    QString measureType = "cutVoltage";
    int coilToMeasure = 1;
    int nContacts = 1;
    int startVoltage_cV = 0;
    int stopVoltage_cV = 0;
    int voltageStep_cV = 0;
    int maxCurrent_mA = 200;
    int nMeasures = 16;

    struct SuccessValues {
        int maxContactOnVoltage_mV = 0;
        int minContactOffVoltage_mV = 0;
        int minCutVoltage_cV = 0;
    } successValues;
};

#endif // STEPCUTVOLTAGE_H
