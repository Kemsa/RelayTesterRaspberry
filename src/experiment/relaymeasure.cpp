#include "relaymeasure.h"

#include "steps/stepcoilresistance.h"
#include "steps/stepcontactresistance.h"
#include "steps/stepcutvoltage.h"
#include "steps/stepselftestresistance.h"
#include "steps/stepswitchingtime.h"
#include "steps/stepswitchingvoltage.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

RelayMeasure::RelayMeasure() {}

static int intValueOrDefault(const QJsonObject& object, const QString& key, int defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isDouble() ? value.toInt(defaultValue) : defaultValue;
}

static double doubleValueOrDefault(const QJsonObject& object, const QString& key, double defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isDouble() ? value.toDouble(defaultValue) : defaultValue;
}

static QString stringValueOrDefault(const QJsonObject& object, const QString& key, const QString& defaultValue) {
    const QJsonValue value = object.value(key);
    return value.isString() ? value.toString(defaultValue) : defaultValue;
}

void RelayMeasure::fromJSON(const QString& jsonString) {
    m_steps.clear();

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return;
    }

    const QJsonObject rootObject = document.object();
    const QJsonArray measures = rootObject.value(QStringLiteral("measures")).toArray();

    for (const QJsonValue& measureValue : measures) {
        if (!measureValue.isObject()) {
            continue;
        }

        const QJsonObject measureObject = measureValue.toObject();
        const QString name = stringValueOrDefault(measureObject, QStringLiteral("name"), QString());
        qDebug() << "RelayMeasure: measure name =" << name;

        const QJsonObject parameterObject = measureObject.value(QStringLiteral("parameters")).toObject();
        const QString measureType = stringValueOrDefault(parameterObject, QStringLiteral("measureType"), QString());
        qDebug() << "RelayMeasure: measureType =" << measureType;

        if (measureType == QStringLiteral("coilResistance")) {
            auto step = std::make_unique<StepCoilResistance>(name);
            step->fromJSON(parameterObject);
            m_steps.push_back(std::move(step));
            continue;
        }

        if (measureType == QStringLiteral("switchingVoltage")) {
            auto step = std::make_unique<StepSwitchingVoltage>(name);
            step->fromJSON(parameterObject);
            m_steps.push_back(std::move(step));
            continue;
        }

        if (measureType == QStringLiteral("cutVoltage")) {
            auto step = std::make_unique<StepCutVoltage>(name);
            step->fromJSON(parameterObject);
            m_steps.push_back(std::move(step));
            continue;
        }

        if (measureType == QStringLiteral("contactResistance")) {
            auto step = std::make_unique<StepContactResistance>(name);
            step->fromJSON(parameterObject);
            m_steps.push_back(std::move(step));
            continue;
        }

        if (measureType == QStringLiteral("switchingTime")) {
            auto step = std::make_unique<StepSwitchingTime>(name);
            step->fromJSON(parameterObject);
            m_steps.push_back(std::move(step));
            continue;
        }

        if (measureType == QStringLiteral("selfTestResistance")) {
            auto step = std::make_unique<StepSelfTestResistance>(name);
            step->fromJSON(parameterObject);
            m_steps.push_back(std::move(step));
            continue;
        }
    }
}
