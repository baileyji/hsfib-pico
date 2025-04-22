//
// Created by Jeb Bailey on 4/21/25.
//
#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "adc_task.hpp"
#include "dac_task.hpp"
#include "pub_task.hpp"
#include "rep_task.hpp"
#include "queue.h"

#define configENABLE_MPU                        0
#define configENABLE_TRUSTZONE                  0
#define configRUN_FREERTOS_SECURE_ONLY          1
#define configENABLE_FPU                        1
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    16


QueueHandle_t pub_queue;
QueueHandle_t dac_command_queue;

extern "C" void vApplicationMallocFailedHook() { while (1); }
extern "C" void vApplicationStackOverflowHook(TaskHandle_t, char*) { while (1); }

int main() {
    stdio_init_all();

    pub_queue = xQueueCreate(10, sizeof(char[128]));
    dac_command_queue = xQueueCreate(10, sizeof(char[128]));

    xTaskCreate(adc_task, "ADC", 1024, pub_queue, 2, NULL);
    xTaskCreate(dac_task, "DAC", 1024, dac_command_queue, 2, NULL);
    xTaskCreate(pub_task, "PUB", 1024, pub_queue, 1, NULL);
    xTaskCreate(rep_task, "REP", 1024, dac_command_queue, 1, NULL);

    vTaskStartScheduler();
    while (true); // Should never reach here
}