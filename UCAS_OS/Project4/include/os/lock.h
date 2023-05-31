/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                                   Thread Lock
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
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

#ifndef INCLUDE_LOCK_H_
#define INCLUDE_LOCK_H_

#include <os/list.h>

#define MAX_LOCK 16
#define MAX_BARRIER 16
#define MAX_SEMAPHORE 16
#define MAX_LENGTH (64)
#define MAX_MAILBOX 16

typedef enum {
    UNLOCKED,
    LOCKED,
} lock_status_t;

typedef struct spin_lock
{
    volatile lock_status_t status;
} spin_lock_t;

typedef struct mutex_lock
{
    spin_lock_t lock;
    list_head block_queue;
    pid_t pid;
} mutex_lock_t;

typedef struct barrier
{
    int count;
    int number;
    list_head block_queue;
} barrier_t;

typedef struct semaphore
{
    int value;
    list_head block_queue;
} semaphore_t;

typedef struct mailbox
{
    int valid;
    int used_space;
    char name[25];
    char msg[MAX_LENGTH];
    int first;
    int end;
    semaphore_t full;
} mailbox_tt;

mutex_lock_t my_lock_t[MAX_LOCK];
mutex_lock_t my_binsem[MAX_LOCK];
barrier_t my_barrier_t[MAX_BARRIER];
semaphore_t my_semaphore_t[MAX_SEMAPHORE];
mailbox_tt my_mailbox[MAX_MAILBOX];

#define MAX_MBOX_LENGTH (64)
#define MBOX_NUM 20
typedef enum
{
    NEG,
    POS,
} state;

typedef struct semaphoret
{
    state status;
    int semnum;
    list_head sem_queue;
} semaphore_tt;

typedef struct mailbox_c
{
    char name[64];
    char msg[MAX_MBOX_LENGTH];
    int datanum;
    state status;
    semaphore_tt empty;
    semaphore_tt full;

} mailbox_c_t;

extern mailbox_c_t mailbox_other[MBOX_NUM];

/* init lock */
void spin_lock_init(spin_lock_t *lock);
int spin_lock_try_acquire(spin_lock_t *lock);
void spin_lock_acquire(spin_lock_t *lock);
void spin_lock_release(spin_lock_t *lock);

void do_mutex_lock_init(void *lock);
void do_mutex_lock_acquire(void *lock);
void do_mutex_lock_release(void *lock);

/*barrier*/
int do_barrier_init(void *barrier, int count);
int do_barrier_wait(void *barrier);
int do_barrier_destroy(void *barrier);

/*semaphore*/
int do_semaphore_init(void *semaphore, int val);
int do_semaphore_up(void *semaphore);
int do_semaphore_down(void *semaphore);
int do_semaphore_destroy(void *semaphore);

/*mailbox*/
int do_mbox_open(char *name);
void do_mbox_close(int mailbox);
int do_mbox_send(int mailbox, void *msg, int msg_length);
int do_mbox_recv(int mailbox, void *msg, int msg_length);

#endif
