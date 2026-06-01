/**
 * @file main.c
 * @author facci
 * @date 2026-05-26
 * @brief Main function
 */
#include "config.h"
#include "timer.h"
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
#include "scheduler.h"


void setup(){

    timer_init();
    led_init();
    uart_init();
    spi_init_pins();
    imu_setup();
    imu_init();
    IR_init();
    lights_init();
    pwm_init();
    motor_init();
    battery_init();
    buttons_init();
    ADC_init();
    scheduler_init();

}

int main(){

    // Add your code here and press Ctrl + Shift + B to build

    
    /*     scheduler_init();

    scheduler_add(task_update_feedback, 1, &controlData);
    scheduler_add(task_update_control, 5, &controlData);
    scheduler_add(task_update_motors, 2, &controlData);

    while(1){
        scheduler_run();
        tmr_wait_period(TIMER1);
    } */

    return 0;
}
