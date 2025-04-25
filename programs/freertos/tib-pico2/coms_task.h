//
// Created by Jeb Bailey on 4/24/25.
//

#ifndef COMS_TASK_H
#define COMS_TASK_H

#include "FreeRTOS.h"
#include "queue.h"


// Queues passed to coms_task for communication
struct ComsQueues {
    QueueHandle_t command_in;  // expects pico_zyre::Message
    QueueHandle_t response_out;  // expects pico_zyre::Message
    QueueHandle_t pub_out;     // expects PubMessage
};

// Main communications task
#ifdef __cplusplus
extern "C" {
#endif

    void coms_task(void* param);

#ifdef __cplusplus
}
#endif

#endif // COMS_TASK_H