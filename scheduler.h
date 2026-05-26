#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef void (*SchedTask)(void);

void scheduler_init(void);
int  scheduler_add(SchedTask fn, int N);
void scheduler_tick(void);

#endif