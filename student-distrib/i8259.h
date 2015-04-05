/* *********************************************************
# FILE NAME: i8259.h
* PURPOSE: header for i8259.c
# AUTHOR: Queeblo OS
* MODIFIED: 10/27/2014
********************************************************* */

#ifndef _I8259_H
#define _I8259_H

#include "types.h"

/* Ports that each PIC sits on */
#define MASTER_8259_PORT 0x20
#define MASTER_8259_DATA 0x21
#define SLAVE_8259_PORT  0xA0
#define SLAVE_8259_DATA  0xA1

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1    0x11
#define ICW2_MASTER   0x20
#define ICW2_SLAVE    0x28
#define ICW3_MASTER   0x04
#define ICW3_SLAVE    0x02
#define ICW4          0x01

/* ZR - IRQ lines defined */
#define IRQ0  0x20
#define IRQ1  0x21
#define IRQ2  0x22
#define IRQ3  0x23
#define IRQ4  0x24
#define IRQ5  0x25
#define IRQ6  0x26
#define IRQ7  0x27
#define IRQ8  0x28
#define IRQ9  0x29
#define IRQ10 0x2A
#define IRQ11 0x2B
#define IRQ12 0x2C
#define IRQ13 0x2D
#define IRQ14 0x2E
#define IRQ15 0x2F

#define IRQ_CAS 2
#define IRQ_SLAVE 8

/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI             0x60

/* Externally-visible functions */

/* Initialize both PICs */
void i8259_init(void);
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);

#endif /* _I8259_H */
