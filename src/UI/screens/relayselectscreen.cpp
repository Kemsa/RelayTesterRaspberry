#include "relayselectscreen.h"

#include "relaylistmodel.h"

#include <QItemSelectionModel>

#include "ui_relayselectscreen.h"

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
}

RelaySelectScreen::~RelaySelectScreen() {
    delete ui;
}

void RelaySelectScreen::updateSelectionState(const QModelIndex& currentIndex) {
    ui->selectRelay_PB->setEnabled(m_model->isFile(currentIndex));
}
