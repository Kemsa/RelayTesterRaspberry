#include "testingscreen.h"
#include "ui_testingscreen.h"

TestScreen::TestScreen(QWidget* parent)
    : QWidget(parent), ui(new Ui::TestScreen) {
    ui->setupUi(this);
}

TestScreen::~TestScreen() {
    delete ui;
}
