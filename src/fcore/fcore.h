///
/// @file        fcore.h
/// @brief       FCORE - Main flight software header file
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#pragma once
#include <stdint.h>
#include <fcore/rtx/RTXEncoder.h>

/// The interval at which FCORE update its internal location using the GNSS
/// receiver.
#define FCORE_GNSS_INTERVAL     20

/// The amount a time a payload can fail to respond before it is taken off
/// of the software bus.
#define FCORE_BUS_MAXATTEMPTS   4

/// The exponential backoff factor for the software bus. Every time a payload
/// fails to respond, the delay until the next communication attempt is
/// multiplied by the backoff factor.
#define FCORE_BUS_BACKOFF       1.2

/// The maximum number of bytes a payload is allowed to return at once. 
/// Returning more than [FCORE_BUS_MAXDATA] is treated as an error, and the
/// partial received data is discarded.
#define FCORE_BUS_MAXDATA       512

/// The interval at which the controller task runs and sends health messages.
#define FCORE_CONT_INTERVAL     30

typedef enum {
    STATUS_BOOT      = 'B',
    STATUS_UP        = 'U',
    STATUS_RECOVERY  = 'R',
    STATUS_DOWN      = 'D',
} FCStatus;

typedef enum {
    PRIORITY_CONTROLLER,
    PRIORITY_GNSS,
    PRIORITY_PAYLOADBUS,
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


typedef struct {
    int32_t         latitude;
    int32_t         longitude;
    uint16_t        altitude;
    uint8_t         satellites;
    uint8_t         fixQuality;
    
    RTXCoder        rtxEncoder;
    
    FCStatus        gnssStatus;
    FCPayload       payloads[FCORE_PAYLOAD_COUNT];
} FCSystem;

extern FCSystem FCORE;

extern void fcore_systemInit(void);

extern void fcore_systemSigUp(uint8_t index);

extern void fcore_systemSigRecovery(uint8_t index);

extern void fcore_systemSigDown(uint8_t index);
