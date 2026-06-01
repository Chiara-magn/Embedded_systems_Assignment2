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
#include "battery_handler.h"


// Struct to hold raw accelerometer data (x, y, z as int16_t) and magnetometer data 
sensor_data_t accel = {0, 0, 0};
sensor_data_t mag = {0, 0, 0};

// Struct to hold computed angles (roll, pitch and yaw as float)
angle_data_t angles = {0.0f, 0.0f, 0.0f};

int obstacle_cm;
int current_speed = 0;
int current_yaw = 0;
float battery_volt = 0;
int current_light = 0; 

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
    imu_read_mag(&mag);
    // lettura accelerometro
    imu_read_acc(&accel);
    // calcolo angoli
    imu_roll_pitch_yaw(&accel, &mag, &angles);
    //messaggio valori
    char distance[20]; // controllare grandezza
    sprintf(distance, "$MDIST,%d*", obstacle_cm);
    uart_send_string(distance);
    // invio $MANGLE,<roll>,<pitch>,<yaw>*
    char msg[50]; // controllare grandezza
    sprintf(msg, "$MANGLE,%.2f,%.2f,%.2f*", (double)angles.roll, (double)angles.pitch, (double)angles.yaw);
    uart_send_string(msg);
}

void task3(void* param) // 1 Hz
{
    // blink led A0
    led_toggle_ld1();
    // $MBATT,v_batt*
    battery_volt = Battery_ReadVoltage();
    char msg[20]; // controllare grandezza
    sprintf(msg, "$MBATT,%.2f*", battery_volt);
    uart_send_string(msg);
    current_light =  get_light_state();
    switch (current_light)
    {
    case 0:
        // HALTED 
        // blinking lights
        right_lights_toggle(); 
        left_lights_toggle();
        // low intensity off
        low_intensity_set(0);
        break;
    case 1:
        // MOVING 
        // left, right off
        left_lights_set(0);
        right_lights_set(0);
        // low intensity on
        low_intensity_set(1);
        break;
    case 2:
        // OBSTACLE_AVOIDANCE 
        // blink right lights
        right_lights_toggle(); 
        // low intensity on 
        low_intensity_set(1);
        // left lights off
        left_lights_set(0);
        break;
    default:
        break;
    }
}
