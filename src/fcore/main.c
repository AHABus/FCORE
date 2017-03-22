///
/// @file        main.c
/// @brief       FCORE - Main entry point for FCORE flight software
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"

#include <fcore/fcore.h>

void user_init(void) {
    fcore_systemInit();
}
