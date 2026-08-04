#ifndef STEPCOILRESISTANCE_H
#define STEPCOILRESISTANCE_H

#include "genericstep.h"
#include <QString>

class QJsonObject;

class StepCoilResistance : public GenericStep {
public:
    StepCoilResistance(QString name);
    void fromJSON(const QJsonObject& object) override;

private:
    QString name;
    QString measureType = QStringLiteral("coilResistance");
    int coilToMeasure = 1;
    int supplyVoltage_cV = 0;
    int maxCurrent_mA = 200;
    int nMeasures = 16;

    struct SuccessValues {
        int minResistance_ohm = 0;
        int maxResistance_ohm = 0;
    } successValues;
};

#endif // STEPCOILRESISTANCE_H
