#ifndef POWERSUPPLYSETUPWIDGET_H
#define POWERSUPPLYSETUPWIDGET_H

#include <QWidget>
#include <QFrame>

namespace Ui {
class PowerSupplySetup;
}

class PowerSupplySetupWidget : public QFrame
{
    Q_OBJECT

public:
    explicit PowerSupplySetupWidget(QWidget *parent = nullptr);
    ~PowerSupplySetupWidget();

private:
    Ui::PowerSupplySetup *ui;
};

#endif // POWERSUPPLYSETUPWIDGET_H
