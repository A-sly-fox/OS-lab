#include <os/time.h>
#include <os/mm.h>
#include <os/irq.h>
#include <type.h>

#include <emacps/xemacps_example.h>
#include <emacps/xemacps_hw.h>
#include <emacps/xemacps_bdring.h>
#include <emacps/xparameters.h>
#include <emacps/xemacps.h>
#include <plic.h>

LIST_HEAD(sleeping_queue);

uint64_t time_elapsed = 0;
uint32_t time_base = 0;

void dma_check()
{
    volatile u32 TXSR_reg;
    volatile u32 RXSR_reg;
    if(!list_empty(&net_send_queue)){
        TXSR_reg = XEmacPs_ReadReg(EmacPsInstance.Config.BaseAddress, XEMACPS_TXSR_OFFSET);
        if (TXSR_reg & XEMACPS_TXSR_TXCOMPL_MASK){
            XEmacPs_WriteReg(EmacPsInstance.Config.BaseAddress, XEMACPS_TXSR_OFFSET, TXSR_reg | XEMACPS_TXSR_TXCOMPL_MASK);
            do_unblock(&net_send_queue);
        }
    }
    
    if(!list_empty(&net_recv_queue)){
        RXSR_reg = XEmacPs_ReadReg(EmacPsInstance.Config.BaseAddress, XEMACPS_RXSR_OFFSET);
        if (RXSR_reg & XEMACPS_RXSR_FRAMERX_MASK){
            XEmacPs_WriteReg(EmacPsInstance.Config.BaseAddress, XEMACPS_RXSR_OFFSET, RXSR_reg | XEMACPS_RXSR_FRAMERX_MASK);
            do_unblock(&net_recv_queue);
        }
    }
}

void check_sleep()
{
    pcb_t *sleep_pcb;
    list_node_t *next_node = sleeping_queue.next;
    while(next_node != &sleeping_queue){
        sleep_pcb = list_entry(next_node, pcb_t, list);
        uint64_t current_ticks = get_ticks();
        if(current_ticks >= sleep_pcb->wake_up_time * time_base){
            list_del(next_node);
            sleep_pcb->status = TASK_READY;
            list_add_tail(&sleep_pcb->list, &ready_queue);
            next_node = sleeping_queue.next;
        }else{
            next_node = next_node -> next;
        }
    } 
}

uint64_t get_ticks()
{
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(time_elapsed));
    return time_elapsed;
}

uint64_t get_timer()
{
    return get_ticks() / time_base;
}

uint64_t get_time_base()
{
    return time_base;
}

void latency(uint64_t time)
{
    uint64_t begin_time = get_timer();

    while (get_timer() - begin_time < time);
    return;
}

uint32_t my_sys_get_wall_time(uint32_t * time_elapsed){
    * time_elapsed = get_ticks();
    return time_base;
}