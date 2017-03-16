///
/// @file        uart.c
/// @brief       FCORE - UART buffered driver for RTTY and UART0
/// @author      Amy Parent
/// @copyright   2017 Amy Parent
///
#include <stdbool.h>
#include "uart.h"
#include "common_macros.h"
#include "esp/interrupts.h"
#include "espressif/esp8266/ets_sys.h"
#include "espressif/esp8266/eagle_soc.h"
#include "esp/uart.h"
#include "uart_register.h"


typedef struct {
    uint8_t data[FCORE_RTX_BUFFERSIZE];
    uint16_t head;
    uint16_t tail;
} FCBuffer;

static volatile FCBuffer _u1TX;
static volatile FCBuffer _u0TX;
static volatile FCBuffer _u0RX;

#define UART_TX_THRESHOLD   2
#define UART_TX_FIFOSIZE    127 // keep some margin, just in case.
#define UART0               0
#define UART1               1

// Taken from: https://github.com/juhovh/esp-uart/blob/master/esp_uart.c
// Define some helper macros to handle FIFO functions
#define UART_TXFIFO_LEN(uart_no) ((READ_PERI_REG(UART_STATUS(uart_no)) >> UART_TXFIFO_CNT_S) & UART_RXFIFO_CNT)
#define UART_TXFIFO_PUT(uart_no, byte) WRITE_PERI_REG(UART_FIFO(uart_no), (byte) & 0xff)

static uint32_t fcore_rtxAvailable() {
    return ((FCORE_RTX_BUFFERSIZE + _u1TX.head - _u1TX.tail) % FCORE_RTX_BUFFERSIZE)
           - FCORE_RTX_BUFFERSIZE;
}

static IRAM void _uartISR() {
    uint32_t u0st = READ_PERI_REG(UART_INT_ST(UART0));
    uint32_t u1st = READ_PERI_REG(UART_INT_ST(UART1));
    
    // Handle transmission for UART0
    if(u0st & UART_TXFIFO_EMPTY_INT_ST) {
        
        while(UART_TXFIFO_LEN(UART0) < UART_TX_FIFOSIZE && _u0TX.head != _u0TX.tail) {
            UART_TXFIFO_PUT(UART1, _u1TX.data[_u0TX.tail]);
            _u1TX.tail = (_u0TX.tail + 1) % FCORE_RTX_BUFFERSIZE;
        }
        
        WRITE_PERI_REG(UART_INT_CLR(UART0), UART_TXFIFO_EMPTY_INT_CLR);
        if(_u0TX.head == _u0TX.tail) {
            CLEAR_PERI_REG_MASK(UART_INT_ENA(UART0), UART_TXFIFO_EMPTY_INT_ENA);
        }
    }
    
    // Handle transmission for UART1
    if(u1st & UART_TXFIFO_EMPTY_INT_ST) {
        
        while(UART_TXFIFO_LEN(UART1) < UART_TX_FIFOSIZE && _u1TX.head != _u1TX.tail) {
            UART_TXFIFO_PUT(UART1, _u1TX.data[_u1TX.tail]);
            _u1TX.tail = (_u1TX.tail + 1) % FCORE_RTX_BUFFERSIZE;
        }
        
        WRITE_PERI_REG(UART_INT_CLR(UART1), UART_TXFIFO_EMPTY_INT_CLR);
        if(_u1TX.head == _u1TX.tail) {
            CLEAR_PERI_REG_MASK(UART_INT_ENA(UART1), UART_TXFIFO_EMPTY_INT_ENA);
        }
    }
}


static bool _uartInit = false;

