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
#define BUFFER_MAX 50

#define SHELL_BEGIN 25

char my_getchar(){
    int c;
    while((c=sys_read())==-1);
    return (char)c;
}

int row = 1;
void task(char *buffer)
{
    int argc = 0;
    char argv[BUFFER_MAX][BUFFER_MAX];
    int i, j;
    for(i = 0; i < BUFFER_MAX; i++)
        for(j = 0; j < BUFFER_MAX; j++)
            argv[i][j] = '\0';
    i = 0, j = 0;
    while(buffer[i] != '\0'){
        if(buffer[i] == ' '){
            ;
        }else{
            if(i == 0) {
                argc++;
                j = 0;
            }else if(buffer[i-1] == ' '){
                argc++;
                j = 0;
            }
            argv[argc - 1][j] = buffer[i];
            j++;
        }
        i++;
    }

    if(!strcmp(argv[0],"clear") && argc == 1){
        row = 1;
        sys_screen_clear();
        sys_move_cursor(1, SHELL_BEGIN);
        printf("------------------- COMMAND -------------------");
    }
    else if(!strcmp(argv[0],"ps") && argc == 1){
        int valid_pcb = sys_ps();
        row += valid_pcb;
    }
    else if(!strcmp(argv[0],"exec")){
        char *otherargv[BUFFER_MAX] = {0};
        int tmp_argc = 0;
        for (int i = 1; i < argc; ++i)
            otherargv[tmp_argc++] = &argv[i];
            
        pid_t pid = sys_exec(argv[1], argc - 1, otherargv, AUTO_CLEANUP_ON_EXIT);
        if(pid != -1){
            printf("\nexec process[%d]",pid);
            row++;
        }else{
            printf("\nexec failed");
            row++;
        }
    }
    // else if(!strcmp(command,"taskset")){
    //     if(!parameter_empty){
    //         row++;
    //         printf("\nUnknown command!!SB!!");
    //     }
    //     else{
    //         row++;
    //         printf("\ntaskset:%d %d",parameter,parameter2);
    //         sys_taskset(test_tasks[parameter2],parameter);
    //     }
    // }
    // else if(!strcmp(command,"taskset-p")){
    //     if(!parameter_empty){
    //         row++;
    //         printf("\nUnknown command!!SB!!");
    //     }
    //     else{
    //         row++;
    //         printf("\n");
    //         printf(buffer);
    //         sys_taskset_p(parameter,parameter2);
    //     }
    // }
    else if(!strcmp(argv[0],"kill") && argc == 2){
        int parameter = (int)atol(argv[1]);
        if(parameter == 1)
            printf("\nshell cannot be killed!!SB!!");
        else{
            if(sys_kill(parameter))
                printf("\nkilled process[%d]",parameter);
            else
                printf("\nprocess[%d] does not existed!!SB!!",parameter);
        }
        row++;
    }
    else if(!strcmp(argv[0],"ls") && argc == 1){
        printf("\n");
        sys_ls();
        row++;
    }
    else{
        row++;
        printf("\nUnknown command!!SB!!");
    }
}

int main()
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
    return 0;
}
