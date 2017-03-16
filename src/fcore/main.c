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
#include "fcore/uart.h"
#include "esp/uart.h"
#include "task.h"
#include "esp8266.h"
#include "espressif/esp_common.h"

static const char* welcome = ">> FCORE RTX TESTS\r\n";

void blinkenRegisterTask(void *pvParameters) {
    GPIO.ENABLE_OUT_SET = BIT(2);
    gpio_set_iomux_function(2, IOMUX_GPIO2_FUNC_UART1_TXD);
    
    fcore_rtxInit(200);
    fcore_uartInit(9600);
    
    fcore_rtxWrite((uint8_t*)welcome, strlen(welcome));
    
    static uint8_t rx[512];
    while(1) {
        
        uint16_t len = fcore_uartRead(rx, 512);
        if(len > 0) {
            fcore_rtxWrite((uint8_t*)"rcvd: ", 6);
            fcore_rtxWrite(rx, len);
            fcore_rtxWrite((uint8_t*)"\r\n", 2);
        }
        else {
            fcore_rtxWrite((uint8_t*)"nothing to display\r\n", strlen("nothing to display\r\n"));
        }
        
        //fcore_rtxWrite("Hello\r\n", 7);
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void user_init(void) {
    //uart_set_baud(1, 200);
    xTaskCreate(blinkenRegisterTask, "blinkenRegisterTask", 256, NULL, 2, NULL);;
    
}
