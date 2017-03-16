///
/// @file        fcore.h
/// @brief       FCORE - Main flight software header file
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#pragma once
#include <stdint.h>

/// An entry in the payload table.
typedef struct {
    /// The payload's address on the I2C FCORE Bus.
    uint8_t         address;
    /// The payload's name. Used mostly for debugging purposes.
    const char*     name;
    /// The payload's priority.
    uint8_t         priority;
} FCPayload;


typedef enum {
    PRIORITY_IDLE,
    PRIORITY_GNSS,
    PRIORITY_PAYLOAD,
} FCPriority;

#define FCORE_MAIN_INCLUDE
#include "__fcore_cfg.h"
#undef FCORE_MAIN_INCLUDE
