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

        if (contactType == DynamicReadings::ContactType::COIL1 || contactType == DynamicReadings::ContactType::COIL2) {
            PowerControl::getInstance()->enableCoil(contactType == DynamicReadings::ContactType::COIL1 ? PowerControl::Coil::COIL1 : PowerControl::Coil::COIL2);
        } else {
            PowerControl::getInstance()->disableCoils();
        }

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
        triggerMeasurementWithCoil(DynamicReadings::ContactType::COILS_OFF);
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
    const char* coilText;

    switch (switchResult->getCoilSwitch()) {
    case DynamicReadings::ContactType::COIL1:
        coilText = "bobine: bobine 1";
        break;
    case DynamicReadings::ContactType::COIL2:
        coilText = "bobine: bobine 2";
        break;
    case DynamicReadings::ContactType::COILS_OFF:
        coilText = "bobine: bobines inactives";
        break;
    case DynamicReadings::ContactType::BOTH_COILS:
        coilText = "bobine: toutes bobines";
        break;
    default:
        coilText = "bobine: mesure indisponible";
        break;
    }

    ui->coill_LBL->setText(coilText);

    ui->contact1_LBL->setText(
        QString("contact 1: %1 us, edge=%2")
            .arg(switchResult->getContactAStableSwitchTime_us())
            .arg(switchResult->getContactATransistionType()));
    ui->contact2_LBL->setText(
        QString("contact 2: %1 us, edge=%2")
            .arg(switchResult->getContactBStableSwitchTime_us())
            .arg(switchResult->getContactBTransistionType()));
}