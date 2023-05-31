#include <mailbox.h>
#include <string.h>
#include <sys/syscall.h>

mailbox_t mbox_open(char *name)
{
    // TODO:
    return sys_mbox_open(name);
}

void mbox_close(mailbox_t mailbox)
{
    // TODO:
    sys_mbox_close((int)mailbox);
}

int mbox_send(mailbox_t mailbox, void *msg, int msg_length)
{
    // TODO:
    return sys_mbox_send((int)mailbox, msg,msg_length);
}

int mbox_recv(mailbox_t mailbox, void *msg, int msg_length)
{
    // TODO:
    return sys_mbox_recv((int)mailbox, msg, msg_length);
}
