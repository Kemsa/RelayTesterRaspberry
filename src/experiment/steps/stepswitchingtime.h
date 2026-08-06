#ifndef STEPSWITCHINGTIME_H
#define STEPSWITCHINGTIME_H

#include "genericstep.h"
#include <QString>

class QJsonObject;

class StepSwitchingTime : public GenericStep {
public:
    StepSwitchingTime(QString name);
    void fromJSON(const QJsonObject& object) override;
    QString getName() const override;
    QString getDescription() const override;
    QString getResultSummary() const override;

private:
    const QString measureType = QStringLiteral("switchingTime");

    int coilToPowerOn = 0;
    int coilToPowerOff = 0;
    QString contactDirection;
    int nContacts = 1;
    int contactMaxVoltageSwitch_mV = 0;
    int contactMinVoltageCut_mV = 0;
    int switchCount = 1;
    int supplyVoltage_cV = 0;
    int maxCurrent_mA = 200;
    int nMeasurePointsBackward = 64;

    struct SuccessValues {
        int maxSwitchTimeOn_ms = 0;
        int maxSwitchTimeOff_ms = 0;
        int maxCutTimeOn_ms = 0;
        int maxCutTimeOff_ms = 0;
    } successValues;
};

#endif // STEPSWITCHINGTIME_H
