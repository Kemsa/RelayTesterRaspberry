#include "calibrationscreen.h"
#include "config.h"
#include "ui_calibrationscreen.h"

#include "powerSupply.h"
#include "powercontrol.h"
#include "staticreadings.h"

#include <QDebug>
#include <QThread>
#include <memory>
#include <thread>

CalibrationScreen::CalibrationScreen(QWidget* parent)
    : QWidget(parent), ui(new Ui::CalibrationScreen) {
    ui->setupUi(this);

    connect(ui->calibratePB, &QPushButton::clicked, this, &CalibrationScreen::calibrate);
    connect(ui->exportPB, &QPushButton::clicked, this, &CalibrationScreen::exportCalibration);
}

CalibrationScreen::~CalibrationScreen() {
    delete ui;
}

void CalibrationScreen::calibrate() {

    std::thread([this]() {
        qDebug() << "Calibration started";

        int nMeasures = 6;

        auto powerControl = PowerControl::getInstance();
        auto powerSupply = powerSupply::instance();
        auto staticReadings = StaticReadings::getInstance();

        staticReadings->resetCoilCalibrationFactor();

        powerSupply->setMaxValues(5, MAX_CURRENT_mA / 1000.0);
        powerSupply->setVoltage(5);

        powerSupply->enableOutput();

        // Measure resistance 1
        powerControl->enableCoilTop(static_cast<PowerControl::Coil>(1));
        powerControl->enableCoilBottom(static_cast<PowerControl::Coil>(1));
        QThread::msleep(100); // Wait for the coil to stabilize

        double currentAverage1 = 0.0;
        double voltageAverage1 = 0.0;

        QMap<StaticReadings::ReadingFlags, std::vector<ADCValue>> readings;
        readings = staticReadings->getMultipleReadings(static_cast<uint8_t>(StaticReadings::ReadingFlags::coil1Current) | static_cast<uint8_t>(StaticReadings::ReadingFlags::coil1Voltage), nMeasures);

        for (int i = 0; i < nMeasures; ++i) {
            currentAverage1 += StaticReadings::toCoilCurrent_mA(readings[StaticReadings::ReadingFlags::coil1Current][i]);
            voltageAverage1 += StaticReadings::toCoilVoltage_V(readings[StaticReadings::ReadingFlags::coil1Voltage][i]);
        }
        currentAverage1 /= nMeasures;
        voltageAverage1 /= nMeasures;

        double targetVoltage1 = SELF_TEST_RESISTANCE_1 * currentAverage1 * 1e-3;
        double correction = targetVoltage1 / voltageAverage1;

        qDebug() << "currentAverage1:" << currentAverage1;
        qDebug() << "voltageAverage1:" << voltageAverage1;
        qDebug() << "targetVoltage1:" << targetVoltage1;

        qDebug() << "resistance before correction:" << voltageAverage1 / currentAverage1 * 1e3;
        qDebug() << "correction factor:" << correction;
        qDebug() << "corrected resistance:" << voltageAverage1 * correction / currentAverage1 * 1e3;

        // Measure resistance 2

        powerControl->enableCoilTop(static_cast<PowerControl::Coil>(2));
        powerControl->enableCoilBottom(static_cast<PowerControl::Coil>(2));

        double currentAverage2 = 0.0;
        double voltageAverage2 = 0.0;

        QMap<StaticReadings::ReadingFlags, std::vector<ADCValue>> readings2;
        readings2 = staticReadings->getMultipleReadings(static_cast<uint8_t>(StaticReadings::ReadingFlags::coil2Current) | static_cast<uint8_t>(StaticReadings::ReadingFlags::coil2Voltage), nMeasures);

        for (int i = 0; i < nMeasures; ++i) {
            currentAverage2 += StaticReadings::toCoilCurrent_mA(readings2[StaticReadings::ReadingFlags::coil2Current][i]);
            voltageAverage2 += StaticReadings::toCoilVoltage_V(readings2[StaticReadings::ReadingFlags::coil2Voltage][i]);
        }
        currentAverage2 /= nMeasures;
        voltageAverage2 /= nMeasures;

        double targetVoltage2 = SELF_TEST_RESISTANCE_2 * currentAverage2 * 1e-3;
        double correction2 = targetVoltage2 / voltageAverage2;
        qDebug() << "resistance before correction:" << voltageAverage2 / currentAverage2 *1e3;
        qDebug() << "correction factor:" << correction2;
        qDebug() << "corrected resistance:" << voltageAverage2 * correction2 / currentAverage2 * 1e3;

        staticReadings->setCoilCalibrationFactor(correction, correction2);
    }).detach();
}

void CalibrationScreen::exportCalibration() {
}
