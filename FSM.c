#include "config.h"
#include "FSM.h"
#include "motor.h"
#include "IR_handler.h"
#include "button_handler.h"

static State current_state = HALTED;

// OBSTACLE PROCEDURE 
int obstacle_procedure(int obstacle_cm); // procedura da eseguire con obstacle avoidance

typedef enum {
    ROTATE_CLOCK,
    GO_FORWARD,
    ROTATE_COUNT_CLOCK,
    CHECK_PROCEDURE,
}Obstacle_state;

static Obstacle_state current_o_state = ROTATE_CLOCK;
static int count_obst = 0; 
static int count_3 = 0;
static int result_oa = 2;  // moving

// LIGHTS
static int current_light_state; // current state 
// current_light_state = 0 --> HALTED
// current_light_state = 1 --> MOVING
// current_light_state = 2 --> OBSTACLE AVOIDANCE

void fsm_update_state(int obstacle_cm){
    switch(current_state) {
        case HALTED:
            motor_stop();
            current_light_state = 0;
            if(button_t2_pressed()){current_state = MOVING;}
            break;
        case MOVING:
            current_light_state = 1;
            if(button_t2_pressed()){current_state = HALTED;}
            if (obstacle_cm < OBSTACLE_DETECTED_THRESHOLD)
                current_state = OBSTACLE_AVOIDANCE;
            break;
        case OBSTACLE_AVOIDANCE:
            current_light_state = 2;
            result_oa = obstacle_procedure(obstacle_cm);
            if(!result_oa){ // se 0
                current_state = HALTED;
                current_o_state = ROTATE_CLOCK;
            } 
            else if(result_oa == 1) { // se 1
                current_state = OBSTACLE_AVOIDANCE;
            }
            else{
                current_state = MOVING;
                current_o_state = ROTATE_CLOCK;
            } // se 2 (altrimenti)
            break;
        default:
            break;
    }
}

State get_current_state(void) {
    return current_state;
}

int get_light_state(void) {
    return current_light_state;
}

int obstacle_procedure(int obstacle_cm)
{
    switch (current_o_state)
    {
    case ROTATE_CLOCK:
        if (count_obst < 250)
        {
            motor_forward_clockwise(50, 0); // rotazione clockwise da verificare
            count_obst++;
        }
        else
        {
            count_obst = 0;
            current_o_state = GO_FORWARD;
        }
        return 1; // =1 rimango in obstacle avoidance
        break;
    case GO_FORWARD:
        if (count_obst < 2000)
        {
            motor_forward(100); 
            count_obst++;
        }
        else
        {
            count_obst = 0;
            current_o_state = ROTATE_COUNT_CLOCK;
        }
        return 1; // =1 rimango in obstacle avoidance
        break;
    case ROTATE_COUNT_CLOCK:
        if (count_obst < 250)
        {
            motor_forward_clockwise(0, 50); // rotazione count clockwise da verificare
            count_obst++;
        }
        else
        {
            count_obst = 0;
            current_o_state = CHECK_PROCEDURE;
        }
        return 1; // =1 rimango in obstacle avoidance
        break;
    case CHECK_PROCEDURE:
        count_3 ++;
        if (count_3 == 2) {
            count_3 = 0; 
            return 0; }// passa alla fsm 0 -> HALTED 
        else{
            if (obstacle_cm < OBSTACLE_DETECTED_THRESHOLD)
            {
                current_o_state =  ROTATE_CLOCK;
                return 1;
            }
            else {
                count_3 = 0;
                return 2; 
            }// per andare in moving
        }
        break;
    default:
        break;
    }
}