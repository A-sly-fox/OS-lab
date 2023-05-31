/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *         The kernel's entry, where most of the initialization work is done.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */
#include <plic.h>
#include <emacps/xemacps_example.h>
#include <net.h>

#include <common.h>
#include <os/irq.h>
#include <os/mm.h>
#include <os/sched.h>
#include <screen.h>
#include <sbi.h>
#include <os/stdio.h>
#include <os/time.h>
#include <os/syscall.h>
#include <os/elf.h>

#include <csr.h>
#include <os/lock.h>
#include <pgtable.h>
#include <user_programs.h>
#include <assert.h>

extern void ret_from_exception();
extern void __global_pointer$();

void init_pcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    pcb_t *pcb, void *argv, int argc)
{
    regs_context_t *pt_regs =
        (regs_context_t *)(kernel_stack - sizeof(regs_context_t));
    
    /* TODO: initialization registers
     * note: sp, gp, ra, sepc, sstatus
     * gp should be __global_pointer$
     * To run the task in user mode,
     * you should set corresponding bits of sstatus(SPP, SPIE, etc.).
     */
    //pt_regs->regs[1] = entry_point;//ra
    pt_regs->regs[2] = user_stack;//sp
    pt_regs->regs[3] = __global_pointer$;//gp
    pt_regs->regs[4] = (reg_t)pcb;
    
    pt_regs->regs[10] = (reg_t)argc;
    pt_regs->regs[11] = (reg_t)argv;

    pt_regs->sepc = entry_point;
    pt_regs->sstatus = SR_SUM;
    pt_regs->sbadaddr = 0;
    // set sp to kernel_stack return from switch_to
    /* TODO: you should prepare a stack, and push some values to
     * simulate a pcb context.
     */
    switchto_context_t *mystack = (switchto_context_t *)(kernel_stack - sizeof(regs_context_t)-sizeof(switchto_context_t));
    if (pcb->type == USER_PROCESS || pcb->type == USER_THREAD)
        mystack->regs[0] = &ret_from_exception;
    else
        mystack->regs[0] = entry_point;
    mystack->regs[1] = user_stack;
    for(int i = 2; i < 14; i++)
        mystack->regs[i] = 0;
}

int valid_pcb = 1;
static void init_pcb()
{
    /* initialize all of your pcb and add them into ready_queue
    * TODO:
    */
    pcb[0].pgdir = allocPage(1);
    clear_pgdir(pcb[0].pgdir);
    pcb[0].kernel_stack_base = allocPage(1) + PAGE_SIZE;
    pcb[0].kernel_sp = pcb[0].kernel_stack_base;
    pcb[0].user_stack_base = USER_STACK_ADDR;
    pcb[0].user_sp = pcb[0].user_stack_base;
    
    uint64_t base_pgdir = pa2kva(PGDIR_PA);
    share_pgtable(pcb[0].pgdir, base_pgdir);
    alloc_page_helper(pcb[0].user_stack_base - 0x1000, pcb[0].pgdir);

    ptr_t entry_point = (ptr_t)load_elf(_elf___test_test_shell_elf,_length___test_test_shell_elf,pcb[0].pgdir,alloc_page_helper);
    
    pcb[0].type = USER_PROCESS;
    init_pcb_stack(pcb[0].kernel_sp, pcb[0].user_sp, entry_point, &pcb[0], NULL, NULL);
    list_add_tail(&pcb[0].list,&ready_queue);
    
    pcb[0].pid = 1;
    pcb[0].status = TASK_READY;
    pcb[0].cursor_x = 1;
    pcb[0].cursor_y = 1;
    pcb[0].kernel_sp = pcb[0].kernel_sp - sizeof(regs_context_t) - sizeof(switchto_context_t);
    pcb[0].user_sp = pcb[0].user_sp;
    pcb[0].mask = 3;

    // pid0_pcb_1.kernel_stack_base = allocPage(1) + PAGE_SIZE;
    // pid0_pcb_1.kernel_sp = pid0_pcb_1.kernel_stack_base;
    // pid0_pcb_1.user_stack_base = allocPage(1) + PAGE_SIZE;
    // pid0_pcb_1.user_sp = pid0_pcb_1.user_stack_base;
    
    // pid0_pcb_1.type = USER_PROCESS;
    // init_pcb_stack(pid0_pcb_1.kernel_sp, pid0_pcb_1.user_sp, (ptr_t)&timer_task, &pid0_pcb_1, NULL);
    // //list_add_tail(&pcb[1].list,&ready_queue);
    
    // pid0_pcb_1.pid = 0;
    // pid0_pcb_1.status = TASK_READY;
    // pid0_pcb_1.cursor_x = 1;
    // pid0_pcb_1.cursor_y = 1;
    // pid0_pcb_1.kernel_sp = pid0_pcb_1.kernel_sp - sizeof(regs_context_t)-sizeof(switchto_context_t);
    // pid0_pcb_1.user_sp = pid0_pcb_1.user_sp;
    // pid0_pcb_1.mask = 3;

    /* remember to initialize `current_running`
     * TODO:
     */
    current_running[0] = &pid0_pcb_0;
    current_running[1] = &pid0_pcb_0;
}

