#include "config.h"
#include "tasks.h"
#include "IR_handler.h"
#include "UART_handler.h"
#include "PWM_handler.h"
#include "lights_handler.h"
#include "led_handler.h"
#include "motor.h"
#include "FSM.h"

void task1(void* param)// 500 Hz 
{
    int obstacle_cm;
    int current_speed;
    int current_yaw;

    obstacle_cm = IR_ReadDistance_cm(); // IR sensor read 

    if(uart_command_buffer()){ 
        current_speed = uart_get_speed();   
        current_yaw = uart_get_yawrate(); 
    }
    fsm_update_state(obstacle_cm);
    motor_speed_yaw(current_speed, current_yaw);
}  