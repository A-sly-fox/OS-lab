#include <sys/syscall.h>
#include <stdint.h>

void sys_sleep(uint32_t time)
{
    // TODO:
    invoke_syscall(SYSCALL_SLEEP, time, IGNORE, IGNORE, IGNORE);
}

void sys_write(char *buff)
{
    // TODO:
    invoke_syscall(SYSCALL_WRITE, (uintptr_t)buff, IGNORE, IGNORE, IGNORE);
}

int sys_read()
{
    // TODO:
    return invoke_syscall(SYSCALL_READ, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_reflush()
{
    // TODO:
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_move_cursor(int x, int y)
{
    // TODO:
    //vt100_move_cursor(x,y);
    invoke_syscall(SYSCALL_CURSOR, x, y, IGNORE, IGNORE);
}

long sys_get_timebase()
{
    // TODO:
    return invoke_syscall(SYSCALL_GET_TIMEBASE, IGNORE, IGNORE, IGNORE, IGNORE);
}

long sys_get_tick()
{
    // TODO:
    return invoke_syscall(SYSCALL_GET_TICK, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_yield()
{
    // TODO:
    invoke_syscall(SYSCALL_YIELD, IGNORE, IGNORE, IGNORE, IGNORE);
    //   or
    //do_scheduler();
    // ???
}

void sys_mutex_lock_init(void *lock)
{
    invoke_syscall(SYSCALL_MUTEX_INIT, lock, IGNORE, IGNORE, IGNORE);
}

void sys_mutex_lock_acquire(void *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK, lock, IGNORE, IGNORE, IGNORE);
}

void sys_mutex_lock_release(void *lock)
{
    invoke_syscall(SYSCALL_MUTEX_UNLOCK, lock, IGNORE, IGNORE, IGNORE);
}

uint32_t sys_get_wall_time(void *time_elapsed)
{
    return invoke_syscall(SYSCALL_GET_WALL_TIME, (uint32_t *)time_elapsed, IGNORE, IGNORE, IGNORE);
}

void sys_screen_clear()
{
    invoke_syscall(SYSCALL_SCREEN_CLEAR, IGNORE, IGNORE, IGNORE, IGNORE);
}

int sys_ps()
{
    return invoke_syscall(SYSCALL_PS, IGNORE, IGNORE, IGNORE, IGNORE);
}

pid_t sys_spawn(task_info_t *info, void* arg, spawn_mode_t mode)
{
    return invoke_syscall(SYSCALL_SPAWN, info, arg, mode, IGNORE);
}

int sys_kill(pid_t pid)
{
    return invoke_syscall(SYSCALL_KILL, pid, IGNORE, IGNORE, IGNORE);
}

int sys_waitpid(pid_t pid)
{
    return invoke_syscall(SYSCALL_WAITPID, pid, IGNORE, IGNORE, IGNORE);
}

void sys_exit()
{
    invoke_syscall(SYSCALL_EXIT, IGNORE, IGNORE, IGNORE, IGNORE);
}

pid_t sys_getpid()
{
    return invoke_syscall(SYSCALL_GETPID, IGNORE, IGNORE, IGNORE, IGNORE);
}

int sys_barrier_init(void *barrier, int num)
{
    return invoke_syscall(SYSCALL_BARRIER_INIT, barrier, num, IGNORE, IGNORE);
}

int sys_barrier_wait(void *barrier)
{
    return invoke_syscall(SYSCALL_BARRIER_WAIT, barrier, IGNORE, IGNORE, IGNORE);
}

int sys_barrier_destroy(void *barrier)
{
    return invoke_syscall(SYSCALL_BARRIER_DESTROY, barrier, IGNORE, IGNORE, IGNORE);
}

int sys_semaphore_init(void* handle, int val)
{
    return invoke_syscall(SYSCALL_SEMAPHORE_INIT, handle, val, IGNORE, IGNORE);
}

int sys_semaphore_up(void* handle)
{
    return invoke_syscall(SYSCALL_SEMAPHORE_UP, handle, IGNORE, IGNORE, IGNORE);
}

int sys_semaphore_down(void* handle)
{
    return invoke_syscall(SYSCALL_SEMAPHORE_DOWN, handle, IGNORE, IGNORE, IGNORE);
}

int sys_semaphore_destroy(void* handle)
{
    return invoke_syscall(SYSCALL_SEMAPHORE_DESTROY, handle, IGNORE, IGNORE, IGNORE);
}

int sys_mbox_open(char *name)
{
    return invoke_syscall(SYSCALL_MBOX_OPEN, name, IGNORE, IGNORE, IGNORE);
}

void sys_mbox_close(int mailbox)
{
    invoke_syscall(SYSCALL_MBOX_CLOSE, mailbox, IGNORE, IGNORE, IGNORE);
}

int sys_mbox_send(int mailbox, void *msg, int msg_length)
{
    return invoke_syscall(SYSCALL_MBOX_SEND, mailbox, msg, msg_length, IGNORE);
}

int sys_mbox_recv(int mailbox, void *msg, int msg_length)
{
    return invoke_syscall(SYSCALL_MBOX_RECV, mailbox, msg, msg_length, IGNORE);
}

void sys_taskset(task_info_t *task, int mask)
{
    invoke_syscall(SYSCALL_TASKSET, task, mask, IGNORE, IGNORE);
}

void sys_taskset_p(int mask, int pid)
{
    invoke_syscall(SYSCALL_TASKSET_P, mask, pid, IGNORE, IGNORE);
}

void sys_ls(void)
{
    invoke_syscall(SYSCALL_LS, IGNORE, IGNORE, IGNORE, IGNORE);
}

pid_t sys_exec(const char *file_name, int argc, char* argv[], spawn_mode_t mode)
{
    return invoke_syscall(SYSCALL_EXEC, (uintptr_t)file_name, argc, (uintptr_t)argv, mode);
}

int binsemget(int key)
{
    return invoke_syscall(SYSCALL_BINSEMGET, key, IGNORE, IGNORE, IGNORE);
}

int binsemop(int binsem_id, int op)
{
    return invoke_syscall(SYSCALL_BINSEMOP, binsem_id, op, IGNORE, IGNORE);
}