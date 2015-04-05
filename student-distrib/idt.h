/* *********************************************************
# FILE NAME: idt.h
* PURPOSE: header for idt.c
# AUTHOR: Queeblo OS
* MODIFIED: 10/27/2014
********************************************************* */

#ifndef _IDT_H
#define _IDT_H

#include "types.h"
#define USER_DPL 3
#define KERNEL_DPL 0

#define IRQ_1 1
#define IRQ_8 8
#define IRQ_0 0

#define SYSCALL 128

/* size of int desc array */
#define SUPPORTED_INT 18


/* structure passed by assembly containing regs, flags, and interrupt info */
typedef struct registers
{
   uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* from pusha */
   uint32_t int_num, error_code;    /* interrupt number, error code */
   uint32_t eip, cs, eflags, useresp, ss; /* pushed by hardware */
} registers_t;

void idt_init();


#endif /* _IDT_H */
