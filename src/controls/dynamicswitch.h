#ifndef DYNAMICSWITCH_H
#define DYNAMICSWITCH_H

#include "dynamicreadings.h"
class DynamicSwitch {
public:
    DynamicSwitch(DynamicReadings::ContactType coilSwitch, GPIOHandler::InterruptStatus coilStatus, GPIOHandler::InterruptStatus contact1Status, GPIOHandler::InterruptStatus contact2Status)
        : m_coilSwitch(coilSwitch), coilStatus(coilStatus), contact1Status(contact1Status), contact2Status(contact2Status) {};

    const GPIOHandler::InterruptStatus getCoilStatus() const { return coilStatus; }
    const GPIOHandler::InterruptStatus getContactAStatus() const { return contact1Status; }
    const GPIOHandler::InterruptStatus getContactBStatus() const { return contact2Status; }
    const DynamicReadings::ContactType getCoilSwitch() const { return m_coilSwitch; }

    bool isValid() const {
        return coilStatus.statusOK != -1 && (contact1Status.statusOK != -1 || contact2Status.statusOK != -1);
    }

    int getContactASwitchTime();
    int getContactATransistionType();

    int getContactBSwitchTime();
    int getContactBTransistionType();

private:
    const DynamicReadings::ContactType m_coilSwitch;
    const GPIOHandler::InterruptStatus coilStatus;
    const GPIOHandler::InterruptStatus contact1Status;
    const GPIOHandler::InterruptStatus contact2Status;
};

#endif // DYNAMICSWITCH_H
