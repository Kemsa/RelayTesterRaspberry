#include "relayselectscreen.h"
#include "navigator.h"
#include "relaylistmodel.h"
#include "ui_relayselectscreen.h"
#include <QItemSelectionModel>

RelaySelectScreen::RelaySelectScreen(QWidget* parent)
    : QWidget(parent),
      ui(new Ui::RelaySelect),
      m_model(std::make_unique<relayListModel>(this)) {
    ui->setupUi(this);
    ui->relaySelect_CV->setModel(m_model.get());
    ui->selectRelay_PB->setEnabled(false);

    connect(ui->relaySelect_CV->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex& current, const QModelIndex&) {
                updateSelectionState(current);
            });

    connect(ui->selectRelay_PB, &QPushButton::clicked, this, [this]() {
        QModelIndex currentIndex = ui->relaySelect_CV->currentIndex();
        if (m_model->isFile(currentIndex)) {
            QString selectedRelayPath = m_model->filePath(currentIndex);
            Navigator::instance().navigateToWithData(Navigator::RelayMeasure_screen, std::make_shared<QString>(selectedRelayPath));
        }
    });
}

RelaySelectScreen::~RelaySelectScreen() {
    delete ui;
}

void RelaySelectScreen::updateSelectionState(const QModelIndex& currentIndex) {
    ui->selectRelay_PB->setEnabled(m_model->isFile(currentIndex));
}
