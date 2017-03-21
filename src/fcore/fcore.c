///
/// @file        fcore.h
/// @brief       FCORE - Main flight software header file
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include <fcore/fcore.h>
#include <fcore/buses/uart.h>
#include <fcore/buses/i2c.h>
#include <fcore/tasks/gnss.h>
#include <fcore/tasks/bus.h>
#include <fcore/tasks/controller.h>

FCSystem FCORE;

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
    fcore_startGNSSTask();
    fcore_startControllerTask();
    
    for(uint8_t i = 0; i < FCORE_PAYLOAD_COUNT; ++i) {
        FCORE.payloads[i] = fcore_payloads[i];
        FCORE.payloads[i].status = STATUS_BOOT;
        fcore_startBusTask(i);
    }
    
    // TODO: send boot message.
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

