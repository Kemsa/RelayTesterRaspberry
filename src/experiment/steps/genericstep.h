#ifndef GENERICSTEP_H
#define GENERICSTEP_H

#include <QJsonObject>
#include <QObject>
#include <atomic>
#include <future>
#include <thread>

#define STEP_CHECK_STOP_TOKEN()                         \
    if (stopToken.load()) {                             \
        qDebug() << "Stop requested for step:" << name; \
        return ResultStopped;                           \
    }

class GenericStep : public QObject {
    Q_OBJECT
public:
    enum ResultStatus {
        ResultUnknown,
        ResultSuccess,
        ResultFailure,
        ResultMeasuring,
        ResultNotStarted,
        ResultStopPending,
        ResultStopped,
        ResultCantMeasure,
    };
    Q_ENUM(ResultStatus)

    GenericStep(QString name, QObject* parent = nullptr);
    virtual ~GenericStep() = default;

    virtual void fromJSON(const QJsonObject& object) = 0;

    std::future<GenericStep::ResultStatus> measureAsync();
    void stopMeasure();
    virtual QString getName() const;
    virtual QString getDescription() const = 0;
    virtual QString getResultSummary() const;
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
    std::atomic<bool> stopRequested{false};
    void setResultStatus(ResultStatus status);

    virtual ResultStatus runMeasureAsync(const std::atomic<bool>& stopToken) = 0;
};

Q_DECLARE_METATYPE(GenericStep::ResultStatus)

#endif // GENERICSTEP_H
