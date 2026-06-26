#include "mainwindow.h"
#include "calibration.h"
#include "homescreen.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->setFixedSize(800, 480);

    this->statusBar()->showMessage("test status");
    // this->setWindowFlags(Qt::FramelessWindowHint);

    Navigator::initialize(this, ui->stackedWidget);

    connect(ui->backPB, &QPushButton::clicked, this, [this]() {
        Navigator::instance().goBack();
    });
    connect(ui->homePB, &QPushButton::clicked, this, [this]() {
        Navigator::instance().goHome();
    });
}

MainWindow::~MainWindow() {
    delete ui;
}
