#include <stdatomic.h>
#include <mthread.h>
#include <sys/syscall.h>


int mthread_mutex_init(void* handle)
{
    /* TODO: */
    mthread_mutex_t *a = (mthread_mutex_t *)handle;
    sys_mutex_lock_init(a->my_lock);
    //do_mutex_lock_init(a->my_lock);
    return 0;
}
int mthread_mutex_lock(void* handle) 
{
    /* TODO: */
    mthread_mutex_t *a = (mthread_mutex_t *)handle;
    sys_mutex_lock_acquire(a->my_lock);
    //do_mutex_lock_acquire(a->my_lock);
    return 0;
}
int mthread_mutex_unlock(void* handle)
{
    /* TODO: */
    mthread_mutex_t *a = (mthread_mutex_t *)handle;
    sys_mutex_lock_release(a->my_lock);
    //do_mutex_lock_release(a->my_lock);
    return 0;
}
