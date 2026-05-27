#include "config.h"
#include "FSM.h"
#include "motor.h"
#include "IR_handler.h"
#include "button_handler.h"

static State current_state = HALTED;
int left_pwm;
int right_pwm;

void fsm_update_state(int speed, int yawrate, int obstacle_cm){
    switch(current_state) {
        case HALTED:
            motor_stop();
            if(button_t2_pressed()){current_state = MOVING;}
            // mancano luci
            break;
        case MOVING:
            if(button_t2_pressed()){current_state = HALTED;}
            left_pwm =  speed - yawrate;
            right_pwm = speed + yawrate;
            // mancano luci
            if (obstacle_cm < OBSTACLE_DETECTED_THRESHOLD)
                current_state = OBSTACLE_AVOIDANCE;
            break;
        case OBSTACLE_AVOIDANCE:
          /*   if(!obstacle_procedure()){
                current_state = HALTED;
            }
            else current_state = MOVING; */
            break;
        default:
            break;
    }
}


// get functions to avoid global variables
int get_left_pwm(){
    return left_pwm;
}

int get_right_pwm(){
    return right_pwm;
}

State get_current_state(void) {
    return current_state;
}

//
/* int obstacle_procedure(){

// da implementare, probabilmente con un'altra FSM
} */