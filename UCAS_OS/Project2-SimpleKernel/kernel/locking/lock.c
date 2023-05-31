#include <os/lock.h>
#include <os/sched.h>
#include <atomic.h>

mutex_lock_t my_lock_t[MAX_LOCK];
int count = 0;
int array[MAX_LOCK];

int mutex_get(int key){
    for(int i = 0; i < MAX_LOCK; i++){
        if(array[i] == key)
            return i;
    }
    return -1;
}


void do_mutex_lock_init(void *lock)
{
    /* TODO */
    int id = count;
    array[count++] = *(int *)lock;
    my_lock_t[id].lock.status = UNLOCKED;
    init_list_head(&my_lock_t[id].block_queue);
}

void do_mutex_lock_acquire(void *lock)
{
    /* TODO */
    int id = mutex_get(*(int*)lock);
    if(my_lock_t[id].lock.status == LOCKED)
        do_block(&current_running->list, &my_lock_t[id].block_queue);
    else
        my_lock_t[id].lock.status = LOCKED;
}

void do_mutex_lock_release(void *lock)
{
    /* TODO */
    int id = mutex_get(*(int*)lock);
    if(list_empty(&my_lock_t[id].block_queue))
        my_lock_t[id].lock.status = UNLOCKED;
    else{
        do_unblock(&my_lock_t[id].block_queue);
    }
}
