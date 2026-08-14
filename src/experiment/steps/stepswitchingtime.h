#ifndef STEPSWITCHINGTIME_H
#define STEPSWITCHINGTIME_H

#include "genericstep.h"
#include <QString>

class QJsonObject;

class StepSwitchingTime : public GenericStep {
public:
    enum switchTimeType {
        WorkTime,
        WorkTimeStable,
        WorkReboundTime,
        ReleaseTime,
        ReleaseTimeStable,
        ReleaseReboundTime,
    };

    StepSwitchingTime(QString name);
    void fromJSON(const QJsonObject& object) override;
    QString getName() const override;
    QString getDescription() const override;
    QString getResultSummary() const override;

private:
    const QString measureType = QStringLiteral("switchingTime");

    int coilToPowerOn = 0;
    int coilToPowerOff = 0;
    int nContacts = 1;
    int switchCount = 1;
    int supplyVoltage_cV = 0;
    int maxCurrent_mA = 200;

    struct SuccessValues {
        int maxWorkTime_ms = 0;
        int maxWorkTimeRebound_ms = 0;
        int maxCutTime_ms = 0;
        int maxCutTimeRebound_ms = 0;
    } successValues;

    struct MeasurementValues {
        std::vector<QMap<switchTimeType, std::vector<int>>> contactASwitchTimes_us;
        std::vector<QMap<switchTimeType, std::vector<int>>> contactBSwitchTimes_us;
    } measurementValues;

    ResultStatus runMeasureAsync(const std::atomic<bool>& stopToken) override;

    QString getFormattedResults() const;
};

#endif // STEPSWITCHINGTIME_H
