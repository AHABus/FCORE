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
#include <fcore/buses/uart.h>
#include <fcore/buses/i2c.h>
#include <fcore/rtx/RTXEncoder.h>
#include "FreeRTOS.h"
#include "task.h"

static RTXCoder encoder;


#define BUS_REGTX       0x00
#define BUS_REGLEN      0x01
#define BUS_DATA        0x10

static uint8_t busBuffer[FCORE_MAXDATA];

void fcore_busTask(void* parameters) {
    FCPayload* payload = (FCPayload*)parameters;
    
    uint32_t interval = 60;
    uint32_t attempts = 0;
    uint32_t actualInterval = interval;
    
    while(true) {
        vTaskDelay(actualInterval * 1000 / portTICK_PERIOD_MS);
        
        // MARK: Make sure clock stretch is large (~40000us worked here)
        
        // First, write 0xff in the tx flag
        taskENTER_CRITICAL();
        uint8_t start[2] = {BUS_REGTX, 0xff};
        if(!i2c_slave_write(payload->address, start, 2)) {
            goto fail;
        }
        
        // Then read the length;
        uint8_t reg[2];
        if(!i2c_slave_read(payload->address, BUS_REGLEN, reg, 2)) {
            goto fail;
        }
        
        // Check that we have less than the max amount of data
        uint16_t length = (reg[0] << 8) | reg[1];
        if(length > FCORE_MAXDATA) {
            goto fail;
        }
        
        // Read in the data
        if(!i2c_slave_read(payload->address, BUS_DATA, busBuffer, length)) {
            goto fail;
        }
        
        // close the communication
        uint8_t stop[2] = {BUS_REGTX, 0x00};
        if(!i2c_slave_write(payload->address, stop, 2)) {
            goto fail;
        }
        
        RTXPacketHeader header;
        header.payloadID = payload->address;
        header.length = length;
        header.latitude = FCORE.latitude;
        header.longitude = FCORE.longitude;
        header.altitude = FCORE.altitude;
        header.data = busBuffer;

        fcoreRTXEncodePacket(&encoder, &header);
        taskEXIT_CRITICAL();
        
        attempts = 0;
        actualInterval = interval;
        
        continue;
        
    fail:
        taskEXIT_CRITICAL();
        attempts += 1;
        actualInterval *= FCORE_BUS_BACKOFF;
        continue;
    }
}

void fcore_initBus() {
    i2c_init(5, 4);
    encoder.sequenceNumber = 0;
}

void fcore_startBusTask(FCPayload* payload) {
    xTaskCreate(&fcore_busTask, "fcore_bus", 2048,
                (void*)payload, PRIORITY_PAYLOAD, NULL);
}
