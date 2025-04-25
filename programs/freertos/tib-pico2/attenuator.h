//
// Created by Jeb Bailey on 4/22/25.
//

#ifndef ATTENUATOR_H
#define ATTENUATOR_H

#include "dacx578.h"

class Attenuator {
public:
    Attenuator(DacX578& dac, uint8_t channel);
    void set(float voltage);
    float get() const;

private:
    DacX578& _dac;
    uint8_t _channel;
    float _voltage;
};


#endif //ATTENUATOR_H
