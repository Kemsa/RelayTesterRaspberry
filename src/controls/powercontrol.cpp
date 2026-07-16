#include "powercontrol.h"
#include "GPIOHandler.h"
#include <QDebug>
#include <stdexcept>

PowerControl* PowerControl::s_instance = nullptr;

PowerControl::PowerControl(int coil1Pin, int coil2Pin, int contactPowerEnablePin,
                           int reedPin, int boardPin)
    : m_coil1Pin(coil1Pin), m_coil2Pin(coil2Pin), m_contactPowerEnablePin(contactPowerEnablePin),
      m_reedPin(reedPin), m_boardPin(boardPin) {

    m_GPIOHandler->setPinMode(m_coil1Pin, GPIOHandler::PinMode::WPI_OUTPUT);
    m_GPIOHandler->setPinMode(m_coil2Pin, GPIOHandler::PinMode::WPI_OUTPUT);
    m_GPIOHandler->setPinMode(m_contactPowerEnablePin, GPIOHandler::PinMode::WPI_OUTPUT);
    m_GPIOHandler->setPinMode(m_reedPin, GPIOHandler::PinMode::WPI_INPUT);
    m_GPIOHandler->setPinMode(m_boardPin, GPIOHandler::PinMode::WPI_INPUT);

    m_GPIOHandler->pinWrite(m_coil1Pin, GPIOHandler::Level::WPI_LOW);
    m_GPIOHandler->pinWrite(m_coil2Pin, GPIOHandler::Level::WPI_LOW);
    m_GPIOHandler->pinWrite(m_contactPowerEnablePin, GPIOHandler::Level::WPI_LOW);

    m_GPIOHandler->setupInterrupt(m_reedPin, GPIOHandler::Interrupt::WPI_INT_EDGE_BOTH, staticReedInterrupt, 10, nullptr);
    m_GPIOHandler->setupInterrupt(m_boardPin, GPIOHandler::Interrupt::WPI_INT_EDGE_BOTH, staticBoardInterrupt, 10, nullptr);

    reedClosed = m_GPIOHandler->pinRead(m_reedPin) == GPIOHandler::Level::WPI_HIGH;
    boardClosed = m_GPIOHandler->pinRead(m_boardPin) == GPIOHandler::Level::WPI_HIGH;
}

PowerControl* PowerControl::getInstance() {
    if (!s_instance) {
        throw std::runtime_error("PowerControl instance not initialized. Call initialize() first.");
    }
    return s_instance;
}

PowerControl* PowerControl::initialize(int coil1Pin, int coil2Pin, int contactPowerEnablePin,
                                       int reedPin, int boardPin) {
    if (!s_instance) {
        s_instance = new PowerControl(coil1Pin, coil2Pin, contactPowerEnablePin, reedPin, boardPin);
    }
    return s_instance;
}

bool PowerControl::enableCoil(Coil coil) {

    switch (coil) {
    case COIL1:
        m_GPIOHandler->pinWrite(m_coil2Pin, GPIOHandler::Level::WPI_LOW);
        m_GPIOHandler->pinWrite(m_coil1Pin, GPIOHandler::Level::WPI_HIGH);
        qDebug() << "PowerControl: Coil 1 enabled";
        return true;
    case COIL2:
        m_GPIOHandler->pinWrite(m_coil1Pin, GPIOHandler::Level::WPI_LOW);
        m_GPIOHandler->pinWrite(m_coil2Pin, GPIOHandler::Level::WPI_HIGH);
        qDebug() << "PowerControl: Coil 2 enabled";
        return true;
    default:
        return false;
    }
}

bool PowerControl::disableCoils() {
    m_GPIOHandler->pinWrite(m_coil1Pin, GPIOHandler::Level::WPI_LOW);
    m_GPIOHandler->pinWrite(m_coil2Pin, GPIOHandler::Level::WPI_LOW);
    qDebug() << "PowerControl: coils disabled";
    return true;
}

// bool PowerControl::enableContactPower() {
//     m_GPIOHandler->pinWrite(m_contactPowerEnablePin, GPIOHandler::Level::WPI_HIGH);
//     qDebug() << "PowerControl: contact power enabled";
//     return true;
// }

// bool PowerControl::disableContactPower() {
//     m_GPIOHandler->pinWrite(m_contactPowerEnablePin, GPIOHandler::Level::WPI_LOW);
//     qDebug() << "PowerControl: contact power disabled";
//     return true;
// }

void PowerControl::handleReedInterrupt(GPIOHandler::InterruptStatus wfiStatus) {
    reedClosed = (wfiStatus.statusOK == 1) && (wfiStatus.edge == GPIOHandler::Interrupt::WPI_INT_EDGE_RISING);
    qDebug() << "PowerControl: Reed interrupt triggered: " << (reedClosed ? "closed" : "open");
    reedStatusChanged(reedClosed);

    if (!reedClosed) {
        qInfo() << "PowerControl: Capot ouvert, coupure des bobines et contacts";
        disableCoils();
        // disableContactPower();
        safetyStatusChanged(false);
    } else if (reedClosed && boardClosed) {
        qInfo() << "PowerControl: Capot fermé et carte en place, sécurité OK";
        safetyStatusChanged(true);
    }
}

void PowerControl::handleBoardInterrupt(GPIOHandler::InterruptStatus wfiStatus) {
    boardClosed = (wfiStatus.statusOK == 1) && (wfiStatus.edge == GPIOHandler::Interrupt::WPI_INT_EDGE_RISING);
    qDebug() << "PowerControl: Board interrupt triggered: " << (boardClosed ? "closed" : "open");
    boardStatusChanged(boardClosed);

    if (!boardClosed) {
        qInfo() << "PowerControl: Carte mal placée, coupure des bobines et contacts";
        disableCoils();
        // disableContactPower();
    } else if (reedClosed && boardClosed) {
        qInfo() << "PowerControl: Capot fermé et carte en place, sécurité OK";
        safetyStatusChanged(true);
    }
}

bool PowerControl::forceCheckSafetyStatus() {
    bool currentReedStatus = m_GPIOHandler->pinRead(m_reedPin) == GPIOHandler::Level::WPI_HIGH;
    bool currentBoardStatus = m_GPIOHandler->pinRead(m_boardPin) == GPIOHandler::Level::WPI_HIGH;

    if (currentReedStatus != reedClosed) {
        reedClosed = currentReedStatus;
        reedStatusChanged(reedClosed);
    }

    if (currentBoardStatus != boardClosed) {
        boardClosed = currentBoardStatus;
        boardStatusChanged(boardClosed);
    }

    bool isSafe = reedClosed && boardClosed;
    safetyStatusChanged(isSafe);
    return isSafe;
}