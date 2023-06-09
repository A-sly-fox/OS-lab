/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#define CSR_SIE			0x104
#define CSR_SIP			0x144
#define KERNEL_STACK		0x50500000

.section ".entry_function","ax"
.global _start
_start:
  /* Mask all interrupts */
  csrw CSR_SIE, zero
  csrw CSR_SIP, zero

  /* Load the global pointer */
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop
  
  /* Clear BSS for flat non-ELF images */
  la t2, __bss_start
  la t3, __BSS_END__
  bge t2, t3, done 
  li t4, 0
LOOP:
  sb  t4, 0(t2)
  addi t2, t2, 1 // i++ 
	// if (i < n) goto loop
	blt t2, t3, LOOP 
done: 
  /* setup C environment (set sp register)*/
  la sp, KERNEL_STACK
  /* Jump to the code in kernel.c*/
  call main

loop:
  wfi
  j loop

