///
/// @file        tasks/bus.h
/// @brief       FCORE - Task used to communicate with AHABus payloads.
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#pragma once
#include <fcore/fcore.h>

extern void fcore_initBus();

extern void fcore_startBusTask(FCPayload* payload);
