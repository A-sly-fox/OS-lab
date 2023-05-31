#include <os/lock.h>
#include <os/sched.h>
#include <atomic.h>

int lock_count = 0;
int barrier_count = 0;
int semaphore_count = 0;
int lock_array[MAX_LOCK];
int barrier_array[MAX_BARRIER];
int semaphore_array[MAX_SEMAPHORE];
int mailbox_array[MAX_MAILBOX];

int mutex_get(int key){
    for(int i = 0; i < MAX_LOCK; i++){
        if(lock_array[i] == key)
            return i;
    }
    return -1;
}

int barrier_get(int key){
    for(int i = 0; i < MAX_BARRIER; i++){
        if(barrier_array[i] == key)
            return i;
    }
    return -1;
}

int semaphore_get(int key){
    for(int i = 0; i < MAX_SEMAPHORE; i++){
        if(semaphore_array[i] == key)
            return i;
    }
    return -1;
}

int mailbox_get(int key){
    for(int i = 0; i < MAX_MAILBOX; i++){
        if(mailbox_array[i] == key)
            return i;
    }
    return -1;
}

void do_mutex_lock_init(void *lock)
{
    int cpu_id = get_current_cpu_id();
    int id = lock_count;
    lock_array[lock_count++] = (int *)lock;
    my_lock_t[id].lock.status = UNLOCKED;
    my_lock_t[id].pid = current_running[cpu_id]->pid;
    init_list_head(&my_lock_t[id].block_queue);
}

void do_mutex_lock_acquire(void *lock)
{
    int cpu_id = get_current_cpu_id();
    int id = mutex_get((int*)lock);
    if(my_lock_t[id].lock.status == LOCKED)
        do_block(&current_running[cpu_id]->list, &my_lock_t[id].block_queue);
    else
        my_lock_t[id].lock.status = LOCKED;
}

void do_mutex_lock_release(void *lock)
{
    int id = mutex_get((int*)lock);
    if(list_empty(&my_lock_t[id].block_queue)){
        my_lock_t[id].pid = -1;
        my_lock_t[id].lock.status = UNLOCKED;
    }else{
        do_unblock(&my_lock_t[id].block_queue);
    }
}

int do_barrier_init(void *barrier, int num)
{
    int id = barrier_count;
    barrier_array[barrier_count++] = (int *)barrier;
    my_barrier_t[id].number = num;
    my_barrier_t[id].count = 0;
    init_list_head(&my_barrier_t[id].block_queue);
    return 0;
}

int do_barrier_wait(void *barrier)
{
    int cpu_id = get_current_cpu_id();
    int id = barrier_get((int*)barrier);
    my_barrier_t[id].count++;
    if(my_barrier_t[id].number == my_barrier_t[id].count){
        while(!list_empty(&my_barrier_t[id].block_queue))
            do_unblock(&my_barrier_t[id].block_queue);
        my_barrier_t[id].count = 0;
    }else{
        do_block(&current_running[cpu_id]->list, &my_barrier_t[id].block_queue);
    }
    return 0;
}

int do_barrier_destroy(void *barrier)
{
    int id = barrier_get((int*)barrier);
    my_barrier_t[id].count = 0;
    my_barrier_t[id].number = 0;
    barrier_array[id] = 0;
    return 0;
}

int do_semaphore_init(void *semaphore, int val)
{
    int id = semaphore_count;
    semaphore_array[semaphore_count++] = (int *)semaphore;
    my_semaphore_t[id].value = val;
    init_list_head(&my_semaphore_t[id].block_queue);
    return 0;
}

int do_semaphore_up(void *semaphore)
{
    int id = semaphore_get((int*)semaphore);
    my_semaphore_t[id].value++;
    if(my_semaphore_t[id].value >= 0 && !list_empty(&my_semaphore_t[id].block_queue)){
        do_unblock(&my_semaphore_t[id].block_queue);
    }
    return 0;
}

int do_semaphore_down(void *semaphore)
{
    int cpu_id = get_current_cpu_id();
    int id = semaphore_get((int*)semaphore);
    my_semaphore_t[id].value--;
    if(my_semaphore_t[id].value < 0){
        do_block(&current_running[cpu_id]->list, &my_semaphore_t[id].block_queue);
    }
    return 0;
}

int do_semaphore_destroy(void *semaphore)
{
    int id = semaphore_get((int*)semaphore);
    my_semaphore_t[id].value = 0;
    semaphore_array[id] = 0;
    return 0;
}

// int do_mbox_open(char *name)
// {
//     int i,j;
//     for(i = 0; i < MAX_MAILBOX; i++){
//         if(my_mailbox[i].valid && !strcmp(name,my_mailbox[i].name)){
//             // return mailbox_array[i];
//             return i;
//         }
//     }
//     for(i = 0; i < MAX_MAILBOX; i++){
//         if(!my_mailbox[i].valid){
//             // mailbox_array[i] = (int *)name;
//             my_mailbox[i].valid = 1;
//             my_mailbox[i].used_space = 0;
//             my_mailbox[i].first = 0;
//             my_mailbox[i].end = 0;
//             my_mailbox[i].full.value = 0;
//             init_list_head(&my_mailbox[i].full.block_queue);
//             for (j = 0; my_mailbox[i].name[j] != '\0'; j++){
//                 my_mailbox[i].name[j] = name[j];
//             }
//             my_mailbox[i].name[j] = '\0';
//             // return mailbox_array[i];
//             return i;
//         }
//     }
//     return -1;
// }

