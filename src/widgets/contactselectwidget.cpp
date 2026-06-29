#include "contactselectwidget.h"
#include "contactselector.h"
#include "ui_contactselectwidget.h"

ContactSelectWidget::ContactSelectWidget(QWidget* parent)
    : QFrame(parent), ui(new Ui::ContactSelectWidget) {
    ui->setupUi(this);

    connect(ui->contact1PB, &QPushButton::clicked, this, [this]() {
        ContactSelector::instance().selectContact(1);
    });
    connect(ui->contact2PB, &QPushButton::clicked, this, [this]() {
        ContactSelector::instance().selectContact(2);
    });
    connect(ui->contact3PB, &QPushButton::clicked, this, [this]() {
        ContactSelector::instance().selectContact(3);
    });
    connect(ui->contact4PB, &QPushButton::clicked, this, [this]() {
        ContactSelector::instance().selectContact(4);
    });
    connect(ui->contact5PB, &QPushButton::clicked, this, [this]() {
        ContactSelector::instance().selectContact(5);
    });
    connect(ui->contact6PB, &QPushButton::clicked, this, [this]() {
        ContactSelector::instance().selectContact(6);
    });
    connect(ui->contact7PB, &QPushButton::clicked, this, [this]() {
        ContactSelector::instance().selectContact(7);
    });
    connect(ui->contact8PB, &QPushButton::clicked, this, [this]() {
        ContactSelector::instance().selectContact(8);
    });
    connect(ui->contactNonePB, &QPushButton::clicked, this, [this]() {
        ContactSelector::instance().selectContact(0);
    });

    connect(&ContactSelector::instance(), &ContactSelector::contactSelected, this, [this](int contactIndex) {
        // Update the UI based on the selected contact
        if (contactIndex == 0) {
            ui->selectedContactLabel->setText("Aucun");
        } else {
            ui->selectedContactLabel->setNum(contactIndex);
        }
    });
}

ContactSelectWidget::~ContactSelectWidget() {
    delete ui;
}
