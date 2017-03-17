///
/// @file        gnss.h
/// @brief       FCORE - GNSS// parser for NMEA sentences sent by GNSS modules
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#pragma once
#include <stdbool.h>

typedef enum {
    GNSS_PARSING,
    GNSS_VALID,
    GNSS_OVERFLOW,
    GNSS_INVALID,
    GNSS_NOTGGA,
} GNSSStatus;

extern void fcore_gnssReset();

extern GNSSStatus fcore_gnssFeed(char c);
