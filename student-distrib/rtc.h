/* *********************************************************
# FILE NAME: rtc.h
* PURPOSE: header for rtc.c
# AUTHOR: Queeblo OS
* MODIFIED: 10/27/2014
********************************************************* */

#ifndef _RTC_H
#define _RTC_H

#include "lib.h"

/* rtc status registers */
#define REG_A 0x0A
#define REG_B 0x0B
#define REG_C 0x0C
#define RTC_INDEX 0x70
#define RTC_DATA 0x71
#define RTC_IRQ 8

#define BITMASK6 0x40
#define SHIFT_FOR_2HZ 15
#define FREQ_BASE 0x8000
#define MAX_FREQ 1024


uint8_t RTC_READ;
uint32_t rtc_count;
uint32_t rtc_freq;
uint8_t rtc_test_flag;

/* initialize the rtc */
void rtc_init();

/* write a certain frequency to rtc */
int32_t rtc_write(uint8_t* fname, void* buf, int32_t nbytes);

/* read, aka wait until next rtc interrupt */
int32_t rtc_read(uint8_t* fname, void* buf, int32_t nbytes);

/* open the rtc with 2 hz */
int32_t rtc_open();

/* close the rtc, return 0 */
int32_t rtc_close();

/* change the rtc_open flag */
void clear_rtc_read();

/* set rtc_test_flag for testing CP1_and_CP2 */
void rtc_set_flag();


#endif /* _RTC_H */
