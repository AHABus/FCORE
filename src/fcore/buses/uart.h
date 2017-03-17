///
/// @file        uart.h
/// @brief       FCORE - UART buffered driver for RTTY and UART0
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#pragma once
#include <stdint.h>

void fcore_rtxInit(uint16_t baudRate);
void fcore_uartInit(uint16_t baudRate);

void fcore_rtxStop();
void fcore_uartStop();

void fcore_rtxWrite(const uint8_t* bytes, uint16_t length);
void fcore_uartWrite(const uint8_t* bytes, uint16_t length);

uint16_t fcore_uartAvailable();
uint16_t fcore_uartRead(uint8_t* bytes, uint16_t length);
char fcore_uartReadChar();

void fcore_uartClear();
