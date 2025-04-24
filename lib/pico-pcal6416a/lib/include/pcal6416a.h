#ifndef PCAL6416A_H
#define PCAL6416A_H

#include "hardware/i2c.h"
#include <stdint.h>
#include <stdbool.h>

class PCAL6416A {
public:
	PCAL6416A(i2c_inst_t* i2c, uint8_t address = 0x21);

	bool setPinMode(uint8_t pin, bool output);
	bool writePin(uint8_t pin, bool level);
	bool readPin(uint8_t pin, bool& level);

	bool writeMasked(uint16_t pin_mask, uint16_t values);

	uint16_t readAll();
	uint8_t getAddress() const;

private:
	i2c_inst_t* _i2c;
	uint8_t _addr;

	bool readRegister(uint8_t reg, uint16_t& value);
	bool writeRegister(uint8_t reg, uint16_t value);

	static constexpr uint8_t REG_INPUT        = 0x00;
	static constexpr uint8_t REG_OUTPUT       = 0x02;
	static constexpr uint8_t REG_CONFIG       = 0x06;
};

#endif // PCAL6416A_H
