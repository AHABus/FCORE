///
/// @file        tasks/controller.h
/// @brief       FCORE - Task used track up/down payloads and send health msgs.
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include <stdbool.h>
#include <stdint.h>
#include <fcore/fcore.h>
#include <fcore/tasks/controller.h>
#include "FreeRTOS.h"
#include "task.h"

// We need a compact, but readable log message system. (even if the packet
// system fails). So ASCII, with one letter for function, one for index, and
// one character for status:
//
// Functions: G (GNSS), Pi (Payload #i)
// Status:
//      B (booting, set before first contact with the paylaod)
//      U (up, everything okay)
//      R (error, but still attempting recovery)
//      D (down (payload kicked out of bus))
//
// Example item: $G/U,$P5f/D,$P0A/U,$P0B/U,$P10/E
//      GNSS is up,
//      Payload 0x5f is down and done with,
//      Payloads 0x0A and 0x0B are up,
//      Payloads 0x10 failed last time but is still in the circuit.

#define BUFFER_SIZE 100
static uint8_t _statusBuffer[BUFFER_SIZE];
static const char* _statusHeader = "FCORE//SYS_HEALTH :: ";

static inline uint8_t _writePID(uint8_t idx, uint8_t id, uint8_t buffer[BUFFER_SIZE]) {
    if(idx >= (BUFFER_SIZE-5)) { return idx; }
    
    uint8_t high = ((id >> 4) & 0x0f);
    uint8_t low = (id & 0x0f);
    
    buffer[idx++] = 'P';
    buffer[idx++] = (high > 0x09) ? ('A' + (high - 0x0A)) : ('0' + high);
    buffer[idx++] = (low > 0x09) ? ('A' + (low - 0x0A)) : ('0' + low);
    buffer[idx++] = '/';
    
    return idx;
}

// Creates the status string in the buffer and returns its length.
int8_t fcore_createStatus(uint8_t buffer[100]) {
    uint8_t idx = 0;
    
    const char* header = _statusHeader;
    while(*header != '\0') {
        buffer[idx++] = *(header++);
    }
    
    buffer[idx++] = 'G';
    buffer[idx++] = '/';
    
    buffer[idx++] = FCORE.gnssStatus;
    
    for(int i = 0; i < FCORE_PAYLOAD_COUNT; ++i) {
        buffer[idx++] = ',';
        idx = _writePID(idx, FCORE.payloads[i].address, buffer);
        buffer[idx++] = FCORE.payloads[i].status;
    }
    
    return idx;
}

void fcore_controllerTask(void* parameters) {
    
    while(true) {
        vTaskDelay((FCORE_CONT_INTERVAL * 1000) / portTICK_PERIOD_MS);
        
        taskENTER_CRITICAL();

        int8_t length = fcore_createStatus(_statusBuffer);
        RTXPacketHeader header;
        header.payloadID = 0x00;
        header.length = length;
        header.latitude = FCORE.latitude;
        header.longitude = FCORE.longitude;
        header.altitude = FCORE.altitude;
        header.data = _statusBuffer;

        fcoreRTXEncodePacket(&FCORE.rtxEncoder, &header);
        taskEXIT_CRITICAL();
    }
}

bool fcore_startControllerTask(void) {
    return xTaskCreate(&fcore_controllerTask, "fcore_bus", 256,
                       NULL, PRIORITY_CONTROLLER, NULL) == pdPASS;
}