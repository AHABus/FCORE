///
/// @file        fcore.h
/// @brief       FCORE - Main flight software header file
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include <stdbool.h>
#include <stdio.h>
#include <fcore/fcore.h>
#include <fcore/buses/uart.h>
#include <fcore/buses/i2c.h>
#include <fcore/tasks/gnss.h>
#include <fcore/tasks/bus.h>
#include <fcore/tasks/controller.h>

FCSystem FCORE;

static void _sendStartMessage(const char* status) {
    
    static char message[64];
    uint16_t length = snprintf(message, 64, "FCORE//SYS_INIT :: %s", status);
    
    RTXPacketHeader header;
    header.payloadID = 0x00;
    header.length = length;
    header.latitude = FCORE.latitude;
    header.longitude = FCORE.longitude;
    header.altitude = FCORE.altitude;
    header.data = (uint8_t*)message;
    
    fcoreRTXEncodePacket(&FCORE.rtxEncoder, &header);
}

void fcore_systemInit(void) {
    FCORE.latitude = 0;
    FCORE.longitude = 0;
    FCORE.altitude = 0;
    FCORE.satellites = 0;
    FCORE.fixQuality = 0;
    
    FCORE.rtxEncoder.sequenceNumber = 0;
    
    FCORE.gnssStatus = STATUS_BOOT;

    i2c_init(5, 4);
    fcore_uartInit(9600);
    fcore_rtxInit(200);
    
    if(!fcore_startControllerTask()) {
        _sendStartMessage("CTRL_STARTFAIL");
        return;
    }
    
    if(!fcore_startGNSSTask()) {
        _sendStartMessage("GNSS_STARTFAIL");
        return;
    }
    
    for(uint8_t i = 0; i < FCORE_PAYLOAD_COUNT; ++i) {
        FCORE.payloads[i] = fcore_payloads[i];
        FCORE.payloads[i].status = STATUS_BOOT;
        if(!fcore_startBusTask(i)) {
            _sendStartMessage("BUS_STARTFAIL");
            return;
        }
    }
    _sendStartMessage("BOOT_DONE");
}

void fcore_systemSigUp(uint8_t index) {
    if(index >= FCORE_PAYLOAD_COUNT) { return; }
    FCORE.payloads[index].status = STATUS_UP;
}

void fcore_systemSigRecovery(uint8_t index) {
    if(index >= FCORE_PAYLOAD_COUNT) { return; }
    FCORE.payloads[index].status = STATUS_RECOVERY;
}

void fcore_systemSigDown(uint8_t index) {
    if(index >= FCORE_PAYLOAD_COUNT) { return; }
    FCORE.payloads[index].status = STATUS_DOWN;
    
}

