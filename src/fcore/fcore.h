///
/// @file        fcore.h
/// @brief       FCORE - Main flight software header file
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#pragma once
#include <stdint.h>

typedef struct {
    uint8_t         address;
    const char*     name;
    uint8_t         priority;
} FCPayload;

#define FCORE_MAIN_INCLUDE
#include "__fcore_cfg.h"
#undef FCORE_MAIN_INCLUDE
