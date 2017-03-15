///
/// @file        main.c
/// @brief       FCORE - Main entry point for FCORE flight software
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include "FreeRTOS.h"
#include "task.h"
#include "fcore.h"
#include <stdio.h>
#include <string.h>
#include "rtx/RTX.h"
#include "esp/uart.h"
#include "task.h"
#include "esp8266.h"
#include "espressif/esp_common.h"

static const char* data = "$ABTY TEST BCN 001espressif/esp_common.hespressif/esp_common.hespressif/esp_common.hespressif/esp_common.hespressif/esp_common.hespressif/esp_common.hespressif/esp_common.hespressif/esp_common.hespressif/esp_common.hespressif/esp_common.hespressif/esp_common.hespressif/esp_common.hespressif/esp_common.hespressif/esp_common.hespressif/esp_common.hespressif/esp_common.hespressif/esp_common.h\r\n";

void blinkenRegisterTask(void *pvParameters)
{
    GPIO.ENABLE_OUT_SET = BIT(2);
    //IOMUX_GPIO2 = IOMUX_GPIO2_FUNC_GPIO | IOMUX_PIN_OUTPUT_ENABLE; /* change this line if you change 'gpio' */
    while(1) {

        fcore_rtxInit(200);
        fcore_rtxWriteBytes((uint8_t*)data, strlen(data));
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void user_init(void) {
    gpio_set_iomux_function(2, IOMUX_GPIO2_FUNC_UART1_TXD);
    //uart_set_baud(0, 200);
    //uart_set_baud(1, 200);
    xTaskCreate(blinkenRegisterTask, "blinkenRegisterTask", 256, NULL, 2, NULL);;
    
}
