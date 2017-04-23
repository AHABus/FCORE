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

/// Sends the [status] start message, originating from subsystem [sys].
static void _sendStartMessage(const char* sys, const char* status) {
    
    static char message[128];
    uint16_t length = snprintf(message, 128, "FCORE//SYS_INIT :: %s (%s)",
                               status, sys);
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
    
    // Initialise the shared data to sane values.
    FCORE.latitude = 0;
    FCORE.longitude = 0;
    FCORE.altitude = 0;
    FCORE.rtxEncoder.sequenceNumber = 0;
    FCORE.gnssStatus = STATUS_BOOT;
    
    // Initialise the hardware buses used by FCORE. Only 200bps is supported
    // for the radio so far.
    // TODO: generate radio BPS from configuration tool.
    i2c_init(5, 4);
    fcore_uartInit(9600);
    fcore_rtxInit(200);
    
    // Start the system health/control task.
    if(!fcore_startControllerTask()) {
        _sendStartMessage("CTRL", "STARTFAIL");
        return;
    }
    
    // Start the GNSS parser task.
    if(!fcore_startGNSSTask()) {
        _sendStartMessage("GNSS", "STARTFAIL");
        return;
    }
    
    // Start a task for each configured payload, and initialise the payloads'
    // status in the shared data struct.
    for(uint8_t i = 0; i < FCORE_PAYLOAD_COUNT; ++i) {
        FCORE.payloads[i] = fcore_payloads[i];
        FCORE.payloads[i].status = STATUS_BOOT;
        
        // We need to check and normalise the priority of each payload
        if(FCORE.payloads[i].priority > 3) { FCORE.payloads[i].priority = 3; }
        if(FCORE.payloads[i].priority < 1) { FCORE.payloads[i].priority = 1; }
        
        if(!fcore_startBusTask(i)) {
            _sendStartMessage(FCORE.payloads[i].name, "BUS_STARTFAIL");
            return;
        }
    }
    
    // If no early returns we have a good boot, send start message.
    _sendStartMessage(FCORE_MISSION_NAME, "BOOT_DONE");
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

