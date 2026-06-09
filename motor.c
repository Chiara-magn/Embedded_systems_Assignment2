#include "motor.h"
#include "PWM_handler.h"


// left - A=RD1/B=RD2
// right - C=RD3/D=RD4

//RD2/4 forward
//RD1/3 backward

// !! Only one pin per side is active at a time.
// duty cycle = speed (% of the high time in PWM signal)

// Motor initialization, sets all motors to stop state
void motor_init(void) {
   motor_stop(); 
}

// stop all motors
void motor_stop(void) {
    setPWM_A(0);
    setPWM_B(0);
    setPWM_C(0);
    setPWM_D(0);
}

// moving forward with same speed on both motors
void motor_forward(int duty) {   
    setPWM_A(0);
    setPWM_B(duty);
    setPWM_C(0);
    setPWM_D(duty);
}

// moving forward with clockwise rotation
// duty_left > duty_right
void motor_forward_clockwise(int duty_left, int duty_right) {

    // left faster to turn clockwise (right)
    setPWM_A(0);
    setPWM_B(duty_left);

    // right slowe
    setPWM_C(0);
    setPWM_D(duty_right);
}

/*
  Function that converts a (speed, yaw) command into left/right PWM signals:
    L = speed + yaw
    R = speed - yaw
  Both L and R are saturated to [-100, 100] to avoid overflow.

  Positive yaw → turn left  (L slows down, R speeds up)
  Negative yaw → turn right (L speeds up, R slows down)

  Speed and yaw are expected to be in the range [-100, 100], where:
    -100 = full speed backward / maximum right turn
     0   = stop / no turn
    +100 = full speed forward / maximum left turn
 */
void motor_speed_yaw(int speed, int yaw) {

    // differential speed
    int L = speed + yaw;
    int R = speed - yaw;

    // saturation to avoid overflow and keep duty cycle in valid range
    if(L > 100) L = 100;
    if(L < -100) L = -100;
    if(R > 100) R = 100;
    if(R < -100) R = -100;

    // LEFT SIDE
    if(L > 0) {          // forward
        setPWM_A(0);
        setPWM_B(L);
    }
    else if(L < 0) {    // backward
        setPWM_A(-L);
        setPWM_B(0);
    }
    else {                // stop
        setPWM_A(0);
        setPWM_B(0);
    }

    // RIGHT SIDE
    if(R > 0) {          // forward
        setPWM_C(0);
        setPWM_D(R);
    }
    else if(R < 0) {    // backward
        setPWM_C(-R);
        setPWM_D(0);
    }
    else {                // stop
        setPWM_C(0);
        setPWM_D(0);
    }
}
