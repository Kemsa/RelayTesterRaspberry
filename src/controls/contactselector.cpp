#include "contactselector.h"
#include "GPIOHandler.h"
#include <QDebug>

ContactSelector* ContactSelector::s_instance = nullptr;

ContactSelector& ContactSelector::initialize(int s0, int s1, int s2, int en) {
    if (!s_instance) {
        s_instance = new ContactSelector(s0, s1, s2, en);
    }
    return *s_instance;
}

ContactSelector& ContactSelector::instance() {
    Q_ASSERT_X(s_instance, "ContactSelector::instance", "ContactSelector is not initialized");
    return *s_instance;
}

ContactSelector::ContactSelector(int s0, int s1, int s2, int en) : m_s0(s0), m_s1(s1), m_s2(s2), m_en(en) {

    m_contactMap.insert(0, PinSelection::None);
    m_contactMap.insert(1, PinSelection::Contact1);
    m_contactMap.insert(2, PinSelection::Contact2);
    m_contactMap.insert(3, PinSelection::Contact3);
    m_contactMap.insert(4, PinSelection::Contact4);
    m_contactMap.insert(5, PinSelection::Contact5);
    m_contactMap.insert(6, PinSelection::Contact6);
    m_contactMap.insert(7, PinSelection::Contact7);
    m_contactMap.insert(8, PinSelection::Contact8);

    // Set pin modes for s0, s1, s2, and en
    GPIOHandler::instance()->setPinMode(m_s0, GPIOHandler::PinMode::WPI_OUTPUT);
    GPIOHandler::instance()->setPinMode(m_s1, GPIOHandler::PinMode::WPI_OUTPUT);
    GPIOHandler::instance()->setPinMode(m_s2, GPIOHandler::PinMode::WPI_OUTPUT);
    GPIOHandler::instance()->setPinMode(m_en, GPIOHandler::PinMode::WPI_OUTPUT);

    // Set initial state to None (no contact selected)
    selectContact(0);
}

void ContactSelector::selectContact(int contactIndex) {
    if (m_currentContact == contactIndex) {
        return;
    }

    PinSelection selection = m_contactMap.value(contactIndex, PinSelection::None);

    qDebug() << "Selecting contact index:" << contactIndex << "with selection value:" << static_cast<int>(selection);
    GPIOHandler::instance()->pinWrite(m_en, (GPIOHandler::Level)((selection & enable_mask) > 0));
    GPIOHandler::instance()->pinWrite(m_s0, (GPIOHandler::Level)((selection & s0_mask) > 0));
    GPIOHandler::instance()->pinWrite(m_s1, (GPIOHandler::Level)((selection & s1_mask) > 0));
    GPIOHandler::instance()->pinWrite(m_s2, (GPIOHandler::Level)((selection & s2_mask) > 0));

    m_currentContact = contactIndex;
    // Emit the signal with the selected contact name
    emit contactSelected(contactIndex);
}
