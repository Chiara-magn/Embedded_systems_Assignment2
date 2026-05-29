#include "config.h"
#include "tasks.h"
#include "IR_handler.h"
#include "UART_handler.h"
#include "PWM_handler.h"
#include "lights_handler.h"
#include "led_handler.h"
#include "motor.h"
#include "FSM.h"
#include "IMU_handler.h"

// Struct to hold raw accelerometer data (x, y, z as int16_t)
accel_data_t accel = {0, 0, 0};

// Struct to hold computed angles (roll and pitch as float)
angle_data_t angles = {0.0f, 0.0f};

int obstacle_cm;
int current_speed = 0;
int current_yaw = 0;

void task1(void* param)// 500 Hz 
{
    obstacle_cm = IR_ReadDistance_cm(); // IR sensor read 

    if(uart_command_buffer()){ 
        current_speed = uart_get_speed();   
        current_yaw = uart_get_yawrate(); 
    }
    fsm_update_state(obstacle_cm);
    if(get_current_state() == MOVING)
        motor_speed_yaw(current_speed, current_yaw);
    else
        motor_stop();
    // mancano luci
}  

void task2(void* param) // 10 Hz
{
    // lettura magnetometro 
    imu_read_mag_x();
    // lettura accelerometro
    imu_read_acc(&accel);
    // calcolo angoli
    imu_roll_pitch(&accel, &angles);
    //messaggio valori
    char distance[20]; // controllare grandezza
    sprintf(distance, "$MDIST,%d*", obstacle_cm);
    uart_send_string(distance);
    // invio $MANGLE,<roll>,<pitch>,<yaw>*
    char msg[50]; // controllare grandezza
    sprintf(msg, "$MANGLE,%.2f,%.2f,%.2f*", (double)angles.roll, (double)angles.pitch, 0.0);
    uart_send_string(msg);
}

