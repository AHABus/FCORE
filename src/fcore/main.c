///
/// @file        main.c
/// @brief       FCORE - Main entry point for FCORE flight software
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include "esp/uart.h"
#include "esp8266.h"
#include "espressif/esp_common.h"

#include <fcore/fcore.h>
#include <fcore/buses/uart.h>
#include <fcore/tasks/gnss.h>
#include <fcore/tasks/bus.h>

void user_init(void) {
    //uart_set_baud(1, 200);
    //xTaskCreate(blinkenRegisterTask, "blinkenRegisterTask", 256, NULL, 2, NULL);
    GPIO.ENABLE_OUT_SET = BIT(2);
    gpio_set_iomux_function(2, IOMUX_GPIO2_FUNC_UART1_TXD);
    fcore_initBus();
    fcore_rtxInit(200);
    fcore_startGNSSTask();
    fcore_startBusTask((FCPayload*)&fcore_payloads[0]);
}
