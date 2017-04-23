///
/// @file        tasks/gnss.c
/// @brief       FCORE - Task used for polling GNSS status regularly
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <fcore/fcore.h>
#include <fcore/tasks/gnss.h>
#include <fcore/gnss/gnss_parser.h>
#include <fcore/buses/uart.h>
#include "FreeRTOS.h"
#include "task.h"

// The maximum number of NMEA sentences that will be parsed trying to find a
// GGA one, before the GNSS subsystem is flagged as recovering.
#define GNSS_MAX_ATTEMPTS 4

// The GNSS task routine
void fcore_gnssTask(void* parameters) {
    
    // Main task loop.
    while(true) {
        
        // Clear the UART Rx buffer first, then wait for a few seconds for the
        // GNSS receiver to fill it with sentences.
        fcore_uartClear();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        
        // Parse up to GNSS_MAX_ATTEMPTS NMEA sentences, or until we find a
        // GGA sentence.
        taskENTER_CRITICAL();
        GNSSStatus state;
        for(int i = 0; i < GNSS_MAX_ATTEMPTS; ++i) {
            
            // Reset the GNSS NMEA parser
            state = GNSS_PARSING;
            fcore_gnssReset();
            
            // Get the next sentence from UART0
            while(fcore_uartAvailable() > 0 && state == GNSS_PARSING) {
                state = fcore_gnssFeed(fcore_uartReadChar());
            }
            
            // Found a valid GGA sentence, move to parsing.
            if(state == GNSS_VALID) {
                break;
            }
        }
        
        // Set the status, depending on whether we found a GGA sentence.
        switch(state) {
            case GNSS_VALID:
                FCORE.gnssStatus = STATUS_UP;
                break;
            case GNSS_NOTGGA:
                FCORE.gnssStatus = STATUS_RECOVERY;
                break;
            default:
                FCORE.gnssStatus = STATUS_DOWN;
                break;
        }
        taskEXIT_CRITICAL();
        vTaskDelay((FCORE_GNSS_INTERVAL * 1000) / portTICK_PERIOD_MS);
    }
}

bool fcore_startGNSSTask(void) {
    return xTaskCreate(&fcore_gnssTask, "fcore_gnss", 256,
                       NULL, PRIORITY_GNSS, NULL) == pdPASS;
}
