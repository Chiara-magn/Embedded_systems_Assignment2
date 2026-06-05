#include "config.h"
#include "FSM.h"
#include "motor.h"
#include "IR_handler.h"
#include "button_handler.h"
#include "IMU_handler.h"

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
static int current_light_state = 0; // current state HALTED
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
            if(button_t2_pressed()) {
                current_state = HALTED;
                current_o_state = ROTATE_CLOCK;  // reset anche la sotto-FSM
                count_obst = 0;
                count_3 = 0;
                break;  // <-- esci subito, non eseguire obstacle_procedure
            }
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
        if (count_obst == 0) { // reset calcolo yaw con gyro
            imu_reset_yaw_gyro();
            count_obst = 1;  // <-- segna che il reset è già stato fatto
        }
            imu_update_yaw();   // integrazione qui
            motor_forward_clockwise(50, 0);

        if (fabs(imu_get_yaw_gyro()) >= 90.0f) {
            motor_stop();
            count_obst = 0;
            current_o_state = GO_FORWARD;
        }
        return 1; // =1 rimango in obstacle avoidance
        break;
    case GO_FORWARD:
        if (count_obst < 1000)  // 1000 x 2ms = 2000 ms = 2s
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
        if (count_obst == 0) {
            imu_reset_yaw_gyro();
            count_obst = 1;
        }
        imu_update_yaw();
        motor_forward_clockwise(0, 50);

        if (fabs(imu_get_yaw_gyro()) >= 90.0f) {
            motor_stop();
            count_obst = 0;
            count_3 ++;
            current_o_state = CHECK_PROCEDURE;
        }
        return 1;
        break;
    case CHECK_PROCEDURE:
        if (count_3 >= 3) {
            count_3 = 0; 
            if (obstacle_cm < OBSTACLE_DETECTED_THRESHOLD)
                return 0;  // ostacolo ancora presente -> HALTED
            else
                return 2;  // ostacolo sparito -> MOVING
            }// passa alla fsm 0 -> HALTED 
        else{
            if (obstacle_cm < OBSTACLE_DETECTED_THRESHOLD)
            {
                current_o_state =  ROTATE_CLOCK;
                return 1;
            }
            else {
                return 2; 
            }// per andare in moving
        }
        break;
    default:
    return 1; // da verificare 
        break;
    }
}