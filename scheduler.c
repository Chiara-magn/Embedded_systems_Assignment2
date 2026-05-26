#include "scheduler.h"

typedef struct {
    int n;          // contatore
    int N;          // periodo
    SchedTask fn;   // funzione da eseguire
} SchedSlot;

#define SCHED_MAX 8

static SchedSlot slots[SCHED_MAX];

void scheduler_init(void){
    for(int i = 0; i < SCHED_MAX; i++){
        slots[i].fn = 0;
        slots[i].n = 0;
        slots[i].N = 0;
    }
}

int scheduler_add(SchedTask fn, int N){
    for(int i = 0; i < SCHED_MAX; i++){
        if(slots[i].fn == 0){
            slots[i].fn = fn;
            slots[i].N = N;
            slots[i].n = 0;
            return i;
        }
    }
    return -1;
}

void scheduler_tick(void){
    for(int i = 0; i < SCHED_MAX; i++){
        if(slots[i].fn){
            slots[i].n++;

            if(slots[i].n >= slots[i].N){
                slots[i].fn();
                slots[i].n = 0;
            }
        }
    }
}