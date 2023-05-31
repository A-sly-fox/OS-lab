#include <os/list.h>
#include <os/mm.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/irq.h>
#include <screen.h>
#include <os/stdio.h>
#include <assert.h>
#include <sys/syscall.h>
#include <sbi.h>
#include <user_programs.h>
#include <os/elf.h>

pcb_t pcb[NUM_MAX_TASK];
const ptr_t pid0_stack_0 = INIT_KERNEL_STACK + PAGE_SIZE;
pcb_t pid0_pcb_0 = {
    .pid = 0,
    .kernel_sp = (ptr_t)pid0_stack_0,
    .user_sp = (ptr_t)pid0_stack_0,
    .preempt_count = 0
};
const ptr_t pid0_stack_1 = INIT_KERNEL_STACK + 2 * PAGE_SIZE;
pcb_t pid0_pcb_1;

LIST_HEAD(ready_queue);
LIST_HEAD(xemacps_queue);

/* current running task PCB */
pcb_t * volatile current_running[2];

/* global process id */
pid_t process_id = 1;

void do_scheduler(void)
{
    // TODO schedule
    // Modify the current_running pointer.

    __asm__ __volatile__("csrr x0, sscratch\n");
    int cpu_id = get_current_cpu_id();

    pcb_t *prev_running,*work_pcb1,*work_pcb2;
    prev_running = current_running[cpu_id];
    if(list_empty(&ready_queue)){
        return;
    }
    
    list_node_t * next_node;
    next_node = ready_queue.next;
    
    if(current_running[cpu_id]->pid != 0 && current_running[cpu_id]->status == TASK_RUNNING){
        list_del(&prev_running->list);
        list_add_tail(&prev_running->list, &ready_queue);
    }
    if(prev_running->status == TASK_RUNNING)
        prev_running->status = TASK_READY;
    work_pcb1 = list_entry(next_node,pcb_t,list);
    while(work_pcb1->mask + cpu_id == 2){
        next_node = next_node->next;
        work_pcb1 = list_entry(next_node,pcb_t,list);
    }
    if(current_running[(cpu_id+1)%2]->pid == work_pcb1->pid){
        if(next_node->next == &ready_queue)
            current_running[cpu_id] = &pid0_pcb_1;
        else{
            work_pcb2 = list_entry(next_node->next,pcb_t,list);
            while(work_pcb2->mask + cpu_id == 2 && next_node != &ready_queue){
                next_node = next_node->next;
                work_pcb2 = list_entry(next_node,pcb_t,list);
            }
            if(next_node == &ready_queue)
                current_running[cpu_id] = &pid0_pcb_1;
            else
                current_running[cpu_id] = work_pcb2;
        }
    }else{
        current_running[cpu_id] = work_pcb1;
    }
    
    current_running[cpu_id]->status=TASK_RUNNING;
    set_satp(SATP_MODE_SV39, current_running[cpu_id]->pid, kva2pa(current_running[cpu_id]->pgdir) >> NORMAL_PAGE_SHIFT);
    local_flush_tlb_all();

    // restore the current_runnint's cursor_x and cursor_y
    vt100_move_cursor(current_running[cpu_id]->cursor_x,
                      current_running[cpu_id]->cursor_y);
    
    // TODO: switch_to current_running
    switch_to(prev_running,current_running[cpu_id]);
}

void do_sleep(uint32_t sleep_time)
{
    // TODO: sleep(seconds)
    // note: you can assume: 1 second = `timebase` ticks
    int cpu_id = get_current_cpu_id();
    // 1. block the current_running
    current_running[cpu_id]->status = TASK_BLOCKED;
    // 2. create a timer which calls `do_unblock` when timeout
    uint64_t current_time = get_timer();
    current_running[cpu_id]->wake_up_time = sleep_time + current_time;
    list_del(&current_running[cpu_id]->list);
    list_add_tail(&current_running[cpu_id]->list, &sleeping_queue);
    // 3. reschedule because the current_running is blocked.
    do_scheduler();
}

