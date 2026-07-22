#include "relaymeasurescreen.h"
#include "navigator.h"
#include "ui_relaymeasurescreen.h"
#include <QDebug>
#include <QShowEvent>
#include <memory>

RelayMeasureScreen::RelayMeasureScreen(QWidget* parent)
    : QWidget(parent), ui(new Ui::RelayMeasureScreen) {
    ui->setupUi(this);
}

RelayMeasureScreen::~RelayMeasureScreen() {
    delete ui;
}

void RelayMeasureScreen::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);

    // Check navigator exchange data for this screen
    auto data = Navigator::instance().getScreenExchangeData(Navigator::RelayMeasure_screen);
    if (data) {
        auto strptr = std::static_pointer_cast<QString>(data);
        if (strptr) {
            emit navigatedTo(*strptr);
            onNavigatedTo(*strptr);
            return;
        }
    }

    emit navigatedTo(QString());
    onNavigatedTo(QString());
}

void RelayMeasureScreen::onNavigatedTo(const QString& path) {
    Q_UNUSED(path);
    // Default handler: override or connect to navigatedTo() to react when shown

    qDebug() << "RelayMeasureScreen navigated to path:" << path;
}
