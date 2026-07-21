#ifndef RELAYSELECT_H
#define RELAYSELECT_H

#include <QWidget>

namespace Ui {
class RelaySelect;
}

class RelaySelectScreen : public QWidget {
    Q_OBJECT

public:
    explicit RelaySelectScreen(QWidget* parent = nullptr);
    ~RelaySelectScreen();

private:
    Ui::RelaySelect* ui;
};

#endif // RELAYSELECT_H
