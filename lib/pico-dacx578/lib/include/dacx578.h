// dacx578.h
#ifndef DACX578_HPP
#define DACX578_HPP

#include <stdint.h>
#include "hardware/i2c.h"

// Command codes (upper nibble pre-shifted)
#define DACX578_CMD_WRITE               (0x0 << 4)
#define DACX578_CMD_UPDATE              (0x1 << 4)
#define DACX578_CMD_WRITE_GLOBAL_UPDATE (0x2 << 4)
#define DACX578_CMD_WRITE_UPDATE        (0x3 << 4)
#define DACX578_CMD_POWERDOWN           (0x4 << 4)
#define DACX578_CMD_RESET               (0x5 << 4)
#define DACX578_CMD_LDAC_MASK           (0x6 << 4)
#define DACX578_CMD_INTERNAL_REF        (0x7 << 4)

// Default I2C address
#define DACX578_DEFAULT_ADDR 0x47
#define DACX578_CHANNEL_BROADCAST 0xF

// Optional debugging (no printf by default; prefer clean silent operation)
#define DACX578_DEBUG 0

#if DACX578_DEBUG
#include <stdio.h>
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

// Clear code register options
enum class DacClearCode : uint8_t {
  Zero = 0,
  Midscale = 1,
  Fullscale = 2,
  NoOp = 3
};

class DacX578 {
public:
  DacX578(i2c_inst_t* i2c, uint8_t address = DACX578_DEFAULT_ADDR, uint8_t resolution = 12);

  bool begin();
  bool reset();

  bool writeChannel(uint8_t channel, uint16_t value);
  bool updateChannel(uint8_t channel);
  bool writeAndUpdateChannel(uint8_t channel, uint16_t value);
  bool writeAndGlobalUpdateChannel(uint8_t channel, uint16_t value);

  bool readChannel(uint8_t channel, uint16_t& value);
  bool readInputChannel(uint8_t channel, uint16_t& value);

  bool setClearCode(DacClearCode code);
  bool getClearCode(DacClearCode& code);

  bool writeLDACMask(uint8_t ldacMask);
  bool readLDACMask(uint8_t& ldacMask);

private:
  bool commandWrite(uint8_t command, uint16_t value);
  bool commandRead(uint8_t command, uint16_t& value);

  i2c_inst_t* _i2c;
  uint8_t _address;
  uint8_t _resolution;
};

#endif // DACX578_HPP