void init_pcb_again()
{
    /* initialize all of your pcb and add them into ready_queue
    * TODO:
    */
    pcb[0].pgdir = allocPage(1);
    clear_pgdir(pcb[0].pgdir);
    pcb[0].kernel_stack_base = allocPage(1) + PAGE_SIZE;
    pcb[0].kernel_sp = pcb[0].kernel_stack_base;
    pcb[0].user_stack_base = USER_STACK_ADDR;
    pcb[0].user_sp = pcb[0].user_stack_base;
    
    uint64_t base_pgdir = pa2kva(PGDIR_PA);
    share_pgtable(pcb[0].pgdir, base_pgdir);
    alloc_page_helper(pcb[0].user_stack_base - 0x1000, pcb[0].pgdir);

    ptr_t entry_point = (ptr_t)load_elf(_elf___test_test_shell_elf,_length___test_test_shell_elf,pcb[0].pgdir,alloc_page_helper);
    
    pcb[0].type = USER_PROCESS;
    init_pcb_stack(pcb[0].kernel_sp, pcb[0].user_sp, entry_point, &pcb[0], NULL, NULL);
    init_list_head(&ready_queue);
    list_add_tail(&pcb[0].list,&ready_queue);
    
    pcb[0].pid = 1;
    pcb[0].status = TASK_READY;
    pcb[0].cursor_x = 1;
    pcb[0].cursor_y = 1;
    pcb[0].kernel_sp = pcb[0].kernel_sp - sizeof(regs_context_t) - sizeof(switchto_context_t);
    pcb[0].user_sp = pcb[0].user_sp;
    pcb[0].mask = 3;

    // pid0_pcb_1.kernel_stack_base = allocPage(1) + PAGE_SIZE;
    // pid0_pcb_1.kernel_sp = pid0_pcb_1.kernel_stack_base;
    // pid0_pcb_1.user_stack_base = allocPage(1) + PAGE_SIZE;
    // pid0_pcb_1.user_sp = pid0_pcb_1.user_stack_base;
    
    // pid0_pcb_1.type = USER_PROCESS;
    // init_pcb_stack(pid0_pcb_1.kernel_sp, pid0_pcb_1.user_sp, (ptr_t)&timer_task, &pid0_pcb_1, NULL);
    // //list_add_tail(&pcb[1].list,&ready_queue);
    
    // pid0_pcb_1.pid = 0;
    // pid0_pcb_1.status = TASK_READY;
    // pid0_pcb_1.cursor_x = 1;
    // pid0_pcb_1.cursor_y = 1;
    // pid0_pcb_1.kernel_sp = pid0_pcb_1.kernel_sp - sizeof(regs_context_t)-sizeof(switchto_context_t);
    // pid0_pcb_1.user_sp = pid0_pcb_1.user_sp;
    // pid0_pcb_1.mask = 3;

    /* remember to initialize `current_running`
     * TODO:
     */
    current_running[0] = &pid0_pcb_0;
    current_running[1] = &pid0_pcb_0;
}

