/* 
 * File:   pwm.h
 * Author: Daniel
 *
 * Created on 25 December 2014, 9:43 PM
 */

#ifndef PWM_H
#define	PWM_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "definitions.h"
#include <xc.h>
#include <libpic30.h>

#define PWM_CHANNEL_1 0x01
#define PWM_CHANNEL_2 0x02
#define PWM_CHANNEL_3 0x04
#define PWM_CHANNEL_4 0x08
#define PWM_CHANNEL_ALL 0xFF

void pwm_init();
void pwm_set(unsigned int channels, unsigned char value);

#ifdef	__cplusplus
}
#endif

#endif	/* PWM_H */

