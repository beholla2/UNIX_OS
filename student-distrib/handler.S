###########################################################
# FILE NAME: handler.S
# PURPOSE: handlers for interrupts 0-31, irq 0-15, syscall
# AUTHOR: Queeblo OS
# MODIFIED: 10/27/2014
###########################################################

.text

.globl handler_0, handler_1, handler_2, handler_3, handler_4, handler_5, handler_6, handler_7, handler_8, handler_9
.globl handler_10, handler_11, handler_12, handler_13, handler_14, handler_15, handler_16, handler_17, handler_18, handler_19
.globl handler_20, handler_21, handler_22, handler_23, handler_24, handler_25, handler_26, handler_27, handler_28, handler_29
.globl handler_30, handler_31

.globl handler_irq0, handler_irq1, handler_irq2, handler_irq3, handler_irq4, handler_irq6, handler_irq8, handler_irq10
.globl handler_irq11, handler_irq12, handler_irq13, handler_irq14, handler_irq15

.globl handler_syscall


#
# Handlers use cli to clear the interrupt flag and prevent race conditions.
# If the error code for an interrupt is not pushed to the stack, $0 is pushed.
# Then the interrupt number is pushed and the common handler is called.
#

# Divide Error
handler_0:
  cli
  pushl $0
  pushl $0
  jmp common_handler
# Debug
handler_1:
  cli
  pushl $0
  pushl $1
  jmp common_handler
# Not Used (NMI)
handler_2:
  cli
  pushl $0
  pushl $2
  jmp common_handler
# Breakpoint
handler_3:
  cli
  pushl $0
  pushl $3
  jmp common_handler
# Overflow
handler_4:
  cli
  pushl $0
  pushl $4
  jmp common_handler
# Bounds check
handler_5:
  cli
  pushl $0
  pushl $5
  jmp common_handler
# Invalid opcode
handler_6:
  cli
  pushl $0
  pushl $6
  jmp common_handler
# Device not available
handler_7:
  cli
  pushl $0
  pushl $7
  jmp common_handler
# Double fault
handler_8:
  cli
  pushl $8
  jmp common_handler
# coprocessor segment overrun
handler_9:
  cli
  pushl $0
  pushl $9
  jmp common_handler
# invalid TSS (task state segment)
handler_10:
  cli
  pushl $10
  jmp common_handler
# segment not present
handler_11:
  cli
  pushl $11
  jmp common_handler
# stack segment fault
handler_12:
  cli
  pushl $12
  jmp common_handler
# general protection
handler_13:
  cli
  pushl $13
  jmp common_handler
# page fault
handler_14:
  cli
  pushl $14
  jmp common_handler
# reserved by intel
handler_15:
  cli
  pushl $0
  pushl $15
  jmp common_handler
# floating point error
handler_16:
  cli
  pushl $0
  pushl $16
  jmp common_handler
# alignment check
handler_17:
  cli
  pushl $0
  pushl $17
  jmp common_handler
# machine check
handler_18:
  cli
  pushl $0
  pushl $18
  jmp common_handler
# SIMD floating point exception
handler_19:
  cli
  pushl $0
  pushl $19
  jmp common_handler

#
# handler 20-31 are intel reserved
#
handler_20:
  cli
  pushl $0
  pushl $20
  jmp common_handler
handler_21:
  cli
  pushl $0
  pushl $21
  jmp common_handler
handler_22:
  cli
  pushl $0
  pushl $22
  jmp common_handler
handler_23:
  cli
  pushl $0
  pushl $23
  jmp common_handler
handler_24:
  cli
  pushl $0
  pushl $24
  jmp common_handler
handler_25:
  cli
  pushl $0
  pushl $25
  jmp common_handler
handler_26:
  cli
  pushl $0
  pushl $26
  jmp common_handler
handler_27:
  cli
  pushl $0
  pushl $27
  jmp common_handler
handler_28:
  cli
  pushl $0
  pushl $28
  jmp common_handler
handler_29:
  cli
  pushl $0
  pushl $29
  jmp common_handler
handler_30:
  cli
  pushl $0
  pushl $30
  jmp common_handler
handler_31:
  cli
  pushl $0
  pushl $31
  jmp common_handler

#
# handler irq0-irq15 are called by the pic
#

# timer
handler_irq0:
  cli
  pushl $0
  pushl $0
  jmp irq_handler
# keyboard
handler_irq1:
  cli
  pushl $0
  pushl $1
  jmp irq_handler
# pic cascading
handler_irq2:
  cli
  pushl $0
  pushl $2
  jmp irq_handler
# second serial port
handler_irq3:
  cli
  pushl $0
  pushl $3
  jmp irq_handler
# first serial port
handler_irq4:
  cli
  pushl $0
  pushl $4
  jmp irq_handler
# floppy disk
handler_irq6:
  cli
  pushl $0
  pushl $6
  jmp irq_handler
# system clock
handler_irq8:
  cli
  pushl $0
  pushl $8
  jmp irq_handler
# network interface
handler_irq10:
  cli
  pushl $0
  pushl $10
  jmp irq_handler
# usb port, sound card
handler_irq11:
  cli
  pushl $0
  pushl $11
  jmp irq_handler
# ps2 mouse
handler_irq12:
  cli
  pushl $0
  pushl $12
  jmp irq_handler
# mathematical coprocessor
handler_irq13:
  cli
  pushl $0
  pushl $13
  jmp irq_handler
# EIDE disk controller's first chain
handler_irq14:
  cli
  pushl $0
  pushl $14
  jmp irq_handler
# EIDE disk controller's second chain
handler_irq15:
  cli
  pushl $0
  pushl $15
  jmp irq_handler

#system call jump table
sys_call_table:
  .long 0
  .long halt
  .long execute
  .long read
  .long write
  .long open
  .long close
  .long getargs
  .long vidmap
  .long set_handler
  .long sigreturn

# syscall handler
handler_syscall:
  pusha
  # arguments
  pushl %edx 
  pushl %ecx
  pushl %ebx

  cmpl $1, %eax
  jl sys_error
  cmpl $10, %eax
  jg sys_error

  sti
  call *sys_call_table(, %eax, 4)
  jmp sys_success

sys_error:
  addl $12,%esp
  popa
  movl $-1, %eax
  iret

sys_success:
  addl $12, %esp
  # restore all regs except eax
  pushl %eax
  addl $4, %esp
  popa
  addl $-36, %esp
  popl %eax
  addl $32, %esp
  
  iret


##
# common_handler
# INPUT: error code(or $0), interrupt number
# OUTPUT: none
# DESCRIPTION: calls the idt_handler function in idt.c, irets after
##
common_handler:
  cld
  
  pusha # pushes in order eax, ecx, edx, ebx, esp, ebp, esi, edi        
  call idt_handler
  popa
  
  addl $8, %esp
  sti
  iret 

##
# irq_handler
# INPUT: error code(or $0), irq number
# OUTPUT: none
# DESCRIPTION: calls the pic_handler function in idt.c, irets after
##
irq_handler:
  cld 

  pusha
  call pic_handler
  popa

  addl $8, %esp
  sti
  iret 
