#ifndef DYNAMICSWIDGET_H
#define DYNAMICSWIDGET_H

#include <QFrame>
#include <memory>

class DynamicSwitch;

namespace Ui {
class DynamicsWidget;
}

class DynamicsWidget : public QFrame {
    Q_OBJECT

public:
    explicit DynamicsWidget(QWidget* parent = nullptr);
    ~DynamicsWidget();

private:
    Ui::DynamicsWidget* ui;

    void handleSwitchResult(std::shared_ptr<DynamicSwitch> switchResult);
};

#endif // DYNAMICSWIDGET_H
