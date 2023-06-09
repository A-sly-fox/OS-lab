/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                       System call related processing
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

#ifndef OS_SYSCALL_NUMBER_H_
#define OS_SYSCALL_NUMBER_H_

#define IGNORE 0
#define NUM_SYSCALLS 64

/* define */
#define SYSCALL_SPAWN 0
#define SYSCALL_EXIT 1
#define SYSCALL_SLEEP 2
#define SYSCALL_KILL 3
#define SYSCALL_WAITPID 4
#define SYSCALL_PS 5
#define SYSCALL_GETPID 6
#define SYSCALL_YIELD 7

#define SYSCALL_FUTEX_WAIT 8
#define SYSCALL_FUTEX_WAKEUP 9

#define SYSCALL_WRITE 10
#define SYSCALL_READ 11
#define SYSCALL_CURSOR 12
#define SYSCALL_REFLUSH 13
#define SYSCALL_SERIAL_READ 14
#define SYSCALL_SERIAL_WRITE 15
#define SYSCALL_READ_SHELL_BUFF 16
#define SYSCALL_SCREEN_CLEAR 17

#define SYSCALL_GET_TIMEBASE 18
#define SYSCALL_GET_TICK 19
#define SYSCALL_GET_CHAR 20

#define SYSCALL_MUTEX_INIT 21
#define SYSCALL_MUTEX_LOCK 22
#define SYSCALL_MUTEX_UNLOCK 23
#define SYSCALL_GET_WALL_TIME 24

#define SYSCALL_BARRIER_INIT 25
#define SYSCALL_BARRIER_WAIT 26
#define SYSCALL_BARRIER_DESTROY 27
#define SYSCALL_SEMAPHORE_INIT 28
#define SYSCALL_SEMAPHORE_UP 29
#define SYSCALL_SEMAPHORE_DOWN 30
#define SYSCALL_SEMAPHORE_DESTROY 31
#define SYSCALL_MBOX_OPEN 32
#define SYSCALL_MBOX_CLOSE 33
#define SYSCALL_MBOX_SEND 34
#define SYSCALL_MBOX_RECV 35
#define SYSCALL_TASKSET 36
#define SYSCALL_TASKSET_P 37
#define SYSCALL_LS 38
#define SYSCALL_EXEC 39

#define SYSCALL_BINSEMGET 40
#define SYSCALL_BINSEMOP 41
#define SYSCALL_MCREATE 42
#define SYSCALL_MJOIN 43
#define SYSCALL_NET_RECV 44
#define SYSCALL_NET_SEND 45
#define SYSCALL_NET_IRQ_MODE 46

#define SYSCALL_MKFS 47
#define SYSCALL_STATFS 48
#define SYSCALL_FS_LS 49
#define SYSCALL_MKDIR 50
#define SYSCALL_CD 51
#define SYSCALL_RMDIR 52
#define SYSCALL_TOUCH 53
#define SYSCALL_FOPEN 54
#define SYSCALL_FWRITE 55
#define SYSCALL_FREAD 56
#define SYSCALL_CLOSE 57
#define SYSCALL_CAT 58
#define SYSCALL_FS_LS_L 59
#define SYSCALL_LN 60
#define SYSCALL_RM 61
#define SYSCALL_LSEEK 62

#endif
