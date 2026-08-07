#ifndef RELAYMEASURE_H
#define RELAYMEASURE_H

#include "genericstep.h"
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <atomic>
#include <cassert>
#include <memory>
#include <vector>

class GenericStep;

class RelayMeasure : public QObject {
    Q_OBJECT
public:
    explicit RelayMeasure(QJsonObject schema, QObject* parent = nullptr);

    void fromJSON(const QString& jsonString);
    QString getModel() const;
    QString getBrand() const;

    QMap<int, QString> getSteps();
    QString getStepDescription(int index);

    void measureAllAsync();
    void measureOneAsync(int index);
    void stopMeasure();
    GenericStep::ResultStatus getStepResultStatus(int index);

signals:
    void stepUpdated(int index);
    void stepStatusChanged(int index, GenericStep::ResultStatus status);
    void measureAllFinished();

private:
    QJsonObject m_schema;
    bool m_hasSchema = false;
    std::vector<std::unique_ptr<GenericStep>> m_steps;

    QString model;
    QString brand;
    std::atomic<bool> m_stopRequested{false};
    std::atomic<int> m_currentStep{-1};
};

#endif // RELAYMEASURE_H
