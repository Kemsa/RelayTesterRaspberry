#include "relaymeasurescreen.h"
#include "ui_relaymeasurescreen.h"

RelayMeasureScreen::RelayMeasureScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RelayMeasureScreen)
{
    ui->setupUi(this);
}

RelayMeasureScreen::~RelayMeasureScreen()
{
    delete ui;
}
