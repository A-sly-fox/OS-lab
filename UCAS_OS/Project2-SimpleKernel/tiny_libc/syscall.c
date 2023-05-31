#include <sys/syscall.h>
#include <stdint.h>

void sys_sleep(uint32_t time)
{
    // TODO:
    invoke_syscall(SYSCALL_SLEEP, time, IGNORE, IGNORE);
}

void sys_write(char *buff)
{
    // TODO:
    invoke_syscall(SYSCALL_WRITE, (uintptr_t)buff, IGNORE, IGNORE);
}

void sys_reflush()
{
    // TODO:
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE);
}

void sys_move_cursor(int x, int y)
{
    // TODO:
    //vt100_move_cursor(x,y);
    invoke_syscall(SYSCALL_CURSOR, x, y, IGNORE);
}

long sys_get_timebase()
{
    // TODO:
    return invoke_syscall(SYSCALL_GET_TIMEBASE, IGNORE, IGNORE, IGNORE);
}

long sys_get_tick()
{
    // TODO:
    return invoke_syscall(SYSCALL_GET_TICK, IGNORE, IGNORE, IGNORE);
}

void sys_yield()
{
    // TODO:
    invoke_syscall(SYSCALL_YIELD, IGNORE, IGNORE, IGNORE);
    //   or
    //do_scheduler();
    // ???
}

void sys_mutex_lock_init(void *lock)
{
    invoke_syscall(SYSCALL_MUTEX_INIT, lock, IGNORE, IGNORE);
}

void sys_mutex_lock_acquire(void *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK, lock, IGNORE, IGNORE);
}

void sys_mutex_lock_release(void *lock)
{
    invoke_syscall(SYSCALL_MUTEX_UNLOCK, lock, IGNORE, IGNORE);
}

uint32_t sys_get_wall_time(void *time_elapsed)
{
    return invoke_syscall(SYSCALL_GET_WALL_TIME, (uint32_t *)time_elapsed, IGNORE, IGNORE);
}