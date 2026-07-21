#include "staticreadings.h"
#include "ADC24.h"
#include "config.h"
#include <QDebug>
#include <memory>
#include <vector>

StaticReadings* StaticReadings::s_instance = nullptr;

StaticReadings::StaticReadings() {
    m_adc = std::unique_ptr<ADCBase>(new ADC24());
}

StaticReadings* StaticReadings::getInstance() {
    if (!s_instance) {
        s_instance = new StaticReadings();
    }
    return s_instance;
}

StaticReadings* StaticReadings::initialize() {
    StaticReadings* instance = getInstance();
    instance->checkOpen();

    return instance;
}

bool StaticReadings::checkOpen() const {
    bool res = m_adc->isOpen();

    // if ADC not open, try to open it
    if (!res) {
        m_adc->open();
        res = m_adc->isOpen();
        if (!res) {
            qCritical() << "Readings: Failed to open ADC device";
        }
    }

    return res;
}

bool StaticReadings::getReading(ReadingFlags type, std::shared_ptr<ADCValue> reading, ADCBase::ADCCaliber caliber) {
    if (!checkOpen()) {
        return false;
    }

    if (reading == nullptr) {
        qWarning() << "Readings: Reading pointer is null";
        return false;
    }

    if (caliber == ADCBase::Caliber_Auto) {
        caliber = selectCaliberForChannel(type, reading);

        if (caliber < 0 || caliber >= ADCBase::Caliber_Max) {
            qWarning() << "Readings: Invalid caliber selected for channel";
            return false;
        }
    }

    configureValueForChannel(type, reading, caliber);

    if (!m_adc->getSingleValue(reading.get())) {
        qWarning() << "Readings: Failed to get reading for channel";
        return false;
    }

    return true;
}

int StaticReadings::getNReadings(ReadingFlags type, int nReadings, ADCValue readings[], ADCBase::ADCCaliber caliber) {
    if (!checkOpen()) {
        return 0;
    }

    if (nReadings <= 0 || readings == nullptr) {
        qWarning() << "Readings: Invalid number of readings or readings array is null";
        return 0;
    }

    int successfulReadings = 0;

    if (caliber == ADCBase::Caliber_Auto) {
        caliber = selectCaliberForChannel(type, std::make_shared<ADCValue>());
        if (caliber < 0 || caliber >= ADCBase::Caliber_Max) {
            qWarning() << "Readings: Invalid caliber selected for channel";
            return 0;
        }
    }

    for (int i = 0; i < nReadings; ++i) {
        std::shared_ptr<ADCValue> reading = std::make_shared<ADCValue>();
        if (getReading(type, reading, caliber)) {
            readings[i] = *reading;
            ++successfulReadings;
        }
    }

    return successfulReadings;
}

QMap<StaticReadings::ReadingFlags, std::vector<ADCValue>> StaticReadings::getMultipleReadings(uint8_t types, int nReadings) {
    if (!checkOpen()) {
        return QMap<ReadingFlags, std::vector<ADCValue>>();
    }

    QMap<ReadingFlags, std::vector<ADCValue>> result;

    // make readings for each selected channel. Uses bitmap system
    for (uint8_t i = 0; i < 7; ++i) {
        ReadingFlags flag = static_cast<ReadingFlags>(1 << i);
        // channel is active if the corresponding bit is set in types
        if (types & (1 << i)) {
            std::vector<ADCValue> channelReadings(nReadings);
            int successfulReadings = getNReadings(flag, nReadings, channelReadings.data(), ADCBase::Caliber_Auto);
            if (successfulReadings > 0) {
                channelReadings.resize(successfulReadings);
                result.insert(flag, channelReadings);
            }
        }
    }

    return result;
}

ADCBase::ADCCaliber StaticReadings::selectCaliberForChannel(ReadingFlags type, std::shared_ptr<ADCValue> reading) {

    std::shared_ptr<ADCValue> tempValue = std::make_shared<ADCValue>();
    configureValueForChannel(type, tempValue, ADCBase::Caliber_2500mV);
    if (!m_adc->getSingleValue(tempValue.get())) {
        return ADCBase::Caliber_error;
    }

    if (reading != nullptr) {
        *reading = *tempValue;
    }

    if (tempValue->getMillivolts() > 1250.0 * 1.1) { // 10% margin
        return ADCBase::Caliber_2500mV;
    } else if (tempValue->getMillivolts() > 625.0 * 1.1) {
        return ADCBase::Caliber_1250mV;
    } else if (tempValue->getMillivolts() > 313.0 * 1.1) {
        return ADCBase::Caliber_625mV;
    } else if (tempValue->getMillivolts() > 156.0 * 1.1) {
        return ADCBase::Caliber_313mV;
    } else if (tempValue->getMillivolts() > 78.0 * 1.1) {
        return ADCBase::Caliber_156mV;
    } else if (tempValue->getMillivolts() > 39.0 * 1.1) {
        return ADCBase::Caliber_78mV;
    } else {
        return ADCBase::Caliber_39mV;
    }
}

void StaticReadings::configureValueForChannel(ReadingFlags type, std::shared_ptr<ADCValue> reading, ADCBase::ADCCaliber caliber) {

    switch (type) {
    case ReadingFlags::coil1Voltage:
        reading->setConfiguration(ADC24_COIL1_VOLTAGE_CHANNEL, static_cast<HRDL_RANGE>(caliber), HRDL_60MS, ADC24_COIL1_VOLTAGE_CHANNEL_IS_SINGLE_ENDED);
        break;
    case ReadingFlags::coil1Current:
        reading->setConfiguration(ADC24_COIL1_CURRENT_CHANNEL, static_cast<HRDL_RANGE>(caliber), HRDL_60MS, ADC24_COIL1_CURRENT_CHANNEL_IS_SINGLE_ENDED);
        break;
    case ReadingFlags::coil2Voltage:
        reading->setConfiguration(ADC24_COIL2_VOLTAGE_CHANNEL, static_cast<HRDL_RANGE>(caliber), HRDL_60MS, ADC24_COIL2_VOLTAGE_CHANNEL_IS_SINGLE_ENDED);
        break;
    case ReadingFlags::coil2Current:
        reading->setConfiguration(ADC24_COIL2_CURRENT_CHANNEL, static_cast<HRDL_RANGE>(caliber), HRDL_60MS, ADC24_COIL2_CURRENT_CHANNEL_IS_SINGLE_ENDED);
        break;
    case ReadingFlags::contactAVoltage:
        reading->setConfiguration(ADC24_CONTACT12_VOLTAGE_CHANNEL, static_cast<HRDL_RANGE>(caliber), HRDL_60MS, ADC24_CONTACT12_VOLTAGE_CHANNEL_IS_SINGLE_ENDED);
        break;
    case ReadingFlags::contactBVoltage:
        reading->setConfiguration(ADC24_CONTACT13_VOLTAGE_CHANNEL, static_cast<HRDL_RANGE>(caliber), HRDL_60MS, ADC24_CONTACT13_VOLTAGE_CHANNEL_IS_SINGLE_ENDED);
        break;
    case ReadingFlags::contactCurrent:
        reading->setConfiguration(ADC24_CONTACT_CURRENT_CHANNEL, static_cast<HRDL_RANGE>(caliber), HRDL_60MS, ADC24_CONTACT_CURRENT_CHANNEL_IS_SINGLE_ENDED);
        break;
    default:
        break;
    }
}