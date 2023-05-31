/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode.
 *       The main function is to make system calls through the user's output.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
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

#include <test.h>
#include <string.h>
#include <os.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
#define BUFFER_MAX 20

struct task_info task_test_waitpid = {
    (uintptr_t)&wait_exit_task, USER_PROCESS};
struct task_info task_test_semaphore = {
    (uintptr_t)&semaphore_add_task1, USER_PROCESS};
struct task_info task_test_barrier = {
    (uintptr_t)&test_barrier, USER_PROCESS};
    
struct task_info strserver_task = {(uintptr_t)&strServer, USER_PROCESS};
struct task_info strgenerator_task = {(uintptr_t)&strGenerator, USER_PROCESS};

struct task_info task_test_multicore = {(uintptr_t)&test_multicore, USER_PROCESS};
struct task_info task_test_affinity = {(uintptr_t)&test_affinity, USER_PROCESS};

static struct task_info *test_tasks[16] = {&task_test_waitpid,
                                           &task_test_semaphore,
                                           &task_test_barrier,
                                           &task_test_multicore,
                                           &strserver_task, &strgenerator_task,&task_test_affinity};
static int num_test_tasks = 8;

#define SHELL_BEGIN 25

char my_getchar(){
    int c;
    while((c=sys_read())==-1);
    return (char)c;
}

int row = 1;
int valid_pcb;
void task(char *buffer)
{
    int parameter_empty = 0;
    char command[BUFFER_MAX];
    int parameter = 0,i,j,error = 0,parameter2 = 0;
    for (j = 0; j < BUFFER_MAX; j++)
        command[j] = '\0';
    i = j = 0;
    while (buffer[i] == ' ')
        i++;
    while (buffer[i] != '\0'){
        while (buffer[i] == ' ')
            i++;
        if(buffer[i] <= '9' && buffer[i] >= '0')
            break;
        else if(buffer[i] == '\0')
            break;
        command[j++] = buffer[i++];
    }
    if(buffer[i] == '\0')
        ;
    else if(buffer[i] != '\0' && buffer[i+1] != 'x'){
        for (;;i++){
            if(buffer[i] == '\0' || buffer[i] == ' '){
                break;
            }else if(buffer[i] <= '9' && buffer[i] >= '0'){
                parameter = parameter * 10 + buffer[i] - '0';
                parameter_empty++;
            }else{
                error = 1;
                break;
            }
        }
    }
    else if(buffer[i] != '\0' && buffer[i+1] == 'x'){
        i+=2;
        for (;;i++){
            if(buffer[i] == '\0' || buffer[i] == ' '){
                break;
            }else if(buffer[i] <= '9' && buffer[i] >= '0'){
                parameter = parameter * 10 + buffer[i] - '0';
                parameter_empty++;
            }else{
                error = 1;
                break;
            }
        }
        while (buffer[i] == ' ')
            i++;
        for (;;i++){
            if(buffer[i] == '\0'){
                break;
            }else if(buffer[i] <= '9' && buffer[i] >= '0'){
                parameter2 = parameter2 * 10 + buffer[i] - '0';
                parameter_empty++;
            }else{
                error = 1;
                break;
            }
        }
    }

    if(error){
        row++;
        printf("\nUnknown command!!SB!!");
    }
    else if(!strcmp(buffer,"clear")){
        row = 1;
        sys_screen_clear();
        sys_move_cursor(1, SHELL_BEGIN);
        printf("------------------- COMMAND -------------------");
    }
    else if(!strcmp(buffer,"ps")){
        sys_ps();
        row = row + valid_pcb + 1;
    }
    else if(!strcmp(command,"exec")){
        if(!parameter_empty){
            row++;
            printf("\nexec failed");
        }
        else{
            pid_t pid = sys_spawn(test_tasks[parameter], NULL, AUTO_CLEANUP_ON_EXIT);
            if(pid == -1){
                row++;
                printf("\nexec failed");
            }else{
                row++;
                printf("\nexec process[%d]",pid);
            }
        }
    }
    else if(!strcmp(command,"taskset")){
        if(!parameter_empty){
            row++;
            printf("\nUnknown command!!SB!!");
        }
        else{
            row++;
            printf("\ntaskset:%d %d",parameter,parameter2);
            sys_taskset(test_tasks[parameter2],parameter);
        }
    }
    else if(!strcmp(command,"taskset-p")){
        if(!parameter_empty){
            row++;
            printf("\nUnknown command!!SB!!");
        }
        else{
            row++;
            printf("\n");
            printf(buffer);
            sys_taskset_p(parameter,parameter2);
        }
    }
    else if(!strcmp(command,"kill")){
        if(parameter == 0 || parameter == 1)
            printf("\npid_0 and shell cannot be killed!!SB!!");
        else{
            if(sys_kill(parameter))
                printf("\nkilled process[%d]",parameter);
            else
                printf("\nprocess[%d] does not existed!!SB!!",parameter);
        }
        row++;
    }
    else{
        row++;
        printf("\nUnknown command!!SB!!");
    }
}

void test_shell()
{
    // TODO:
    char c;
    char *name = "> DSBDSB_OS@UCAS_OS: ";
    int length = strlen(name);
    sys_screen_clear();
    sys_move_cursor(1, SHELL_BEGIN);
    printf("------------------- COMMAND -------------------\n");
    printf(name);

    char buffer[BUFFER_MAX];
    int i = 0;
    while (1)
    {
        // TODO: call syscall to read UART port
        c = my_getchar();
        // TODO: parse input
        // note: backspace maybe 8('\b') or 127(delete)
        // TODO: ps, exec, kill, clear
        if(c=='\r'){
            buffer[i]='\0';
            i=0;
            row++;
            task(buffer);
            sys_move_cursor(1, SHELL_BEGIN + row);
            printf(name);
        }else if(c == 8 || c == 127 ){
            if(i != 0)
                buffer[--i] ='\0';
            sys_move_cursor(length, SHELL_BEGIN + row);
            printf("%s",buffer);
            printf(" ");
        }else{
            if(i == BUFFER_MAX - 1)
                continue;
            buffer[i++] = c;
            buffer[i] = '\0';
            sys_move_cursor(length, SHELL_BEGIN + row);
            printf("%s",buffer);
        }
    }
}
