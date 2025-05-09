//
// Created by Jeb Bailey on 4/21/25.
//
#include <stdio.h>
#include <string>
#include <array>
#include <iostream>

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"



#include "photodiode_task.h"
#include "executor_task.h"
#include "pico_zyre.h"
#include "coms_task.h"
// #include "photodiode.h"
#include "attenuator.h"
#include "semphr.h"
#include "dacx578.h"
#include "mems_switching.h"
#include "hardware_context.h"
#include "nanomodbus.h"
#include "maiman.h"
#include "log_util.h"

#include "wizchip_conf.h"
extern "C" {
#include "wizchip_spi.h"
}



#define PLL_SYS_KHZ (150 * 1000)  //wiz examples seem to set to 133  MHz

#define configENABLE_MPU                        0
#define configENABLE_TRUSTZONE                  0
#define configRUN_FREERTOS_SECURE_ONLY          1
#define configENABLE_FPU                        1
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    16

// #define configSUPPORT_STATIC_ALLOCATION         0
// #define configSUPPORT_DYNAMIC_ALLOCATION        1
// #define configUSE_PREEMPTION                    1
// #define configUSE_IDLE_HOOK                     0
// #define configUSE_TICK_HOOK                     0
#define configCPU_CLOCK_HZ                      ( PLL_SYS_KHZ * 1000 )
#define configTICK_RATE_HZ                      ( 1000 )
#define configMAX_PRIORITIES                    ( 5 )
#define configMINIMAL_STACK_SIZE                ( 128 )
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1



#define YJ_PHOTODIODE_CHANNEL 0
#define HK_PHOTODIODE_CHANNEL 1

// #define UART0_TX_PIN 0
// #define UART0_RX_PIN 1
// #define UART1_TX_PIN 4
// #define UART1_RX_PIN 5
// #define MB_UART_ID uart1
// #define MB_UART_BAUDRATE 115200


// Define UART pins and settings
// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_ODD

#define UART_TX_PIN 8
#define UART_RX_PIN 9
// #define UART_DE_PIN 2    // Data Enable pin for RS485

#define I2C0_SDA_PIN 4
#define I2C0_SCK_PIN 5

#define I2C1_SDA_PIN 2
#define I2C1_SCK_PIN 3

#define POWER_PIN 6

#define DAC_I2C_ADDRESS 0x4C
#define ADC_I2C_ADDRESS 0x48
#define GPIOEXP_I2C_ADDRESS 0x20


SemaphoreHandle_t global_log_mutex;


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
    // gpio_init(UART_DE_PIN);
    // gpio_set_dir(UART_DE_PIN, GPIO_OUT);
    // gpio_put(UART_DE_PIN, 0);    // Set DE low to enable receiving initially


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


void measure_freqs(void) {
    uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
    uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
    uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
    uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
    uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
    uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
#ifdef CLOCKS_FC0_SRC_VALUE_CLK_RTC
    uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);
#endif

    printf("pll_sys  = %dkHz", f_pll_sys);
    printf("pll_usb  = %dkHz", f_pll_usb);
    printf("rosc     = %dkHz", f_rosc);
    printf("clk_sys  = %dkHz", f_clk_sys);
    printf("clk_peri = %dkHz", f_clk_peri);
    printf("clk_usb  = %dkHz", f_clk_usb);
    printf("clk_adc  = %dkHz", f_clk_adc);
#ifdef CLOCKS_FC0_SRC_VALUE_CLK_RTC
    printf("clk_rtc  = %dkHz\n", f_clk_rtc);
#endif

    // Can't measure clk_ref / xosc as it is the ref
}

