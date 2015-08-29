#include "pwm.h"

void pwm_init()
{
    OC1CON = 0x0000;
    OC2CON = 0x0000;
    OC3CON = 0x0000;
    OC4CON = 0x0000;

    // 1mS duty cycle
    OC1R   = 0x0271;
    OC1RS  = 0x0271;
    OC2R   = 0x0271;
    OC2RS  = 0x0271;
    OC3R   = 0x0271;
    OC3RS  = 0x0271;
    OC4R   = 0x0271;
    OC4RS  = 0x0271;

    // PWM mode w/o fault protection, output compare continues in idle mode, timer 2 as clock source.
    OC1CON = 0x0006;
    OC2CON = 0x0006;
    OC3CON = 0x0006;
    OC4CON = 0x0006;

    // Timer 2 setup
    PR2    = 0x30D3; // 20mS period
    T2CONbits.TCKPS = 2; // 1:64 prescale
    T2CONbits.TON = 1;
}

void pwm_set(unsigned int channels, unsigned char value)
{
    // 1ms = 625 timer ticks.
    // 2ms = 1250 timer ticks.
    // (1250-625)/256 = 2.44140625
    unsigned int ticks = (unsigned int)((2.44140625f * value) + 625);

    if ((channels & PWM_CHANNEL_1) != 0)
        OC1RS = ticks;

    if ((channels & PWM_CHANNEL_2) != 0)
        OC2RS = ticks;

    if ((channels & PWM_CHANNEL_3) != 0)
        OC3RS = ticks;

    if ((channels & PWM_CHANNEL_4) != 0)
        OC4RS = ticks;
}
