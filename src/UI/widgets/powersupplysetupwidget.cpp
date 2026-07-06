#include "powersupplysetupwidget.h"
#include "powerSupply.h"
#include "ui_powersupplysetupwidget.h"
#include <QDebug>

PowerSupplySetupWidget::PowerSupplySetupWidget(QWidget* parent)
    : QFrame(parent), ui(new Ui::PowerSupplySetup) {
    ui->setupUi(this);

    connect(ui->toggleSupply_PB, &QPushButton::clicked, this, [this]() {
        powerSupply* supply = powerSupply::instance();
        if (!supply && !supply->isConnected()) {
            qWarning() << "Power supply instance is not initialized and connected.";
            return;
        }

        if (ui->toggleSupply_PB->text() == "Toggle OFF") {
            supply->disableOutput();
            ui->toggleSupply_PB->setText("Toggle ON");
            ui->supply_LED->setState(false);
        } else {
            supply->setMaxValues(ui->voltage_SB->value(), ui->current_SB->value() / 1000.0);
            supply->setVoltage(ui->voltage_SB->value());
            supply->enableOutput();
            ui->toggleSupply_PB->setText("Toggle OFF");
            ui->supply_LED->setState(true);
        }
    });

    connect(ui->predef12V_PB, &QPushButton::clicked, this, [this]() {
        powerSupply* supply = powerSupply::instance();
        if (!supply && !supply->isConnected()) {
            qWarning() << "Power supply instance is not initialized and connected.";
            return;
        }

        supply->setVoltage(12.0);
        ui->voltage_SB->setValue(12.0);
    });
    connect(ui->predef24V_PB, &QPushButton::clicked, this, [this]() {
        powerSupply* supply = powerSupply::instance();
        if (!supply && !supply->isConnected()) {
            qWarning() << "Power supply instance is not initialized and connected.";
            return;
        }

        supply->setVoltage(24.0);
        ui->voltage_SB->setValue(24.0);
    });
    connect(ui->predef48V_PB, &QPushButton::clicked, this, [this]() {
        powerSupply* supply = powerSupply::instance();
        if (!supply && !supply->isConnected()) {
            qWarning() << "Power supply instance is not initialized and connected.";
            return;
        }

        supply->setVoltage(48.0);
        ui->voltage_SB->setValue(48.0);
    });

    connect(ui->voltage_SB, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
        powerSupply* supply = powerSupply::instance();
        if (!supply || !supply->isConnected()) {
            qWarning() << "Power supply instance is not initialized and connected.";
            return;
        }

        supply->setVoltage(value);
    });

    connect(ui->current_SB, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        powerSupply* supply = powerSupply::instance();
        if (!supply || !supply->isConnected()) {
            qWarning() << "Power supply instance is not initialized and connected.";
            return;
        }

        supply->setCurrent(value / 1000.0);
    });

    ui->supply_LED->setState(powerSupply::instance() && powerSupply::instance()->isEnabled());
}

PowerSupplySetupWidget::~PowerSupplySetupWidget() {
    delete ui;
}
