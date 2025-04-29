// dacx578.cpp
#include "dacx578.h"
#include <string.h>
#include "pico/stdlib.h"

DacX578::DacX578(i2c_inst_t* i2c, uint8_t address, uint8_t resolution)
    : _i2c(i2c), _address(address), _resolution(resolution) {
    if (_resolution != 8 && _resolution != 10 && _resolution != 12) {
        DEBUG_PRINT("Invalid resolution %u, defaulting to 8-bit.\n", _resolution);
        _resolution = 8;
    }
}

bool DacX578::begin() {
    return true;  // assume I2C is already initialized externally
}

bool DacX578::reset() {
    uint8_t cmd = DACX578_CMD_RESET;
    int ret = i2c_write_blocking(_i2c, _address, &cmd, 1, false);
    DEBUG_PRINT("Reset command write returned %d\n", ret);
    return ret == 1;
}

bool DacX578::writeChannel(uint8_t channel, uint16_t value) {
    if (channel > 7 && channel != DACX578_CHANNEL_BROADCAST) return false;
    return commandWrite(DACX578_CMD_WRITE | (channel & 0xF), value);
}

bool DacX578::updateChannel(uint8_t channel) {
    if (channel > 7 && channel != DACX578_CHANNEL_BROADCAST) return false;
    return commandWrite(DACX578_CMD_UPDATE | (channel & 0xF), 0);
}

bool DacX578::writeAndUpdateChannel(uint8_t channel, uint16_t value) {
    if (channel > 7 && channel != DACX578_CHANNEL_BROADCAST) return false;
    return commandWrite(DACX578_CMD_WRITE_UPDATE | (channel & 0xF), value);
}

bool DacX578::writeAndGlobalUpdateChannel(uint8_t channel, uint16_t value) {
    if (channel > 7 && channel != DACX578_CHANNEL_BROADCAST) return false;
    return commandWrite(DACX578_CMD_WRITE_GLOBAL_UPDATE | (channel & 0xF), value);
}

bool DacX578::setClearCode(DacClearCode code) {
    uint8_t codeMasked = static_cast<uint8_t>(code) & 0x03;
    uint8_t buf[3] = { DACX578_CMD_RESET, 0x00, codeMasked };
    int ret = i2c_write_blocking(_i2c, _address, buf, 3, false);
    DEBUG_PRINT("SetClearCode write returned %d\n", ret);
    return ret == 3;
}

bool DacX578::getClearCode(DacClearCode& code) {
    uint8_t cmd = DACX578_CMD_RESET;
    uint8_t buf[3] = {};
    int ret = i2c_write_blocking(_i2c, _address, &cmd, 1, true);
    if (ret != 1) return false;
    ret = i2c_read_blocking(_i2c, _address, buf, 3, false);
    if (ret != 3) return false;
    code = static_cast<DacClearCode>(buf[2] & 0x03);
    return true;
}

bool DacX578::writeLDACMask(uint8_t ldacMask) {
    uint8_t buf[3] = { DACX578_CMD_LDAC_MASK, ldacMask, 0x00 };
    int ret = i2c_write_blocking(_i2c, _address, buf, 3, false);
    return ret == 3;
}

bool DacX578::readLDACMask(uint8_t& ldacMask) {
    uint8_t cmd = DACX578_CMD_LDAC_MASK;
    uint8_t buf[2] = {};
    if (i2c_write_blocking(_i2c, _address, &cmd, 1, true) != 1) return false;
    if (i2c_read_blocking(_i2c, _address, buf, 2, false) != 2) return false;
    ldacMask = buf[1];
    return true;
}

bool DacX578::readChannel(uint8_t channel, uint16_t& value) {
    if (channel > 7) return false;
    return commandRead(DACX578_CMD_UPDATE | (channel & 0xF), value);
}

bool DacX578::readInputChannel(uint8_t channel, uint16_t& value) {
    if (channel > 7) return false;
    return commandRead(DACX578_CMD_WRITE | (channel & 0xF), value);
}

bool DacX578::commandWrite(uint8_t command, uint16_t value) {
    uint8_t shift = 16 - _resolution;
    value <<= shift;
    uint8_t buf[3] = {
        command,
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>(value & 0xFF)
    };
    int ret = i2c_write_blocking(_i2c, _address, buf, 3, false);
    DEBUG_PRINT("CommandWrite 0x%02X returned %d\n", command, ret);
    return ret == 3;
}

bool DacX578::commandRead(uint8_t command, uint16_t& value) {
    uint8_t buf[2] = {};
    if (i2c_write_blocking(_i2c, _address, &command, 1, true) != 1) return false;
    if (i2c_read_blocking(_i2c, _address, buf, 2, false) != 2) return false;
    value = ((uint16_t(buf[0]) << 8) | buf[1]) >> (16 - _resolution);
    return true;
}