int main() {
    stdio_init_all();

    printf("Setting up pico2...\n");

    measure_freqs();

    // printf("Configuring clocks...\n");
    // // set a system clock frequency in khz
    // set_sys_clock_khz(PLL_SYS_KHZ, true);
    //
    // // configure the specified clock
    // clock_configure(
    //     clk_peri,
    //     0,                                                // No glitchless mux
    //     CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
    //     PLL_SYS_KHZ * 1000,                               // Input frequency
    //     PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    // );
    // measure_freqs();

    printf("Setting up w5500...");
    wizchip_spi_initialize();
    printf("spi...");
    wizchip_cris_initialize();
    printf("cris..n");
    wizchip_reset();
    printf("reset...");
    wizchip_initialize();
    printf("init...");
    wizchip_check();
    printf("check.\n");

    printf("Setting up IO...");

    // Static hardware and messaging setup
    gpio_init(POWER_PIN);
    gpio_put(POWER_PIN, 0);
    gpio_set_dir(POWER_PIN, GPIO_OUT);

    // init I2c pins and interface
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCK_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCK_PIN);
    i2c_init(i2c0, 400 * 1000);

    gpio_set_function(I2C1_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SCK_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_SDA_PIN);
    gpio_pull_up(I2C1_SCK_PIN);
    i2c_init(i2c1, 400 * 1000);

    // gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_NULL);
    // gpio_set_function(I2C0_SCK_PIN, GPIO_FUNC_NULL);
    // i2c_deinit(_i2c);

    printf("done\n");

    static DacX578 dac(i2c1, DAC_I2C_ADDRESS, 12);
    dac.begin();
    printf("DAC initialized\n");

    static PICO_ADS1115 adc;
    if (!adc.beginADSX(PICO_ADS1115::ADSX_ADDRESS_GND, i2c0, 50000)) {
        printf("ADC Failure \r\n");
    } else {
        printf("ADC initialized\n");
    }

    static PCAL6416A pcagpio(i2c1, 0x21);
    printf("GPIO Expander initialized\n");

    static std::array<MEMSSwitch, 8> switches = {
        MEMSSwitch(pcagpio, 0, 1),
        MEMSSwitch(pcagpio, 2, 3),
        MEMSSwitch(pcagpio, 4, 5),
        MEMSSwitch(pcagpio, 6, 7),
        MEMSSwitch(pcagpio, 8, 9),
        MEMSSwitch(pcagpio, 10, 11),
        MEMSSwitch(pcagpio, 12, 13),
        MEMSSwitch(pcagpio, 14, 15)};

    printf("MEMS Switches initialized\n");

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

    printf("MEMS Switch names initialized\n");

    static MEMSRouter router(named_switches, std::size(named_switches));

    printf("MEMS Router initialized\n");

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

    printf("Lightpath routes defined\n");

    // static SemaphoreHandle_t pd_lock     = xSemaphoreCreateMutex();
    // static Photodiode yj_photodiode = Photodiode(adc, YJ_PHOTODIODE_CHANNEL, pd_lock);
    // static Photodiode hk_photodiode = Photodiode(adc, HK_PHOTODIODE_CHANNEL, pd_lock);

    printf("Photodiodes\n");

    static std::array<Attenuator, 6> attenuators = {
        Attenuator(dac, 0), Attenuator(dac, 1), Attenuator(dac, 2),
        Attenuator(dac, 3), Attenuator(dac, 4), Attenuator(dac, 5)
    };

    printf("Attenuators\n");


    static nmbs_t nanomodbus;
    setup_485(nanomodbus);
    static std::array<MaimanDriver, 6> lasers = {
        MaimanDriver(&nanomodbus, 0), MaimanDriver(&nanomodbus, 1), MaimanDriver(&nanomodbus, 2),
        MaimanDriver(&nanomodbus, 3), MaimanDriver(&nanomodbus, 4), MaimanDriver(&nanomodbus, 5)
    };

    printf("Maiman Drivers\n");

    static QueueHandle_t command_queue   = xQueueCreate(10, sizeof(pico_zyre::Command));
    static QueueHandle_t response_queue  = xQueueCreate(10, sizeof(pico_zyre::Response));
    static QueueHandle_t pub_queue       = xQueueCreate(10, sizeof(pico_zyre::PubMessage));


    global_log_mutex = xSemaphoreCreateMutex();
    static HardwareContext ctx = {
        .adc = &adc,
        .attenuators = attenuators,
        .lasers = lasers,
        .router = &router,

        .command_in = command_queue,
        .response_out = response_queue,
        .pub_out = pub_queue,
        .log_mutex = &global_log_mutex
    };

    printf("Creating tasks...\n");

    xTaskCreate(executor_task, "executor", 2048, &ctx, 2, nullptr);
    xTaskCreate(photodiode_task, "photodiode", 1024, &ctx, 1, nullptr);
    xTaskCreate(coms_task, "coms", 2048, &ctx, 2, nullptr);


    printf("Starting Scheduler\n");
    vTaskStartScheduler();
    while (true); // should never reach here
}