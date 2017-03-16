///
/// @file        RTX.h
/// @brief       FCORE - UART buffered driver for RTTY radio link
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include "RTX.h"
#include "common_macros.h"
#include "esp/interrupts.h"
#include "espressif/esp8266/ets_sys.h"
#include "espressif/esp8266/eagle_soc.h"
#include "esp/uart.h"
#include "uart_register.h"

static volatile uint8_t _rtxBuffer[FCORE_RTX_BUFFERSIZE];
static volatile uint32_t _rtxHead;
static volatile uint32_t _rtxTail;

#define RTX_WRITESTATIC(str) fcore_rtxWriteBytes((str), sizeof(str))

#define UART_TX_THRESHOLD 10
#define UART_TX_FIFOSIZE 127 // keep some margin, just in case.
#define UART0            0
#define UART1            1

// Taken from: https://github.com/juhovh/esp-uart/blob/master/esp_uart.c
// Define some helper macros to handle FIFO functions
#define UART_TXFIFO_LEN(uart_no) ((READ_PERI_REG(UART_STATUS(uart_no)) >> UART_TXFIFO_CNT_S) & UART_RXFIFO_CNT)
#define UART_TXFIFO_PUT(uart_no, byte) WRITE_PERI_REG(UART_FIFO(uart_no), (byte) & 0xff)

static uint32_t fcore_rtxAvailable() {
    return ((FCORE_RTX_BUFFERSIZE + _rtxHead - _rtxTail) % FCORE_RTX_BUFFERSIZE) - FCORE_RTX_BUFFERSIZE;
}


static IRAM void _rtxISR() {
    uint32_t status = READ_PERI_REG(UART_INT_ST(UART1));
    if(status & UART_TXFIFO_EMPTY_INT_ST) {
        
        while(UART_TXFIFO_LEN(UART1) < UART_TX_FIFOSIZE && _rtxHead != _rtxTail) {
            UART_TXFIFO_PUT(UART1, _rtxBuffer[_rtxTail]);
            _rtxTail = (_rtxTail + 1) % FCORE_RTX_BUFFERSIZE;
        }
        
        WRITE_PERI_REG(UART_INT_CLR(UART1), 0xffff);
        if(_rtxHead == _rtxTail) {
            CLEAR_PERI_REG_MASK(UART_INT_ENA(UART1), UART_TXFIFO_EMPTY_INT_ENA);
        }
    }
}

void fcore_rtxInit(uint32_t baudRate) {
    _xt_isr_mask(1 << INUM_UART);
    
    SET_PERI_REG_MASK(UART_CONF0(UART1), UART_RXFIFO_RST | UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(UART1), UART_RXFIFO_RST | UART_TXFIFO_RST);
    
    uint32_t clkdiv = APB_CLK_FREQ / baudRate;
    WRITE_PERI_REG(UART_CLKDIV(UART1), clkdiv);
    
    // Configure the TX
    
    // We want to obtain a 8N2 setup to comply with FCORE RTTY reqs.
    SET_PERI_REG_MASK(UART_CONF0(UART1), (3 << UART_STOP_BIT_NUM_S));
    CLEAR_PERI_REG_MASK(UART_CONF0(UART1), UART_PARITY_EN);
    
    WRITE_PERI_REG(UART_CONF1(UART1), (UART_TX_THRESHOLD << UART_TXFIFO_EMPTY_THRHD_S));
    
    WRITE_PERI_REG(UART_INT_ENA(UART0), 0x0000);
    
    WRITE_PERI_REG(UART_INT_CLR(UART1), 0xffff);
    WRITE_PERI_REG(UART_INT_ENA(UART1), 0x0000);
    
    _rtxHead = 0;
    _rtxTail = 0;
    
    _xt_isr_attach(INUM_UART, _rtxISR);
    _xt_isr_unmask(1 << INUM_UART);
    
    fcore_rtxWriteBytes(">> FCORE RTX INIT <<\r\n", 22);
}

void fcore_rtxStop() {
    
}

void fcore_rtxWrite(uint8_t byte) {
    uint32_t nextHead = (_rtxHead + 1) % FCORE_RTX_BUFFERSIZE;
    while(nextHead == _rtxTail){}
    _rtxBuffer[_rtxHead] = byte;
    _rtxHead = nextHead;
    SET_PERI_REG_MASK(UART_INT_ENA(UART1), UART_TXFIFO_EMPTY_INT_ENA);
}

void fcore_rtxWriteBytes(uint8_t* bytes, uint32_t length) {
    //if(length > FCORE_RTX_BUFFERSIZE) { return; }
    while(fcore_rtxAvailable() < length) {}

    _xt_isr_mask(1 << INUM_UART);
    for(uint32_t i = 0; i < length; ++i) {
        uint32_t nextHead = (_rtxHead + 1) % FCORE_RTX_BUFFERSIZE;
        _rtxBuffer[_rtxHead] = bytes[i];
        _rtxHead = nextHead;
    }
    _xt_isr_unmask(1 << INUM_UART);
    SET_PERI_REG_MASK(UART_INT_ENA(UART1), UART_TXFIFO_EMPTY_INT_ENA);
}
