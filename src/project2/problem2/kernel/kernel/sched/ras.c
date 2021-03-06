/*
 * RAS Scheduler (mapped to SCHED_RAS policy)
 */

#include "sched.h"
#include <linux/slab.h>

static inline struct task_struct *ras_task_of(struct sched_ras_entity *ras_se)
{
	return container_of(ras_se, struct task_struct, ras);
}

static inline struct ras_rq *ras_rq_of_se(struct sched_ras_entity *ras_se)
{
	struct task_struct *p = ras_task_of(ras_se);
	struct rq *rq = task_rq(p);

	return &rq->ras;
}

static inline int on_ras_rq(struct sched_ras_entity *ras_se)
{
	return !list_empty(&ras_se->run_list);
}

// Update the current task's runtime statistics. Skip current tasks that
// are not in our scheduling class.
static void update_curr_ras(struct rq *rq)
{
	struct task_struct *curr = rq->curr;
	struct sched_ras_entity *ras_se = &curr->ras;
	struct ras_rq *ras_rq = &rq->ras;
	u64 delta_exec;

	if (curr->sched_class != &ras_sched_class)
		return;

	delta_exec = rq->clock_task - curr->se.exec_start;
	if (unlikely((s64)delta_exec < 0))
		delta_exec = 0;

	schedstat_set(curr->se.statistics.exec_max,
				  max(curr->se.statistics.exec_max, delta_exec));

	curr->se.sum_exec_runtime += delta_exec;
	curr->se.exec_start = rq->clock_task;
}

static void requeue_task_ras(struct rq *rq, struct task_struct *p, int head)
{
	struct sched_ras_entity *ras_se = &p->ras;
	struct ras_rq *ras_rq = &rq->ras;
	if (on_ras_rq(ras_se))
	{
		struct list_head *queue = &ras_rq->queue;
		if (head)
			list_move(&ras_se->run_list, queue);
		else
			list_move_tail(&ras_se->run_list, queue);
		// printk(KERN_DEBUG "[requeue] move task %d to tail",p->pid);
	}
}

// Add a task to the run queue.
static void
enqueue_task_ras(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_ras_entity *ras_se = &p->ras;

	if (flags & ENQUEUE_WAKEUP)
		ras_se->timeout = 0;

	struct ras_rq *ras_rq = &rq->ras;
	struct list_head *queue = &ras_rq->queue;

	if (flags & ENQUEUE_HEAD)
		list_add(&ras_se->run_list, queue);
	else
		list_add_tail(&ras_se->run_list, queue);

	ras_rq->ras_nr_running++;

	// printk(KERN_DEBUG "[enqueue] task %d enqueue, task count: %d",p->pid,ras_rq->ras_nr_running);
	inc_nr_running(rq);
}

// Remove a task from the run queue.
static void
dequeue_task_ras(struct rq *rq, struct task_struct *p, int flags)
{
	struct sched_ras_entity *ras_se = &p->ras;

	update_curr_ras(rq);

	struct ras_rq *ras_rq = &rq->ras;

	list_del_init(&ras_se->run_list);

	ras_rq->ras_nr_running--;
	// printk(KERN_DEBUG "[dequeue] task %d dequeue, task count: %d",p->pid,ras_rq->ras_nr_running);
	dec_nr_running(rq);
}

static void
yield_task_ras(struct rq *rq)
{
	// printk(KERN_DEBUG "[yield]");
	requeue_task_ras(rq, rq->curr, 0);
}

// Preempt the current task with a newly woken task if needed.
// RAS scheduler without prio would never preempt.
static void
check_preempt_curr_ras(struct rq *rq, struct task_struct *p, int flags)
{
	// resched_task(rq->curr);
	return;
}

// Pick next task in run queue. If no task in run queue, return null.
static struct task_struct *
pick_next_task_ras(struct rq *rq)
{
	struct sched_ras_entity *ras_se;
	struct task_struct *p;
	struct ras_rq *ras_rq = &rq->ras;

	// No task in run queue, return null.
	if (!ras_rq->ras_nr_running){
		return NULL;
	}
		
	struct list_head *queue = &ras_rq->queue;

	ras_se = list_entry(queue->next, struct sched_ras_entity, run_list);

	p = ras_task_of(ras_se);
	p->se.exec_start = rq->clock_task;

	// printk(KERN_DEBUG "[pick next] pick task %d as the next to run",p->pid);

	return p;
}

static void
put_prev_task_ras(struct rq *rq, struct task_struct *p)
{
	update_curr_ras(rq);
}

static void
set_curr_task_ras(struct rq *rq)
{
	struct task_struct *p = rq->curr;
	p->se.exec_start = rq->clock_task;
}

