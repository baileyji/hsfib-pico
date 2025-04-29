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


// Define UART pins and settings
// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_ID uart0
#define BAUD_RATE 19200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_ODD

#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define UART_DE_PIN 2    // Data Enable pin for RS485



// Function prototypes
void onError();
int32_t read_serial(uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* arg);
int32_t write_serial(const uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* arg);

void onError() {
    // Make the LED blink on error
}

int32_t read_serial(uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* arg) {
    uint64_t start_time = time_us_64();
    int32_t bytes_read = 0;
    uint64_t timeout_us = (uint64_t) byte_timeout_ms * 1000;

    while (time_us_64() - start_time < timeout_us && bytes_read < count) {
        if (uart_is_readable(UART_ID)) {
            buf[bytes_read++] = uart_getc(UART_ID);
            start_time = time_us_64();    // Reset start time after a successful read
        }
    }

    return bytes_read;
}

int32_t write_serial(const uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* arg) {
    uart_write_blocking(UART_ID, buf, count);
    return count;
}

void setup_485() {
    // Initialize the LED pin

    // Initialize the UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Initialize the DE pin as output for RS485
    gpio_init(UART_DE_PIN);
    gpio_set_dir(UART_DE_PIN, GPIO_OUT);
    gpio_put(UART_DE_PIN, 0);    // Set DE low to enable receiving initially


    printf("Setting up Modbus platform configuration...\n");
    nmbs_platform_conf platform_conf;
    nmbs_platform_conf_create(&platform_conf);
    platform_conf.transport = NMBS_TRANSPORT_RTU;
    platform_conf.read = read_serial;
    platform_conf.write = write_serial;

    printf("Creating Modbus client...\n");
    nmbs_t nmbs;
    nmbs_error err = nmbs_client_create(&nmbs, &platform_conf);
    if (err != NMBS_ERROR_NONE) {
        printf("Error creating Modbus client: %d\n", err);
        onError();
    }
    nmbs_set_read_timeout(&nmbs, 1000);
    nmbs_set_byte_timeout(&nmbs, 100);
    nmbs_set_destination_rtu_address(&nmbs, 1);


}


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

    // printf("Writing 2 holding registers at address 26...\n");
    // uint16_t w_regs[2] = {123, 124};
    // err = nmbs_write_multiple_registers(&nmbs, 26, 2, w_regs);
    // if (err != NMBS_ERROR_NONE) {
    //     printf("Error writing multiple registers: %d\n", err);
    //     return false;
    // }
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
