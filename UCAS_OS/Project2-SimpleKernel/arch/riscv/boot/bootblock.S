#include <asm/sbidef.h>
#include <asm/sbiasm.h>

// #define BOOT_LOADER_SIG_OFFSET 0x1fe
// #define OS_SIZE_LOC (BOOT_LOADER_SIG_OFFSET - 2)
// #define OS_SIZE_LOC 0x1fc

.equ os_size_loc1, 0x502001fc
.equ os_size_loc2, 0x502001fa

// 1. kernel address (move kernel to here ~)
.equ kernel, 0x50200000

// 2. kernel main address (jmp here to start kernel main!)
.equ kernel_main, 0x50200000

.equ bootloader, 0x504ffe00

.text
.global main

main:
	/* Load the global pointer */
	.option push
	.option norelax
	la gp, __global_pointer$
	.option pop

	// fence on all memory and I/O
	fence

	// print a \n to avoid system output error...
	li a0, 10
	SBI_CALL SBI_CONSOLE_PUTCHAR

	// 1) task1 call BIOS print string "It's bootblock!" and "open which kernel? 0 or else\n"
	la a0, msg
	SBI_CALL SBI_CONSOLE_PUTSTR
	la a0, str
	SBI_CALL SBI_CONSOLE_PUTSTR

	// 2) task2 call BIOS read kernel in SD card and jump to kernel start
	li a2, 0
	li	a1, 1
	la a0, bootloader
	SBI_CALL SBI_SD_READ
	fence.i
	la t1, bootloader
	la t2, Location
	la t3, kernel
	sub t2, t2, t3
	add t1, t1, t2
	jr t1
	
Location:

	li t0, -1
loop:
	SBI_CALL SBI_CONSOLE_GETCHAR
	beq a0, t0, loop
debug:
	li t0, 48
	beq a0, t0, elseif
	li a2, 1
	la t4, os_size_loc1
	lh	a1, 0(t4)
	add a2, a1, a2
	la t4, os_size_loc2
	lh	a1, 0(t4)
	la t5, done
	jr t5
elseif:
	li a2, 1
	la t4, os_size_loc1
	lh	a1, 0(t4)
done:	
	la a0, kernel
	SBI_CALL SBI_SD_READ
	fence.i
	la t1, kernel_main
	jr t1

// while(1) --> stop here
stop:
	j stop
	nop

.data

msg: .string "It's a bootloader...\n\r"
str: .string "open which kernel? 0 or else\n"
