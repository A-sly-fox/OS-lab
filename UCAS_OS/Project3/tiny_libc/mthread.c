#include <stdatomic.h>
#include <mthread.h>
#include <sys/syscall.h>


int mthread_mutex_init(void* handle)
{
    /* TODO: */
    sys_mutex_lock_init(handle);
    //do_mutex_lock_init(a);
    return 0;
}
int mthread_mutex_lock(void* handle) 
{
    /* TODO: */
    sys_mutex_lock_acquire(handle);
    //do_mutex_lock_acquire(a);
    return 0;
}
int mthread_mutex_unlock(void* handle)
{
    /* TODO: */
    sys_mutex_lock_release(handle);
    //do_mutex_lock_release(a);
    return 0;
}

int mthread_barrier_init(void* handle, unsigned count)
{
    // TODO:
    return sys_barrier_init(handle,count);
}
int mthread_barrier_wait(void* handle)
{
    // TODO:
    return sys_barrier_wait(handle);
}
int mthread_barrier_destroy(void* handle)
{
    // TODO:
    return sys_barrier_destroy(handle);
}

int mthread_semaphore_init(void* handle, int val)
{
    // TODO:
    return sys_semaphore_init(handle,val);
}
int mthread_semaphore_up(void* handle)
{
    // TODO:
    return sys_semaphore_up(handle);
}
int mthread_semaphore_down(void* handle)
{
    // TODO:
    return sys_semaphore_down(handle);
}
int mthread_semaphore_destroy(void* handle)
{
    // TODO:
    return sys_semaphore_destroy(handle);
}
