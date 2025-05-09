//
// Created by Jeb Bailey on 4/22/25.
//

#include "photodiode_task.h"
// #include "photodiode.h"
#include "pico/stdlib.h"
#include <cstdio>
#include "mktl_keys.h"
#include "pico_zyre.h"
#include "FreeRTOS.h"
#include "hardware_context.h"


void photodiode_task(void *param) {
    auto* ctx = static_cast<HardwareContext*>(param);

    pico_zyre::PubMessage msg;
    std::string yj_key = std::string(mktl_keys::PD_YJ_PREFIX)+std::string(mktl_keys::PD_VALUE_SUFFIX);
    std::string hk_key = std::string(mktl_keys::PD_HK_PREFIX)+std::string(mktl_keys::PD_VALUE_SUFFIX);

    // // TODO make sure the address is correct
    // if (!ctx->adc->beginADSX(PICO_ADS1X15::ADSX_ADDRESS_GND, i2c0, 50000)) {
    //     printf("ADC Failure \r\n");
    //     // hardware_faults.adc = true;
    //     while (1) {vTaskDelay(pdMS_TO_TICKS(24));};
    // }
    ctx->adc->setGain(PICO_ADS1X15::ADSXGain_TWOTHIRDS);
    ctx->adc->setDataRate(PICO_ADS1X15::RATE_ADS1115_128SPS);


    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t sample_interval = pdMS_TO_TICKS(10); // 100Hz loop rate (10ms period)

    while (true) {
        int16_t voltage;
        PICO_ADS1X15::ADSXRegConfig_e channel;
        static bool sample_yj = true;

        channel = sample_yj ? PICO_ADS1X15::ADSXRegConfigMuxSingle_0 : PICO_ADS1X15::ADSXRegConfigMuxSingle_1;
        ctx->adc->startADCReading(channel, PICO_ADS1X15::ADSSingleShotMode);
        vTaskDelay(pdMS_TO_TICKS(8)); // Allow ADC conversion time
        voltage = ctx->adc->getLastConversionResults();

        msg.topic = sample_yj ? yj_key: hk_key;
        msg.payload = std::to_string(voltage);
        xQueueSend(ctx->pub_out, &msg, portMAX_DELAY);

        sample_yj = !sample_yj; // toggle between channels

        // Delay until next expected cycle (10ms later)
        vTaskDelayUntil(&last_wake_time, sample_interval);
    }
}