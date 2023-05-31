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

#include <common.h>
#include <os/irq.h>
#include <os/mm.h>
#include <os/sched.h>
#include <screen.h>
#include <sbi.h>
#include <stdio.h>
#include <os/time.h>
#include <os/syscall.h>
#include <test.h>

#include <csr.h>
#include <os/lock.h>

extern void ret_from_exception();
extern void __global_pointer$();

static void init_pcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    pcb_t *pcb)
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
    pt_regs->sepc = entry_point;
    pt_regs->sstatus = 0;
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

static void init_pcb()
{
    /* initialize all of your pcb and add them into ready_queue
    * TODO:
    */
    int priority[3] = {30,20,10};

    /**for(int i = 0; i < num_priority_tasks; i++){**/
    for(int i = 0; i < num_timer_tasks + num_lock2_tasks + num_sched2_tasks; i++){

        pcb[i].kernel_sp = allocPage(1) + PAGE_SIZE;
        pcb[i].user_sp = allocPage(1) + PAGE_SIZE;
        
        if(i < num_timer_tasks){
            pcb[i].type = timer_tasks[i]->type;
            init_pcb_stack(pcb[i].kernel_sp, pcb[i].user_sp, timer_tasks[i]->entry_point,&pcb[i]);
        }else if(i < num_timer_tasks + num_lock2_tasks){
            pcb[i].type = lock2_tasks[i-num_timer_tasks]->type;
            init_pcb_stack(pcb[i].kernel_sp, pcb[i].user_sp, lock2_tasks[i-num_timer_tasks]->entry_point,&pcb[i]);
        }else{
            pcb[i].type = sched2_tasks[i-num_timer_tasks - num_lock2_tasks]->type;
            init_pcb_stack(pcb[i].kernel_sp, pcb[i].user_sp, sched2_tasks[i-num_timer_tasks - num_lock2_tasks]->entry_point,&pcb[i]);
        }
        /**pcb[i].type = priority_tasks[i]->type;
        init_pcb_stack(pcb[i].kernel_sp, pcb[i].user_sp, priority_tasks[i]->entry_point,&pcb[i]);
        pcb[i].init_priority = priority[i];
        pcb[i].now_priority = priority[i];**/

        list_add_tail(&pcb[i].list,&ready_queue);
        
        pcb[i].pid = i + 1;
        pcb[i].status = TASK_READY;
        pcb[i].cursor_x = 1;
        pcb[i].cursor_y = 1;
        pcb[i].kernel_sp = pcb[i].kernel_sp - sizeof(regs_context_t)-sizeof(switchto_context_t);
        pcb[i].user_sp = pcb[i].user_sp;
    }
    /* remember to initialize `current_running`
     * TODO:
     */
    current_running = &pid0_pcb;
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
    syscall[SYSCALL_CURSOR] = &screen_move_cursor;
    //syscall[SYSCALL_CURSOR] = &vt100_move_cursor;
    syscall[SYSCALL_REFLUSH] = &screen_reflush;

    syscall[SYSCALL_GET_TIMEBASE] = &get_time_base;
    syscall[SYSCALL_GET_TICK] = &get_ticks;

    syscall[SYSCALL_MUTEX_INIT] = &do_mutex_lock_init;
    syscall[SYSCALL_MUTEX_LOCK] = &do_mutex_lock_acquire;
    syscall[SYSCALL_MUTEX_UNLOCK] = &do_mutex_lock_release;
    syscall[SYSCALL_GET_WALL_TIME] = &my_sys_get_wall_time;
}

// jump from bootloader.
// The beginning of everything >_< ~~~~~~~~~~~~~~
int main()
{
    // init Process Control Block (-_-!)
    init_pcb();
    printk("> [INIT] PCB initialization succeeded.\n\r");

    // read CPU frequency
    time_base = sbi_read_fdt(TIMEBASE);

    // init interrupt (^_^)
    init_exception();
    printk("> [INIT] Interrupt processing initialization succeeded.\n\r");

    // init system call table (0_0)
    init_syscall();
    printk("> [INIT] System call initialized successfully.\n\r");

    // fdt_print(riscv_dtb);

    // init screen (QAQ)
    init_screen();
    printk("> [INIT] SCREEN initialization succeeded.\n\r");

    // TODO:
    // Setup timer interrupt and enable all interrupt
    
    enable_interrupt();
    reset_irq_timer();

    while (1) {
        // (QAQQQQQQQQQQQ)
        // If you do non-preemptive scheduling, you need to use it
        // to surrender control do_scheduler();
        // enable_interrupt();
        // __asm__ __volatile__("wfi\n\r":::);
        //do_scheduler();
    };
    return 0;
}
