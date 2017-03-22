///
/// @file        tasks/bus.c
/// @brief       FCORE - Task used to communicate with AHABus payloads.
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include <stdio.h>
#include <stdbool.h>
#include <fcore/fcore.h>
#include <fcore/tasks/bus.h>
#include <fcore/buses/uart.h>
#include <fcore/buses/i2c.h>
#include <fcore/rtx/RTXEncoder.h>
#include "FreeRTOS.h"
#include "task.h"

#define BUS_REGTX       0x00
#define BUS_REGLEN      0x01
#define BUS_DATA        0x10

static uint8_t busBuffer[FCORE_BUS_MAXDATA];

void fcore_busTask(void* parameters) {
    uint8_t payloadID = (int)parameters;
    FCPayload* payload = &FCORE.payloads[payloadID];
    
    uint32_t interval = 30 * payload->priority;
    uint32_t attempts = 0;
    
    while(true) {
        vTaskDelay((interval * 1000) / portTICK_PERIOD_MS);
        
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
        if(length > FCORE_BUS_MAXDATA) {
            goto fail;
        }
        
        // Read in the data
        uint16_t readBytes = 0;
        while(readBytes < length) {
            uint8_t chunkSize = length - readBytes;
            if(chunkSize <= 0) { break; }
            if(chunkSize > 32) { chunkSize = 32; }
            if(!i2c_slave_read(payload->address,
                               BUS_DATA + readBytes,
                               busBuffer + readBytes,
                               chunkSize)) {
                goto fail;
            }
            readBytes += chunkSize;
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

        fcoreRTXEncodePacket(&FCORE.rtxEncoder, &header);
        taskEXIT_CRITICAL();
        
        attempts = 0;
        fcore_systemSigUp(payloadID);
        continue;
        
    fail:
        taskEXIT_CRITICAL();
        attempts += 1;
        if(attempts >= FCORE_BUS_MAXATTEMPTS) {
            fcore_systemSigDown(payloadID);
            vTaskDelete(NULL);
        } else {
            fcore_systemSigRecovery(payloadID);
        }
    }
}

bool fcore_startBusTask(uint8_t payloadIDX) {
    return xTaskCreate(&fcore_busTask, "fcore_bus", 512, (void*)((int)payloadIDX),
                       PRIORITY_PAYLOADBUS, NULL) == pdPASS;
}
