///
/// @file        tasks/gnss.c
/// @brief       FCORE - Task used for polling GNSS status regularly
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include <fcore/fcore.h>
#include <fcore/tasks/gnss.h>
#include <fcore/gnss/nmea.h>
#include <fcore/buses/uart.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"

void fcore_gnssTask(void* parameters) {
    
    fcore_uartInit(9600);
    
    while(true) {
        fcore_uartClear();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        
        NMEAStatus state = NMEA_PARSING;
        while(fcore_uartAvailable() > 0 && state == NMEA_PARSING) {
            state = fcore_nmeaFeed(fcore_uartReadChar());
        }
        if(state == NMEA_VALID) {
            fcore_rtxWrite((uint8_t*)"Valid NMEA packet\r\n", 19);
        } else {
            fcore_rtxWrite((uint8_t*)"Invalid NMEA packet\r\n", 21);
        }

        vTaskDelay(30000 / portTICK_PERIOD_MS);
    }
}

extern void fcore_startGNSSTask() {
    xTaskCreate(&fcore_gnssTask, "fcore_gnss", 256, NULL, 2, NULL);
}
