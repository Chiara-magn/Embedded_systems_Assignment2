/* #include "config.h"
#include "tasks.h"
#include "IR_handler.h"
#include "UART_handler.h"
#include "PWM_handler.h"
#include "lights_handler.h"
#include "led_handler.h"
#include "motor.h"

int obstacle_cm;

task1()// 500 Hz 
{
    obstacle_cm = IR_ReadDistance_cm(); // IR sensor read 



    fsm_update_state(speed, yawrate, obstacle_cm);

}  */