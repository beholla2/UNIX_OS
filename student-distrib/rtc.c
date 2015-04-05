/* *********************************************************
# FILE NAME: rtc.c
* PURPOSE: initalizes rtc interrupts
# AUTHOR: Queeblo OS
* MODIFIED: 10/27/2014
********************************************************* */

#include "rtc.h"

/* rtc_init()
 * INPUT: none
 * OUTPUT: none, zero
 * DESCRIPTION: enables periodic RTC interrrupts on IRQ8
 */
void
rtc_init()
{
    cli();

	char prev;

	outb(REG_B, RTC_INDEX); 
	prev = inb(RTC_DATA);
	outb(REG_B, RTC_INDEX);
	outb(prev | BITMASK6, RTC_DATA);    

	sti();
    rtc_count = 0;
    rtc_freq = 2; 
    rtc_test_flag = 0;
}

/* rtc_write()
 * INPUT: 
 * OUTPUT:
 * DESCRIPTION: write to the rtc at a given frequency
 */
int32_t 
rtc_write(uint8_t* fname, void* buf, int32_t nbytes)
{
    uint32_t freq;
	uint32_t rate;
    //int16_t num_to_shift = FREQ_BASE;   /* AW: I changed type from uint8_t to int16_t due to compile error */   

    
	
    if(buf != NULL)
        freq = *((uint32_t*)buf);    /* buffer contains frequency we want */ 
          
    /* check if freq is greater than 1024 */
    if(freq > MAX_FREQ)
        return -1;
    
    /* might be a better way than a switch statement */
    switch(freq) {
        case 1024:
            rate = 6;
            break;
        case 512:
            rate = 7;
            break;
        case 256:
            rate = 8;
            break;
        case 128:  
            rate = 9;
            break;
        case 64:
            rate = 10;
            break;
        case 32:   
            rate = 11;
            break;
        case 16:
            rate = 12;
            break; 
        case 8:
            rate = 13;
            break;
        case 4:
            rate = 14;
            break;
        case 2:
            rate = 15;
            break;
        case 0:
            rate = 0;
            break;
        default:
            return -1; /* if not a power of 2 return 0 */
    }
    
    /* use rate we calculated to write the given freq to Hz */

	cli();
    /* to change rtc freq, use reg_a */

    rate = rate;
	char prev;
	outb(REG_A, RTC_INDEX); 
	prev = inb(RTC_DATA);
	outb(REG_A, RTC_INDEX);
	outb( (prev & BITMASK6) | rate, RTC_DATA);    
    
	sti();

    return 0;
}

/* rtc_read()
 * INPUT: 
 * OUTPUT:
 * DESCRIPTION: wait (spin) until next rtc interrupt happens then return 0
 */
int32_t 
rtc_read(uint8_t* fname, void* buf, int32_t nbytes)
{
    /* set rtc_read for next tick */
    RTC_READ = 1;
	/* waiting for next rtc tick */
	while(RTC_READ){}
	return 0;	
}

/* rtc_open()
 * INPUT: 
 * OUTPUT:
 * DESCRIPTION: initializes rtc to 2Hz and returns 0 
 * using the formula: frequency =  32768 >> (rate-1); we 
 * arrive at rate being defined as 14 for our 2Hz
 */
int32_t 
rtc_open()
{

	cli();
    /* to change rtc freq, use reg_a */
	char prev;
	outb(REG_A, RTC_INDEX); 
	prev = inb(RTC_DATA);
	outb(REG_A, RTC_INDEX);
	outb( (prev & BITMASK6) | SHIFT_FOR_2HZ, RTC_DATA);    

	sti();

	return 0;
}

/* rtc_close()
 * INPUT: 
 * OUTPUT:
 * DESCRIPTION: just return 0 
 */
int32_t 
rtc_close()
{
	return 0;
}

/* rtc_int()
 * INPUT: none
 * OUTPUT: none
 * DESCRIPTION: clear rtc_open flag
 */
void
clear_rtc_read()
{ 
    uint8_t* dummy;
    RTC_READ = 0;  /* clear rtc read */

    /* showcasing rtc_write changing frequencies 
     * UNCOMMENT OUT IF YOU WANT TO TEST CHANGING FREQ 
    */
    if(rtc_test_flag)
    {
        if((rtc_count == 10 || rtc_count == 30
            || rtc_count == 70 || rtc_count == 100
            || rtc_count == 200) && rtc_freq < 1024)
        {
            rtc_freq = rtc_freq * 2;
            rtc_write(dummy, &rtc_freq, 4);
        }
    }

}


/* rtc_set_flag()
 * INPUT:           none
 * OUTPUT:          none
 * DESCRIPTION:     sets flag for testing cp2
 */
void
rtc_set_flag()
{
    rtc_test_flag = 1;
}





