//
// Created by Jeb Bailey on 4/22/25.
//

#include "attenuator_task.h"
#include "dacx578.h"
#include "attenuator.h"
#include <cstring>
#include <cstdio>
#include <array>


static DacX578 dac(i2c0, 0x47, 12);

static std::array<Attenuator, 6> attenuators = {
    Attenuator(dac, 0),
    Attenuator(dac, 1),
    Attenuator(dac, 2),
    Attenuator(dac, 3),
    Attenuator(dac, 4),
    Attenuator(dac, 5),
};

void attenuator_task(void* param) {
    QueueHandle_t cmd_queue = static_cast<QueueHandle_t>(param);
    dac.begin();

    char msg[128];

    while (true) {
        if (xQueueReceive(cmd_queue, &msg, portMAX_DELAY) == pdTRUE) {
            int id = 0;
            float val = 0.0f;
            if (sscanf(msg, "{\"id\":%d,\"value\":%f}", &id, &val) == 2 && id >= 0 && id < 6) {
                attenuators[id].set(val);
            } else {
                printf("DAC: invalid message or attenuator id: %s\n", msg);
            }
        }
    }
}