#include "config.h"
#include "FSM.h"
#include "motor.h"
#include "IR_handler.h"
#include "button_handler.h"
#include "IMU_handler.h"

/*
  Finite State Machine - Robot behavior controller called every 500 Hz.
 
  Main states (State):
    HALTED              - Robot stopped, waiting for button press
    MOVING              - Robot moving, controlled via UART speed/yawrate 
    OBSTACLE_AVOIDANCE  - Obstacle detected, executing avoidance maneuver
 
  Obstacle avoidance sub-FSM (Obstacle_state):
    ROTATE_CLOCK        - Rotate 90° clockwise (gyro-controlled)
    GO_FORWARD          - Move forward for 2 seconds (1000 * 2ms)
    ROTATE_COUNT_CLOCK  - Rotate 90° counter-clockwise (gyro-controlled)
    CHECK_PROCEDURE     - Check if obstacle still present, decide next action
 
  obstacle_procedure() return values:
    0 - Maneuver failed (obstacle still present after 3 attempts → HALTED)
    1 - Maneuver in progress → stay in OBSTACLE_AVOIDANCE
    2 - Maneuver succeeded (path clear) → MOVING
 */


// start in HALTED state 
static State current_state = HALTED;
// start in ROTATE_CLOCK state for obstacle avoidance
static Obstacle_state current_o_state = ROTATE_CLOCK;

// obstacle procedure variable used as: 
// - flag for resetting angles in ROTATIONS 
// - counter for 2s FORWARD moving
static int count_obst = 0; 

static int count_3 = 0;    // max 3 obstacles detected in a row before giving up and halting
static int result_oa = 2;  // obstacle avoidance succeded back in moving

// LIGHTS
// will be handled in the tasks, each state sets different lights
static int current_light_state = 0; // current state HALTED
// current_light_state = 0 --> HALTED
// current_light_state = 1 --> MOVING
// current_light_state = 2 --> OBSTACLE AVOIDANCE



void fsm_update_state(int obstacle_cm){
    switch(current_state) {

        case HALTED:
            //motor_stop();
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
                /* Emergency stop: reset everything and go to HALTED */
                current_state = HALTED;
                current_o_state = ROTATE_CLOCK;  
                count_obst = 0;
                count_3 = 0;
                break;  // <-- stop, no need to check obstacle procedure
            }

            // 0 = go HALTED
            // 1 = stay in OBSTACLE_AVOIDANCE
            // 2 = go MOVING
            result_oa = obstacle_procedure(obstacle_cm); 
            if(!result_oa){ // if 0
                current_state = HALTED;
                current_o_state = ROTATE_CLOCK;
            } 
            else if(result_oa == 1) { // if 1
                current_state = OBSTACLE_AVOIDANCE;
            }
            else{
                current_state = MOVING;
                current_o_state = ROTATE_CLOCK;
            } // if 2 
            break;
        default:
            break;
    }
}

/*
  obstacle_procedure - Sub-FSM for obstacle avoidance
  Executes a fixed sequence: rotate CW 90° → forward 2s → rotate CCW 90° → check.
  Repeats up to 3 times if obstacle persists; halts if still blocked after 3 attempts.
  Returns: 0 = go HALTED, 1 = stay in OBSTACLE_AVOIDANCE, 2 = go MOVING.
 */


int obstacle_procedure(int obstacle_cm)
{
    switch (current_o_state)
    {

    case ROTATE_CLOCK:
        if (count_obst == 0) { 
            imu_reset_yaw_gyro(); // reset yaw accumulator before rotation
            count_obst = 1;  // reset done, dont do it again
        }
            imu_update_yaw();     // euler integation of gyro to get current yaw
            
        if (fabs(imu_get_yaw_gyro()) >= 90.0f) { // if rotated 90° 
            count_obst = 0; // reset counter for next state
            current_o_state = GO_FORWARD;
        }
        return 1; // =1 stay in obstacle avoidance.
        break;

    case GO_FORWARD:
        // move forward for 2 seconds (1000 * 2ms = 2000 ms)
        if (count_obst < 1000)
        { 
            count_obst++;
        }
        else // after 2 seconds move to next state
        {
            count_obst = 0;
            current_o_state = ROTATE_COUNT_CLOCK;
        }
        return 1; // =1 stay in obstacle avoidance
        break;

    case ROTATE_COUNT_CLOCK:
        if (count_obst == 0) {
            imu_reset_yaw_gyro(); // reset yaw accumulator before rotation
            count_obst = 1; // reset done, dont do it again
        }
        imu_update_yaw(); // euler integation of gyro to get current yaw

        if (fabs(imu_get_yaw_gyro()) >= 90.0f) { // if rotated 90°
            // after completing the maneuver, check if obstacle still 
            // present for 3 times before halting
            count_obst = 0;
            count_3 ++;
            current_o_state = CHECK_PROCEDURE;
        }
        return 1;
        break;

    case CHECK_PROCEDURE:
    // if already tried 3 times
        if (count_3 >= 3) { 
            count_3 = 0; 
            if (obstacle_cm < OBSTACLE_DETECTED_THRESHOLD)
                return 0;  // obstacle still present -> HALTED
            else
                return 2;  // obstacle gone -> MOVING
            }

    // if not tried 3 times yet
        else{ 
            if (obstacle_cm < OBSTACLE_DETECTED_THRESHOLD)
            {current_o_state =  ROTATE_CLOCK; //restart obstacle procedure
                return 1;}
            else {
                return 2;}
            }
        break;

    default:
    return 1; // default to stay in obstacle avoidance 
        break;
    }
}

// Getter functions for current state and light state
State get_current_state(void) {
    return current_state;
}

Obstacle_state get_current_obstacle_state(void) {
    return current_o_state;
}

int get_light_state(void) {
    return current_light_state;
}