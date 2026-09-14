#ifndef LOGSCREEN_H
#define LOGSCREEN_H

#include <QVector>
#include <QWidget>
#include <QtGlobal>

namespace Ui {
class LogScreen;
}

class Log {
public:
    QString date;
    QtMsgType type = QtInfoMsg;
    QString content;
};

class LogScreen : public QWidget {
    Q_OBJECT

public:
    explicit LogScreen(QWidget* parent = nullptr);
    ~LogScreen() override;

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void refreshLogDisplay();

private:
    Ui::LogScreen* ui;
    QVector<Log> m_allLogLines;
};

#endif // LOGSCREEN_H
