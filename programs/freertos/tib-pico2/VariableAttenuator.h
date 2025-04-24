//
// Created by Jeb Bailey on 4/22/25.
//

#ifndef VARIABLEATTENUATOR_H
#define VARIABLEATTENUATOR_H

#include "dacx578.h"

class VariableAttenuator {
public:
    VariableAttenuator(DacX578& dac, uint8_t channel);
    void set(float voltage);
    float get() const;

private:
    DacX578& _dac;
    uint8_t _channel;
    float _voltage;
};


#endif //VARIABLEATTENUATOR_H
