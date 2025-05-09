//
// Created by Jeb Bailey on 4/28/25.
//

#include "maiman.h"

#include "hardware/gpio.h"
#include "pico/types.h"
#include "hardware/timer.h"
#include "hardware/uart.h"
#include "nanomodbus.h"
#include "pico/stdlib.h"
#include <stdio.h>


MaimanDriver::MaimanDriver(nmbs_t* modbus, uint8_t slaveId)
    : modbus_(modbus), slaveId_(slaveId) {
}


bool MaimanDriver::readU16(uint16_t address, uint16_t& value) {
    modbus_->dest_address_rtu=slaveId_;
    nmbs_error err = nmbs_read_holding_registers(modbus_, address, 2, &value);
    if (err != NMBS_ERROR_NONE) {
        printf("Error reading holding registers: %d\n", err);
        return false;
    }
    return true;
}


bool MaimanDriver::writeU16(uint16_t address, uint16_t value) {
    modbus_->dest_address_rtu=slaveId_;
    nmbs_error error = nmbs_write_single_register(modbus_, address, value);
    if (error != NMBS_ERROR_NONE) {
        printf("Error writing registers: %d\n", error);
        return false;
    }

    return true;
}


int16_t MaimanDriver::toSigned(uint16_t value) {
    return static_cast<int16_t>(value);
}

// Read functions
float MaimanDriver::getTecTemperatureMeasured() {
    uint16_t raw;
    if (readU16(REG_TEC_TEMPERATURE_MEASURED, raw)) {
        return toSigned(raw) / DIVIDER_TEMPERATURE;
    }
    return -273.15f;
}

float MaimanDriver::getPcbTemperatureMeasured() {
    uint16_t raw;
    if (readU16(REG_PCB_TEMPERATURE_MEASURED, raw)) {
        return toSigned(raw) / DIVIDER_TEMPERATURE;
    }
    return -273.15f;
}

float MaimanDriver::getTecTemperatureValue() {
    uint16_t raw;
    if (readU16(REG_TEC_TEMPERATURE_VALUE, raw)) {
        return toSigned(raw) / DIVIDER_TEMPERATURE;
    }
    return -273.15f;
}

float MaimanDriver::getCurrentMeasured() {
    uint16_t raw;
    if (readU16(REG_CURRENT_MEASURED, raw)) {
        return static_cast<float>(raw) / DIVIDER_CURRENT;
    }
    return -1.0f;
}

float MaimanDriver::getFrequency() {
    uint16_t raw;
    if (readU16(REG_FREQUENCY, raw)) {
        return static_cast<float>(raw) / DIVIDER_FREQUENCY;
    }
    return -1.0f;
}

float MaimanDriver::getDuration() {
    uint16_t raw;
    if (readU16(REG_DURATION, raw)) {
        return static_cast<float>(raw) / DIVIDER_DURATION;
    }
    return -1.0f;
}

bool MaimanDriver::getCurrent(float& value) {
    uint16_t raw;
    if (readU16(REG_CURRENT, raw)) {
        value = static_cast<float>(raw) / DIVIDER_CURRENT;
        return true;
    }
    return false;
}

float MaimanDriver::getVoltageMeasured() {
    uint16_t raw;
    if (readU16(REG_VOLTAGE_MEASURED, raw)) {
        return static_cast<float>(raw) / DIVIDER_VOLTAGE;
    }
    return -1.0f;
}

float MaimanDriver::getCurrentMaxLimit() {
    uint16_t raw;
    if (readU16(REG_CURRENT_MAX_LIMIT, raw)) {
        return static_cast<float>(raw) / DIVIDER_CURRENT;
    }
    return -1.0f;
}

float MaimanDriver::getCurrentProtectionThreshold() {
    uint16_t raw;
    if (readU16(REG_CURRENT_PROTECTION_THRESHOLD, raw)) {
        return static_cast<float>(raw) / DIVIDER_CURRENT;
    }
    return -1.0f;
}

float MaimanDriver::getCurrentSetCalibration() {
    uint16_t raw;
    if (readU16(REG_CURRENT_SET_CALIBRATION, raw)) {
        return static_cast<float>(raw) / DIVIDER_CURRENT;
    }
    return -1.0f;
}

float MaimanDriver::getNtcB25_100Coefficient() {
    uint16_t raw;
    if (readU16(REG_NTC_COEFFICIENT, raw)) {
        return static_cast<float>(raw) / DIVIDER_NTC_COEFFICIENT;
    }
    return -1.0f;
}

float MaimanDriver::getTecCurrentMeasured() {
    uint16_t raw;
    if (readU16(REG_TEC_CURRENT_MEASURED, raw)) {
        return static_cast<float>(raw) / DIVIDER_CURRENT;
    }
    return -1.0f;
}

float MaimanDriver::getTecVoltage() {
    uint16_t raw;
    if (readU16(REG_TEC_VOLTAGE, raw)) {
        return static_cast<float>(raw) / DIVIDER_VOLTAGE;
    }
    return -1.0f;
}

uint16_t MaimanDriver::getSerialNumber() {
    uint16_t raw;
    if (readU16(REG_SERIAL_NUMBER, raw)) {
        return raw;
    }
    return 0;
}

uint16_t MaimanDriver::getRawStatus() {
    uint16_t raw;
    if (readU16(REG_STATE_OF_DEVICE_COMMAND, raw)) {
        return raw;
    }
    return 0;
}

bool MaimanDriver::isBitSet(uint16_t bitmask) {
    uint16_t status = getRawStatus();
    return (status & bitmask) != 0;
}

bool MaimanDriver::isOperationStarted() {
    return isBitSet(OPERATION_STATE_STARTED);
}

bool MaimanDriver::isCurrentSetInternal() {
    return isBitSet(CURRENT_SET_INTERNAL);
}

bool MaimanDriver::isEnableInternal() {
    return isBitSet(ENABLE_INTERNAL);
}

bool MaimanDriver::isExternalNtcDenied() {
    return isBitSet(EXTERNAL_NTC_INTERLOCK_DENIED);
}

bool MaimanDriver::isInterlockDenied() {
    return isBitSet(INTERLOCK_DENIED);
}

// Write functions
bool MaimanDriver::setCurrent(float current) {
    uint16_t value = static_cast<uint16_t>(current * DIVIDER_CURRENT);
    return writeU16(REG_CURRENT, value);
}

bool MaimanDriver::setFrequency(float frequency) {
    uint16_t value = static_cast<uint16_t>(frequency * DIVIDER_FREQUENCY);
    return writeU16(REG_FREQUENCY, value);
}

bool MaimanDriver::setDuration(float duration) {
    uint16_t value = static_cast<uint16_t>(duration * DIVIDER_DURATION);
    return writeU16(REG_DURATION, value);
}

bool MaimanDriver::startDevice() {
    return writeU16(REG_STATE_OF_DEVICE_COMMAND, MODBUS_START_COMMAND_VALUE);
}

bool MaimanDriver::stopDevice() {
    return writeU16(REG_STATE_OF_DEVICE_COMMAND, MODBUS_STOP_COMMAND_VALUE);
}
