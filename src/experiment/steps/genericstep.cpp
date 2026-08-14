#include "genericstep.h"
#include <QMetaType>

GenericStep::GenericStep(QString name, QObject* parent)
    : QObject(parent), name(name) {
    qRegisterMetaType<GenericStep::ResultStatus>("ResultStatus");
}

void GenericStep::setResultStatus(ResultStatus status) {
    if (resultStatus != status) {
        resultStatus = status;
        emit measureStatusChanged(resultStatus);
    }
}

GenericStep::ResultStatus GenericStep::getResultStatus() const {
    return resultStatus;
}

QString GenericStep::getName() const {
    return name;
}

std::future<GenericStep::ResultStatus> GenericStep::measureAsync() {

    // Measure is a blocking function, so we need to run the measureAsync in a separate thread to avoid blocking the main thread.

    setResultStatus(GenericStep::ResultNotStarted);
    stopRequested.store(false, std::memory_order_relaxed);

    auto promise = std::make_shared<std::promise<GenericStep::ResultStatus>>();
    std::thread([this, promise]() {
        qDebug() << "Starting measure for step:" << name;
        setResultStatus(GenericStep::ResultMeasuring);
        GenericStep::ResultStatus result = runMeasureAsync(stopRequested);
        promise->set_value(result);
        setResultStatus(result);
    }).detach();

    return promise->get_future();
}

void GenericStep::stopMeasure() {
    setResultStatus(GenericStep::ResultStopPending);
    stopRequested.store(true, std::memory_order_relaxed);
}

QString GenericStep::getResultSummary() const {
    switch (resultStatus) {
    case ResultSuccess:
        return QString::fromUtf8(R"(Résultat: Succès
 )");
    case ResultFailure:
        return QString::fromUtf8(R"(Résultat: Échec
 )");
    case ResultStopped:
        return QString::fromUtf8(R"(Résultat: Mesure arrêtée
 )");
    case ResultStopPending:
        return QString::fromUtf8(R"(Résultat: Arrêt de la mesure en cours...
 )");
    case ResultMeasuring:
        return QString::fromUtf8(R"(Résultat: Mesure en cours... )");
    case ResultNotStarted:
        return QString::fromUtf8(R"(Résultat: Mesure non démarrée )");
    case ResultCantMeasure:
        return QString::fromUtf8(R"(Résultat: Impossible de mesurer, veuillez vérifier les connexions et l'état de sécurité avant de relancer la mesure.)");
    default:
        return QString::fromUtf8(R"(Résultat: Inconnu, veuillez faire une mesure pour obtenir un résultat.
 )");
    }
}