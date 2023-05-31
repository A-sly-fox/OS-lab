#include <os/list.h>
#include <os/mm.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/irq.h>
#include <screen.h>
#include <stdio.h>
#include <assert.h>

#include <sbi.h>

pcb_t pcb[NUM_MAX_TASK];
const ptr_t pid0_stack = INIT_KERNEL_STACK + PAGE_SIZE;
pcb_t pid0_pcb = {
    .pid = 0,
    .kernel_sp = (ptr_t)pid0_stack,
    .user_sp = (ptr_t)pid0_stack,
    .preempt_count = 0
};

LIST_HEAD(ready_queue);

/* current running task PCB */
pcb_t * volatile current_running;

/* global process id */
pid_t process_id = 1;

void do_scheduler(void)
{
    // TODO schedule
    // Modify the current_running pointer.
    //screen_reflush();
    //check_sleep();
    //sbi_set_timer(get_ticks() + time_base / INTERRUPT_NUM);
    __asm__ __volatile__("csrr x0, sscratch\n");
    if(list_empty(&ready_queue))
        return;
    pcb_t *prev_running;
    list_node_t * next_node, * priority_node;
    prev_running = current_running;
    next_node = ready_queue.next;
    int highest_priority = 0;
    
    if(current_running->pid == 0){
        prev_running->status = TASK_READY;
    }else if(current_running->status == TASK_RUNNING){
        prev_running->status = TASK_READY;
        list_del(next_node);
        list_add_tail(next_node, &ready_queue);
    }
    current_running = list_entry(ready_queue.next,pcb_t,list);
    /**while(next_node != &ready_queue){
        current_running = list_entry(next_node,pcb_t,list);
        if(current_running->now_priority > highest_priority){
            priority_node = next_node;
            highest_priority = current_running->now_priority;
        }
        next_node = next_node->next;
    }
    next_node = ready_queue.next;
    while(next_node != &ready_queue){
        current_running = list_entry(next_node,pcb_t,list);
        current_running->now_priority += 3; 
        next_node = next_node->next;
    }

    current_running = list_entry(priority_node,pcb_t,list);
    current_running->now_priority = current_running->init_priority;**/
    
    current_running->status=TASK_RUNNING;
    // restore the current_runnint's cursor_x and cursor_y
    vt100_move_cursor(current_running->cursor_x,
                      current_running->cursor_y);
    screen_cursor_x = current_running->cursor_x;
    screen_cursor_y = current_running->cursor_y;
    
    // TODO: switch_to current_running
    switch_to(prev_running,current_running);
}

void do_sleep(uint32_t sleep_time)
{
    // TODO: sleep(seconds)
    // note: you can assume: 1 second = `timebase` ticks
    // 1. block the current_running
    current_running->status = TASK_BLOCKED;
    // 2. create a timer which calls `do_unblock` when timeout
    uint64_t current_time = get_timer();
    current_running->wake_up_time = sleep_time + current_time;
    list_del(&current_running->list);
    list_add_tail(&current_running->list, &sleeping_queue);
    // 3. reschedule because the current_running is blocked.
    do_scheduler();
}

void do_block(list_node_t *pcb_node, list_head *queue)
{
    // TODO: block the pcb task into the block queue
    ready_queue.next->next->prev = &ready_queue;
    ready_queue.next = ready_queue.next->next;

    list_add_tail(pcb_node, queue);
    current_running->status = TASK_BLOCKED;
    do_scheduler();
}

void do_unblock(list_node_t *pcb_node)
{
    // TODO: unblock the `pcb` from the block queue
    pcb_t* unblock_task = list_entry(pcb_node->next, pcb_t, list);
    list_del(pcb_node->next);
    unblock_task->status = TASK_READY;
    list_add_tail(&unblock_task->list, &ready_queue);
}
