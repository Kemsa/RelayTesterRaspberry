#ifndef STEPCOILRESISTANCE_H
#define STEPCOILRESISTANCE_H

#include "genericstep.h"
#include <QString>

class QJsonObject;

class StepCoilResistance : public GenericStep {
public:
    StepCoilResistance(QString name);
    void fromJSON(const QJsonObject& object) override;
    QString getName() const override;
    QString getDescription() const override;
    QString getResultSummary() const override;

private:
    const QString measureType = QStringLiteral("coilResistance");

    int coilToMeasure = 1;
    int supplyVoltage_cV = 0;
    int maxCurrent_mA = 200;
    int nMeasures = 16;

    struct SuccessValues {
        int minResistance_ohm = 0;
        int maxResistance_ohm = 0;
    } successValues;

    struct MeasurementValues {
        double averageCurrent_mA;
        double averageVoltage_V;
        double averageResistance_ohm;
    } measurementValues;

    ResultStatus runMeasureAsync(const std::atomic<bool>& stopToken) override;
};

#endif // STEPCOILRESISTANCE_H
