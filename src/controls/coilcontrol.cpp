#include "coilcontrol.h"
#include <QDebug>
#include <stdexcept>

CoilControl* CoilControl::s_instance = nullptr;

CoilControl::CoilControl(int coil1Pin, int coil2Pin)
    : m_coil1Pin(coil1Pin), m_coil2Pin(coil2Pin) {

    m_GPIOHandler->setPinMode(m_coil1Pin, GPIOHandler::PinMode::WPI_OUTPUT);
    m_GPIOHandler->setPinMode(m_coil2Pin, GPIOHandler::PinMode::WPI_OUTPUT);

    m_GPIOHandler->pinWrite(m_coil1Pin, GPIOHandler::Level::WPI_LOW);
    m_GPIOHandler->pinWrite(m_coil2Pin, GPIOHandler::Level::WPI_LOW);
}

CoilControl* CoilControl::getInstance() {
    if (!s_instance) {
        throw std::runtime_error("CoilControl instance not initialized. Call initialize() first.");
    }
    return s_instance;
}

CoilControl* CoilControl::initialize(int coil1Pin, int coil2Pin) {
    if (!s_instance) {
        s_instance = new CoilControl(coil1Pin, coil2Pin);
    }
    return s_instance;
}

bool CoilControl::enableCoil(Coil coil) {

    switch (coil) {
    case COIL1:
        m_GPIOHandler->pinWrite(m_coil2Pin, GPIOHandler::Level::WPI_LOW);
        m_GPIOHandler->pinWrite(m_coil1Pin, GPIOHandler::Level::WPI_HIGH);
        qDebug() << "CoilControl: Coil 1 enabled";
        return true;
    case COIL2:
        m_GPIOHandler->pinWrite(m_coil1Pin, GPIOHandler::Level::WPI_LOW);
        m_GPIOHandler->pinWrite(m_coil2Pin, GPIOHandler::Level::WPI_HIGH);
        qDebug() << "CoilControl: Coil 2 enabled";
        return true;
    default:
        return false;
    }
}

bool CoilControl::disableCoils() {
    m_GPIOHandler->pinWrite(m_coil1Pin, GPIOHandler::Level::WPI_LOW);
    m_GPIOHandler->pinWrite(m_coil2Pin, GPIOHandler::Level::WPI_LOW);
    qDebug() << "CoilControl: coils disabled";
    return true;
}
