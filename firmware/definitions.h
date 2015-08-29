/* 
 * File:   definitions.h
 * Author: Daniel
 *
 * Created on 25 December 2014, 9:44 PM
 */

#ifndef DEFINITIONS_H
#define	DEFINITIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

#define FOSC 80000000
#define FCY (FOSC/2)

#define UART1_BRGH 0
#define UART1_BAUDRATE 115200
#define UART1_BRGVAL ((FCY/UART1_BAUDRATE)/16)-1

//#define UART1_BRGH 1
//#define UART1_BAUDRATE 460800
//#define UART1_BRGVAL (FCY/(4*UART1_BAUDRATE))-1

#ifdef	__cplusplus
}
#endif

#endif	/* DEFINITIONS_H */