static void init_syscall(void)
{
    // initialize system call table.
    int i;
    for(i = 0; i < NUM_SYSCALLS; i++)
        syscall[i] = &handle_other;
    syscall[SYSCALL_SLEEP] = &do_sleep;
    syscall[SYSCALL_YIELD] = &do_scheduler;

    syscall[SYSCALL_WRITE] = &screen_write;
    //syscall[SYSCALL_WRITE] = &printk;
    syscall[SYSCALL_READ] = &sbi_console_getchar;    
 
    net_poll_mode = 0;
    // xemacps_example_main();
    syscall[SYSCALL_CURSOR] = &screen_move_cursor;
    //syscall[SYSCALL_CURSOR] = &vt100_move_cursor;
    syscall[SYSCALL_REFLUSH] = &screen_reflush;
    syscall[SYSCALL_SCREEN_CLEAR] = &screen_clear;

    syscall[SYSCALL_GET_TIMEBASE] = &get_time_base;
    syscall[SYSCALL_GET_TICK] = &get_ticks;

    syscall[SYSCALL_MUTEX_INIT] = &do_mutex_lock_init;
    syscall[SYSCALL_MUTEX_LOCK] = &do_mutex_lock_acquire;
    syscall[SYSCALL_MUTEX_UNLOCK] = &do_mutex_lock_release;
    syscall[SYSCALL_GET_WALL_TIME] = &my_sys_get_wall_time;

    syscall[SYSCALL_PS] = &do_process_show;
    syscall[SYSCALL_SPAWN] = &do_spawn;
    syscall[SYSCALL_KILL] = &do_kill;
    syscall[SYSCALL_WAITPID] = &do_waitpid;
    syscall[SYSCALL_EXIT] = &do_exit;
    syscall[SYSCALL_GETPID] = &do_getpid;
    syscall[SYSCALL_BARRIER_INIT] = &do_barrier_init;
    syscall[SYSCALL_BARRIER_WAIT] = &do_barrier_wait;
    syscall[SYSCALL_BARRIER_DESTROY] = &do_barrier_destroy;
    syscall[SYSCALL_SEMAPHORE_INIT] = &do_semaphore_init;
    syscall[SYSCALL_SEMAPHORE_UP] = &do_semaphore_up;
    syscall[SYSCALL_SEMAPHORE_DOWN] = &do_semaphore_down;
    syscall[SYSCALL_SEMAPHORE_DESTROY] = &do_semaphore_destroy;
    syscall[SYSCALL_MBOX_OPEN] = &do_mbox_open;
    syscall[SYSCALL_MBOX_CLOSE] = &do_mbox_close;
    syscall[SYSCALL_MBOX_SEND] = &do_mbox_send;
    syscall[SYSCALL_MBOX_RECV] = &do_mbox_recv;
    syscall[SYSCALL_TASKSET] = &do_taskset;
    syscall[SYSCALL_TASKSET_P] = &do_taskset_p;
    syscall[SYSCALL_LS] = &do_ls;
    syscall[SYSCALL_EXEC] = &do_exec;
    syscall[SYSCALL_BINSEMGET] = &do_binsemget;
    syscall[SYSCALL_BINSEMOP] = &do_binsemop;
    syscall[SYSCALL_NET_RECV] = &do_net_recv;
    syscall[SYSCALL_NET_SEND] = &do_net_send;
    syscall[SYSCALL_NET_IRQ_MODE] = &do_net_irq_mode;
}

