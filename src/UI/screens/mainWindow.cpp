#include "mainwindow.h"
#include "calibrationscreen.h"
#include "homescreen.h"
#include "logbus.h"
#include "navigator.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->setFixedSize(800, 480);

    connect(&LogBus::instance(), &LogBus::messageLogged, this, [this](QtMsgType type, const QString& msg) {
        displayMessage(type, msg);
    });

#ifdef Q_OS_LINUX
    this->setWindowFlags(Qt::FramelessWindowHint);
#endif

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

void MainWindow::displayMessage(QtMsgType type, const QString& msg) {

    switch (type) {
    case QtDebugMsg:
        break;
    case QtInfoMsg:
        this->ui->statusbar->setStyleSheet("color: WhiteSmoke");
        this->ui->statusbar->showMessage(msg, 5000); // Display the message for 5 seconds
        break;
    case QtWarningMsg:
        this->ui->statusbar->setStyleSheet("color: GoldenRod");
        this->ui->statusbar->showMessage(msg, 5000); // Display the message for 5 seconds
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        this->ui->statusbar->setStyleSheet("color: Red"); // Set the text color to blue for info messages
        this->ui->statusbar->showMessage(msg, 10000);     // Display the message for 10 seconds
        break;
    }
}