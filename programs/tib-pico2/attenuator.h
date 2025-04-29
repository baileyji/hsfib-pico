//
// Created by Jeb Bailey on 4/22/25.
//

#ifndef ATTENUATOR_H
#define ATTENUATOR_H

#include "dacx578.h"

class Attenuator {
public:
    Attenuator(DacX578& dac, uint8_t channel);
    bool set(float voltage);
    bool get(float& voltage);

private:
    DacX578& _dac;
    uint8_t _channel;
    float _voltage;
};


#endif //ATTENUATOR_H