void boot_first_core(void)
{
    // init Process Control Block (-_-!)
    init_pcb();
    printk("> [INIT] PCB initialization succeeded.\n\r");

    // setup timebase
    // fdt_print(_dtb);
    // get_prop_u32(_dtb, "/cpus/cpu/timebase-frequency", &time_base);
    time_base = sbi_read_fdt(TIMEBASE);
    uint32_t slcr_bade_addr = 0, ethernet_addr = 0;

    // get_prop_u32(_dtb, "/soc/slcr/reg", &slcr_bade_addr);
    slcr_bade_addr = sbi_read_fdt(SLCR_BADE_ADDR);
    printk("[slcr] phy: 0x%x\n\r", slcr_bade_addr);

    // get_prop_u32(_dtb, "/soc/ethernet/reg", &ethernet_addr);
    ethernet_addr = sbi_read_fdt(ETHERNET_ADDR);
    printk("[ethernet] phy: 0x%x\n\r", ethernet_addr);

    uint32_t plic_addr = 0;
    // get_prop_u32(_dtb, "/soc/interrupt-controller/reg", &plic_addr);
    plic_addr = sbi_read_fdt(PLIC_ADDR);
    printk("[plic] plic: 0x%x\n\r", plic_addr);

    uint32_t nr_irqs = sbi_read_fdt(NR_IRQS);
    // get_prop_u32(_dtb, "/soc/interrupt-controller/riscv,ndev", &nr_irqs);
    printk("[plic] nr_irqs: 0x%x\n\r", nr_irqs);

    XPS_SYS_CTRL_BASEADDR =
        (uintptr_t)ioremap((uint64_t)slcr_bade_addr, NORMAL_PAGE_SIZE);
    xemacps_config.BaseAddress =
        (uintptr_t)ioremap((uint64_t)ethernet_addr, NORMAL_PAGE_SIZE);
    uintptr_t _plic_addr =
        (uintptr_t)ioremap((uint64_t)plic_addr, 0x4000*NORMAL_PAGE_SIZE);
    // XPS_SYS_CTRL_BASEADDR = slcr_bade_addr;
    // xemacps_config.BaseAddress = ethernet_addr;
    xemacps_config.DeviceId        = 0;
    xemacps_config.IsCacheCoherent = 0;

    printk(
        "[slcr_bade_addr] phy:%x virt:%lx\n\r", slcr_bade_addr,
        XPS_SYS_CTRL_BASEADDR);
    printk(
        "[ethernet_addr] phy:%x virt:%lx\n\r", ethernet_addr,
        xemacps_config.BaseAddress);
    printk("[plic_addr] phy:%x virt:%lx\n\r", plic_addr, _plic_addr);
    plic_init(_plic_addr, nr_irqs);
    
    long status = EmacPsInit(&EmacPsInstance);
    if (status != XST_SUCCESS) {
        printk("Error: initialize ethernet driver failed!\n\r");
        assert(0);
    }

    local_flush_tlb_all();
    init_pcb_again();

    // init interrupt (^_^)
    init_exception();
    printk(
        "> [INIT] Interrupt processing initialization "
        "succeeded.\n\r");

    // init system call table (0_0)
    init_syscall();
    printk("> [INIT] System call initialized successfully.\n\r");

    // enable_interrupt();
    net_poll_mode = 0;
    // xemacps_example_main();

    // init screen (QAQ)
    init_screen();
    printk("> [INIT] SCREEN initialization succeeded.\n\r");
    // screen_clear(0, SCREEN_HEIGHT - 1);
}

// jump from bootloader.
// The beginning of everything >_< ~~~~~~~~~~~~~~
int main()
{
    if(get_current_cpu_id() == 0){
        boot_first_core();
        // local_flush_tlb_all();
        smp_init();
        lock_kernel();
        uintptr_t pgdir = 8 + PGDIR_PA + TRANS_OFFSET;
        *(PTE *)pgdir = 0;
        wakeup_other_hart();
    }else{
        lock_kernel();
        setup_exception();
    }
    // TODO:
    // Setup timer interrupt and enable all interrupt

    // enable_interrupt();
    reset_irq_timer();

    while (1) {
        // (QAQQQQQQQQQQQ)
        // If you do non-preemptive scheduling, you need to use it
        // to surrender control do_scheduler();
        enable_interrupt();
        __asm__ __volatile__("wfi\n\r":::);
        //do_scheduler();
    };
    return 0;
}
