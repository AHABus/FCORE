///
/// @file        tasks/gnss.c
/// @brief       FCORE - Task used for polling GNSS status regularly
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include <stdio.h>
#include <fcore/fcore.h>
#include <fcore/tasks/gnss.h>
#include <fcore/gnss/gnss_parser.h>
#include <fcore/buses/uart.h>
#include <stdbool.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#define GNSS_MAX_ATTEMPTS 4

void fcore_gnssTask(void* parameters) {
    
    fcore_uartInit(9600);
    fcore_rtxWrite((uint8_t*)"FCORE//GNSS TASK INIT\r\n", strlen("FCORE//GNSS TASK INIT\r\n"));
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    
    while(true) {
        fcore_uartClear();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        
        
        for(int i = 0; i < GNSS_MAX_ATTEMPTS; ++i) {
            
            // Reset the GNSS NMEA parser
            GNSSStatus state = GNSS_PARSING;
            fcore_gnssReset();
            
            // Get the next sentence from UART0
            while(fcore_uartAvailable() > 0 && state == GNSS_PARSING) {
                state = fcore_gnssFeed(fcore_uartReadChar());
            }
            
            if(state == GNSS_VALID) {
                char info[64];
                uint32_t len = snprintf(info, 64, "valid NMEA packet {%d, %d, %d} (%d sats, %d)\r\n",
                                        FCORE.latitude, FCORE.longitude, FCORE.altitude, FCORE.satellites, FCORE.fixQuality);
                fcore_rtxWrite((uint8_t*)info, len);
                break;
            }
        }

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

extern void fcore_startGNSSTask() {
    xTaskCreate(&fcore_gnssTask, "fcore_gnss", 256, NULL, 2, NULL);
}
