#ifndef GENERICSTEP_H
#define GENERICSTEP_H

#include <QJsonObject>
#include <QObject>

class GenericStep : public QObject {
    Q_OBJECT
public:
    enum ResultStatus {
        ResultUnknown,
        ResultSuccess,
        ResultFailure,
        ResultMeasuring,
        ResultNotStarted
    };

    virtual ~GenericStep() = default;

    virtual void fromJSON(const QJsonObject& object) = 0;

    virtual void startMeasure() = 0;
    virtual void stopMeasure() = 0;
    virtual QString getName() const = 0;
    virtual QString getDescription() const = 0;
    virtual QString getResultSummary() const = 0;
    GenericStep::ResultStatus getResultStatus() const;

signals:
    void measureStarted();
    void measureStopped();
    void measureFinished(ResultStatus success);
    void measureUpdated(int percentComplete);
    void measureStatusChanged(ResultStatus status);

protected:
    ResultStatus resultStatus = ResultNotStarted;
    void setResultStatus(ResultStatus status);
};

#endif // GENERICSTEP_H
