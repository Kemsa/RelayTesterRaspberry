#include "relayselectscreen.h"
#include "ui_relayselectscreen.h"

RelaySelectScreen::RelaySelectScreen(QWidget* parent)
    : QWidget(parent), ui(new Ui::RelaySelect) {
    ui->setupUi(this);
}

RelaySelectScreen::~RelaySelectScreen() {
    delete ui;
}
