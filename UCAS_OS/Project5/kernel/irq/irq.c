#include <os/irq.h>
#include <os/time.h>
#include <os/sched.h>
#include <os/string.h>
#include <os/stdio.h>
#include <assert.h>
#include <sbi.h>
#include <csr.h>
#include <screen.h>

#include <emacps/xemacps_example.h>
#include <plic.h>

handler_t irq_table[IRQC_COUNT];
handler_t exc_table[EXCC_COUNT];
uintptr_t riscv_dtb;

void reset_irq_timer()
{
    // TODO clock interrupt handler.
    // TODO: call following functions when task4
    screen_reflush();
    dma_check();
    check_sleep();
    // note: use sbi_set_timer
    // remember to reschedule
    
    sbi_set_timer(get_ticks() + time_base / INTERRUPT_NUM);
    do_scheduler();
}

void interrupt_helper(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    // TODO interrupt handler.
    // call corresponding handler by the value of `cause`
    uint64_t interrupt = cause & SCAUSE_IRQ_FLAG;
    uint64_t entry = cause & ~SCAUSE_IRQ_FLAG;
    if(interrupt)
        irq_table[entry](regs, stval, cause);
    else
        exc_table[entry](regs, stval, cause);
}

void handle_int(regs_context_t *regs, uint64_t interrupt, uint64_t cause)
{
    reset_irq_timer();
}

void handle_pagefault(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    uint64_t cpu_id = get_current_cpu_id();
    uintptr_t kva;
    kva = alloc_page_helper(stval, current_running[cpu_id]->pgdir);
}

void handle_irq(regs_context_t *regs, int irq)
{
    // TODO: 
    // handle external irq from network device
    // let PLIC know that handle_irq has been finished
    int type;
    u32 RegISR = XEmacPs_ReadReg(EmacPsInstance.Config.BaseAddress, XEMACPS_ISR_OFFSET);

	/* Clear the interrupt status register */
	XEmacPs_WriteReg(EmacPsInstance.Config.BaseAddress, XEMACPS_ISR_OFFSET, RegISR);
	type = ((RegISR & XEMACPS_IXR_FRAMERX_MASK) != 0x00000000U);

    if (XEmacPs_ReadReg(EmacPsInstance.Config.BaseAddress, XEMACPS_RXSR_OFFSET) & XEMACPS_RXSR_FRAMERX_MASK)
    {
        u32 RXSR_reg = XEmacPs_ReadReg(EmacPsInstance.Config.BaseAddress, XEMACPS_RXSR_OFFSET);
        XEmacPs_WriteReg(EmacPsInstance.Config.BaseAddress, XEMACPS_RXSR_OFFSET, RXSR_reg | XEMACPS_RXSR_FRAMERX_MASK);
        XEmacPs_IntDisable(&EmacPsInstance,(XEMACPS_IXR_FRAMERX_MASK | XEMACPS_IXR_RX_ERR_MASK));
    }
    if (XEmacPs_ReadReg(EmacPsInstance.Config.BaseAddress, XEMACPS_TXSR_OFFSET) & XEMACPS_TXSR_TXCOMPL_MASK)
    {
        u32 TXSR_reg = XEmacPs_ReadReg(EmacPsInstance.Config.BaseAddress, XEMACPS_TXSR_OFFSET) & XEMACPS_TXSR_TXCOMPL_MASK;
        XEmacPs_WriteReg(EmacPsInstance.Config.BaseAddress, XEMACPS_TXSR_OFFSET, TXSR_reg | XEMACPS_TXSR_TXCOMPL_MASK);
        XEmacPs_IntDisable(&EmacPsInstance, (XEMACPS_IXR_TXCOMPL_MASK | XEMACPS_IXR_TX_ERR_MASK));
    }
    if (type % 2 == 0 && !list_empty(&net_send_queue))
        do_unblock(&net_send_queue);
    if (type && !list_empty(&net_recv_queue))
        do_unblock(&net_recv_queue);
    // let PLIC know that handle_irq has been finished
    plic_irq_eoi(irq);
}

void init_exception()
{
    /* TODO: initialize irq_table and exc_table */
    /* note: handle_int, handle_syscall, handle_other, etc.*/
    for (int i = 0; i < IRQC_COUNT; i++)
        irq_table[i] = &handle_other;
    for (int i = 0; i < EXCC_COUNT; i++)
        exc_table[i] = &handle_other;
    irq_table[IRQC_S_TIMER] = &handle_int;
    irq_table[IRQC_S_EXT] = &plic_handle_irq;
    exc_table[EXCC_SYSCALL] = &handle_syscall;
    // exc_table[EXCC_STORE_PAGE_FAULT] = &handle_pagefault;
    // exc_table[EXCC_INST_PAGE_FAULT] = &handle_pagefault;
    // exc_table[EXCC_LOAD_PAGE_FAULT] = &handle_pagefault;
    setup_exception();
}

void handle_other(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    char* reg_name[] = {
        "zero "," ra  "," sp  "," gp  "," tp  ",
        " t0  "," t1  "," t2  ","s0/fp"," s1  ",
        " a0  "," a1  "," a2  "," a3  "," a4  ",
        " a5  "," a6  "," a7  "," s2  "," s3  ",
        " s4  "," s5  "," s6  "," s7  "," s8  ",
        " s9  "," s10 "," s11 "," t3  "," t4  ",
        " t5  "," t6  "
    };
    for (int i = 0; i < 32; i += 3) {
        for (int j = 0; j < 3 && i + j < 32; ++j) {
            printk("%s : %016lx ",reg_name[i+j], regs->regs[i+j]);
        }
        printk("\n\r");
    }
    printk("sstatus: 0x%lx sbadaddr: 0x%lx scause: %lx\n\r",
           regs->sstatus, regs->sbadaddr, regs->scause);
    printk("stval: 0x%lx cause: %lx\n\r",
           stval, cause);
    printk("sepc: 0x%lx\n\r", regs->sepc);
    // printk("mhartid: 0x%lx\n\r", get_current_cpu_id());

    uintptr_t fp = regs->regs[8], sp = regs->regs[2];
    printk("[Backtrace]\n\r");
    printk("  addr: %lx sp: %lx fp: %lx\n\r", regs->regs[1] - 4, sp, fp);
    // while (fp < USER_STACK_ADDR && fp > USER_STACK_ADDR - PAGE_SIZE) {
    while (fp > 0x10000) {
        uintptr_t prev_ra = *(uintptr_t*)(fp-8);
        uintptr_t prev_fp = *(uintptr_t*)(fp-16);

        printk("  addr: %lx sp: %lx fp: %lx\n\r", prev_ra - 4, fp, prev_fp);

        fp = prev_fp;
    }

    assert(0);
}
