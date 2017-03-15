///
/// @file        RTX.h
/// @brief       FCORE - UART buffered driver for RTTY radio link
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#pragma once
#include <stdint.h>

#define FCORE_RTX_BUFFERSIZE    2048

void fcore_rtxInit(uint16_t baudRate);

void fcore_rtxStop();

void fcore_rtxWrite(uint8_t byte);

void fcore_rtxWriteBytes(uint8_t* bytes, uint16_t length);

