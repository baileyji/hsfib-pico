//
// Created by Jeb Bailey on 4/21/25.
//
#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "photodiode_task.h"
#include "attenuator_task.h"
#include "switching_task.h"
#include "pico_zyre.h"
#include "coms_task.h"
#include "queue.h"


#define configENABLE_MPU                        0
#define configENABLE_TRUSTZONE                  0
#define configRUN_FREERTOS_SECURE_ONLY          1
#define configENABLE_FPU                        1
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    16


ComsQueues queues;


extern "C" void vApplicationMallocFailedHook() { while (1); }
extern "C" void vApplicationStackOverflowHook(TaskHandle_t, char*) { while (1); }

int main() {
    stdio_init_all();

    queues.pub_out = xQueueCreate(10, sizeof(pico_zyre::PubMessage));
    queues.command_in = xQueueCreate(10, sizeof(pico_zyre::Message));
    queues.response_out = xQueueCreate(10, sizeof(pico_zyre::Message));

    xTaskCreate(coms_task, "coms", 1024, &queues, 1, nullptr);

    xTaskCreate(switching_task, "Switching", 1024, queues.command_in, 1, nullptr);
    xTaskCreate(photodiode_task, "Photodiode", 1024, queues.pub_out, 2, nullptr);
    xTaskCreate(attenuator_task, "Attenuator", 1024, queues.command_in, 2, nullptr);

    vTaskStartScheduler();
    while (true); // Should never reach here
}