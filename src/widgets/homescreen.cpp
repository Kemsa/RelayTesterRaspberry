#include "homescreen.h"
#include "navigator.h"
#include "ui_homescreen.h"

HomeScreen::HomeScreen(QWidget* parent)
    : QWidget(parent), ui(new Ui::HomeScreen) {
    ui->setupUi(this);

    connect(ui->calibrationPB, &QPushButton::clicked, this, [this]() {
        Navigator::instance().navigateTo(Navigator::Calibration_screen);
    });
}

HomeScreen::~HomeScreen() {
    delete ui;
}
