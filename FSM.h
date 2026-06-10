#ifndef FSM_HANDLER
#define FSM_HANDLER


typedef enum {
    HALTED,
    MOVING,
    OBSTACLE_AVOIDANCE
} State;

typedef enum {
    ROTATE_CLOCK,
    GO_FORWARD,
    ROTATE_COUNT_CLOCK,
    CHECK_PROCEDURE,
} Obstacle_state;


int obstacle_procedure(int obstacle_cm); 
void fsm_update_state( int obstacle_cm);
int get_light_state(void);
State get_current_state(void);
Obstacle_state get_current_obstacle_state(void);

#endif