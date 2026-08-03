#include "currentadjuster.h"
#include <QDebug>
#include <cmath>
#include <QThread>

#define DELAY_AFTER_COMMAND_MS 1

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

bool CurrentAdjuster::isInitialized() {
    return s_instance != nullptr;
}

CurrentAdjuster::CurrentAdjuster(StaticReadings* readings, int resistanceAddress)
    : m_readings(readings), m_varResistorAddress(resistanceAddress) {

    m_varResistorDevice = GPIOHandler::instance()->createI2CDevice(m_varResistorAddress);
}

void CurrentAdjuster::closeConnection() {
    if (!m_varResistorDevice) {
        return;
    }

    qInfo() << "CurrentAdjuster: Closing I2C connection.";
    m_varResistorDevice->close();
    m_varResistorDevice.reset();
}

bool CurrentAdjuster::setWiper(uint16_t value) {
    //Never go beyond these values, it blocks the wiper and the device will not respond to any commands until reset
    if (value >= MCP4551_WIPER_A || value <= MCP4551_WIPER_B) {
        return false;
    }

    uint16_t command = (MCP4551_CMD_WRITE << 8) | (value & 0x01FF);
    m_varResistorDevice->writeWord(command);
    QThread::msleep(DELAY_AFTER_COMMAND_MS); // Allow time for the wiper to move
    return true;
}

bool CurrentAdjuster::incWiper(void) {
    uint8_t command = MCP4551_CMD_INC;
    m_varResistorDevice->writeByte(command);
    QThread::msleep(DELAY_AFTER_COMMAND_MS);
    return true;
}

bool CurrentAdjuster::decWiper(void) {
    uint8_t command = MCP4551_CMD_DEC;
    m_varResistorDevice->writeByte(command);
    QThread::msleep(DELAY_AFTER_COMMAND_MS);
    return true;
}

int16_t CurrentAdjuster::getWiper(void) {
    uint8_t command = MCP4551_CMD_READ;
    m_varResistorDevice->writeByte(command);
    QThread::msleep(DELAY_AFTER_COMMAND_MS);
    int16_t value = m_varResistorDevice->readWord();
    QThread::msleep(DELAY_AFTER_COMMAND_MS);
    if (value < 0) {
        qDebug() << "CurrentAdjuster: Failed to read wiper value from device." << "Error code:" << value;
        return -1; // Error
    }
    return value & 0x01FF; // Return only the 9-bit wiper value
}

int CurrentAdjuster::adjustCurrentToTarget(float targetCurrent, float tolerance) {
    if (!m_readings) {
        qCritical() << "CurrentAdjuster: StaticReadings instance is not set.";
        return -1; // Error
    }

    int currentWiper;
    // int currentWiper = getWiper();
    // if (currentWiper < 0) {
    //     qCritical() << "CurrentAdjuster: Failed to read wiper value: " << currentWiper;
    //     return -1; // Error
    // }

    std::shared_ptr<ADCValue> reading = std::make_shared<ADCValue>();
    if (!m_readings->getReading(StaticReadings::ReadingFlags::contactCurrent, reading)) {
        qWarning() << "CurrentAdjuster: Failed to read current.";
        return -1; // Error
    }

    setWiper(MCP4551_WIPER_MID); // Set to mid-point for initial reading
    currentWiper = MCP4551_WIPER_MID;
    m_readings->getReading(StaticReadings::ReadingFlags::contactCurrent, reading);
    qDebug() << "CurrentAdjuster: Initial wiper set to mid-point. Current reading:" << reading->value
     << "|" << StaticReadings::toContactCurrent(*reading) << "mA";


    float current = StaticReadings::toContactCurrent(*reading);
    const int stepSizes[] = {100, 10, 1};

    for (int stepSize : stepSizes) {
        while (std::abs(current - targetCurrent) > tolerance) {

            const bool increaseWiper = current < targetCurrent;
            const int nextWiper = currentWiper + (increaseWiper ? stepSize : -stepSize);

            if (nextWiper <= MCP4551_WIPER_B || nextWiper >= MCP4551_WIPER_A) {
                qDebug() << "CurrentAdjuster: Wiper adjustment out of bounds. Current wiper:" << currentWiper << "Next wiper:" << nextWiper;
                break;
            }

            qDebug() << "CurrentAdjuster: Adjusting wiper from" << currentWiper << "to" << nextWiper << "with step size" << stepSize;
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
            qDebug() << "CurrentAdjuster: Current reading after adjustment:" << reading->value << "|" << StaticReadings::toContactCurrent(*reading) << "mA";

            // set mA converted value when ready
            current = StaticReadings::toContactCurrent(*reading);

            if ((increaseWiper && current >= targetCurrent) || (!increaseWiper && current <= targetCurrent)) {
                qDebug() << "CurrentAdjuster: Target current reached or exceeded. Current:" << current << "Target:" << targetCurrent;
                break;
            }
        }

        if (std::abs(current - targetCurrent) <= tolerance) {
            break;
        }
    }
    qDebug() << "CurrentAdjuster: Final wiper value:" << currentWiper << "Final current reading:" << reading->value << "|" << StaticReadings::toContactCurrent(*reading) << "mA";

    m_varResistorWiper = currentWiper;
    return currentWiper; // Return the final wiper value
}