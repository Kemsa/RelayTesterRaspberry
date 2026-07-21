#include "dynamicswidget.h"
#include "dynamicreadings.h"
#include "dynamicswitch.h"
#include "powercontrol.h"
#include "ui_dynamicswidget.h"

#include <QMetaObject>
#include <QPointer>

#include <thread>

DynamicsWidget::DynamicsWidget(QWidget* parent)
    : QFrame(parent), ui(new Ui::DynamicsWidget) {
    ui->setupUi(this);

    const auto triggerMeasurementWithCoil = [this](DynamicReadings::ContactType contactType) {
        QPointer<DynamicsWidget> self(this);
        auto future = DynamicReadings::getInstance()->waitAndProcessOneSwitch(contactType, 500);

        PowerControl::getInstance()->enableCoil(contactType == DynamicReadings::ContactType::COIL1 ? PowerControl::Coil::COIL1 : PowerControl::Coil::COIL2);

        std::thread([self, future = std::move(future)]() mutable {
            std::shared_ptr<DynamicSwitch> switchResult = future.get();
            if (!self) {
                return;
            }

            QMetaObject::invokeMethod(self, [self, switchResult]() {
                if (!self) {
                    return;
                }
                self->handleSwitchResult(switchResult);
            });
        }).detach();
    };

    const auto triggerMeasurementWithDisableCoils = [triggerMeasurementWithCoil]() {
        PowerControl::getInstance()->disableCoils();
        triggerMeasurementWithCoil(DynamicReadings::ContactType::COIL2);
    };

    connect(ui->coil1Active_PB, &QPushButton::clicked, this,
            [triggerMeasurementWithCoil]() { triggerMeasurementWithCoil(DynamicReadings::ContactType::COIL1); });
    connect(ui->coil2Active_PB, &QPushButton::clicked, this,
            [triggerMeasurementWithCoil]() { triggerMeasurementWithCoil(DynamicReadings::ContactType::COIL2); });
    connect(ui->inactive_PB, &QPushButton::clicked, this,
            [triggerMeasurementWithDisableCoils]() { triggerMeasurementWithDisableCoils(); });
}

DynamicsWidget::~DynamicsWidget() {
    delete ui;
}

void DynamicsWidget::handleSwitchResult(std::shared_ptr<DynamicSwitch> switchResult) {
    if (!switchResult) {
        ui->coill_LBL->setText("bobine: mesure indisponible");
        ui->contact1_LBL->setText("contact 1: mesure indisponible");
        ui->contact2_LBL->setText("contact 2: mesure indisponible");
        return;
    }
    if (switchResult->isValid() == false) {
        ui->coill_LBL->setText("bobine: mesure invalide");
        ui->contact1_LBL->setText("contact 1: mesure invalide");
        ui->contact2_LBL->setText("contact 2: mesure invalide");
        return;
    }
    const char* coilText = switchResult->getCoilSwitch() == DynamicReadings::ContactType::COIL1
                               ? "bobine: bobine 1"
                               : "bobine: bobine 2";
    ui->coill_LBL->setText(coilText);

    ui->contact1_LBL->setText(
        QString("contact 1: %1 us, edge=%2")
            .arg(switchResult->getContact1SwitchTime())
            .arg(switchResult->getContact1TransistionType()));
    ui->contact2_LBL->setText(
        QString("contact 2: %1 us, edge=%2")
            .arg(switchResult->getContact2SwitchTime())
            .arg(switchResult->getContact2TransistionType()));
}