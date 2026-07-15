#ifndef DYNAMICSWITCH_H
#define DYNAMICSWITCH_H

#include "dynamicreadings.h"
class DynamicSwitch {
public:
    DynamicSwitch(DynamicReadings::ContactType coilSwitch, GPIOHandler::InterruptStatus coilStatus, GPIOHandler::InterruptStatus contact1Status, GPIOHandler::InterruptStatus contact2Status)
        : m_coilSwitch(coilSwitch), coilStatus(coilStatus), contact1Status(contact1Status), contact2Status(contact2Status) {};

    const GPIOHandler::InterruptStatus getCoilStatus() const { return coilStatus; }
    const GPIOHandler::InterruptStatus getContact1Status() const { return contact1Status; }
    const GPIOHandler::InterruptStatus getContact2Status() const { return contact2Status; }
    const DynamicReadings::ContactType getCoilSwitch() const { return m_coilSwitch; }

    bool isValid() const {
        return coilStatus.statusOK != -1 && (contact1Status.statusOK != -1 || contact2Status.statusOK != -1);
    }

    int getContact1SwitchTime();
    int getContact1TransistionType();

    int getContact2SwitchTime();
    int getContact2TransistionType();

private:
    const DynamicReadings::ContactType m_coilSwitch;
    const GPIOHandler::InterruptStatus coilStatus;
    const GPIOHandler::InterruptStatus contact1Status;
    const GPIOHandler::InterruptStatus contact2Status;
};

#endif // DYNAMICSWITCH_H
