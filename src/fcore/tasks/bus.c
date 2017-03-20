///
/// @file        tasks/bus.c
/// @brief       FCORE - Task used to communicate with AHABus payloads.
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <fcore/fcore.h>
#include <fcore/tasks/bus.h>
#include <fcore/rtx/RTXEncoder.h>
#include "FreeRTOS.h"
#include "task.h"

static RTXCoder encoder;

static const char* testData = "Hello, world. This is in a packet!";


void fcore_busTask(void* parameters) {
    FCPayload* payload = (FCPayload*)parameters;
    
    uint32_t interval = 60;
    //uint32_t attempts = 0;
    //uint32_t actualInterval = interval;
    
    while(true) {
        vTaskDelay(interval * 1000 / portTICK_PERIOD_MS);

        RTXPacketHeader header;
        header.payloadID = 22;
        header.length = strlen(testData);

        header.latitude = FCORE.latitude;
        header.longitude = FCORE.longitude;
        header.altitude = FCORE.altitude;
        
        header.data = (const uint8_t*)testData;
        
        taskENTER_CRITICAL();
        fcoreRTXEncodePacket(&encoder, &header);
        taskEXIT_CRITICAL();
    }
}

void fcore_initBus() {
    encoder.sequenceNumber = 0;
}

void fcore_startBusTask(FCPayload* payload) {
    xTaskCreate(&fcore_busTask, "fcore_bus", 1024,
                (void*)payload, PRIORITY_PAYLOAD, NULL);
}
