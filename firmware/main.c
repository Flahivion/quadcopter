/* 
 * File:   main.c
 * Author: Daniel
 *
 * Created on 6 December 2014, 9:49 PM
 * 
 * Device: PIC24HJ64GP202
 *
 */

#include "definitions.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <xc.h>
#include <libpic30.h>
#include "pwm.h"
#include "serial.h"
#include "inv_mpu.h"
#include "pic_i2c.h"
#include "fifo.h"

_FOSCSEL(FNOSC_FRC);
_FOSC(FCKSM_CSECMD & OSCIOFNC_OFF & POSCMD_NONE);
_FWDT(FWDTEN_OFF);

#define CMD_SETDUTYCYCLES 0xD1
#define CMD_SETDUTYCYCLES_LEN 4

#define CMD_STATE_NONE 0
#define CMD_STATE_READING 1

void clock_init();
void logwrite(char* str, ...);

void process_command(unsigned char* data);
void update_pwm();

unsigned char _pwmChannel1;
unsigned char _pwmChannel2;
unsigned char _pwmChannel3;
unsigned char _pwmChannel4;
unsigned char _pwmFailsafeEnabled;
unsigned int _timer3high;

/*
 * 
 */
int main()
{
    unsigned char cmd_buf[8];
    int cmd_len;
    int cmd_len_expected;
    int cmd_state;
    int status;
    
    clock_init();

    AD1PCFGL = 0xFFFF; // All pins as digital.
    
    // UART pins/mapping
    TRISBbits.TRISB3 = 1;
    RPOR1bits.RP2R = 3; // Assign UART1 TX to RP2 (pin 6).
    RPINR18bits.U1RXR = 3; // Assign UART1 RX to RP3 (pin 7).

    serial_init();

    // PWM pins/mapping
    TRISBbits.TRISB15 = 0;
    TRISBbits.TRISB14 = 0;
    TRISBbits.TRISB13 = 0;
    TRISBbits.TRISB12 = 0;

    RPOR7 = 0b0001001000010011; // RP15 - OC1, RP14 - OC2
    RPOR6 = 0b0001010000010101; // RP13 - OC3, RP12 - OC4

    _pwmChannel1 = 0;
    _pwmChannel2 = 0;
    _pwmChannel3 = 0;
    _pwmChannel4 = 0;
    _pwmFailsafeEnabled = 0;

    pwm_init();
    update_pwm();

    // Set up timer 3 failsafe to determine when commands are no longer
    // being sent. When triggered, gradually reduce PWM values until
    // the next command is sent or the PWM value reaches minimum.
    T3CON = 0;
    T3CONbits.TCKPS = 0b11; // 1:256 prescale
    IFS0bits.T3IF = 0; // Clear interrupt flag
    IEC0bits.T3IE = 1; // Enable timer interrupt
    TMR3 = 0;
    PR3 = 0xFFFF;
    _timer3high = 0; // High word to create 32-bit timer from 16-bit timer
    T3CONbits.TON = 0b1; // Enable timer 3
    
    cmd_len = 0;
    cmd_len_expected = 0;
    cmd_state = CMD_STATE_NONE;

    while (1)
    {
        update_pwm();
        
        status = serial_readchar(&cmd_buf[cmd_len]);
        if (status == SERIAL_STATUS_NODATA)
        {
            continue;
        }
        else if (status == SERIAL_STATUS_ERROR)
        {
            cmd_state = CMD_STATE_NONE;
            cmd_len = 0;
            cmd_len_expected = 0;
        }
        else if (status == SERIAL_STATUS_DATA)
        {
            cmd_len++;
            if (cmd_len >= 8)
            {
                // Command data overflow.
                cmd_state = CMD_STATE_NONE;
                cmd_len = 0;
                cmd_len_expected = 0;
                continue;
            }
            
            if (cmd_state == CMD_STATE_NONE)
            {
                // New command - check start byte.
                if (cmd_buf[0] == CMD_SETDUTYCYCLES)
                {
                    cmd_state = CMD_STATE_READING;
                    cmd_len_expected = CMD_SETDUTYCYCLES_LEN;
                }
                else
                {
                    // Invalid command.
                    cmd_state = CMD_STATE_NONE;
                    cmd_len = 0;
                    cmd_len_expected = 0;
                }
            }

            // Check for data portion of command (if any).
            if (cmd_state == CMD_STATE_READING)
            {
                if (cmd_len - 1 == cmd_len_expected)
                {
                    process_command(cmd_buf);
                    cmd_state = CMD_STATE_NONE;
                    cmd_len = 0;
                    cmd_len_expected = 0;
                }
            }
        }
    }

    return 0;
}

void process_command(unsigned char* data)
{
    if (data[0] == CMD_SETDUTYCYCLES)
    {
        _pwmChannel1 = data[1];
        _pwmChannel2 = data[2];
        _pwmChannel3 = data[3];
        _pwmChannel4 = data[4];

        _pwmFailsafeEnabled = 0;
        _timer3high = 0;
        TMR3 = 0;
    }
}

void update_pwm()
{
    pwm_set(PWM_CHANNEL_1, _pwmChannel1);
    pwm_set(PWM_CHANNEL_2, _pwmChannel2);
    pwm_set(PWM_CHANNEL_3, _pwmChannel3);
    pwm_set(PWM_CHANNEL_4, _pwmChannel4);
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{
    IFS0bits.T3IF = 0;

    _timer3high++;
    unsigned long val = (((unsigned long)_timer3high) << 16) | TMR3;
    if (val >= (FCY / 256)) // 1 second at 1:256 prescale.
    {
        _pwmFailsafeEnabled = 1;
        _timer3high = 0;
    }

    if (_pwmFailsafeEnabled)
    {
        if (_pwmChannel1 > 0)
            _pwmChannel1--;

        if (_pwmChannel2 > 0)
            _pwmChannel2--;

        if (_pwmChannel3 > 0)
            _pwmChannel3--;

        if (_pwmChannel4 > 0)
            _pwmChannel4--;
    }

    if (_timer3high >= 65535)
        _timer3high = 0;
}

void clock_init()
{
    PLLFBD = 41;
    CLKDIVbits.PLLPOST = 0;
    CLKDIVbits.PLLPRE = 0;
    OSCTUN = 0;

    RCONbits.SWDTEN = 0;

    __builtin_write_OSCCONH(0x01);
    __builtin_write_OSCCONL(0x01);

    while (OSCCONbits.COSC != 0b001) {}
    while (OSCCONbits.LOCK != 1) {}
}

void logwrite(char* str, ...)
{
    const char* prepend = "[PIC] ";
    const char* append = "\r\n";
    char output[255];
    va_list argptr;

    va_start(argptr, str);
    vsprintf(output + strlen(prepend), str, argptr);
    va_end(argptr);

    strncpy(output, prepend, strlen(prepend));
    strcpy(output + strlen(output), append);

    serial_write_data(output, strlen(output), 1);
}
