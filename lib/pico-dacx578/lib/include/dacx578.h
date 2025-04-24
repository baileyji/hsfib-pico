// dacx578.hpp
#ifndef DACX578_HPP
#define DACX578_HPP

#include <stdint.h>
#include "hardware/i2c.h"

#define DACX578_CMD_WRITE               (0x0 << 4)
#define DACX578_CMD_UPDATE              (0x1 << 4)
#define DACX578_CMD_WRITE_GLOBAL_UPDATE (0x2 << 4)
#define DACX578_CMD_WRITE_UPDATE        (0x3 << 4)
#define DACX578_CMD_RESET               (0x5 << 4)
#define DACX578_CMD_LDAC_MASK           (0x6 << 4)
#define DACX578_DEFAULT_ADDR            0x47
#define DACX578_CHANNEL_BROADCAST       0xF

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
  bool setClearCode(DacClearCode code);

private:
  bool commandWrite(uint8_t command, uint16_t value);
  i2c_inst_t* _i2c;
  uint8_t _address;
  uint8_t _resolution;
};

#endif // DACX578_HPP