#ifndef COILCONTROL_H
#define COILCONTROL_H

#include "GPIOHandler.h"

class CoilControl {
public:
    enum Coil {
        COIL1,
        COIL2
    };

    static CoilControl* getInstance();
    static CoilControl* initialize(int coil1Pin, int coil2Pin);

    bool enableCoil(Coil coil);
    bool disableCoils();

private:
    static CoilControl* s_instance;

    CoilControl(int coil1Pin, int coil2Pin);

    int m_coil1Pin;
    int m_coil2Pin;
    GPIOHandler* m_GPIOHandler = GPIOHandler::instance();
};

#endif // COILCONTROL_H
