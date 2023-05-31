#include <sbi.h>
#include <atomic.h>
#include <os/sched.h>
#include <os/smp.h>
#include <os/lock.h>

spin_lock_t big_kernel_lock;

void smp_init()
{
    /* TODO: */
    spin_lock_init(&big_kernel_lock);
}

void wakeup_other_hart()
{
    /* TODO: */
    sbi_send_ipi(NULL); 
    __asm__ __volatile__("csrw sip, zero\n");
}

void lock_kernel()
{
    /* TODO: */
    spin_lock_acquire(&big_kernel_lock);
}

void unlock_kernel()
{
    /* TODO: */
    spin_lock_release(&big_kernel_lock);
}

