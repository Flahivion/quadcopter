#include "definitions.h"
#include "serial.h"
#include "fifo.h"
#include <xc.h>
#include <libpic30.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

unsigned char _serial_buffer[255] __attribute__((space(dma)));

void serial_init()
{
    U1BRG = UART1_BRGVAL;
    U1STA = 0;

    U1MODE = 0;
    U1MODEbits.PDSEL = 2; // 8-bit data, odd parity.
    U1MODEbits.STSEL = 0; // 1 stop bit.
    U1MODEbits.BRGH = UART1_BRGH;
    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;

    DMA0CON = 0;
    DMA0CONbits.SIZE = 1; // Byte
    DMA0CONbits.DIR = 1; // RAM-to-peripheral
    DMA0CONbits.AMODE = 0; // Register indirect with post-increment
    DMA0CONbits.MODE = 0b01; // One-shot, ping-pong disabled
    DMA0REQ = 0x000C;

    DMA0PAD = (volatile unsigned int)&U1TXREG;
    DMA0STA = __builtin_dmaoffset(_serial_buffer);

    IFS0bits.U1TXIF = 0; // Clear TX interrupt flag.
    IEC0bits.U1TXIE = 1; // Enable TX interrupt.
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 0;

    IFS0bits.DMA0IF = 0; // Clear DMA0 interrupt flag.
    IEC0bits.DMA0IE = 1; // Enable DMA0 interrupt.
}

int serial_readchar(unsigned char* chr)
{
    int status = SERIAL_STATUS_NODATA;

    if (U1STAbits.URXDA == 1)
    {
        *chr = U1RXREG;
        status = SERIAL_STATUS_DATA;
    }

    if (U1STAbits.FERR == 1)
        return SERIAL_STATUS_ERROR;

    if (U1STAbits.PERR == 1)
        return SERIAL_STATUS_ERROR;

    if (U1STAbits.OERR == 1)
    {
        U1STAbits.OERR = 0;
        return SERIAL_STATUS_ERROR;
    }

    return status;
}

void serial_writeline(char* str, ...)
{
    char output[255];
    va_list argptr;

    va_start(argptr, str);
    vsprintf(output, str, argptr);
    va_end(argptr);
    
    strcat(output, "\r\n");

    serial_write_data(output, strlen(output), 1);
}

void serial_writeline_nowait(char* str, ...)
{
    char output[255];
    va_list argptr;

    va_start(argptr, str);
    vsprintf(output, str, argptr);
    va_end(argptr);

    strcat(output, "\r\n");

    serial_write_data(output, strlen(output), 0);
}

void serial_write(int wait, char* str, ...)
{
    char output[255];
    va_list argptr;

    va_start(argptr, str);
    vsprintf(output, str, argptr);
    va_end(argptr);

    serial_write_data(output, strlen(output), wait);
}

void serial_write_data(char* data, unsigned int length, int wait)
{
    if (wait)
    {
        while (U1STAbits.TRMT == 0)
        {
        }
    }
    else
    {
        if (U1STAbits.TRMT == 0)
            return;
    }
    
    DMA0CNT = length - 1;

    memcpy(_serial_buffer, data, length);

    DMA0CONbits.CHEN = 1;
    DMA0REQbits.FORCE = 1;
}

void __attribute__((interrupt, no_auto_psv)) _DMA0Interrupt(void)
{
    IFS0bits.DMA0IF = 0;
}

void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void)
{
    IFS0bits.U1TXIF = 0;
}
