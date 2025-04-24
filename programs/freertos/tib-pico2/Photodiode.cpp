//
// Created by Jeb Bailey on 4/22/25.
//

#include "Photodiode.h"
#include "ads1x15.h"  // From ADS1x15_PICO library
#include "hardware/i2c.h"
#include <cmath>


Photodiode::Photodiode(PICO_ADS1X15& adc, int channel)
    : _adc(adc), _channel(channel) {}

float Photodiode::read_voltage() const {
    int16_t raw = _adc.readADC_SingleEnded(static_cast<PICO_ADS1X15::ADSX_AINX_e>(_channel));
    return _adc.computeVolts(raw);
}

int Photodiode::channel() const {
    return _channel;
}
