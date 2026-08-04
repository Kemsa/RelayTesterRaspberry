#ifndef STEPSWITCHINGVOLTAGE_H
#define STEPSWITCHINGVOLTAGE_H

#include "genericstep.h"
#include <QString>

class QJsonObject;

class StepSwitchingVoltage : public GenericStep {
public:
    StepSwitchingVoltage(QString name);
    void fromJSON(const QJsonObject& object) override;

private:
    QString name;
    QString measureType = "switchingVoltage";
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
        int maxSwitchingVoltage_cV = 0;
    } successValues;
};

#endif // STEPSWITCHINGVOLTAGE_H
