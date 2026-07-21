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
        float tolerance = targetCurrent * 0.1f; // 10% tolerance
        int result = CurrentAdjuster::instance().adjustCurrentToTarget(static_cast<float>(targetCurrent), tolerance);
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
            headerText = "B1 V";
            break;
        case StaticReadings::ReadingFlags::coil1Current:
            headerText = "B1 I";
            break;
        case StaticReadings::ReadingFlags::coil2Voltage:
            headerText = "B2 V";
            break;
        case StaticReadings::ReadingFlags::coil2Current:
            headerText = "B2 I";
            break;
        case StaticReadings::ReadingFlags::contact12Voltage:
            headerText = "C1 V";
            break;
        case StaticReadings::ReadingFlags::contact13Voltage:
            headerText = "C2 V";
            break;
        case StaticReadings::ReadingFlags::contactCurrent:
            headerText = "C I";
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
    }
}