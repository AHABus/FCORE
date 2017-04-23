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

/// Resets the GNSS state machine to its start state.
extern void fcore_gnssReset();

/// Feeds [c] to the GNSS state machine and returns its new status.
extern GNSSStatus fcore_gnssFeed(char c);
