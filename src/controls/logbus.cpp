#include "logbus.h"

namespace {
LogBus* g_logBus = nullptr;
}

void relayTesterQtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    if (g_logBus != nullptr) {
        g_logBus->dispatch(type, context, msg);
    }
}

LogBus& LogBus::instance() {
    static LogBus instance;
    return instance;
}

LogBus::LogBus() {
    g_logBus = this;
}

LogBus::~LogBus() {
    if (m_handlerInstalled) {
        qInstallMessageHandler(m_previousHandler);
    }

    if (g_logBus == this) {
        g_logBus = nullptr;
    }
}

void LogBus::installQtMessageHandler() {
    if (m_handlerInstalled) {
        return;
    }

    m_previousHandler = qInstallMessageHandler(&relayTesterQtMessageHandler);
    m_handlerInstalled = true;
}

void LogBus::dispatch(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    emit messageLogged(type, msg);

    if (m_previousHandler != nullptr) {
        m_previousHandler(type, context, msg);
    }
}
