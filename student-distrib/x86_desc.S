# x86_desc.S - Set up x86 segment descriptors, descriptor tables 
# vim:ts=4 noexpandtab

#define ASM     1
#include "x86_desc.h"

.text

.globl  ldt_size, tss_size
.globl  gdt_desc, ldt_desc, tss_desc
.globl  tss, tss_desc_ptr, ldt, ldt_desc_ptr
.globl  gdt_ptr
.globl  idt_desc_ptr, idt
.globl 	pt_0_4, pd

.align 4


tss_size:
	.long tss_bottom - tss - 1

ldt_size:
	.long ldt_bottom - ldt - 1

	.word 0 # Padding
ldt_desc:
	.word KERNEL_LDT
	.long ldt

	.align 4
tss:
_tss:
	.rept 104
	.byte 0
	.endr
tss_bottom:

	.align  16

# AW:	Initialize gdt_desc
#		In the GDT descriptor we put size - 1 (http://wiki.osdev.org/Global_Descriptor_Table)
#		The offset is the gdt label.
gdt_desc:
	.word gdt_bottom - gdt -1	# AW: size of gdt
	.long gdt 					# AW: offset = gdt

gdt:
_gdt:

	# First GDT entry cannot be used
	.quad 0
	
	# NULL entry
	.quad 0
	
	# Segmentation will not be used
	# CS and DS both are 0-4GB r/w segments
	#
	# The layout is (from Intel IA-32 reference manual):
	#  31        24 23  22  21  20  19   16 15  14 13 12  11   8 7          0
	# |----------------------------------------------------------------------|
	# |            |   | D |   | A |  Seg  |   |  D  |   |      |            |
	# | Base 31:24 | G | / | 0 | V | Limit | P |  P  | S | Type | Base 23:16 |
	# |            |   | B |   | L | 19:16 |   |  L  |   |      |            |
	# |----------------------------------------------------------------------|
	#
	# |----------------------------------------------------------------------|
	# |                                    |                                 |
	# | Base 15:0                          | Segment Limit 15:0              |
	# |                                    |                                 |
	# |----------------------------------------------------------------------|
	
gdt_ptr:
	# Set up an entry for kernel CS
	.quad 0x00CF9A000000FFFF

	# Set up an entry for kernel DS
	.quad 0x00CF92000000FFFF

	# Set up an entry for user CS
	.quad 0x00CFFA000000FFFF

	# Set up an entry for user DS
	.quad 0x00CFF2000000FFFF

	# Set up an entry for TSS
tss_desc_ptr:
	.quad 0

	# Set up one LDT
ldt_desc_ptr:
	.quad 0

	# AW set up entry for 

gdt_bottom:		# AW: this is the end of the GDT

	.align 16
ldt:
	.rept 4
	.quad 0
	.endr
ldt_bottom:

.align 4
	.word 0 # Padding
idt_desc_ptr:
	.word idt_bottom - idt - 1 	# AW: (this code was provided) this is the size of IDT (http://wiki.osdev.org/Interrupts_Descriptor_Table)
	.long idt 					# AW: (this code was provided) this is the base of IDT
	
# AW: Interrupt Descriptor Table
	.align  16
idt:
_idt:
	.rept NUM_VEC
	.quad 0
	.endr

idt_bottom:


# AW: Page Directory
.align 4096
pd:
_pd:
	.rept PAGE_ENTRY
	.long 0 			# repeat 32-bit 0, 1024 times
	.endr

# AW: Page Table 0 kb thru 4 kb
.align  4096
pt_0_4:
_pt_0_4:
	.rept PAGE_ENTRY
	.long 0 			# repeat 32-bit 0, 1024 times
	.endr
