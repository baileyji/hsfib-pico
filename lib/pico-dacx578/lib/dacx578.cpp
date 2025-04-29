// dacx578.cpp
#include "dacx578.h"
#include <hardware/i2c.h>
//#include <string.h>

DacX578::DacX578(i2c_inst_t* i2c, uint8_t address, uint8_t resolution)
    : _i2c(i2c), _address(address), _resolution(resolution) {
    if (_resolution != 8 && _resolution != 10 && _resolution != 12) {
        _resolution = 8;
    }
}

bool DacX578::begin() {
    // No init sequence needed for DACX578, assume I2C is initialized elsewhere
    return true;
}

bool DacX578::reset() {
    uint8_t cmd = DACX578_CMD_RESET;
    return i2c_write_blocking(_i2c, _address, &cmd, 1, false) == 1;
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
    if ((uint8_t)code > 3) return false;
    uint8_t buf[3] = { DACX578_CMD_RESET, 0x00, static_cast<uint8_t>(code) };
    return i2c_write_blocking(_i2c, _address, buf, 3, false) == 3;
}

bool DacX578::readChannel(uint8_t channel, uint16_t& value) {
    //TODO
    // if (channel > 7 && channel != DACX578_CHANNEL_BROADCAST) return false;
    // uint8_t buf[3] = { DACX578_CMD_READ | (channel & 0xF), 0x00, 0x00 };
    // if (i2c_write_blocking(_i2c, _address, buf, 3, true) != 3) return false;
    // if (i2c_read_blocking(_i2c, _address, buf, 3, false) != 3) return false;
    // value = (buf[1] << 8) | buf[2];
    return true;
}

bool DacX578::commandWrite(uint8_t command, uint16_t value) {
    uint8_t shift = 16 - _resolution;
    value <<= shift;
    uint8_t buf[3] = {
        command,
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>(value & 0xFF)
    };
    return i2c_write_blocking(_i2c, _address, buf, 3, false) == 3;
}
