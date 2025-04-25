//
// Created by Jeb Bailey on 4/25/25.
//

#ifndef HARDWARE_CONTEXT_H
#define HARDWARE_CONTEXT_H


#include "pico_zyre.h"
#include "queue.h"
#include <array>
#include "photodiode.h"
#include "attenuator.h"
#include "semphr.h"
#include "mems_switching.h"

struct HardwareContext {
    // Hardware ownership
    Photodiode* yj_photodiode;
    Photodiode* hk_photodiode;
    std::array<Attenuator, 6>* attenuators;
    MEMSRouter* router;

    // Messaging infrastructure
    QueueHandle_t command_in;   // accepts Command
    QueueHandle_t response_out; // produces Message
    QueueHandle_t pub_out;      // produces PubMessage
};


#endif //HARDWARE_CONTEXT_H
