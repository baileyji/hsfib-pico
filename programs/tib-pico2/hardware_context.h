//
// Created by Jeb Bailey on 4/25/25.
//

#ifndef HARDWARE_CONTEXT_H
#define HARDWARE_CONTEXT_H


#include "pico_zyre.h"
#include "FreeRTOS.h"
#include "queue.h"
#include <array>
#include "photodiode.h"
#include "attenuator.h"
#include "maiman.h"
#include "mems_switching.h"

struct HardwareContext {
    // Hardware ownership
    PICO_ADS1115* adc;
    std::array<Attenuator, 6>& attenuators;
    std::array<MaimanDriver, 6>& lasers;
    MEMSRouter* router;

    // Messaging infrastructure
    QueueHandle_t command_in;   // accepts Command
    QueueHandle_t response_out; // produces Message
    QueueHandle_t pub_out;      // produces PubMessage
};


#endif //HARDWARE_CONTEXT_H
