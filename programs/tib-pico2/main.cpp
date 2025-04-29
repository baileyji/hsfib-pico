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
#include "nanomodbus.h"
#include "maiman.h"

#define configENABLE_MPU                        0
#define configENABLE_TRUSTZONE                  0
#define configRUN_FREERTOS_SECURE_ONLY          1
#define configENABLE_FPU                        1
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    16


#define YJ_PHOTODIODE_CHANNEL 0
#define HK_PHOTODIODE_CHANNEL 1

#define UART0_TX_PIN 0
#define UART0_RX_PIN 1
#define UART1_TX_PIN 4
#define UART1_RX_PIN 5
#define MB_UART_ID uart1
#define MB_UART_BAUDRATE 115200


// Define UART pins and settings
// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_ID uart0
#define BAUD_RATE 19200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_ODD

#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define UART_DE_PIN 2    // Data Enable pin for RS485


int32_t read_serial(uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* arg) {
    uint64_t start_time = time_us_64();
    int32_t bytes_read = 0;
    uint64_t timeout_us = (uint64_t) byte_timeout_ms * 1000;

    while (time_us_64() - start_time < timeout_us && bytes_read < count) {
        if (uart_is_readable(UART_ID)) {
            buf[bytes_read++] = uart_getc(UART_ID);
            start_time = time_us_64();    // Reset start time after a successful read
        }
    }

    return bytes_read;
}

int32_t write_serial(const uint8_t* buf, uint16_t count, int32_t byte_timeout_ms, void* arg) {
    uart_write_blocking(UART_ID, buf, count);
    return count;
}


 void setup_485(nmbs_t &nmbs) {
    // Initialize the LED pin

    // Initialize the UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int __unused actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Initialize the DE pin as output for RS485
    gpio_init(UART_DE_PIN);
    gpio_set_dir(UART_DE_PIN, GPIO_OUT);
    gpio_put(UART_DE_PIN, 0);    // Set DE low to enable receiving initially


    printf("Setting up Modbus platform configuration...\n");
    nmbs_platform_conf platform_conf;
    nmbs_platform_conf_create(&platform_conf);
    platform_conf.transport = NMBS_TRANSPORT_RTU;
    platform_conf.read = read_serial;
    platform_conf.write = write_serial;

    printf("Creating Modbus client...\n");
    nmbs_error err = nmbs_client_create(&nmbs, &platform_conf);
    if (err != NMBS_ERROR_NONE) {
        printf("Error creating Modbus client: %d\n", err);
    }
    nmbs_set_read_timeout(&nmbs, 1000);
    nmbs_set_byte_timeout(&nmbs, 100);
    nmbs_set_destination_rtu_address(&nmbs, 1);

}


#define I2C0_SDA_PIN 6
#define I2C0_SCK_PIN 7

