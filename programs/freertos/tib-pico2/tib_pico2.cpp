//
// Created by Jeb Bailey on 4/21/25.
//
#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "photodiode_task.h"
#include "executor_task.h"
#include "pico_zyre.h"
#include "coms_task.h"
#include "queue.h"
#include <array>
#include "photodiode.h"
#include "attenuator.h"
#include "semphr.h"
#include "dacx578.h"
#include <string>
#include "mems_switching.h"
#include "hardware_context.h"

#define configENABLE_MPU                        0
#define configENABLE_TRUSTZONE                  0
#define configRUN_FREERTOS_SECURE_ONLY          1
#define configENABLE_FPU                        1
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    16


ComsQueues queues;



// static const CommandEntry dispatch_table[] = {
//     {"photodiode.0.gain", [](const Command& cmd) {
//         PhotodiodeCommand pcmd;
//         pcmd.index = 0;
//         pcmd.type = (cmd.type == MsgType::SET) ? PhotodiodeCommand::SetGain : PhotodiodeCommand::GetGain;
//         pcmd.value = cmd.args["gain"];
//         xQueueSend(photodiode_queue, &pcmd, 0);
//     }},
// };



int main() {
    stdio_init_all();

    // Static hardware and messaging setup

    static DacX578 dac(i2c0, 0x47, 12);
    static PICO_ADS1115 adc;

    dac.begin();
    adc.beginADSX(PICO_ADS1X15::ADSX_ADDRESS_GND, i2c0, 400, 4, 5, 50000);

    static PCAL6416A pcagpio(i2c0, 0x21);
    static std::array<MEMSSwitch, 8> switches = {
        MEMSSwitch(pcagpio, 0, 1),
        MEMSSwitch(pcagpio, 2, 3),
        MEMSSwitch(pcagpio, 4, 5),
        MEMSSwitch(pcagpio, 6, 7),
        MEMSSwitch(pcagpio, 8, 9),
        MEMSSwitch(pcagpio, 10, 11),
        MEMSSwitch(pcagpio, 12, 13),
        MEMSSwitch(pcagpio, 14, 15)};

    static const std::pair<std::string_view, MEMSSwitch*> named_switches[] = {
        {"sw0", &switches[0]},
        {"sw1", &switches[1]},
        {"sw2", &switches[2]},
        {"sw3", &switches[3]},
        {"sw4", &switches[4]},
        {"sw5", &switches[5]},
        {"sw6", &switches[6]},
        {"sw7", &switches[7]},
    };
    static MEMSRouter router(named_switches, std::size(named_switches));
    router.defineRoute("input1", "output1", {
        {"sw1", 'A'},
        {"sw2", 'B'}
    });

    router.defineRoute("input1", "output2", {
        {"sw1", 'B'},
        {"sw2", 'A'}
    });


    static std::array<Photodiode, 2> photodiodes = {
        Photodiode(adc, 0), Photodiode(adc, 1)
    };

    static std::array<Attenuator, 6> attenuators = {
        Attenuator(dac, 0), Attenuator(dac, 1), Attenuator(dac, 2),
        Attenuator(dac, 3), Attenuator(dac, 4), Attenuator(dac, 5)
    };

    static QueueHandle_t command_queue   = xQueueCreate(10, sizeof(pico_zyre::Command));
    static QueueHandle_t response_queue  = xQueueCreate(10, sizeof(pico_zyre::Response));
    static QueueHandle_t pub_queue       = xQueueCreate(10, sizeof(pico_zyre::PubMessage));
    static SemaphoreHandle_t pd_lock     = xSemaphoreCreateMutex();

    static HardwareContext ctx = {
        .photodiodes = &photodiodes,
        .attenuators = &attenuators,
        .router = &router,

        .command_in = command_queue,
        .response_out = response_queue,
        .pub_out = pub_queue,
        .photodiode_lock = pd_lock,
    };

    xTaskCreate(executor_task, "executor", 2048, &ctx, 2, nullptr);
    xTaskCreate(photodiode_task, "pd", 1024, &ctx, 1, nullptr);
    xTaskCreate(coms_task, "coms", 2048, &ctx, 2, nullptr);

    vTaskStartScheduler();
    while (true); // should never reach here
}