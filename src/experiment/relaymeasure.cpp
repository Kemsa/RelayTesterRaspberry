#include "relaymeasure.h"
#include "experiment/jsonvalidator.h"

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
#include <QMetaType>

RelayMeasure::RelayMeasure(QJsonObject schema, QObject* parent)
    : QObject(parent), m_schema(std::move(schema)), m_hasSchema(true) {
    qRegisterMetaType<GenericStep::ResultStatus>("ResultStatus");
}

static QString definitionForMeasureType(const QString& measureType) {
    if (measureType == QStringLiteral("coilResistance")) {
        return QStringLiteral("coilResistanceMeasure");
    }
    if (measureType == QStringLiteral("switchingVoltage")) {
        return QStringLiteral("switchingVoltageMeasure");
    }
    if (measureType == QStringLiteral("cutVoltage")) {
        return QStringLiteral("cutVoltageMeasure");
    }
    if (measureType == QStringLiteral("contactResistance")) {
        return QStringLiteral("contactResistanceMeasure");
    }
    if (measureType == QStringLiteral("switchingTime")) {
        return QStringLiteral("switchingTimeMeasure");
    }
    if (measureType == QStringLiteral("selfTestResistance")) {
        return QStringLiteral("selfTestResistanceMeasure");
    }
    return QString();
}

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

    model = stringValueOrDefault(rootObject, QStringLiteral("relayModel"), QString());
    brand = stringValueOrDefault(rootObject, QStringLiteral("relayBrand"), QString());

    for (const QJsonValue& measureValue : measures) {
        if (!measureValue.isObject()) {
            continue;
        }

        const QJsonObject measureObject = measureValue.toObject();
        const QString name = stringValueOrDefault(measureObject, QStringLiteral("name"), QString());

        QJsonObject parameterObject = measureObject.value(QStringLiteral("parameters")).toObject();
        const QString measureType = stringValueOrDefault(parameterObject, QStringLiteral("measureType"), QString());

        if (m_hasSchema) {
            const QString definitionName = definitionForMeasureType(measureType);
            if (!definitionName.isEmpty()) {
                const QJsonObject definitions = m_schema.value(QStringLiteral("definitions")).toObject();
                const QJsonObject measureSchema = definitions.value(definitionName).toObject();
                QString defaultsError;
                experiment::JsonValidator::applyDefaults(&parameterObject, measureSchema, m_schema, &defaultsError);
            }
        }

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

    for (size_t i = 0; i < m_steps.size(); ++i) {
        connect(m_steps[i].get(), &GenericStep::measureUpdated, this, [this, i](int percentComplete) {
            Q_UNUSED(percentComplete);
            emit stepUpdated(static_cast<int>(i));
        });
        connect(m_steps[i].get(), &GenericStep::measureStatusChanged, this, [this, i](GenericStep::ResultStatus status) {
            emit stepStatusChanged(static_cast<int>(i), status);
        });
    }
}

QString RelayMeasure::getModel() const {
    return model;
}

QString RelayMeasure::getBrand() const {
    return brand;
}

QMap<int, QString> RelayMeasure::getSteps() {
    QMap<int, QString> steps;
    for (size_t i = 0; i < m_steps.size(); ++i) {
        steps[i] = m_steps[i]->getName();
    }
    return steps;
}

QString RelayMeasure::getStepDescription(int index) {
    if (index >= 0 && index < static_cast<int>(m_steps.size())) {
        return m_steps[index]->getDescription();
    }
    return QString();
}

GenericStep::ResultStatus RelayMeasure::getStepResultStatus(int index) {
    if (index >= 0 && index < static_cast<int>(m_steps.size())) {
        return m_steps[index]->getResultStatus();
    }
    return GenericStep::ResultUnknown;
}

void RelayMeasure::measureAllAsync() {
    // run the sequence in a background thread so UI thread is not blocked
    m_stopRequested.store(false, std::memory_order_relaxed);
    std::thread([this]() {
        m_currentStep.store(-1, std::memory_order_relaxed);
        for (size_t i = 0; i < m_steps.size(); ++i) {
            if (m_stopRequested.load(std::memory_order_relaxed)) {
                break;
            }
            m_currentStep.store(static_cast<int>(i), std::memory_order_relaxed);
            if (m_steps[i]) {
                m_steps[i]->measureAsync().get(); // Wait for the measure to complete
            }
            // after step finished, clear current and continue
            m_currentStep.store(-1, std::memory_order_relaxed);
        }
        m_currentStep.store(-1, std::memory_order_relaxed);
        emit measureAllFinished();
    }).detach();
}

void RelayMeasure::stopMeasure() {
    m_stopRequested.store(true, std::memory_order_relaxed);
    int current = m_currentStep.load(std::memory_order_relaxed);
    if (current >= 0 && current < static_cast<int>(m_steps.size())) {
        if (m_steps[static_cast<size_t>(current)]) {
            m_steps[static_cast<size_t>(current)]->stopMeasure();
        }
    }
}