//
// Created by Jeb Bailey on 4/22/25.
//

#include "VariableAttenuator.h"

#include <algorithm>
#include <cstdio>

VariableAttenuator::VariableAttenuator(DacX578& dac, uint8_t channel)
    : _dac(dac), _channel(channel), _voltage(0.0f) {}

void VariableAttenuator::set(float voltage) {
    _voltage = std::clamp(voltage, 0.0f, 4.096f); // Clamp to DAC range
    uint16_t digital = static_cast<uint16_t>((_voltage / 4.096f) * 4095);
    _dac.writeAndUpdateChannel(_channel, digital);
    printf("Attenuator channel %d set to %.3f V\n", _channel, _voltage);
}

float VariableAttenuator::get() const {
    return _voltage;
}