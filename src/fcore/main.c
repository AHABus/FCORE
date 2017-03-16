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

static const char* data = "Aldrin got involved in 2016 when 8i was working on a NASA-funded project that was testing emotional reaction to 3D projections sent to isolated astronauts. (Remember the scene in Interstellar where Matthew McConaughey gets all those videos from his family?) Aldrin ended up being the test ambassador to the astronauts; eventually, the idea will be to send 3D messages from Earth to astronauts in orbit.\r\n\r\n\r\n\r\n\r\n";

void blinkenRegisterTask(void *pvParameters) {
    gpio_set_iomux_function(2, IOMUX_GPIO2_FUNC_UART1_TXD);
    gpio_set_iomux_function(14, IOMUX_GPIO14_FUNC_GPIO);
    GPIO.ENABLE_OUT_SET = BIT(14);
    fcore_rtxInit(200);
    while(1) {
        gpio_write(14, 1);
        fcore_rtxWriteBytes((uint8_t*)data, strlen(data));
        gpio_write(14, 0);
        vTaskDelay(30000 / portTICK_PERIOD_MS);
    }
}

void user_init(void) {
    //uart_set_baud(1, 200);
    xTaskCreate(blinkenRegisterTask, "blinkenRegisterTask", 256, NULL, 2, NULL);;
    
}
