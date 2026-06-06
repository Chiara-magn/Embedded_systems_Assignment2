#include "scheduler.h"
#include "tasks.h"

#define MAX_TASKS 3 // abbiamo solo 3 task

static Heartbeat tasks[MAX_TASKS];
static int task_count = 0;

void scheduler_init(void){
    for(int i = 0; i < MAX_TASKS; i++){
        tasks[i].n = 0;
        tasks[i].N = 0;
        tasks[i].enable = 0;
        tasks[i].f = 0;
        tasks[i].params = 0;
    }
    task_count = 0;
}

int scheduler_add(TaskFn f, int N, int offset, void* params){
    if(task_count >= MAX_TASKS)
        return -1;

    tasks[task_count].n = offset;  // ← parte da offset invece che da 0
    //tasks[task_count].n = 0;
    tasks[task_count].N = N;
    tasks[task_count].enable = 1;
    tasks[task_count].f = f;
    tasks[task_count].params = params;

    return task_count++;
}

void scheduler_run(void){
    for(int i = 0; i < task_count; i++){
        tasks[i].n++;

        if(tasks[i].enable && tasks[i].n >= tasks[i].N){
            tasks[i].f(tasks[i].params);
            tasks[i].n = 0;
        }
    }
}