void do_block(list_node_t *pcb_node, list_head *queue)
{
    // TODO: block the pcb task into the block queue
    int cpu_id = get_current_cpu_id();
    // ready_queue.next->next->prev = &ready_queue;
    // ready_queue.next = ready_queue.next->next;
    list_del(&current_running[cpu_id]->list);

    list_add_tail(pcb_node, queue);
    current_running[cpu_id]->status = TASK_BLOCKED;
    do_scheduler();
}

void do_unblock(list_node_t *pcb_node)
{
    // TODO: unblock the `pcb` from the block queue
    pcb_t* unblock_task = list_entry(pcb_node->next, pcb_t, list);
    list_del(pcb_node->next);
    unblock_task->status = TASK_READY;
    list_add_tail(&unblock_task->list, &ready_queue);
}

int do_process_show()
{
    prints("\n[PROCESS TABLE]\n");
    int pcb_num = 1;
    int max_pcb_num = (NUM_MAX_TASK > process_id) ? process_id : NUM_MAX_TASK; 
    for(int i = 0; i < max_pcb_num; i++){
        if(pcb[i].pid == -1)
            continue;
        if(pcb[i].status==TASK_RUNNING){
            prints("[%d] PID : %d STATUS : RUNNING MASK:0x%x on Core %d\n", i, pcb[i].pid, pcb[i].mask, (current_running[1] == &pcb[i]));
            pcb_num++;
        }else if(pcb[i].status==TASK_READY){
            prints("[%d] PID : %d STATUS : READY MASK:0x%x\n", i, pcb[i].pid, pcb[i].mask);
            pcb_num++;
        }else if(pcb[i].status==TASK_BLOCKED){
            prints("[%d] PID : %d STATUS : BLOCKED MASK:0x%x\n", i, pcb[i].pid, pcb[i].mask);
            pcb_num++;
        }
    }
    return pcb_num;
}

pid_t do_spawn(task_info_t *info, void* arg, spawn_mode_t mode){
    int pcb_num,i;
    int cpu_id = get_current_cpu_id();

    if(process_id < NUM_MAX_TASK){
        pcb_num = process_id;
        pcb[pcb_num].kernel_stack_base = allocPage(1) + PAGE_SIZE;
        pcb[pcb_num].user_stack_base = allocPage(1) + PAGE_SIZE;
    }else{
        for (i = 0; i < NUM_MAX_TASK; i++){
            if(pcb[i].status == TASK_EXITED || pcb[i].status == TASK_ZOMBIE){
                pcb_num = i;
                break;
            }
        }
        if(i == NUM_MAX_TASK)
            return -1;
    }

    pcb[pcb_num].kernel_sp = pcb[pcb_num].kernel_stack_base;
    pcb[pcb_num].user_sp = pcb[pcb_num].user_stack_base;
    
    pcb[pcb_num].type = info->type;
    init_pcb_stack(pcb[pcb_num].kernel_sp, pcb[pcb_num].user_sp, (ptr_t)info->entry_point, &pcb[pcb_num], NULL, arg);
    list_add_tail(&pcb[pcb_num].list,&ready_queue);
    init_list_head(&pcb[pcb_num].wait_list);
    
    pcb[pcb_num].pid = ++process_id;
    pcb[pcb_num].status = TASK_READY;
    pcb[pcb_num].cursor_x = 1;
    pcb[pcb_num].cursor_y = 1;
    pcb[pcb_num].kernel_sp = pcb[pcb_num].kernel_sp - sizeof(regs_context_t)-sizeof(switchto_context_t);
    pcb[pcb_num].user_sp = pcb[pcb_num].user_sp;
    pcb[pcb_num].mode = mode;
    pcb[pcb_num].mask = current_running[cpu_id]->mask;
    valid_pcb++;
    return process_id;
}