int main() {
    stdio_init_all();

    // Static hardware and messaging setup

    // init I2c pins and interface
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCK_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCK_PIN);
    i2c_init(i2c0, 400 * 1000);
    // gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_NULL);
    // gpio_set_function(I2C0_SCK_PIN, GPIO_FUNC_NULL);
    // i2c_deinit(_i2c);

    static DacX578 dac(i2c0, 0x47, 12);
    static PICO_ADS1115 adc;

    dac.begin();

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
        {"yj_cal_laser", &switches[0]},
        {"hk_cal_laser", &switches[1]},

        {"yj_ao_fei", &switches[2]},
        {"hk_ao_fei", &switches[3]},

        {"yj_forward_retro", &switches[4]},
        {"hk_forward_retro", &switches[5]},

        {"yj_mm_sm", &switches[6]},
        {"hk_mm_sm", &switches[7]},
    };

    static MEMSRouter router(named_switches, std::size(named_switches));

    router.defineRoute("yj_1430", "yj_ao", {
        {"yj_cal_laser", 'B'},
        {"yj_forward_retro", 'A'},
        {"yj_ao_fei", 'A'}
    });
    router.defineRoute("yj_1430", "yj_fei", {
        {"yj_cal_laser", 'B'},
        {"yj_forward_retro", 'A'},
        {"yj_ao_fei", 'B'}
    });

    router.defineRoute("yj_cal", "yj_ao", {
        {"yj_cal_laser", 'A'},
        {"yj_ao_fei", 'A'}
    });
    router.defineRoute("yj_cal", "yj_fei", {
        {"yj_cal_laser", 'A'},
        {"yj_ao_fei", 'B'}
    });

    router.defineRoute("yj_laser", "yj_ao", {
        {"yj_cal_laser", 'B'},
        {"yj_ao_fei", 'A'}
    });
    router.defineRoute("yj_laser", "yj_fei", {
        {"yj_cal_laser", 'B'},
        {"yj_ao_fei", 'B'}
    });

    router.defineRoute("yj_mm", "yj_pd", {
        {"yj_mm_sm", 'A'}
    });
    router.defineRoute("yj_sm", "yj_pd", {
        {"yj_mm_sm", 'B'}
    });

    router.defineRoute("yj_1430", "yj_fei", {
        {"yj_cal_laser", 'B'},
        {"yj_forward_retro", 'A'},
        {"yj_ao_fei", 'B'}
    });

    router.defineRoute("hk_1430", "hk_ao", {
        {"hk_cal_laser", 'B'},
        {"hk_forward_retro", 'A'},
        {"hk_ao_fei", 'A'}
    });
    router.defineRoute("hk_1430", "hk_fei", {
        {"hk_cal_laser", 'B'},
        {"hk_forward_retro", 'A'},
        {"hk_ao_fei", 'B'}
    });

    router.defineRoute("hk_cal", "hk_ao", {
        {"hk_cal_laser", 'A'},
        {"hk_ao_fei", 'A'}
    });
    router.defineRoute("hk_cal", "hk_fei", {
        {"hk_cal_laser", 'A'},
        {"hk_ao_fei", 'B'}
    });

    router.defineRoute("hk_laser", "hk_ao", {
        {"hk_cal_laser", 'B'},
        {"hk_ao_fei", 'A'}
    });
    router.defineRoute("hk_laser", "hk_fei", {
        {"hk_cal_laser", 'B'},
        {"hk_ao_fei", 'B'}
    });

    router.defineRoute("hk_mm", "hk_pd", {
        {"hk_mm_sm", 'A'}
    });
    router.defineRoute("hk_sm", "hk_pd", {
        {"hk_mm_sm", 'B'}
    });


    static SemaphoreHandle_t pd_lock     = xSemaphoreCreateMutex();
    static Photodiode yj_photodiode = Photodiode(adc, YJ_PHOTODIODE_CHANNEL, pd_lock);
    static Photodiode hk_photodiode = Photodiode(adc, HK_PHOTODIODE_CHANNEL, pd_lock);

    static std::array<Attenuator, 6> attenuators = {
        Attenuator(dac, 0), Attenuator(dac, 1), Attenuator(dac, 2),
        Attenuator(dac, 3), Attenuator(dac, 4), Attenuator(dac, 5)
    };


    static nmbs_t nanomodbus;
    setup_485(nanomodbus);
    static std::array<MaimanDriver, 6> lasers = {
        MaimanDriver(&nanomodbus, 0), MaimanDriver(&nanomodbus, 1), MaimanDriver(&nanomodbus, 2),
        MaimanDriver(&nanomodbus, 3), MaimanDriver(&nanomodbus, 4), MaimanDriver(&nanomodbus, 5)
    };


    static QueueHandle_t command_queue   = xQueueCreate(10, sizeof(pico_zyre::Command));
    static QueueHandle_t response_queue  = xQueueCreate(10, sizeof(pico_zyre::Response));
    static QueueHandle_t pub_queue       = xQueueCreate(10, sizeof(pico_zyre::PubMessage));


    static HardwareContext ctx = {
        .adc = &adc,
        .attenuators = &attenuators,
        .lasers = &lasers,
        .router = &router,

        .command_in = command_queue,
        .response_out = response_queue,
        .pub_out = pub_queue,
    };

    xTaskCreate(executor_task, "executor", 2048, &ctx, 2, nullptr);
    xTaskCreate(photodiode_task, "pd", 1024, &ctx, 1, nullptr);
    xTaskCreate(coms_task, "coms", 2048, &ctx, 2, nullptr);

    vTaskStartScheduler();
    while (true); // should never reach here
}