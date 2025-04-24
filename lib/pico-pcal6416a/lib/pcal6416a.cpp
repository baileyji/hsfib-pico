// pcal6416a.cpp
#include "pcal6416a.h"
#include <hardware/i2c.h>
#include <cstring>

PCAL6416A::PCAL6416A(i2c_inst_t* i2c, uint8_t address)
    : _i2c(i2c), _addr(address) {}

bool PCAL6416A::setPinMode(uint8_t pin, bool output) {
  if (pin > 15) return false;
  uint16_t config;
  if (!readRegister(REG_CONFIG, config)) return false;
  if (output) config &= ~(1 << pin);
  else        config |= (1 << pin);
  return writeRegister(REG_CONFIG, config);
}

bool PCAL6416A::writePin(uint8_t pin, bool level) {
  if (pin > 15) return false;
  uint16_t out;
  if (!readRegister(REG_OUTPUT, out)) return false;
  if (level) out |= (1 << pin);
  else       out &= ~(1 << pin);
  return writeRegister(REG_OUTPUT, out);
}

bool PCAL6416A::readPin(uint8_t pin, bool& level) {
  if (pin > 15) return false;
  uint16_t in;
  if (!readRegister(REG_INPUT, in)) return false;
  level = in & (1 << pin);
  return true;
}

bool PCAL6416A::writeMasked(uint16_t mask, uint16_t values) {
  uint16_t out;
  if (!readRegister(REG_OUTPUT, out)) return false;
  out = (out & ~mask) | (values & mask);
  return writeRegister(REG_OUTPUT, out);
}

uint16_t PCAL6416A::readAll() {
  uint16_t in = 0;
  readRegister(REG_INPUT, in);
  return in;
}

uint8_t PCAL6416A::getAddress() const {
  return _addr;
}

bool PCAL6416A::readRegister(uint8_t reg, uint16_t& value) {
  uint8_t buf[2];
  if (i2c_write_blocking(_i2c, _addr, &reg, 1, true) != 1) return false;
  if (i2c_read_blocking(_i2c, _addr, buf, 2, false) != 2) return false;
  value = (buf[1] << 8) | buf[0];
  return true;
}

bool PCAL6416A::writeRegister(uint8_t reg, uint16_t value) {
  uint8_t buf[3] = { reg, static_cast<uint8_t>(value & 0xFF), static_cast<uint8_t>((value >> 8) & 0xFF) };
  return i2c_write_blocking(_i2c, _addr, buf, 3, false) == 3;
}
