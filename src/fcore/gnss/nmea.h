///
/// @file        nmea.c
/// @brief       FCORE - GNSS// parser for NMEA sentences sent by GNSS modules
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#pragma once
#include <stdbool.h>

typedef enum {
    NMEA_PARSING,
    NMEA_VALID,
    NMEA_OVERFLOW,
    NMEA_INVALID,
} NMEAStatus;

void fcore_nmeaInit();

NMEAStatus fcore_nmeaFeed(char c);
