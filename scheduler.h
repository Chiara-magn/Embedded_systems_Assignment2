#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef void (*TaskFn)(void*); // function pointer type for tasks (takes void* params)

typedef struct {
    int n;        // counter
    int N;        // period (number of ticks between executions)
    int enable;   // flag to enable/disable the task
    TaskFn f;     // function pointer to the task function
    void* params; // pointer to parameters for the task function (can be NULL if not needed)
} Heartbeat;

void scheduler_init(void);
void scheduler_run(void);
int scheduler_add(TaskFn f, int N, int offset, void* params);

#endif