#ifndef POWERCONTROL_H
#define POWERCONTROL_H

#include "GPIOHandler.h"
#include <QObject>

class PowerControl : public QObject {
    Q_OBJECT
public:
    enum Coil {
        COIL1,
        COIL2
    };

    static PowerControl* getInstance();
    static PowerControl* initialize(int coil1Pin, int coil2Pin, int contactPowerEnablePin,
                                    int reedPin, int boardPin);
    bool checkSafetyStatus() const { return reedClosed && boardClosed; }
    bool checkReedStatus() const { return reedClosed; }
    bool checkBoardStatus() const { return boardClosed; }
    bool forceCheckSafetyStatus();

public slots:
    bool enableCoil(Coil coil);
    bool disableCoils();
    // bool enableContactPower();
    // bool disableContactPower();

signals:
    void safetyStatusChanged(bool isSafe);
    void reedStatusChanged(bool isClosed);
    void boardStatusChanged(bool isClosed);

private:
    static PowerControl* s_instance;

    PowerControl(int coil1Pin, int coil2Pin, int contactPowerEnablePin,
                 int reedPin, int boardPin);

    int m_coil1Pin;
    int m_coil2Pin;
    int m_contactPowerEnablePin;
    int m_reedPin;
    int m_boardPin;

    bool reedClosed = false;
    bool boardClosed = false;

    GPIOHandler* m_GPIOHandler = GPIOHandler::instance();

    void handleReedInterrupt(GPIOHandler::InterruptStatus wfiStatus);
    void handleBoardInterrupt(GPIOHandler::InterruptStatus wfiStatus);

    static void staticReedInterrupt(GPIOHandler::InterruptStatus wfiStatus, void* userdata) {
        PowerControl* instance = PowerControl::getInstance();
        instance->handleReedInterrupt(wfiStatus);
    }
    static void staticBoardInterrupt(GPIOHandler::InterruptStatus wfiStatus, void* userdata) {
        PowerControl* instance = PowerControl::getInstance();
        instance->handleBoardInterrupt(wfiStatus);
    }
};

#endif // POWERCONTROL_H
