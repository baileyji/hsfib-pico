//
// Created by Jeb Bailey on 4/22/25.
//

#ifndef PHOTODIODE_H
#define PHOTODIODE_H
#include "ads1x15.h"

class Photodiode {
public:
    Photodiode(PICO_ADS1X15& adc, int channel);
    float read_voltage() const;
    int channel() const;

private:
    PICO_ADS1X15& _adc;
    int _channel;
};

#endif //PHOTODIODE_H
