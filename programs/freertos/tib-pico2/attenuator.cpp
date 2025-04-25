//
// Created by Jeb Bailey on 4/22/25.
//

#include "attenuator.h"

#include <algorithm>
#include <cstdio>

//TODO The attenuators had a non-linear relationship between dB and voltage that we want to calibrate and then
// then set on dB, not voltage


Attenuator::Attenuator(DacX578& dac, uint8_t channel)
    : _dac(dac), _channel(channel), _voltage(0.0f) {}

void Attenuator::set(float voltage) {
    _voltage = std::clamp(voltage, 0.0f, 4.096f); // Clamp to DAC range
    uint16_t digital = static_cast<uint16_t>((_voltage / 4.096f) * 4095);
    _dac.writeAndUpdateChannel(_channel, digital);
    printf("Attenuator channel %d set to %.3f V\n", _channel, _voltage);
}

float Attenuator::get() const {
    return _voltage;
}