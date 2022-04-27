/*
 * RAS Scheduler (mapped to SCHED_RAS policy)
 */

#include "sched.h"
#include <linux/slab.h>

static inline struct ras_rq *ras_rq_of_se(struct sched_ras_entity *ras_se){
	return ras_se->ras_rq;
}

// Add a task to the run queue
static void 
enqueue_task_ras(struct rq *rq, struct task_struct *p, int flags){
	struct sched_ras_entity *ras_se = &p->ras;

	if (flags & ENQUEUE_WAKEUP)
		ras_se->timeout = 0;

	struct ras_rq *ras_rq = ras_rq_of_se(ras_se);
	struct list_head *queue = &ras_rq->queue;
}

static void 
dequeue_task_ras(struct rq *rq, struct task_struct *p, int flags){

}

static void
yield_task_ras(struct rq *rq){

}

static void 
check_preempt_curr_ras(struct rq *rq, struct task_struct *p, int flags){

}

static struct task_struct*
pick_next_task_ras(struct rq *rq){
    return NULL;
}

static void 
put_prev_task_ras(struct rq *rq, struct task_struct *p){
    
}

static void 
set_curr_task_ras(struct rq *rq){

}

static void 
task_tick_ras(struct rq *rq, struct task_struct *p, int queued){

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