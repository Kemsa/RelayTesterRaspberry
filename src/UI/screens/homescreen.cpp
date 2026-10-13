#include "homescreen.h"
#include "navigator.h"
#include "ui_homescreen.h"

HomeScreen::HomeScreen(QWidget* parent)
    : QWidget(parent), ui(new Ui::HomeScreen) {
    ui->setupUi(this);

    connect(ui->testPB, &QPushButton::clicked, this, [this]() {
        Navigator::instance().navigateTo(Navigator::Test_screen);
    });

    connect(ui->relayMeasure_PB, &QPushButton::clicked, this, [this]() {
        Navigator::instance().navigateTo(Navigator::RelaySelect_screen);
    });
    connect(ui->logs_PB, &QPushButton::clicked, this, [this]() {
        Navigator::instance().navigateTo(Navigator::Logs_screen);
    });
    connect(ui->calibrationPB, &QPushButton::clicked, this, [this]() {
        Navigator::instance().navigateTo(Navigator::Calibration_screen);
    });

    // Not ready for deployment
    ui->calibrationPB->setVisible(false);
}

HomeScreen::~HomeScreen() {
    delete ui;
}
