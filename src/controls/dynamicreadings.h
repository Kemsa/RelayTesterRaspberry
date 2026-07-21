#ifndef DYNAMICREADINGS_H
#define DYNAMICREADINGS_H

#include "GPIOHandler.h"
#include <QList>
#include <QMutex>
#include <cstdint>
#include <future>
#include <memory>

class DynamicSwitch;

class DynamicReadings {
public:
    enum class ContactType {
        COIL1,
        COIL2,
        CONTACT1,
        CONTACT2,
        BOTH_COILS,
    };

    static DynamicReadings* getInstance();
    static DynamicReadings* initialize(int coil1Pin, int coil2Pin, int contact1Pin, int contact2Pin);

    void clearInterrupts();
    std::future<std::shared_ptr<DynamicSwitch>> waitAndProcessOneSwitch(ContactType triggerCoil, int timeoutMs = 500);

private:
    static DynamicReadings* s_instance;

    DynamicReadings(int coil1Pin, int coil2Pin, int contact1Pin, int contact2Pin);

    int m_coil1Pin;
    int m_coil2Pin;
    int m_contact1Pin;
    int m_contact2Pin;

    GPIOHandler* m_GPIOHandler = GPIOHandler::instance();

    QMutex m_mutexWrite;
    QMutex m_mutexRead;
    QList<GPIOHandler::InterruptStatus> m_interruptStatusesCoil1;
    QList<GPIOHandler::InterruptStatus> m_interruptStatusesCoil2;
    QList<GPIOHandler::InterruptStatus> m_interruptStatusesContact1;
    QList<GPIOHandler::InterruptStatus> m_interruptStatusesContact2;

    void interruptHandler(ContactType type, GPIOHandler::InterruptStatus wfiStatus);
    static void staticInterruptHandler(GPIOHandler::InterruptStatus wfiStatus, void* userdata) {
        DynamicReadings* instance = DynamicReadings::getInstance();
        const auto type = static_cast<ContactType>(reinterpret_cast<std::intptr_t>(userdata));
        instance->interruptHandler(type, wfiStatus);
    }

    GPIOHandler::InterruptStatus getEarlierstInterruptStatus(ContactType type);
    GPIOHandler::InterruptStatus getLatestInterruptStatus(ContactType type);
};

#endif // DYNAMICREADINGS_H
