#ifndef FSM_HANDLER
#define FSM_HANDLER

typedef enum {
    HALTED,
    MOVING,
    OBSTACLE_AVOIDANCE
} State;

void fsm_update_state(int speed, int yawrate, int obstacle_cm);

int get_left_pwm();
int get_right_pwm();
State get_current_state(void);

// int obstacle_procedure(); // procedura da eseguire con obstacle avoidance

#endif