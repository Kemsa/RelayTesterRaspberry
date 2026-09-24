#ifndef POWERCONTROL_H
#define POWERCONTROL_H

#include "GPIOHandler.h"
#include <QObject>

class PowerControl : public QObject {
    Q_OBJECT
public:
    enum Coil {
        COIL_NONE = 0,
        COIL1 = 1,
        COIL2 = 2
    };

    static PowerControl* getInstance();
    static PowerControl* initialize(int coil1PinBottom, int coil2PinBottom, int coil1PinTop, int coil2PinTop, int contactPowerEnablePin,
                                    int reedPin, int boardPin);
    bool checkSafetyStatus() const { return reedClosed && boardClosed; }
    bool checkReedStatus() const { return reedClosed; }
    bool checkBoardStatus() const { return boardClosed; }
    bool forceCheckSafetyStatus();

public slots:
    bool enableCoilBottom(Coil coil);
    bool disableCoilsBottom();
    bool enableCoilTop(Coil coil);
    bool disableCoilsTop();
    bool enableContactPower();
    bool disableContactPower();

signals:
    void safetyStatusChanged(bool isSafe);
    void reedStatusChanged(bool isClosed);
    void boardStatusChanged(bool isClosed);

private:
    static PowerControl* s_instance;

    PowerControl(int coil1PinBottom, int coil2PinBottom, int coil1PinTop, int coil2PinTop, int contactPowerEnablePin,
                 int reedPin, int boardPin);

    int m_coil1PinBottom;
    int m_coil2PinBottom;
    int m_coil1PinTop;
    int m_coil2PinTop;
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
