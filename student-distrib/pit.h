#ifndef _PIC_H
#define _PIC_H

#include "types.h"



/* pit_init(uint32_t frequency)
 * INPUT:  		frequency - frequency to set the PIT to fire at.
 * OUTPUT: 		none
 * DESCRIPTION: set the PIT to a certain frequency and then enable IRQ0 
 * 				so the system can recieve interrupts from the PIT.
 */ 
void pit_init(uint32_t frequency);

#endif /* _PIC_H */
