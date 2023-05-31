#include <time.h>
#include "test3.h"
#include <stdio.h>
#include <sys/syscall.h>

//对这个测试文件是有改动的,用来测试waitpid等函数
void waiting_task1(void)
{
    int print_location = 1;

    sys_move_cursor(1, print_location);
    printf("> [TASK] This is a waiting task (pid=%d).\n", print_location);

    sys_waitpid(2);

    sys_move_cursor(1, print_location);
    printf("> [TASK] I am waiting task and already exited successfully.\n");

    sys_exit();
}

void waiting_task2(void)
{
    int print_location = 2;

    sys_move_cursor(1, print_location);
    printf("> [TASK] This is a waiting task (pid=%d).\n", print_location);

    sys_waitpid(2);

    sys_move_cursor(1, print_location);
    printf("> [TASK] I am waiting task and already exited successfully.\n");

    sys_exit();
}

void waited_task(void)
{
    int print_location = 3;
    sys_move_cursor(1, print_location);
    printf("> [TASK] I still have 5 seconds to quit.\n");
    
    struct task_info task1 = {(uintptr_t)&waiting_task1, USER_PROCESS};
    struct task_info task2 = {(uintptr_t)&waiting_task2, USER_PROCESS};

    pid_t pid_task1 = sys_spawn(&task1, NULL, AUTO_CLEANUP_ON_EXIT);
    pid_t pid_task2 = sys_spawn(&task2, NULL, AUTO_CLEANUP_ON_EXIT);
    

    sys_sleep(5);
    
    printf("> [TASK] Wake up.\n");
    sys_exit();
}