static void _uartReset() {
    if(_uartInit) { return; }
    _uartInit = true;
    
    // Reset both buffers for UART0 and UART1
    SET_PERI_REG_MASK(UART_CONF0(UART1), UART_TXFIFO_RST | UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(UART1), UART_TXFIFO_RST | UART_TXFIFO_RST);
    
    SET_PERI_REG_MASK(UART_CONF0(UART0), UART_RXFIFO_RST | UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(UART0), UART_RXFIFO_RST | UART_TXFIFO_RST);
    SET_PERI_REG_MASK(UART_CONF0(UART1), UART_TXFIFO_RST | UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(UART1), UART_TXFIFO_RST | UART_TXFIFO_RST);
    
    // Disable and clear all interrupts on both UARTS
    WRITE_PERI_REG(UART_INT_CLR(UART0), 0xffff);
    WRITE_PERI_REG(UART_INT_ENA(UART0), 0x0000);
    
    WRITE_PERI_REG(UART_INT_CLR(UART1), 0xffff);
    WRITE_PERI_REG(UART_INT_ENA(UART1), 0x0000);
    
    // Clear the software buffers
    _u1TX.head = _u1TX.tail = 0;
    _u0TX.head = _u0TX.tail = 0;
    _u0TX.head = _u0RX.tail = 0;
    
    // Set the TX buffers low threshold
    WRITE_PERI_REG(UART_CONF1(UART0), (UART_TX_THRESHOLD << UART_TXFIFO_EMPTY_THRHD_S));
    WRITE_PERI_REG(UART_CONF1(UART1), (UART_TX_THRESHOLD << UART_TXFIFO_EMPTY_THRHD_S));
    
    // attach the interrupt handler
    _xt_isr_attach(INUM_UART, _uartISR);
}

void fcore_rtxInit(uint16_t baudRate) {
    _xt_isr_mask(1 << INUM_UART);
    _uartReset();
    
    uint32_t clkdiv = APB_CLK_FREQ / baudRate;
    WRITE_PERI_REG(UART_CLKDIV(UART1), clkdiv);
    
    // We want to obtain a 8N2 setup to comply with FCORE RTTY reqs.
    SET_PERI_REG_MASK(UART_CONF0(UART1), (3 << UART_STOP_BIT_NUM_S));
    SET_PERI_REG_MASK(UART_CONF0(UART1), (3 << UART_BIT_NUM_S));
    CLEAR_PERI_REG_MASK(UART_CONF0(UART1), UART_PARITY_EN);
    
    _xt_isr_unmask(1 << INUM_UART);
}

void fcore_uartInit(uint16_t baudRate) {
    _xt_isr_mask(1 << INUM_UART);
    _uartReset();
    
    uint32_t clkdiv = APB_CLK_FREQ / baudRate;
    WRITE_PERI_REG(UART_CLKDIV(UART0), clkdiv);
    
    // We could try a general purpose, set-your-own-settings system, but we can
    // simplify things a lot if we go with 8N1, which is enough for what we do.
    SET_PERI_REG_MASK(UART_CONF0(UART0), (1 << UART_STOP_BIT_NUM_S));
    SET_PERI_REG_MASK(UART_CONF0(UART0), (3 << UART_BIT_NUM_S));
    CLEAR_PERI_REG_MASK(UART_CONF0(UART0), UART_PARITY_EN);
    
    _xt_isr_unmask(1 << INUM_UART);
}

void fcore_rtxStop() {
    WRITE_PERI_REG(UART_INT_CLR(UART1), 0xffff);
    WRITE_PERI_REG(UART_INT_ENA(UART1), 0x0000);
}

void fcore_uartStop() {
    WRITE_PERI_REG(UART_INT_CLR(UART0), 0xffff);
    WRITE_PERI_REG(UART_INT_ENA(UART0), 0x0000);
}

static void _uartWriteBytes(int uart, volatile FCBuffer* buf, uint8_t* bytes, uint32_t length) {
    if(length > FCORE_RTX_BUFFERSIZE) { return; }
    while(fcore_rtxAvailable() < length) {}

    _xt_isr_mask(1 << INUM_UART);
    for(uint32_t i = 0; i < length; ++i) {
        uint32_t nextHead = (buf->head + 1) % FCORE_RTX_BUFFERSIZE;
        buf->data[buf->head] = bytes[i];
        buf->head = nextHead;
    }
    _xt_isr_unmask(1 << INUM_UART);
    SET_PERI_REG_MASK(UART_INT_ENA(uart), UART_TXFIFO_EMPTY_INT_ENA);
}

void fcore_rtxWriteBytes(uint8_t* bytes, uint16_t length) {
    _uartWriteBytes(UART1, &_u1TX, bytes, length);
}

void fcore_uartWriteBytes(uint8_t* bytes, uint16_t length) {
    _uartWriteBytes(UART0, &_u1TX, bytes, length);
}

uint16_t fcore_uartReadBytes(uint8_t* bytes, uint16_t length) {
    // TODO: implementation
    return length;
}
