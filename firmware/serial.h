/* 
 * File:   serial.h
 * Author: Daniel
 *
 * Created on 26 December 2014, 4:42 PM
 */

#ifndef SERIAL_H
#define	SERIAL_H

#define SERIAL_STATUS_NODATA 0
#define SERIAL_STATUS_DATA 1
#define SERIAL_STATUS_ERROR 2

#ifdef	__cplusplus
extern "C" {
#endif

    void serial_init();
    int serial_readchar(unsigned char* chr);
    void serial_write(int wait, char* str, ...);
    void serial_writeline(char* str, ...);
    void serial_writeline_nowait(char* str, ...);

    void serial_write_data(char* data, unsigned int length, int wait);

#ifdef	__cplusplus
}
#endif

#endif	/* SERIAL_H */

