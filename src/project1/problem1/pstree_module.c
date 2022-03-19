#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");

#define __NR_pstreecall 356
#define __MAX_BUFSIZE 1024

struct prinfo
{
    pid_t parent_pid;       // pid of parent
    pid_t pid;              // pid of this process
    pid_t first_child_pid;  // pid of youngest child
    pid_t next_sibling_pid; // pid of older sibling
    long state;             // current state of process
    long uid;               // user id of process owner
    char comm[64];          // name of program executed
};

void convert(struct task_struct *task, struct prinfo *info)
{
    info->parent_pid = task->parent->pid;
    info->pid = task->pid;
    info->state = task->state;
    info->uid = task->cred->uid;
    get_task_comm(info->comm, task);

    // .next or .prev points to task_struct's sibling viarable.
    info->first_child_pid = (list_empty(&(task->children))) ? 0 : list_entry(task->children.next, struct task_struct, sibling)->pid;
    if (list_empty(&(task->sibling)))
    {
        info->next_sibling_pid = 0;
        return;
    }
    if (task->parent->pid == list_entry(task->sibling.next, struct task_struct, sibling)->pid)
    {
        // if process does not have next sibling, "task->sibling.next" will point to task's parent's "sibling" variable.
        info->next_sibling_pid = 0;
        return;
    }
    info->next_sibling_pid = list_entry(task->sibling.next, struct task_struct, sibling)->pid;
}

void dfs(struct task_struct *task, struct prinfo *buf, int *nr)
{
    convert(task, &buf[*nr]);
    *nr = *nr + 1;

    struct list_head *p;
    struct task_struct *t;
    list_for_each(p, &(task->children))
    {
        t = list_entry(p, struct task_struct, sibling);
        dfs(t, buf, nr);
    }
}

static int sys_pstreecall(struct prinfo *buf, int *nr)
{
    // malloc
    // kmalloc : kernel space malloc
    struct prinfo *_buf = kmalloc(sizeof(struct prinfo) * __MAX_BUFSIZE, GFP_KERNEL);
    int *_nr = kmalloc(sizeof(int), GFP_KERNEL);
    *_nr = 0;

    read_lock(&tasklist_lock);
    // deep first search
    // init_task : the initial process
    dfs(&init_task, _buf, _nr);
    read_unlock(&tasklist_lock);

    if (copy_to_user(buf, _buf, sizeof(struct prinfo) * __MAX_BUFSIZE))
    {
        printk(KERN_ERR "copy to user failed.\n");
        return 1;
    }
    if (copy_to_user(nr, _nr, sizeof(int)))
    {
        printk(KERN_ERR "copy to user failed.\n");
        return 1;
    }

    // free
    kfree(_buf);
    kfree(_nr);
    return 0;
}

static int (*oldcall)(void);

static int addsyscall_init(void)
{
    long *syscall = (long *)0xc000d8c4;
    oldcall = (int (*)(void))(syscall[__NR_pstreecall]);
    syscall[__NR_pstreecall] = (unsigned long)sys_pstreecall;
    printk(KERN_INFO "module load!\n");
    return 0;
}

static void addsyscall_exit(void)
{
    long *syscall = (long *)0xc000d8c4;
    syscall[__NR_pstreecall] = (unsigned long)oldcall;
    printk(KERN_INFO "module exit!\n");
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);