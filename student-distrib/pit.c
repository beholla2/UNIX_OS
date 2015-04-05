/* *********************************************************
# FILE NAME: pit.c
* PURPOSE: programmable interval timer driver
# AUTHOR: Queeblo OS
* MODIFIED: 12/07/2014
********************************************************* */

#include "pit.h"
#include "lib.h"
#include "i8259.h"



/* pit_init(uint32_t frequency)
 * INPUT:  		frequency - frequency to set the PIT to fire at.
 * OUTPUT: 		none
 * DESCRIPTION: set the PIT to a certain frequency and then enable IRQ0 
 * 				so the system can recieve interrupts from the PIT.
 */ 
void pit_init(uint32_t frequency)
{
	uint32_t divisor = 1193180 / frequency; /* between 18.2Hz and 1.1931 MHz , HAS TO FIT IN 16 BITS*/

	outb(0x36, 0x43);

	uint8_t l = (uint8_t)(divisor & 0xFF);
	uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

	outb(l, 0x40);
	outb(h, 0x40);

	enable_irq(0);
}
