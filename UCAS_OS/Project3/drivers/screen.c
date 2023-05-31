#include <screen.h>
#include <common.h>
#include <stdio.h>
#include <os/string.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/irq.h>

#define SCREEN_WIDTH    80
#define SCREEN_HEIGHT   50

//int screen_cursor_x;
//int screen_cursor_y;

/* screen buffer */
char new_screen[SCREEN_HEIGHT * SCREEN_WIDTH] = {0};
char old_screen[SCREEN_HEIGHT * SCREEN_WIDTH] = {0};

/* cursor position */
void vt100_move_cursor(int x, int y)
{
    // \033[y;xH
    disable_preempt();
    int cpu_id = get_current_cpu_id();
    printk("%c[%d;%dH", 27, y, x);
    current_running[cpu_id]->cursor_x = x;
    current_running[cpu_id]->cursor_y = y;
    enable_preempt();
}

/* clear screen */
static void vt100_clear()
{
    // \033[2J
    printk("%c[2J", 27);
}

/* hidden cursor */
static void vt100_hidden_cursor()
{
    // \033[?25l
    printk("%c[?25l", 27);
}

/* write a char */
static void screen_write_ch(char ch)
{
    int cpu_id = get_current_cpu_id();
    if (ch == '\n')
    {
        current_running[cpu_id]->cursor_x = 1;
        current_running[cpu_id]->cursor_y++;
    }
    else
    {
        new_screen[(current_running[cpu_id]->cursor_y - 1) * SCREEN_WIDTH + (current_running[cpu_id]->cursor_x - 1)] = ch;
        current_running[cpu_id]->cursor_x++;
    }
}

void init_screen(void)
{
    vt100_hidden_cursor();
    vt100_clear();
}

void screen_clear(void)
{
    int cpu_id = get_current_cpu_id();
    int i, j;
    for (i = 0; i < SCREEN_HEIGHT; i++)
    {
        for (j = 0; j < SCREEN_WIDTH; j++)
        {
            new_screen[i * SCREEN_WIDTH + j] = ' ';
        }
    }
    current_running[cpu_id]->cursor_x = 1;
    current_running[cpu_id]->cursor_y = 1;
    screen_reflush();
}

void screen_move_cursor(int x, int y)
{
    int cpu_id = get_current_cpu_id();
    current_running[cpu_id]->cursor_x = x;
    current_running[cpu_id]->cursor_y = y;
}

void screen_write(char *buff)
{
    int i = 0;
    int l = kstrlen(buff);

    for (i = 0; i < l; i++)
    {
        screen_write_ch(buff[i]);
    }
}

/*
 * This function is used to print the serial port when the clock
 * interrupt is triggered. However, we need to pay attention to
 * the fact that in order to speed up printing, we only refresh
 * the characters that have been modified since this time.
 */
void screen_reflush(void)
{
    int i, j;
    int cpu_id = get_current_cpu_id();
    int store_x = current_running[cpu_id]->cursor_x;
    int store_y = current_running[cpu_id]->cursor_y;

    /* here to reflush screen buffer to serial port */
    for (i = 0; i < SCREEN_HEIGHT; i++)
    {
        for (j = 0; j < SCREEN_WIDTH; j++)
        {
            /* We only print the data of the modified location. */
            if (new_screen[i * SCREEN_WIDTH + j] != old_screen[i * SCREEN_WIDTH + j])
            {
                vt100_move_cursor(j + 1, i + 1);
                port_write_ch(new_screen[i * SCREEN_WIDTH + j]);
                old_screen[i * SCREEN_WIDTH + j] = new_screen[i * SCREEN_WIDTH + j];
            }
        }
    }

    /* recover cursor position */
    vt100_move_cursor(store_x, store_y);
    current_running[cpu_id]->cursor_x = store_x;
    current_running[cpu_id]->cursor_y = store_y;
}
