#include "dynamicswitch.h"

int DynamicSwitch::getContactAStableSwitchTime_us() {
    return contactALatestStatus.timeStamp_us - coilStatus.timeStamp_us;
}

int DynamicSwitch::getContactAWorkSwitchTime_us() {
    return contactAEarliestStatus.timeStamp_us - coilStatus.timeStamp_us;
}

int DynamicSwitch::getContactAReboundTime_us() {
    return contactALatestStatus.timeStamp_us - contactAEarliestStatus.timeStamp_us;
}

int DynamicSwitch::getContactATransistionType() {
    return contactALatestStatus.edge;
}

int DynamicSwitch::getContactBStableSwitchTime_us() {
    return contactBLatestStatus.timeStamp_us - coilStatus.timeStamp_us;
}

int DynamicSwitch::getContactBWorkSwitchTime_us() {
    return contactBEarliestStatus.timeStamp_us - coilStatus.timeStamp_us;
}

int DynamicSwitch::getContactBReboundTime_us() {
    return contactBLatestStatus.timeStamp_us - contactBEarliestStatus.timeStamp_us;
}

int DynamicSwitch::getContactBTransistionType() {
    return contactBLatestStatus.edge;
}
