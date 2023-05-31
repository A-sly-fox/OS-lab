#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>

#include <os.h>

static char buff[64];

int main(void)
{
    int i, j;
    int fd = sys_fopen("10.txt", O_RDWR);

    // write 'hello world!' * 10
    lseek(fd, 8*1024*1024, SEEK_SET);

    for (i = 0; i < 10; i++)
    {
        sys_fwrite(fd, "hello ososo!\n", 13);
    }
    for (i = 0; i < 10; i++)
    {
        sys_fwrite(fd, "hello world!\n", 13);
    }

    // read
    // lseek(fd, 13*8, SEEK_CUR);
    lseek(fd, -13*11, SEEK_END);
    for (i = 0; i < 11; i++)
    {
        sys_fread(fd, buff, 13);
        for (j = 0; j < 13; j++)
        {
            printf("%c", buff[j]);
        }
    }

    sys_close(fd);
}