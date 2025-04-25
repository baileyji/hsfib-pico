//
// Created by Jeb Bailey on 4/22/25.
//

#include "photodiode.h"
#include "ads1x15.h"  // From ADS1x15_PICO library
#include "hardware/i2c.h"
#include <cmath>


// TODO  need to sort out gain commands

Photodiode::Photodiode(PICO_ADS1X15& adc, int channel, SemaphoreHandle_t& lock)
    : _adc(adc), _channel(channel), lock(lock) {}

float Photodiode::read_voltage() const {
    xSemaphoreTake(lock, portMAX_DELAY);
    int16_t raw = _adc.readADC_SingleEnded(static_cast<PICO_ADS1X15::ADSX_AINX_e>(_channel));
    xSemaphoreGive(lock);
    return _adc.computeVolts(raw);
}

bool Photodiode::setGain(float gain) {
    if (gain < 0.0f || gain > 4.096f)
        return false;
    xSemaphoreTake(lock, portMAX_DELAY);
    _adc.setGain(gain);
    xSemaphoreGive(lock);
    return true;
}

float Photodiode::getGain() {
    xSemaphoreTake(lock, portMAX_DELAY);
    float gain = _adc.getGain();
    xSemaphoreGive(lock);
    return gain;
}

int Photodiode::channel() const {
    return _channel;
}
