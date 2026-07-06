#include "calibrationscreen.h"
#include "ui_calibrationscreen.h"

CalibrationScreen::CalibrationScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Calibration)
{
    ui->setupUi(this);
}

CalibrationScreen::~CalibrationScreen()
{
    delete ui;
}
