//
// Created by Jeb Bailey on 4/23/25.
//

#include "switching_task.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "mems_switching.h"

// #include <string>
// #include <cstring>

static PCAL6416A gpio(i2c0, 0x21);
static MEMSSwitch s1(gpio, 0, 1);
static MEMSSwitch s2(gpio, 2, 3);
static MEMSRouter router;

void switching_task(void* param) {
    router.addSwitch("sw1", &s1);
    router.addSwitch("sw2", &s2);

    router.defineRoute("input1", "output1", {
        {"sw1", 'A'},
        {"sw2", 'B'}
    });

    router.defineRoute("input1", "output2", {
        {"sw1", 'B'},
        {"sw2", 'A'}
    });

    vTaskDelay(pdMS_TO_TICKS(100)); // give time for other tasks to init

    // Optional initial test
    router.route("input1", "output1");

    while (true) {
        char msg[64];
        if (xQueueReceive(static_cast<QueueHandle_t>(param), &msg, pdMS_TO_TICKS(100)) == pdTRUE) {
            char input[32], output[32];
            if (sscanf(msg, "{\"input\":\"%31[^\"]\",\"output\":\"%31[^\"]\"}", input, output) == 2) {
                router.route(input, output);
            }
        }
    }
}