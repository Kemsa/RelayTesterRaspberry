#ifndef DYNAMICSWITCH_H
#define DYNAMICSWITCH_H

#include "dynamicreadings.h"
class DynamicSwitch {
public:
    DynamicSwitch(DynamicReadings::ContactType coilSwitch, GPIOHandler::InterruptStatus coilStatus,
                  GPIOHandler::InterruptStatus contactAEarliestStatus, GPIOHandler::InterruptStatus contactALatestStatus,
                  GPIOHandler::InterruptStatus contactBEarliestStatus, GPIOHandler::InterruptStatus contactBLatestStatus)
        : m_coilSwitch(coilSwitch), coilStatus(coilStatus), contactALatestStatus(contactALatestStatus), contactBLatestStatus(contactBLatestStatus),
          contactAEarliestStatus(contactAEarliestStatus), contactBEarliestStatus(contactBEarliestStatus) {};

    const GPIOHandler::InterruptStatus getCoilStatus() const { return coilStatus; }
    const GPIOHandler::InterruptStatus getContactALatestStatus() const { return contactALatestStatus; }
    const GPIOHandler::InterruptStatus getContactAEarliestStatus() const { return contactAEarliestStatus; }
    const GPIOHandler::InterruptStatus getContactBLatestStatus() const { return contactBLatestStatus; }
    const GPIOHandler::InterruptStatus getContactBEarliestStatus() const { return contactBEarliestStatus; }
    const DynamicReadings::ContactType getCoilSwitch() const { return m_coilSwitch; }

    bool isValid() const {
        return coilStatus.statusOK != -1 && (contactALatestStatus.statusOK != -1 || contactBLatestStatus.statusOK != -1);
    }

    int getContactAStableSwitchTime_us();
    int getContactAWorkSwitchTime_us();
    int getContactAReboundTime_us();
    int getContactATransistionType();

    int getContactBStableSwitchTime_us();
    int getContactBWorkSwitchTime_us();
    int getContactBReboundTime_us();
    int getContactBTransistionType();

private:
    const DynamicReadings::ContactType m_coilSwitch;
    const GPIOHandler::InterruptStatus coilStatus;
    const GPIOHandler::InterruptStatus contactALatestStatus;
    const GPIOHandler::InterruptStatus contactBLatestStatus;
    const GPIOHandler::InterruptStatus contactAEarliestStatus;
    const GPIOHandler::InterruptStatus contactBEarliestStatus;
};

#endif // DYNAMICSWITCH_H
