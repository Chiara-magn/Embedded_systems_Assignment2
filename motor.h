#ifndef MOTOR_H
#define MOTOR_H


void motor_init(void);
void motor_stop(void);
void motor_forward(int duty);
void motor_forward_clockwise(int duty_left, int duty_right);
void motor_speed_yaw(int speed, int yaw);

#endif