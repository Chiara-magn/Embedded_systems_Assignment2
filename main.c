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

    // Inizializza e registra i task
    scheduler_init();
    scheduler_add(task1, 1,   NULL); // 500 Hz → N=1
    scheduler_add(task2, 50,  NULL); // 10 Hz  → N=50
    scheduler_add(task3, 500, NULL); // 1 Hz   → N=500
}

int main() {
    setup();
    tmr_setup_period(TIMER1, 2);

    while (1) {
        scheduler_run();
        tmr_wait_period(TIMER1);
    }

    return 0;
}