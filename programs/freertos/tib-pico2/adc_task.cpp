//
// Created by Jeb Bailey on 4/22/25.
//

#include "adc_task.h"
#include "Photodiode.h"
#include "pico/stdlib.h"
#include <cstdio>


void adc_task(void *param) {
    QueueHandle_t pub_queue = static_cast<QueueHandle_t>(param);

    static PICO_ADS1115 adc;
    adc.beginADSX(PICO_ADS1X15::ADSX_ADDRESS_GND, i2c0, 400, 4, 5, 50000);
    Photodiode pd0(adc, 0);
    Photodiode pd1(adc, 1);


    char msg[128];

    while (true) {
        float voltage = pd0.read_voltage();

        float voltage2 = pd1.read_voltage();

        snprintf(msg, sizeof(msg), "{\"channel\":%d,\"voltage\":%.3f}", pd0.channel(), voltage);
        xQueueSend(pub_queue, &msg, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
