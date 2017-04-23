///
/// @file        tasks/bus.h
/// @brief       FCORE - Task used to communicate with AHABus payloads.
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#pragma once
#include <fcore/fcore.h>

/// Creates a bus task for the payload at [payloadIDX].
extern bool fcore_startBusTask(uint8_t payloadIDX);
