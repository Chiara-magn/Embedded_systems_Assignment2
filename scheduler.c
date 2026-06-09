#include "scheduler.h"
#include "tasks.h"
#include "config.h"

// array "tasks" of Heartbeat structs to hold scheduled tasks.
static Heartbeat tasks[MAX_TASKS];
// number of tasks currently scheduled (added to the "tasks" array)
static int task_count = 0;

// Initializes the scheduler by clearing the tasks array and resetting the task count.
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


// Adds a new task to the scheduler with the specified function, period, offset, and parameters.
// Returns the index of the added task, or -1 if the task list is full.

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


// Runs the scheduler by executing all enabled tasks whose counters have reached their periods.
// Called in the main loop every 2 ms (500 Hz) to check and execute scheduled tasks.

void scheduler_run(void){
    for(int i = 0; i < task_count; i++){ // loop through all scheduled tasks
        tasks[i].n++; // increment the counter for each task

        // check if task is enabled and counter has reached the period
        if(tasks[i].enable && tasks[i].n >= tasks[i].N){
            tasks[i].f(tasks[i].params); // execute the task function with its parameters
            tasks[i].n = 0;
        }
    }
}