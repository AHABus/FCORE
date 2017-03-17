///
/// @file        tasks/gnss.c
/// @brief       FCORE - Task used for polling GNSS status regularly
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include <fcore/fcore.h>
#include <fcore/tasks/gnss.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"

void fcore_gnssTask(void* parameters) {
    while(true) {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

extern void fcore_startGNSSTask() {
    xTaskCreate(&fcore_gnssTask, "fcore_gnss", 256, NULL, 2, NULL);
}
