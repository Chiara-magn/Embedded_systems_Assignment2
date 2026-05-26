#ifndef MOTOR_H
#define MOTOR_H

// Inizializza il modulo motori (placeholder per coerenza architetturale)
void motor_init(void);

// Ferma completamente il buggy
void motor_stop(void);

// Movimento in avanti con duty fisso (Assignment 1)
void motor_forward(int duty);

// Movimento in avanti con rotazione oraria (Assignment 2)
void motor_forward_clockwise(int duty_left, int duty_right);

// Controllo generale con speed e yaw rate (Assignment 3)
void motor_speed_yaw(int speed, int yaw);

#endif