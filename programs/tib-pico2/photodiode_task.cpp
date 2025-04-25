//
// Created by Jeb Bailey on 4/22/25.
//

#include "photodiode_task.h"
#include "photodiode.h"
#include "pico/stdlib.h"
#include <cstdio>
#import "mktl_keys.h"
#include "pico_zyre.h"
#include "FreeRTOS.h"
#include "hardware_context.h"


void photodiode_task(void *param) {
    auto* ctx = static_cast<HardwareContext*>(param);

    pico_zyre::PubMessage msg;

    std::string yj_key = std::string(mktl_keys::PD_YJ_PREFIX)+std::string(mktl_keys::PD_VALUE_SUFFIX);
    std::string hk_key = std::string(mktl_keys::PD_HK_PREFIX)+std::string(mktl_keys::PD_VALUE_SUFFIX);

    while (true) {
        float yj_voltage, hk_voltage;

        yj_voltage = ctx->yj_photodiode->read_voltage();
        hk_voltage = ctx->hk_photodiode->read_voltage();

        msg.topic = yj_key;
        msg.payload = std::to_string(yj_voltage);
        xQueueSend(ctx->pub_out, &msg, portMAX_DELAY);

        msg.topic = hk_key;
        msg.payload = std::to_string(hk_voltage);
        xQueueSend(ctx->pub_out, &msg, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
