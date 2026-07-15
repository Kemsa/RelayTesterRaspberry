#include "dynamicreadings.h"
#include "dynamicswitch.h"
#include <QDebug>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <thread>

DynamicReadings* DynamicReadings::s_instance = nullptr;

DynamicReadings::DynamicReadings(int coil1Pin, int coil2Pin, int contact1Pin, int contact2Pin)
    : m_coil1Pin(coil1Pin), m_coil2Pin(coil2Pin), m_contact1Pin(contact1Pin), m_contact2Pin(contact2Pin), m_mutexWrite() {

    m_GPIOHandler->setPinMode(m_coil1Pin, GPIOHandler::PinMode::WPI_INPUT);
    m_GPIOHandler->setPinMode(m_coil2Pin, GPIOHandler::PinMode::WPI_INPUT);
    m_GPIOHandler->setPinMode(m_contact1Pin, GPIOHandler::PinMode::WPI_INPUT);
    m_GPIOHandler->setPinMode(m_contact2Pin, GPIOHandler::PinMode::WPI_INPUT);

    m_GPIOHandler->setupInterrupt(m_coil1Pin, GPIOHandler::Interrupt::WPI_INT_EDGE_BOTH, staticInterruptHandler, 0, reinterpret_cast<void*>(static_cast<std::intptr_t>(ContactType::COIL1)));
    m_GPIOHandler->setupInterrupt(m_coil2Pin, GPIOHandler::Interrupt::WPI_INT_EDGE_BOTH, staticInterruptHandler, 0, reinterpret_cast<void*>(static_cast<std::intptr_t>(ContactType::COIL2)));
    m_GPIOHandler->setupInterrupt(m_contact1Pin, GPIOHandler::Interrupt::WPI_INT_EDGE_BOTH, staticInterruptHandler, 0, reinterpret_cast<void*>(static_cast<std::intptr_t>(ContactType::CONTACT1)));
    m_GPIOHandler->setupInterrupt(m_contact2Pin, GPIOHandler::Interrupt::WPI_INT_EDGE_BOTH, staticInterruptHandler, 0, reinterpret_cast<void*>(static_cast<std::intptr_t>(ContactType::CONTACT2)));
}

DynamicReadings* DynamicReadings::getInstance() {
    if (!s_instance) {
        throw std::runtime_error("DynamicReadings instance not initialized. Call initialize() first.");
    }
    return s_instance;
}

DynamicReadings* DynamicReadings::initialize(int coil1Pin, int coil2Pin, int contact1Pin, int contact2Pin) {
    if (!s_instance) {
        s_instance = new DynamicReadings(coil1Pin, coil2Pin, contact1Pin, contact2Pin);
    }
    return s_instance;
}

void DynamicReadings::interruptHandler(ContactType type, GPIOHandler::InterruptStatus wfiStatus) {
    // Handle the interrupt based on the contact type and status

    if (!m_mutexWrite.tryLock(100)) {
        qCritical() << "DynamicReadings: Failed to acquire mutex in interrupt handler";
        return;
    }

    switch (type) {
    case ContactType::COIL1:
        m_interruptStatusesCoil1.append(wfiStatus);
        break;
    case ContactType::COIL2:
        m_interruptStatusesCoil2.append(wfiStatus);
        break;
    case ContactType::CONTACT1:
        m_interruptStatusesContact1.append(wfiStatus);
        break;
    case ContactType::CONTACT2:
        m_interruptStatusesContact2.append(wfiStatus);
        break;
    default:
        qWarning() << "DynamicReadings: Unknown contact type in interrupt handler";
        break;
    }
    m_mutexWrite.unlock();
}

void DynamicReadings::clearInterrupts() {
    if (!m_mutexWrite.tryLock(100)) {
        qCritical() << "DynamicReadings: Failed to acquire mutex in clearInterrupts";
        return;
    }

    m_interruptStatusesCoil1.clear();
    m_interruptStatusesCoil2.clear();
    m_interruptStatusesContact1.clear();
    m_interruptStatusesContact2.clear();

    m_mutexWrite.unlock();
}

std::future<std::shared_ptr<DynamicSwitch>> DynamicReadings::waitAndProcessOneSwitch(ContactType triggerCoil, int timeoutMs) {
    auto promise = std::make_shared<std::promise<std::shared_ptr<DynamicSwitch>>>();
    std::future<std::shared_ptr<DynamicSwitch>> result = promise->get_future();

    std::thread([this, triggerCoil, timeoutMs, promise]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeoutMs));
        if (!m_mutexRead.tryLock(100)) {
            qCritical() << "DynamicReadings: Failed to acquire mutex in waitAndProcessOneSwitch";
            promise->set_value(nullptr);
            return;
        }

        GPIOHandler::InterruptStatus statusCoil = getEarlierstInterruptStatus(triggerCoil);
        GPIOHandler::InterruptStatus statusContact1 = getLatestInterruptStatus(ContactType::CONTACT1);
        GPIOHandler::InterruptStatus statusContact2 = getLatestInterruptStatus(ContactType::CONTACT2);

        auto switchResult = std::make_shared<DynamicSwitch>(triggerCoil, statusCoil, statusContact1, statusContact2);

        m_mutexRead.unlock();
        promise->set_value(switchResult);
    }).detach();

    return result;
}

GPIOHandler::InterruptStatus DynamicReadings::getEarlierstInterruptStatus(ContactType type) {
    const QList<GPIOHandler::InterruptStatus>* interruptList = nullptr;

    switch (type) {
    case ContactType::COIL1:
        interruptList = &m_interruptStatusesCoil1;
        break;
    case ContactType::COIL2:
        interruptList = &m_interruptStatusesCoil2;
        break;
    case ContactType::CONTACT1:
        interruptList = &m_interruptStatusesContact1;
        break;
    case ContactType::CONTACT2:
        interruptList = &m_interruptStatusesContact2;
        break;
    }

    if (interruptList && !interruptList->isEmpty()) {
        int earliestIdx = 0;
        long long earliestTs = interruptList->first().timeStamp_us;
        for (int i = 1; i < interruptList->size(); ++i) {
            const long long ts = interruptList->at(i).timeStamp_us;
            if (ts < earliestTs) {
                earliestTs = ts;
                earliestIdx = i;
            }
        }
        return interruptList->at(earliestIdx);
    }

    return {-1, 0, 0, 0}; // Default status if no interrupt
}

GPIOHandler::InterruptStatus DynamicReadings::getLatestInterruptStatus(ContactType type) {
    const QList<GPIOHandler::InterruptStatus>* interruptList = nullptr;

    switch (type) {
    case ContactType::COIL1:
        interruptList = &m_interruptStatusesCoil1;
        break;
    case ContactType::COIL2:
        interruptList = &m_interruptStatusesCoil2;
        break;
    case ContactType::CONTACT1:
        interruptList = &m_interruptStatusesContact1;
        break;
    case ContactType::CONTACT2:
        interruptList = &m_interruptStatusesContact2;
        break;
    }

    if (interruptList && !interruptList->isEmpty()) {
        int latestIdx = 0;
        long long latestTs = interruptList->first().timeStamp_us;
        for (int i = 1; i < interruptList->size(); ++i) {
            const long long ts = interruptList->at(i).timeStamp_us;
            if (ts > latestTs) {
                latestTs = ts;
                latestIdx = i;
            }
        }
        return interruptList->at(latestIdx);
    }

    return {-1, 0, 0, 0}; // Default status if no interrupt
}