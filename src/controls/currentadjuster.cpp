#include "currentadjuster.h"
#include <QDebug>
#include <cmath>

CurrentAdjuster* CurrentAdjuster::s_instance = nullptr;

CurrentAdjuster& CurrentAdjuster::initialize(StaticReadings* readings, int resistanceAddress) {
    if (!s_instance) {
        s_instance = new CurrentAdjuster(readings, resistanceAddress);
    }
    return *s_instance;
}

CurrentAdjuster& CurrentAdjuster::instance() {
    Q_ASSERT_X(s_instance, "CurrentAdjuster::instance", "CurrentAdjuster is not initialized");
    return *s_instance;
}

CurrentAdjuster::CurrentAdjuster(StaticReadings* readings, int resistanceAddress)
    : m_readings(readings), m_varResistorAddress(resistanceAddress) {

    m_varResistorDevice = GPIOHandler::instance()->createI2CDevice(m_varResistorAddress);
}

bool CurrentAdjuster::setWiper(uint16_t value) {
    if (value > MCP4551_WIPER_A) {
        return false;
    }

    uint16_t command = (MCP4551_CMD_WRITE << 8) | (value & 0x01FF);
    m_varResistorDevice->writeWord(command);
    return true;
}

bool CurrentAdjuster::incWiper(void) {
    uint8_t command = MCP4551_CMD_INC;
    m_varResistorDevice->writeByte(command);
    return true;
}

bool CurrentAdjuster::decWiper(void) {
    uint8_t command = MCP4551_CMD_DEC;
    m_varResistorDevice->writeByte(command);
    return true;
}

int16_t CurrentAdjuster::getWiper(void) {
    uint8_t command = MCP4551_CMD_READ;
    m_varResistorDevice->writeByte(command);
    int16_t value = m_varResistorDevice->readWord();
    if (value < 0) {
        return -1; // Error
    }
    return value & 0x01FF; // Return only the 9-bit wiper value
}

// To be debugged
int CurrentAdjuster::adjustCurrentToTarget(float targetCurrent, float tolerance) {
    if (!m_readings) {
        qCritical() << "CurrentAdjuster: StaticReadings instance is not set.";
        return -1; // Error
    }

    int currentWiper = getWiper();
    if (currentWiper < 0) {
        qCritical() << "CurrentAdjuster: Failed to read wiper value.";
        return -1; // Error
    }

    std::shared_ptr<ADCValue> reading = std::make_shared<ADCValue>();
    if (!m_readings->getReading(StaticReadings::ReadingFlags::contactCurrent, reading)) {
        qWarning() << "CurrentAdjuster: Failed to read current.";
        return -1; // Error
    }

    float current = reading->value;
    const int stepSizes[] = {100, 10, 1};

    for (int stepSize : stepSizes) {
        while (std::abs(current - targetCurrent) > tolerance) {
            // Possible wiper works in reverse => increase wiper reduces current
            const bool increaseWiper = current < targetCurrent;
            const int nextWiper = currentWiper + (increaseWiper ? stepSize : -stepSize);

            if (nextWiper < MCP4551_WIPER_B || nextWiper > MCP4551_WIPER_A) {
                break;
            }

            if (!setWiper(static_cast<uint16_t>(nextWiper))) {
                qWarning() << "CurrentAdjuster: Failed to set wiper.";
                return -1; // Error
            }

            currentWiper = nextWiper;
            m_varResistorWiper = currentWiper;

            if (!m_readings->getReading(StaticReadings::ReadingFlags::contactCurrent, reading)) {
                qWarning() << "CurrentAdjuster: Failed to read current after adjustment.";
                return -1; // Error
            }

            // set mA converted value when ready
            current = reading->value;

            if ((increaseWiper && current >= targetCurrent) || (!increaseWiper && current <= targetCurrent)) {
                break;
            }
        }

        if (std::abs(current - targetCurrent) <= tolerance) {
            break;
        }
    }

    m_varResistorWiper = currentWiper;
    return currentWiper; // Return the final wiper value
}