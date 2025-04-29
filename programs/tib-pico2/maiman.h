//
// Created by Jeb Bailey on 4/28/25.
//

#ifndef MAIMAN_H
#define MAIMAN_H

#include "nanomodbus.h"
#include <cstdint>

// Register addresses
constexpr uint16_t REG_TEC_TEMPERATURE_MEASURED    = 0x1000;
constexpr uint16_t REG_PCB_TEMPERATURE_MEASURED    = 0x1001;
constexpr uint16_t REG_TEC_TEMPERATURE_VALUE       = 0x1002;
constexpr uint16_t REG_CURRENT_MEASURED            = 0x1003;
constexpr uint16_t REG_CURRENT                     = 0x1004;
constexpr uint16_t REG_VOLTAGE_MEASURED             = 0x1005;
constexpr uint16_t REG_CURRENT_MAX_LIMIT           = 0x1006;
constexpr uint16_t REG_CURRENT_PROTECTION_THRESHOLD= 0x1007;
constexpr uint16_t REG_CURRENT_SET_CALIBRATION     = 0x1008;
constexpr uint16_t REG_NTC_COEFFICIENT              = 0x1009;
constexpr uint16_t REG_TEC_CURRENT_MEASURED         = 0x100A;
constexpr uint16_t REG_TEC_VOLTAGE                  = 0x100B;
constexpr uint16_t REG_SERIAL_NUMBER                = 0x100C;
constexpr uint16_t REG_FREQUENCY                    = 0x100D;
constexpr uint16_t REG_DURATION                     = 0x100E;
constexpr uint16_t REG_STATE_OF_DEVICE_COMMAND      = 0x1010;

// Divider constants
constexpr float DIVIDER_TEMPERATURE                 = 10.0f;
constexpr float DIVIDER_CURRENT                     = 100.0f;
constexpr float DIVIDER_VOLTAGE                     = 100.0f;
constexpr float DIVIDER_DURATION                    = 1000.0f;
constexpr float DIVIDER_FREQUENCY                   = 1000.0f;
constexpr float DIVIDER_NTC_COEFFICIENT              = 1000.0f;

// Device state bitmasks
constexpr uint16_t OPERATION_STATE_STARTED          = 0x0001;
constexpr uint16_t CURRENT_SET_INTERNAL             = 0x0002;
constexpr uint16_t ENABLE_INTERNAL                  = 0x0004;
constexpr uint16_t EXTERNAL_NTC_INTERLOCK_DENIED     = 0x0008;
constexpr uint16_t INTERLOCK_DENIED                  = 0x0010;

// Modbus command values
constexpr uint16_t MODBUS_START_COMMAND_VALUE       = 0x0001;
constexpr uint16_t MODBUS_STOP_COMMAND_VALUE        = 0x0000;



class ModbusInterface {
public:
    virtual ~ModbusInterface() = default;
    virtual bool readHoldingRegister(uint8_t slaveId, uint16_t regAddress, uint16_t& regValue) = 0;
    virtual bool writeHoldingRegister(uint8_t slaveId, uint16_t regAddress, uint16_t regValue) = 0;
};


class MaimanDriver {
public:
    MaimanDriver(nmbs_t* modbus, uint8_t slaveId);

    // Read operations
    float getTecTemperatureMeasured();
    float getPcbTemperatureMeasured();
    float getTecTemperatureValue();
    float getCurrentMeasured();
    float getFrequency();
    float getDuration();
    float getCurrent();
    float getVoltageMeasured();
    float getCurrentMaxLimit();
    float getCurrentProtectionThreshold();
    float getCurrentSetCalibration();
    float getNtcB25_100Coefficient();
    float getTecCurrentMeasured();
    float getTecVoltage();
    uint16_t getSerialNumber();
    uint16_t getRawStatus();
    bool isBitSet(uint16_t bitmask);
    bool isOperationStarted();
    bool isCurrentSetInternal();
    bool isEnableInternal();
    bool isExternalNtcDenied();
    bool isInterlockDenied();

    // Write operations
    bool setCurrent(float current);
    bool setFrequency(float frequency);
    bool setDuration(float duration);
    bool startDevice();
    bool stopDevice();

private:
    nmbs_t* modbus_;
    uint8_t slaveId_;

    bool readU16(uint16_t address, uint16_t& value);
    bool writeU16(uint16_t address, uint16_t value);
    int16_t toSigned(uint16_t value);
};

#endif //MAIMAN_H
