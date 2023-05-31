#include <stdio.h>
#include <sys/syscall.h>
#include <test2.h>

void priority_task1(void)
{
    int i;
    int print_location = 5;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is to test priority.(init_priority = 30) (%d)", i);
        //sys_yield();
    }
}

void priority_task2(void)
{
    int i;
    int print_location = 6;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is to test priority.(init_priority = 20) (%d)", i);
        //sys_yield();
    }
}

void priority_task3(void)
{
    int i;
    int print_location = 7;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is to test priority.(init_priority = 10) (%d)", i);
        //sys_yield();
    }
}
