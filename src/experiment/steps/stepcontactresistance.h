#ifndef STEPCONTACTRESISTANCE_H
#define STEPCONTACTRESISTANCE_H

#include "genericstep.h"
#include "staticreadings.h"
#include <QString>

class QJsonObject;

class StepContactResistance : public GenericStep {
public:
    StepContactResistance(QString name);
    void fromJSON(const QJsonObject& object) override;
    QString getName() const override;
    QString getDescription() const override;
    QString getResultSummary() const override;

private:
    const QString measureType = QStringLiteral("contactResistance");

    int coilToPowerOn = 0;
    int coilToPowerOff = 0;
    QString contactDirection;
    int nContacts = 1;
    int nCycles = 3;
    int supplyVoltage_cV = 0;
    int maxCurrent_mA = 200;
    int nMeasures = 16;

    struct SuccessValues {
        int maxResistanceClosed_mOhm = 0;
        int minResistanceOpened_kOhm = 0;
    } successValues;

    struct MeasurementValues {
        std::vector<std::vector<double>> averageResistanceContactA_Ohm; // Map of contact index to average resistance in ohms
        std::vector<std::vector<double>> averageResistanceContactB_Ohm; // Map of contact index to average resistance in ohms
    } measurementValues;

    ResultStatus runMeasureAsync(const std::atomic<bool>& stopToken) override;

    double getResistanceForContact(StaticReadings::ReadingFlags contactFlag, int nMeasures, const std::atomic<bool>& stopToken);
};

#endif // STEPCONTACTRESISTANCE_H
