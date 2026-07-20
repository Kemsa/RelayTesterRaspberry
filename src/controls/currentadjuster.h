#ifndef CURRENTADJUSTER_H
#define CURRENTADJUSTER_H

#include "GPIOHandler.h"
#include "I2CDevice.h"
#include "config.h"
#include "staticreadings.h"
#include <memory>

// Command definitions (sent to WIPER register)
#define MCP4551_CMD_WRITE 0x00
#define MCP4551_CMD_INC 0x04
#define MCP4551_CMD_DEC 0x08
#define MCP4551_CMD_READ 0x0C

// Control bit definitions (sent to TCON register)
#define MCP4551_TCON_GC_EN 0x100
#define MCP4551_TCON_R0_EN 0x008
#define MCP4551_TCON_A_EN 0x004
#define MCP4551_TCON_W_EN 0x002
#define MCP4551_TCON_B_EN 0x001
#define MCP4551_TCON_ALL_EN 0x1FF

// Register addresses
#define MCP4551_RA_WIPER 0x00
#define MCP4551_RA_TCON 0x40

// Common WIPER values
#define MCP4551_WIPER_MID 0x080
#define MCP4551_WIPER_A 0x100
#define MCP4551_WIPER_B 0x000

class CurrentAdjuster {
public:
    static CurrentAdjuster& initialize(StaticReadings* readings, int resistanceAddress = VAR_RESISTANCE_ADDRESS);
    static CurrentAdjuster& instance();

    CurrentAdjuster(const CurrentAdjuster&) = delete;
    CurrentAdjuster& operator=(const CurrentAdjuster&) = delete;
    CurrentAdjuster(CurrentAdjuster&&) = delete;
    CurrentAdjuster& operator=(CurrentAdjuster&&) = delete;

    int adjustCurrentToTarget(float targetCurrent, float tolerance);

private:
    CurrentAdjuster(StaticReadings* readings, int resistanceAddress = VAR_RESISTANCE_ADDRESS);

    // Write the Wiper register
    bool setWiper(uint16_t value); // returns true if no errors

    // Increments the Wiper register
    bool incWiper(void); // increments the wiper register - stops at 0x100

    // Decrements the Wiper register
    bool decWiper(void); // decrements the wiper register - stops at 0x000

    // Read the Wiper register
    int16_t getWiper(void); // returns -1 if errors

    static CurrentAdjuster* s_instance;

    StaticReadings* m_readings = nullptr;
    std::unique_ptr<I2CDevice> m_varResistorDevice;
    int m_varResistorAddress = VAR_RESISTANCE_ADDRESS;

    int m_varResistorWiper = 0;
};

#endif // CURRENTADJUSTER_H
