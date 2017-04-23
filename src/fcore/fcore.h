///
/// @file        fcore.h
/// @brief       FCORE - Main flight software header file
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#pragma once
#include <stdint.h>
#include <fcore/rtx/RTXEncoder.h>

// If defined, this enables the radio debug mode, which corrupts random bytes
//#define FCORE_RTX_DEBUG
#define FCORE_RTX_DEBUGRATE     (1.0-(18.0/256.0))

/// The interval at which FCORE update its internal location using the GNSS
/// receiver.
#define FCORE_GNSS_INTERVAL     20

/// The interval at which the controller task runs and sends health messages.
#define FCORE_CONT_INTERVAL     120

/// Payload statuses - simple to use direct char values than lookup tables.
typedef enum {
    STATUS_BOOT      = 'B',
    STATUS_UP        = 'U',
    STATUS_RECOVERY  = 'R',
    STATUS_DOWN      = 'D',
} FCStatus;

/// The FreeRTOS priorities for each task - not the same as payload priorities.
typedef enum {
    PRIORITY_PAYLOADBUS = 1,
    PRIORITY_GNSS       = 5,
    PRIORITY_CONTROLLER = 10,
} FCPriority;

/// An entry in the payload table.
typedef struct {
    /// The payload's address on the I2C FCORE Bus.
    uint8_t         address;
    /// The payload's name. Used mostly for debugging purposes.
    const char*     name;
    /// The payload's priority.
    uint8_t         priority;
    /// The payload's status
    FCStatus        status;
} FCPayload;

#define FCORE_MAIN_INCLUDE
#include "__fcore_cfg.h"
#undef FCORE_MAIN_INCLUDE

/// A struct that holds the shared data required for every part of FCORE to run
typedef struct {
    /// The last known GNSS co-ordinates, times 10000.
    int32_t         latitude;
    int32_t         longitude;
    /// The last known altitude of the platform.
    uint16_t        altitude;
    
    /// The Radio encoder, used to keep track of the last frame sequence number.
    RTXCoder        rtxEncoder;
    
    /// Last known statuses of FCORE components.
    FCStatus        gnssStatus;
    FCPayload       payloads[FCORE_PAYLOAD_COUNT];
} FCSystem;

/// The shared system data.
extern FCSystem FCORE;

/// Starts FCORE -- boots FreeRTOS tasks and sends the system boot packet.
extern void fcore_systemInit(void);

/// Mark the payload at [index] as Up.
extern void fcore_systemSigUp(uint8_t index);

/// Mark the payload at [index] as Recovering.
extern void fcore_systemSigRecovery(uint8_t index);

/// Mark the payload at [index] as Down.
extern void fcore_systemSigDown(uint8_t index);
