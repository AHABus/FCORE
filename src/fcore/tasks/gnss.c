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

void fcore_gnssTask(void* parameters) {
    
    fcore_uartInit(9600);
    fcore_rtxWrite((uint8_t*)"FCORE//GNSS TASK INIT\r\n", strlen("FCORE//GNSS TASK INIT\r\n"));
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    
    while(true) {
        fcore_uartClear();
        fcore_gnssReset();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        
        GNSSStatus state = GNSS_PARSING;
        while(fcore_uartAvailable() > 0 && state == GNSS_PARSING) {
            state = fcore_gnssFeed(fcore_uartReadChar());
        }
        static char info[64];
        uint32_t len = 0;
        switch(state) {
        case GNSS_VALID:
            len = snprintf(info, 64, "valid NMEA packet {%d, %d, %d} (%d sats, %d)\r\n",
                           FCORE.latitude, FCORE.longitude, FCORE.altitude, FCORE.satellites, FCORE.fixQuality);
            fcore_rtxWrite((uint8_t*)info, len);
            break;
        case GNSS_NOTGGA:
            fcore_rtxWrite((uint8_t*)"Not a GGA packet\r\n", strlen("Not a GGA packet\r\n"));
            break;
        default:
            fcore_rtxWrite((uint8_t*)"Invalid NMEA packet\r\n", strlen("Invalid NMEA packet\r\n"));
            break;
        }

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

extern void fcore_startGNSSTask() {
    xTaskCreate(&fcore_gnssTask, "fcore_gnss", 256, NULL, 2, NULL);
}
