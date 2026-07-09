#pragma once

#include <QObject>
#include <QString>
#include <QtGlobal>

class QMessageLogContext;

class LogBus : public QObject {
    Q_OBJECT

public:
    static LogBus& instance();

    void installQtMessageHandler();

signals:
    void messageLogged(QtMsgType type, const QString& msg);

private:
    LogBus();
    ~LogBus() override;

    void dispatch(QtMsgType type, const QMessageLogContext& context, const QString& msg);

    QtMessageHandler m_previousHandler = nullptr;
    bool m_handlerInstalled = false;

    Q_DISABLE_COPY_MOVE(LogBus)

    friend void relayTesterQtMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
};