static void
task_tick_ras(struct rq *rq, struct task_struct *task, int queued)
{
	struct sched_ras_entity *ras_se = &task->ras;
	struct ras_rq *ras_rq = &rq->ras;

	update_curr_ras(rq);

	if (task->policy != SCHED_RAS)
		return;		

	if (--task->ras.time_slice)
		return;
	
	
	if (ras_rq->ras_nr_running == 1){
		// No race. Set MAX timeslice to avoid frequently schedule.
		task->ras.time_slice = RAS_MAX_TIMESLICE;
		task->ras.total_timeslice = RAS_MAX_TIMESLICE;
		printk(KERN_DEBUG "[tick] task count: 1, no race. set task %d timeslice %d ms",task->pid,RAS_MAX_TIMESLICE * 10);
	} else {
		// Calculate race probability.
		int wcounts = task->wcounts;

		struct list_head *p;
		struct sched_ras_entity *ras_se_tmp;
		struct task_struct *t;
		struct list_head *queue;

		queue = &ras_rq->queue;
		int sum = 0;
		int task_cnt = 0;
		list_for_each(p, queue)
		{
			ras_se_tmp = list_entry(p, struct sched_ras_entity, run_list);
			t = ras_task_of(ras_se_tmp);
			sum += t->wcounts;
			task_cnt++;
		}

		if (sum == 0 || sum == wcounts) {
			// not tracing or no memory write
			task->ras.time_slice = RAS_MAX_TIMESLICE;
			task->ras.total_timeslice = RAS_MAX_TIMESLICE;
			printk(KERN_DEBUG "[tick] task count: %d, no write. set task %d timeslice %d ms",task_cnt, task->pid,RAS_MAX_TIMESLICE * 10);
		} else {
			int race_prob = wcounts * 10 / sum;
			unsigned int timeslice = -1*race_prob + RAS_MAX_TIMESLICE;
			task->ras.time_slice = timeslice;
			task->ras.total_timeslice = timeslice;
			printk(KERN_DEBUG "[tick] task count: %d, wcounts: %d, sum: %d, race_prob: %d. set task %d timeslice %d ms",task_cnt, wcounts,sum,race_prob,task->pid,timeslice * 10);
		}
	}
	
	// Requeue to the end of queue if we are NOT the only element on the queue.
	if (ras_se->run_list.prev != ras_se->run_list.next){
        requeue_task_ras(rq, task, 0);
        set_tsk_need_resched(task);
    }
}

static unsigned int
get_rr_interval_ras(struct rq *rq, struct task_struct *task)
{
	if (task->policy == SCHED_RAS)
	{
		struct sched_ras_entity *ras_se = &task->ras;
		return ras_se->total_timeslice;
	}

	return 0;
}

static void
switched_to_ras(struct rq *rq, struct task_struct *p)
{
	if (p->on_rq && rq->curr != p)
	{
		// printk(KERN_DEBUG "[switch], resched task %d",rq->curr->pid);
		resched_task(rq->curr);
	}
}

void init_ras_rq(struct ras_rq *ras_rq, struct rq *rq)
{
	struct list_head *queue;
	queue = &ras_rq->queue;
	INIT_LIST_HEAD(queue);

	ras_rq->ras_nr_running = 0;
	ras_rq->ras_time = 0;
	ras_rq->ras_throttled = 0;
	ras_rq->ras_runtime = 0;
	raw_spin_lock_init(&ras_rq->ras_runtime_lock); // what is spin lock?
}

void free_ras_sched_group(struct task_group *tg)
{
}

int alloc_ras_sched_group(struct task_group *tg, struct task_group *parent)
{
	// 1 indicates no error
	return 1;
}

/* -- DUMMY -- */
#ifdef CONFIG_SMP
static int
select_task_rq_ras(struct task_struct *p, int sd_flag, int flags)
{
}

static void
set_cpus_allowed_ras(struct task_struct *p, const struct cpumask *new_mask) {}

static void
rq_online_ras(struct rq *rq) {}

static void
rq_offline_ras(struct rq *rq) {}

static void
pre_schedule_ras(struct rq *rq, struct task_struct *prev) {}

static void
post_schedule_ras(struct rq *rq) {}

static void
task_woken_ras(struct rq *rq, struct task_struct *p) {}

static void
switched_from_ras(struct rq *rq, struct task_struct *p) {}
#endif

static void
prio_changed_ras(struct rq *rq, struct task_struct *p, int oldprio) {}
/* -- DUMMY -- */

const struct sched_class ras_sched_class = {
	// .next = &idle_sched_class,		  // required 
	.next = &fair_sched_class,		  // required
	.enqueue_task = enqueue_task_ras, // required
	.dequeue_task = dequeue_task_ras, // required
	.yield_task = yield_task_ras,	  // required

	.check_preempt_curr = check_preempt_curr_ras, // required

	.pick_next_task = pick_next_task_ras, // required
	.put_prev_task = put_prev_task_ras,	  // required

#ifdef CONFIG_SMP
	.select_task_rq = select_task_rq_ras,

	.set_cpus_allowed = set_cpus_allowed_ras,
	.rq_online = rq_online_ras,
	.rq_offline = rq_offline_ras,
	.pre_schedule = pre_schedule_ras,
	.post_schedule = post_schedule_ras,
	.task_woken = task_woken_ras,
	.switched_from = switched_from_ras,
#endif

	.set_curr_task = set_curr_task_ras, // required
	.task_tick = task_tick_ras,			// required

	.get_rr_interval = get_rr_interval_ras, // required

	.prio_changed = prio_changed_ras,
	.switched_to = switched_to_ras, // required
};