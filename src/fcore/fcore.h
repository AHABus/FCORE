///
/// @file        fcore.h
/// @brief       FCORE - Main flight software header file
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#pragma once
#include <stdint.h>

/// The amount a time a payload can fail to respond before it is taken off
/// of the software bus.
#define FCORE_BUS_MAXATTEMPTS   2

/// The exponential backoff factor for the software bus. Every time a payload
/// fails to respond, the delay until the next communication attempt is
/// multiplied by the backoff factor.
#define FCORE_BUS_BACKOFF       1.2

/// An entry in the payload table.
typedef struct {
    /// The payload's address on the I2C FCORE Bus.
    uint8_t         address;
    /// The payload's name. Used mostly for debugging purposes.
    const char*     name;
    /// The payload's priority.
    uint8_t         priority;
} FCPayload;

typedef struct {
    int32_t         latitude;
    int32_t         longitude;
    uint16_t        altitude;
    uint8_t         satellites;
    uint8_t         fixQuality;
} FCSystem;

typedef enum {
    PRIORITY_IDLE,
    PRIORITY_GNSS,
    PRIORITY_PAYLOAD,
} FCPriority;

extern FCSystem FCORE;

#define FCORE_MAIN_INCLUDE
#include "__fcore_cfg.h"
#undef FCORE_MAIN_INCLUDE
