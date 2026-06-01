#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef void (*TaskFn)(void*);

typedef struct {
    int n;
    int N;
    int enable;
    TaskFn f;
    void* params;
} Heartbeat;

void scheduler_init(void);
void scheduler_run(void);
int scheduler_add(TaskFn f, int N, void* params);

#endif