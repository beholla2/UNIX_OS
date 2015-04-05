/* *********************************************************
# FILE NAME: handler.h
* PURPOSE: header for handler.S
# AUTHOR: Queeblo OS
* MODIFIED: 10/27/2014
**********************************************************/

#ifndef _HANDLER_H
#define _HANDLER_H

/* handler functions are defined in handler.S 
these functions are loaded in the idt in idt.c */
extern void handler_0();
extern void handler_1();
extern void handler_2();
extern void handler_3();
extern void handler_4();
extern void handler_5();
extern void handler_6();
extern void handler_7();
extern void handler_8();
extern void handler_9();
extern void handler_10();
extern void handler_11();
extern void handler_12();
extern void handler_13();
extern void handler_14();
extern void handler_15();
extern void handler_16();
extern void handler_17();
extern void handler_18();
extern void handler_19();
extern void handler_20();
extern void handler_21();
extern void handler_22();
extern void handler_23();
extern void handler_24();
extern void handler_25();
extern void handler_26();
extern void handler_27();
extern void handler_28();
extern void handler_29();
extern void handler_30();
extern void handler_31();

extern void handler_irq0();
extern void handler_irq1();
extern void handler_irq2();
extern void handler_irq3();
extern void handler_irq4();
extern void handler_irq6();
extern void handler_irq8();
extern void handler_irq10();
extern void handler_irq11();
extern void handler_irq12();
extern void handler_irq13();
extern void handler_irq14();
extern void handler_irq15();

extern void handler_syscall();

#endif /* _HANDLER_H */
