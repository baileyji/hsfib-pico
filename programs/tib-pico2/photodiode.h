//
// Created by Jeb Bailey on 4/22/25.
//

#ifndef PHOTODIODE_H
#define PHOTODIODE_H
#include "ads1x15.h"
#include "FreeRTOS.h"
#include "semphr.h"

class Photodiode {
public:
    Photodiode(PICO_ADS1X15& adc, int channel, SemaphoreHandle_t& lock);
    float read_voltage() const;
    int channel() const;
    float getGain();
    bool setGain(float gain);
    SemaphoreHandle_t lock;

private:
    PICO_ADS1X15& _adc;
    int _channel;

};

#endif //PHOTODIODE_H
