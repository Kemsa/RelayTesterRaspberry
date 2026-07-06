#pragma once

#include <QLabel>

class LedWidget : public QLabel {
    Q_OBJECT
public:
    explicit LedWidget(QWidget* parent = 0);

    enum State {
        StateOk,
        StateOkBlue,
        StateWarning,
        StateError
    };

signals:

public slots:
    void setState(State state);
    void setState(bool state);
};
