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
#include "button_handler.h"
#include "SPI_handler.h"
#include "ADC_handler.h"

void setup() {
    ANSELA = ANSELB = ANSELC = ANSELD = ANSELE = ANSELG = 0x0000; 
    led_init();
    timer_init();
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

    // inizialization of the scheduler and task registration
    scheduler_init();

    // tasks are never executed together with this timing configuration.
    scheduler_add(task1,   1,   0, NULL); // starts immediately, then every 1 tick (500 Hz)
    scheduler_add(task2,  50,  17, NULL); // first execution at tick 17, then every 50 ticks (10 Hz)
    scheduler_add(task3, 500, 275, NULL); // first execution at tick 275,then every 500 ticks (1 Hz)
}

int missed_deadlines = 0;

int main() {
    setup();
    tmr_setup_period (TIMER1, 2); // scheduler tick every 2 ms (500 Hz)
    tmr_setup_period (TIMER3, 2); // adc
    
   // tmr_setup_period(TIMER4, 100); // debug purpositiones

    while (1) {
        scheduler_run(); 

        if (tmr_wait_period(TIMER1)) {
            missed_deadlines++;
            led_toggle_ld2();  // toggle LED on missed deadlines for visual feedback
        }
    }
    return 0;
} 

