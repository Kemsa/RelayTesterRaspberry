#ifndef RELAYMEASURE_H
#define RELAYMEASURE_H

#include "genericstep.h"
#include <QJsonObject>
#include <QString>
#include <QObject>
#include <cassert>
#include <memory>
#include <vector>

class GenericStep;

class RelayMeasure : public QObject {
    Q_OBJECT
public:
    explicit RelayMeasure(QObject* parent = nullptr);
    explicit RelayMeasure(QJsonObject schema, QObject* parent = nullptr);

    void fromJSON(const QString& jsonString);
    QString getModel() const;
    QString getBrand() const;

    QMap<int, QString> getSteps();
    QString getStepDescription(int index);

    GenericStep::ResultStatus getStepResultStatus(int index);

signals:
    void stepUpdated(int index);
    void stepStatusChanged(int index, GenericStep::ResultStatus status);

private:
    QJsonObject m_schema;
    bool m_hasSchema = false;
    std::vector<std::unique_ptr<GenericStep>> m_steps;

    QString model;
    QString brand;
};

#endif // RELAYMEASURE_H
