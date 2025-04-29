//
// Created by Jeb Bailey on 4/22/25.
//

#include "photodiode_task.h"
// #include "photodiode.h"
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

    // TODO make sure the address is correct
    if (!ctx->adc->beginADSX(PICO_ADS1X15::ADSX_ADDRESS_GND, i2c0, 50000)) {
        printf("ADC Failure \r\n");
        // hardware_faults.adc = true;
        while (1) {vTaskDelay(pdMS_TO_TICKS(24));};
    }
    ctx->adc->setGain(PICO_ADS1X15::ADSXGain_TWOTHIRDS);
    ctx->adc->setDataRate(PICO_ADS1X15::RATE_ADS1115_128SPS);


    while (true) {
        int16_t yj_voltage, hk_voltage;

        ctx->adc->startADCReading(PICO_ADS1X15::ADSXRegConfigMuxSingle_0, PICO_ADS1X15::ADSSingleShotMode);
        vTaskDelay(pdMS_TO_TICKS(8));  //
        yj_voltage = ctx->adc->getLastConversionResults();
        ctx->adc->startADCReading(PICO_ADS1X15::ADSXRegConfigMuxSingle_1, PICO_ADS1X15::ADSSingleShotMode);
        vTaskDelay(pdMS_TO_TICKS(8));
        hk_voltage = ctx->adc->getLastConversionResults();

        msg.topic = yj_key;
        msg.payload = std::to_string(yj_voltage);
        xQueueSend(ctx->pub_out, &msg, portMAX_DELAY);

        msg.topic = hk_key;
        msg.payload = std::to_string(hk_voltage);
        xQueueSend(ctx->pub_out, &msg, portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(8));
    }
}
