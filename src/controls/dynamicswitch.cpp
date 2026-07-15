#include "dynamicswitch.h"

int DynamicSwitch::getContact1SwitchTime() {
    return contact1Status.timeStamp_us - coilStatus.timeStamp_us;
}

int DynamicSwitch::getContact1TransistionType() {
    return contact1Status.edge;
}

int DynamicSwitch::getContact2SwitchTime() {
    return contact2Status.timeStamp_us - coilStatus.timeStamp_us;
}

int DynamicSwitch::getContact2TransistionType() {
    return contact2Status.edge;
}
