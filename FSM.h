#ifndef FSM_HANDLER
#define FSM_HANDLER

typedef enum {
    HALTED,
    MOVING,
    OBSTACLE_AVOIDANCE
} State;

void fsm_update_state( int obstacle_cm);
State get_current_state(void);

#endif