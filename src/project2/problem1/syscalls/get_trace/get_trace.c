#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/errno.h>

MODULE_LICENSE("Dual BSD/GPL");

#define __SYSCALL_NUM 363

static int sys_get_trace(pid_t _pid, int *wcounts)
{
    struct task_struct *task;
    task = get_pid_task(find_get_pid(_pid), PIDTYPE_PID); // get task by pid
    if (!task || task->trace_flag == 0)
        return -EINVAL;

    *wcounts = task->wcounts;
    return 0;
}

static int (*oldcall)(void);

static int addsyscall_init(void)
{
    long *syscall = (long *)0xc000d8c4;
    oldcall = (int (*)(void))(syscall[__SYSCALL_NUM]);
    syscall[__SYSCALL_NUM] = (unsigned long)sys_get_trace;
    printk(KERN_INFO "module load!\n");
    return 0;
}

static void addsyscall_exit(void)
{
    long *syscall = (long *)0xc000d8c4;
    syscall[__SYSCALL_NUM] = (unsigned long)oldcall;
    printk(KERN_INFO "module exit!\n");
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);