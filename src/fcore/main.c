///
/// @file        main.c
/// @brief       FCORE - Main entry point for FCORE flight software
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include <stdio.h>
#include <string.h>
#include <fcore/fcore.h>
#include "FreeRTOS.h"

void user_init(void) {
    fcore_systemInit();
}
