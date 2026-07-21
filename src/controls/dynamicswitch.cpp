#include "dynamicswitch.h"

int DynamicSwitch::getContactASwitchTime() {
    return contact1Status.timeStamp_us - coilStatus.timeStamp_us;
}

int DynamicSwitch::getContactATransistionType() {
    return contact1Status.edge;
}

int DynamicSwitch::getContactBSwitchTime() {
    return contact2Status.timeStamp_us - coilStatus.timeStamp_us;
}

int DynamicSwitch::getContactBTransistionType() {
    return contact2Status.edge;
}
