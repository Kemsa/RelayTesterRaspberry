#include "adc24reading.h"
#include "currentadjuster.h"
#include "staticreadings.h"
#include "ui_adc24reading.h"
#include <QDebug>

ADC24Reading::ADC24Reading(QWidget* parent)
    : QFrame(parent), ui(new Ui::ADC24Reading) {
    ui->setupUi(this);

    connect(ui->getValues_PB, &QPushButton::clicked, this, [this]() {
        int nMeasures = ui->NValues_SB->value();
        makeMeasureAndDisplay(nMeasures);
    });

    ui->values_TBL->setRowCount(3);
    ui->values_TBL->setVerticalHeaderLabels({"Brute", "ADC (mV)", "Finale (V)"});

    connect(ui->currentAdjust_PB, &QPushButton::clicked, this, [this]() {
        int targetCurrent = ui->targetCurrent_SB->value();
        float tolerance = targetCurrent * 0.05f; // 5% tolerance
        int result = CurrentAdjuster::instance()->adjustCurrentToTarget(static_cast<float>(targetCurrent), tolerance);
    });
}

ADC24Reading::~ADC24Reading() {
    delete ui;
}

void ADC24Reading::makeMeasureAndDisplay(int nMeasures) {

    auto results = StaticReadings::getInstance()->getMultipleReadings(
        static_cast<uint8_t>(StaticReadings::ReadingFlags::all),
        nMeasures);

    ui->values_TBL->clearContents();
    ui->values_TBL->setColumnCount(results.size());

    for (int col = 0; col < results.size(); ++col) {
        auto flag = results.keys().at(col);
        auto value = results.value(flag);

        QString headerText;
        switch (flag) {
        case StaticReadings::ReadingFlags::coil1Voltage:
            headerText = "B1 V (V)";
            break;
        case StaticReadings::ReadingFlags::coil1Current:
            headerText = "B1 I (mA)";
            break;
        case StaticReadings::ReadingFlags::coil2Voltage:
            headerText = "B2 V (V)";
            break;
        case StaticReadings::ReadingFlags::coil2Current:
            headerText = "B2 I (mA)";
            break;
        case StaticReadings::ReadingFlags::contactAVoltage:
            headerText = "C1 V (mV)";
            break;
        case StaticReadings::ReadingFlags::contactBVoltage:
            headerText = "C2 V (mV)";
            break;
        case StaticReadings::ReadingFlags::contactCurrent:
            headerText = "C I (mA)";
            break;
        default:
            headerText = "Unknown";
        }

        // display averaged value over N readings
        int64_t averageValueSum = 0;
        for (int j = 0; j < value.size(); ++j) {
            averageValueSum += value[j].getValue();
        }
        int averageValue = 0;
        if (!value.empty()) {
            averageValue = static_cast<int>(averageValueSum / static_cast<qint64>(value.size()));
        }

        ui->values_TBL->setHorizontalHeaderItem(col, new QTableWidgetItem(headerText));
        ui->values_TBL->setItem(0, col, new QTableWidgetItem(QString::number(averageValue)));
        ui->values_TBL->setItem(1, col, new QTableWidgetItem(QString::number(ADCValue::getMillivoltsFromValue(averageValue, value[0].range))));

        switch(flag){
            case StaticReadings::ReadingFlags::coil1Voltage:
            case StaticReadings::ReadingFlags::coil2Voltage:
                ui->values_TBL->setItem(2, col, new QTableWidgetItem(QString::number(StaticReadings::toCoilVoltage_V(value[0]))));
                break;
            case StaticReadings::ReadingFlags::coil1Current:
            case StaticReadings::ReadingFlags::coil2Current:
                ui->values_TBL->setItem(2, col, new QTableWidgetItem(QString::number(StaticReadings::toCoilCurrent_mA(value[0]))));
                break;
            case StaticReadings::ReadingFlags::contactAVoltage:
            case StaticReadings::ReadingFlags::contactBVoltage:
                ui->values_TBL->setItem(2, col, new QTableWidgetItem(QString::number(StaticReadings::toContactVoltage_mV(value[0]))));
                break;
            case StaticReadings::ReadingFlags::contactCurrent:
                ui->values_TBL->setItem(2, col, new QTableWidgetItem(QString::number(StaticReadings::toContactCurrent_mA(value[0]))));
                break;
            default:
                ui->values_TBL->setItem(2, col, new QTableWidgetItem("N/A"));
        }
    }
}