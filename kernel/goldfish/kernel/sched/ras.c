/*
 * RAS Scheduler (mapped to SCHED_RAS policy)
 */

#include "sched.h"
#include <linux/slab.h>

static inline struct ras_rq *ras_rq_of_se(struct sched_ras_entity *ras_se){
	return ras_se->ras_rq;
}

// Update the current task's runtime statistics. Skip current tasks that
// are not in our scheduling class.
static void update_curr_ras(struct rq *rq)
{
	struct task_struct *curr = rq->curr;
	struct sched_ras_entity *ras_se = &curr->ras;
	struct ras_rq *ras_rq = ras_rq_of_se(ras_se);
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

static inline int on_ras_rq(struct sched_ras_entity *ras_se)
{
	return !list_empty(&ras_se->run_list);
}

static void requeue_task_ras(struct rq *rq, struct task_struct *p, int head){
	struct sched_ras_entity *ras_se = &p->ras;
	struct ras_rq *ras_rq = ras_rq_of_se(ras_se);
	if (on_ras_rq(ras_se)) {
		struct list_head *queue = &ras_rq->queue;
		if (head){
			printk(KERN_DEBUG "I'm in requeue_task_ras, move head");
			list_move(&ras_se->run_list, queue);
		} else {
			printk(KERN_DEBUG "I'm in requeue_task_ras, move tail");
			list_move_tail(&ras_se->run_list, queue);
		}
	}
}

// Add a task to the run queue.
static void 
enqueue_task_ras(struct rq *rq, struct task_struct *p, int flags){
	struct sched_ras_entity *ras_se = &p->ras;

	if (flags & ENQUEUE_WAKEUP)
		ras_se->timeout = 0;

	struct ras_rq *ras_rq = ras_rq_of_se(ras_se);
	struct list_head *queue = &ras_rq->queue;

	if (flags & ENQUEUE_HEAD){
		printk(KERN_DEBUG "I'm in enqueue_task_ras, enqueue head");
		list_add(&rt_se->run_list, queue);
	} else{
		printk(KERN_DEBUG "I'm in enqueue_task_ras, enqueue tail");
		list_add_tail(&rt_se->run_list, queue);
	}
		
	ras_rq->ras_nr_running++;	
	inc_nr_running(rq);
}

// Remove a task from the run queue.
static void 
dequeue_task_ras(struct rq *rq, struct task_struct *p, int flags){
	struct sched_ras_entity *ras_se = &p->ras;

	update_curr_ras(rq);

	struct ras_rq *ras_rq = ras_rq_of_se(ras_se);

	list_del_init(&ras_se->run_list);

    ras_rq->ras_nr_running--;
	dec_nr_running(rq);
}

static void
yield_task_ras(struct rq *rq){
	requeue_task_ras(rq, rq->curr, 0);
}

// Preempt the current task with a newly woken task if needed.
static void 
check_preempt_curr_ras(struct rq *rq, struct task_struct *p, int flags){
	if (p->prio < rq->curr->prio) {
		resched_task(rq->curr);
		return;
	}
}

static struct task_struct*
pick_next_task_ras(struct rq *rq){

    return NULL;
}

static void 
put_prev_task_ras(struct rq *rq, struct task_struct *p){
    update_curr_ras(rq);

	if (on_rt_rq(&p->rt) && p->rt.nr_cpus_allowed > 1)
		enqueue_pushable_task(rq, p);
}

static void 
set_curr_task_ras(struct rq *rq){
	struct task_struct *p = rq->curr;
	p->se.exec_start = rq->clock_task;
}

static void 
task_tick_ras(struct rq *rq, struct task_struct *p, int queued){
	struct sched_ras_entity *ras_se = &p->ras;

	update_curr_ras(rq);
}

static unsigned int 
get_rr_interval_ras(struct rq *rq, struct task_struct *task){
    return 0;
}

static void 
switched_to_ras(struct rq *rq, struct task_struct *p){

}

void init_ras_rq(struct ras_rq *ras_rq, struct rq *rq){

}

void free_ras_sched_group(struct task_group *tg){

}

int alloc_ras_sched_group(struct task_group *tg, struct task_group *parent){
    return 0;
}

/* -- DUMMY -- */
#ifdef CONFIG_SMP
static int
select_task_rq_ras(struct task_struct *p, int sd_flag, int flags) {}

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
	.next			= &idle_sched_class,         // required
	.enqueue_task		= enqueue_task_ras,      // required
	.dequeue_task		= dequeue_task_ras,      // required
	.yield_task		= yield_task_ras,            // required

	.check_preempt_curr	= check_preempt_curr_ras,   // required

	.pick_next_task		= pick_next_task_ras,    // required
	.put_prev_task		= put_prev_task_ras,     // required

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_ras,

	.set_cpus_allowed       = set_cpus_allowed_ras,
	.rq_online              = rq_online_ras,
	.rq_offline             = rq_offline_ras,
	.pre_schedule		= pre_schedule_ras,
	.post_schedule		= post_schedule_ras,
	.task_woken		= task_woken_ras,
	.switched_from		= switched_from_ras,
#endif

	.set_curr_task          = set_curr_task_ras,    // required
	.task_tick		= task_tick_ras,                // required

	.get_rr_interval	= get_rr_interval_ras,      // required

	.prio_changed		= prio_changed_ras,
	.switched_to		= switched_to_ras,          // required
};