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

#define BUS_REGTX       0x00
#define BUS_REGLEN      0x01
#define BUS_DATA        0x10

static uint8_t busBuffer[FCORE_BUS_MAXDATA];

// Opens the bus connection with a payload (write 0xFF in REG_TX).
static inline void closeBus(uint8_t address) {
    static uint8_t stop[2] = {BUS_REGTX, 0x00};
    i2c_slave_write(address, stop, 2);
}

// Close the bus connection with a payload (write 0x00 in REG_TX).
static inline bool openBus(uint8_t address) {
    static uint8_t start[2] = {BUS_REGTX, 0xff};
    return i2c_slave_write(address, start, 2);
}

// The Payload Bus task routine
void fcore_busTask(void* parameters) {
    
    // Initialise the bus task
    uint8_t payloadID = (int)parameters;
    FCPayload* payload = &FCORE.payloads[payloadID];
    uint32_t interval = FCORE_BUS_INTERVAL * payload->priority;
    
    // We want to avoid all of the tasks firing on "resonnant" intervals - this
    // would cause big spikes in packet creation. To avoid that, delay the first
    // polling of each payload, so that a payload is first polled every
    // FCORE_BUS_INTERVAL / FCORE_PAYLOAD_COUNT seconds after boot.
    vTaskDelay((((payloadID+1) * (FCORE_BUS_INTERVAL / FCORE_PAYLOAD_COUNT))
                * 1000) / portTICK_PERIOD_MS);
    
    // Main task loop.
    while(true) {
        
        // First, write 0xff in the tx flag
        taskENTER_CRITICAL();
        if(!openBus(payload->address)) {
            goto down;
        }
        
        // Then read the length;
        uint8_t reg[2];
        if(!i2c_slave_read(payload->address, BUS_REGLEN, reg, 2)) {
            goto down;
        }
        
        // Check that we have less than the max amount of data
        uint16_t length = (reg[0] << 8) | reg[1];
        if(length > FCORE_BUS_MAXDATA) {
            goto recovering;
        }
        
        // If there's no data, let's not create a packet.
        if(length == 0) {
            goto success;
        }
        
        // Read in the data. We read in 32-byte chunks to ensure that devices
        // with small I2C buffers (Arduino boards for example) can send
        // everything. If any transfer goes wrong, the payload is marked as
        // down.
        uint16_t readBytes = 0;
        while(readBytes < length) {
            uint16_t chunkSize = length - readBytes;
            if(chunkSize <= 0) { break; }
            if(chunkSize > 32) { chunkSize = 32; }
            if(!i2c_slave_read(payload->address,
                               BUS_DATA + readBytes,
                               busBuffer + readBytes,
                               chunkSize)) {
                goto down;
            }
            readBytes += chunkSize;
        }
        
        // Finally, we send the recovered data in a packet.
        RTXPacketHeader header;
        header.payloadID = payload->address;
        header.length = length;
        header.latitude = FCORE.latitude;
        header.longitude = FCORE.longitude;
        header.altitude = FCORE.altitude;
        header.data = busBuffer;

        fcoreRTXEncodePacket(&FCORE.rtxEncoder, &header);
        
    success:
        fcore_systemSigUp(payloadID);
        goto end;
        
    down:
        fcore_systemSigDown(payloadID);
        goto end;
    
    recovering:
        fcore_systemSigRecovery(payloadID);
        goto end;
        
    end:
        // Whatever the outcome of the transfer, we attempt to clear the
        // connection flag on the payload to allow it to write to its data
        // registers
        closeBus(payload->address);
        taskEXIT_CRITICAL();
        vTaskDelay((interval * 1000) / portTICK_PERIOD_MS);
    }
}

bool fcore_startBusTask(uint8_t payloadIDX) {
    return xTaskCreate(&fcore_busTask, "fcore_bus", 512, (void*)((int)payloadIDX),
                       PRIORITY_PAYLOADBUS, NULL) == pdPASS;
}
