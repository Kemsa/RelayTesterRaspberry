#ifndef GENERICSTEP_H
#define GENERICSTEP_H

#include <QJsonObject>
#include <QObject>
#include <future>
#include <thread>

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

    void startMeasure();
    void stopMeasure();
    virtual QString getName() const;
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
    QString name;
    const QString measureType = QStringLiteral("generic measure");

    ResultStatus resultStatus = ResultNotStarted;
    void setResultStatus(ResultStatus status);

    virtual std::future<ResultStatus> runMeasureAsync();
};

#endif // GENERICSTEP_H