int do_kill(pid_t pid)
{
    int cpu_id = get_current_cpu_id();
    int i;
    for(i = 0; i < NUM_MAX_TASK ; i++){
        if(pcb[i].pid == pid)
            break;
        else if(pcb[i].pid != pid && i == NUM_MAX_TASK-1)
            return 0;
    }
    list_del(&pcb[i].list); 

    while(!list_empty(&pcb[i].wait_list)){
        pcb_t *wait_pcb;
        wait_pcb = list_entry(pcb[i].wait_list.next, pcb_t, list);
        if(wait_pcb->status != TASK_EXITED){
            do_unblock(&current_running[cpu_id]->wait_list);
        }
    }

    for (int i = 0; i < MAX_LOCK; i++){
        if(my_lock_t[i].pid == current_running[cpu_id]->pid){
            if(list_empty(&my_lock_t[i].block_queue)){
                my_lock_t[i].pid = -1;
                my_lock_t[i].lock.status = UNLOCKED;
            }else{
                do_unblock(&my_lock_t[i].block_queue);
            }
        }
    }

    pcb[i].status = TASK_EXITED;
    pcb[i].pid = -1;
    valid_pcb--;
    return 1;
}

void do_exit(void)
{
    int cpu_id = get_current_cpu_id();
    list_del(&current_running[cpu_id]->list); 

    while(!list_empty(&current_running[cpu_id]->wait_list)){
        pcb_t *wait_pcb;
        wait_pcb = list_entry(current_running[cpu_id]->wait_list.next, pcb_t, list);
        if(wait_pcb->status != TASK_EXITED){
            do_unblock(&current_running[cpu_id]->wait_list);
        }
    }

    for (int i = 0; i < MAX_LOCK; i++){
        if(my_lock_t[i].pid == current_running[cpu_id]->pid){
            if(list_empty(&my_lock_t[i].block_queue)){
                my_lock_t[i].pid = -1;
                my_lock_t[i].lock.status = UNLOCKED;
            }else{
                do_unblock(&my_lock_t[i].block_queue);
            }
        }
    }

    if(current_running[cpu_id]->mode == ENTER_ZOMBIE_ON_EXIT)
        current_running[cpu_id]->status = TASK_ZOMBIE;
    else
        current_running[cpu_id]->status = TASK_EXITED;

    current_running[cpu_id]->pid = -1;
    valid_pcb--;
    do_scheduler();
}

int do_waitpid(pid_t pid)
{
    int i;
    int cpu_id = get_current_cpu_id();
    for(i = 0;i < NUM_MAX_TASK; i++)
        if(pcb[i].pid == pid)
            break;
    if (i == NUM_MAX_TASK)
        return -1;
    do_block(&current_running[cpu_id]->list, &pcb[i].wait_list);
    return 0;
}

pid_t do_getpid()
{
    int cpu_id = get_current_cpu_id();
    return current_running[cpu_id]->pid;
}

void do_taskset(task_info_t *info,int mask)
{
    int i;
    pid_t pid = do_spawn(info, NULL, AUTO_CLEANUP_ON_EXIT);
    for (i = 0; i < NUM_MAX_TASK; i++){
        if(pcb[i].pid == pid){
            pcb[i].mask = mask;
            break;
        }
    }
}

void do_taskset_p(int mask,int pid)
{
    int i;
    for (i = 0; i < NUM_MAX_TASK; i++){
        if(pcb[i].pid == pid){
            pcb[i].mask = mask;
            break;
        }
    }
}

void do_ls()
{
    int elf_num = 0;
    for(elf_num = 0; elf_num < ELF_FILE_NUM; elf_num++)
        if(elf_files[elf_num].file_length == 0)
            break;
    for (int i = 0; i < elf_num; i++){
        prints("%s\t",elf_files[i].file_name);
    }
}