// void do_mbox_close(int mailbox)
// {
//     // int id = mailbox_get(mailbox);
//     int id = mailbox;
//     mailbox_array[id] = 0;
//     my_mailbox[id].valid = 0;
// }

// int do_semaphore_for_mailbox_send(semaphore_t *semaphore, int length)
// {
//     int cpu_id = get_current_cpu_id();
//     int count = 0;
//     int forward_value;
//     while(1){
//         forward_value = semaphore->value + length;
//         if(forward_value > MAX_LENGTH){
//             count++;
//             do_block(&current_running[cpu_id]->list, &semaphore->block_queue);
//         }
//         else if(forward_value >= 0){
//             while(!list_empty(&semaphore->block_queue))
//                 do_unblock(&semaphore->block_queue);
//             break;
//         }
//     }
//     semaphore->value = forward_value;
//     return count;
// }

// int do_semaphore_for_mailbox_recv(semaphore_t *semaphore, int length)
// {
//     int cpu_id = get_current_cpu_id();
//     int count = 0;
//     int forward_value;
//     while(1){
//         forward_value = semaphore->value - length;
//         if(forward_value < 0){
//             count++;
//             do_block(&current_running[cpu_id]->list, &semaphore->block_queue);
//         }
//         else if(forward_value <= MAX_LENGTH){
//             while(!list_empty(&semaphore->block_queue))
//                 do_unblock(&semaphore->block_queue);
//             break;
//         }
//     }
//     semaphore->value = forward_value;
//     return count;
// }

// int do_mbox_send(int mailbox, void *msg, int msg_length)
// {
//     // int id = mailbox_get(mailbox);
//     int id = mailbox;
//     int block = do_semaphore_for_mailbox_send(&my_mailbox[id].full, msg_length);
//     for (int i = 0; i < msg_length; i++){
//         my_mailbox[id].msg[my_mailbox[id].end] = ((char *)msg)[i];
//         my_mailbox[id].end = (my_mailbox[id].end + 1) % MAX_LENGTH;
//         my_mailbox[id].used_space++;
//     }
//     return block;
// }

// int do_mbox_recv(int mailbox, void *msg, int msg_length)
// {
//     // int id = mailbox_get(mailbox);
//     int id = mailbox;
//     int block = do_semaphore_for_mailbox_recv(&my_mailbox[id].full, msg_length);
//     for (int i = 0; i < msg_length; i++){
//         ((char *)msg)[i] = my_mailbox[id].msg[my_mailbox[id].first];
//         my_mailbox[id].first = (my_mailbox[id].first + 1) % MAX_LENGTH;
//         my_mailbox[id].used_space--;
//     }
//     return block;
// }

mailbox_c_t mailbox_other[MBOX_NUM];
int do_mbox_open(char *name)
{
	int i;
	for (i = 0; i < MBOX_NUM; i++)
	{
		if (kstrcmp(name, mailbox_other[i].name) == 0)
		{
			return i;
		}
	}
	for (i = 0; i < MBOX_NUM; i++)
	{
		if (mailbox_other[i].status == NEG)
		{
			mailbox_other[i].status = POS;
			int j = 0;
			while (name[j] != '\0')
			{
				mailbox_other[i].name[j] = name[j];
				j++;
			}
			mailbox_other[i].name[j] = '\0';
			return i;
		}
	}
	prints(">ERROR NO  LEFT mailbox \n");
	return -1;
}
void do_mbox_close(int index)
{
	mailbox_other[index].status = NEG;
}

int do_mbox_send(int id, void *msg, int msg_length)
{
	uint64_t core_id;
	core_id = get_current_cpu_id();
	int block_num = 0;
	while ((mailbox_other[id].datanum + msg_length) > MAX_MBOX_LENGTH)
	{
		block_num++;
		do_block(&current_running[core_id]->list, &mailbox_other[id].full.sem_queue);
	}
	int i;
	for (i = 0; i < msg_length; i++)
	{
		mailbox_other[id].msg[mailbox_other[id].datanum++] = ((char *)msg)[i];
	}
	while (!list_empty(&mailbox_other[id].empty.sem_queue))
	{
		do_unblock(&mailbox_other[id].empty.sem_queue);
	}
	return block_num;
}

int do_mbox_recv(int id, void *msg, int msg_length)
{
	uint64_t core_id;
	core_id = get_current_cpu_id();
	int block_num = 0;
	while ((mailbox_other[id].datanum - msg_length) < 0)
	{
		block_num++;
		do_block(&current_running[core_id]->list, &mailbox_other[id].empty.sem_queue);
	}
	int i;
	for (i = 0; i < mailbox_other[id].datanum; i++)
	{
		((char *)msg)[i] = mailbox_other[id].msg[i];
		if (i + msg_length < MAX_MBOX_LENGTH)
			mailbox_other[id].msg[i] = mailbox_other[id].msg[i + msg_length];
	}
	mailbox_other[id].datanum -= msg_length;
	while (!list_empty(&mailbox_other[id].full.sem_queue))
	{
		do_unblock(&mailbox_other[id].full.sem_queue);
	}
	return block_num;
}

void spin_lock_init(spin_lock_t *lock){
    lock->status = UNLOCKED;
}

void spin_lock_acquire(spin_lock_t *lock){
    while((atomic_swap_d(LOCKED, &lock->status)) == LOCKED)
        ;
}

void spin_lock_release(spin_lock_t *lock){
    lock->status = UNLOCKED;
}