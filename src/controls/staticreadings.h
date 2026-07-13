#pragma once

#include "ADCBase.h"
#include "ADCValue.h"
#include <QMap>
#include <cstdint>
#include <memory>
#include <vector>

class StaticReadings {

public:
    enum class ReadingFlags : uint8_t {
        none = 0x00,
        coil1Voltage = 0x01,
        coil1Current = 0x02,
        coil2Voltage = 0x04,
        coil2Current = 0x08,
        contact12Voltage = 0x10,
        contact13Voltage = 0x20,
        contactCurrent = 0x40,

        // easy flags
        coils = coil1Voltage | coil1Current | coil2Voltage | coil2Current,
        contacts = contact12Voltage | contact13Voltage | contactCurrent,
        all = coils | contacts
    };

    static StaticReadings* getInstance();
    static StaticReadings* initialize();

    bool getReading(ReadingFlags type, std::shared_ptr<ADCValue> reading, ADCBase::ADCCaliber caliber = ADCBase::Caliber_Auto);
    int getNReadings(ReadingFlags type, int nReadings, ADCValue readings[], ADCBase::ADCCaliber caliber = ADCBase::Caliber_Auto);

    QMap<ReadingFlags, std::vector<ADCValue>> getMultipleReadings(uint8_t types, int nReadings);

private:
    static StaticReadings* s_instance;

    StaticReadings();
    ~StaticReadings() = default;

    bool checkOpen() const;
    void configureValueForChannel(ReadingFlags type, std::shared_ptr<ADCValue> reading, ADCBase::ADCCaliber caliber);

    ADCBase::ADCCaliber selectCaliberForChannel(ReadingFlags type, std::shared_ptr<ADCValue> reading);

    std::unique_ptr<ADCBase> m_adc;
};