pid_t do_exec(const char *file_name, int argc, char* argv[], spawn_mode_t mode){
    int pcb_num,i;
    int cpu_id = get_current_cpu_id();

    if(process_id < NUM_MAX_TASK){
        pcb_num = process_id;
        pcb[pcb_num].kernel_stack_base = allocPage(1) + PAGE_SIZE;
        pcb[pcb_num].user_stack_base = USER_STACK_ADDR;
    }else{
        for (i = 0; i < NUM_MAX_TASK; i++){
            if(pcb[i].status == TASK_EXITED || pcb[i].status == TASK_ZOMBIE){
                pcb_num = i;
                break;
            }
        }
        if(i == NUM_MAX_TASK)
            return -1;
    }

    pcb[pcb_num].pgdir = allocPage(1);
    clear_pgdir(pcb[pcb_num].pgdir);
    pcb[pcb_num].kernel_sp = pcb[pcb_num].kernel_stack_base;
    pcb[pcb_num].user_sp = pcb[pcb_num].user_stack_base;
    
    pcb[pcb_num].type = USER_PROCESS;
    uint64_t base_pgdir = pa2kva(PGDIR_PA);
    share_pgtable(pcb[pcb_num].pgdir, base_pgdir);
    uint64_t kva_stack = alloc_page_helper(pcb[pcb_num].user_stack_base - 0x1000, pcb[pcb_num].pgdir);

    int elf_num;
    for(elf_num = 0; elf_num < ELF_FILE_NUM; elf_num++){
        if(!kstrcmp(elf_files[elf_num].file_name,file_name)){
            break;
        }
    }
    if(elf_num == ELF_FILE_NUM)
        return -1;
    ptr_t entry_point = (ptr_t)load_elf(elf_files[elf_num].file_content,*(elf_files[elf_num].file_length), pcb[pcb_num].pgdir, alloc_page_helper);

    // init_pcb_stack(pcb[pcb_num].kernel_sp, pcb[pcb_num].user_sp, entry_point, &pcb[pcb_num], NULL);
    uintptr_t uva_argv = 0xf0000f040;
	uintptr_t kva_argvi = kva_stack;
	uintptr_t uva_argvi = 0xf0000f000;
	kva_stack += 0x40;
	init_pcb_stack(pcb[pcb_num].kernel_sp, pcb[pcb_num].user_sp, entry_point, pcb + pcb_num, (void *)uva_argv, argc);
	for (int i = 0; i < argc; i++)
	{
		if (argv[i] == 0)
		{
			break;
		}
		kmemcpy((uint8_t *)kva_argvi, (uint8_t *)argv[i], kstrlen(argv[i]) + 1);
		*(uintptr_t *)kva_stack = uva_argvi;
		kva_stack += 8;
		kva_argvi += kstrlen(argv[i]) + 1;
		uva_argvi += kstrlen(argv[i]) + 1;
	}
    list_add_tail(&pcb[pcb_num].list,&ready_queue);
    init_list_head(&pcb[pcb_num].wait_list);   
    
    pcb[pcb_num].pid = ++process_id;
    pcb[pcb_num].status = TASK_READY;
    pcb[pcb_num].cursor_x = 1;
    pcb[pcb_num].cursor_y = 1;
    pcb[pcb_num].kernel_sp = pcb[pcb_num].kernel_sp - sizeof(regs_context_t)-sizeof(switchto_context_t);
    pcb[pcb_num].user_sp = pcb[pcb_num].user_sp;
    pcb[pcb_num].mode = mode;
    pcb[pcb_num].mask = current_running[cpu_id]->mask;
    valid_pcb++;

    return process_id;
}

int do_binsemget(int key)
{
    do_mutex_lock_init((void*)key);
    return key;
}

int do_binsemop(int binsem_id, int op)
{
    if(op == BINSEM_OP_LOCK){
        do_mutex_lock_acquire((void *)binsem_id);
    }else if(op == BINSEM_OP_UNLOCK){
        do_mutex_lock_release((void *)binsem_id);
    }
    return 1;
}