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
#include "button_handler.h"


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

// Variabile globale o condivisa
static int blink_counter = 0;
static int blink = 0;

// test deadline
unsigned int t1_elapsed = 0;
unsigned int t2_elapsed = 0;
unsigned int t3_elapsed = 0;



void task1(void* param)// 500 Hz 
{
     //   unsigned int t_start = TMR4; // deadline debug
    obstacle_cm = IR_ReadDistance_cm(); // IR sensor read 

     if(uart_command_buffer()){ 
        current_speed = uart_get_speed();   
        current_yaw = uart_get_yawrate(); 
    } 
    fsm_update_state(obstacle_cm);
    switch(get_current_state()) {
        case MOVING:
            if(current_speed == 0 && current_yaw == 0)
                motor_forward(60);
            else
                motor_speed_yaw(current_speed, current_yaw);
            break;
        case OBSTACLE_AVOIDANCE:
            // i motori sono già gestiti dentro fsm_update_state()
            break;
        case HALTED:
        default:
            motor_stop();
            break;
    } 
      //  t1_elapsed = TMR4 - t_start; // deadline debug
}   

/* 
 void task2(void* param) // 10 Hz
{

    // lettura magnetometro 
    imu_read_mag(&mag);
    // lettura accelerometro
    imu_read_acc(&accel);

    // calcolo angoli

    imu_roll_pitch_yaw(&accel, &mag, &angles);

    //messaggio valori

    char distance[30]; // controllare grandezza
    sprintf(distance, "$MDIST,%d*\r\n", obstacle_cm);
    uart_send_string(distance);
    // invio $MANGLE,<roll>,<pitch>,<yaw>*

                unsigned int t_start = TMR4;

    //char msg[50]; // controllare grandezza
    //sprintf(msg, "$MANGLE,%.2f,%.2f,%.2f*\r\n", (double)angles.roll, (double)angles.pitch, (double)angles.yaw);
 

    t2_elapsed = TMR4 - t_start;

    // uart_send_string(msg);
    // Button t3 
    if(button_t3_pressed()) {
        int tx_count = uart_get_tx_count();
        int rx_count = uart_get_rx_count();
        //sprintf(msg, "$MBUF,%d,%d*\r\n", tx_count, rx_count);
       // uart_send_string(msg);
    }

    // Blink a 1 Hz gestito a 10 Hz: toggle ogni 5 chiamate (5 * 100ms = 500ms)
    blink_counter++;
    if (blink_counter >= 10) {
        blink_counter = 0;
    }
    // con if/else
    if (blink_counter == 0) {
        blink = 1;  // acceso solo quando counter vale 0
    } else {
        blink = 0;  // spento per tutti gli altri valori (1,2,3...9)
    }
    // Gestione luci
    current_light = get_light_state();
    switch (current_light)
    {
    case 0: // HALTED 
        // blinking lights
        right_lights_set(blink);
        left_lights_set(blink);
        // low intensity 0
        low_intensity_set(0);
        break;
    case 1: // MOVING
        // lights off
        left_lights_set(0);
        right_lights_set(0);
        // low intensity on
        low_intensity_set(1);
        break;
    case 2: // OBSTACLE_AVOIDANCE
        // right blink
        right_lights_set(blink);
        // left off
        left_lights_set(0);
        // low intensity on
        low_intensity_set(1);
        break;
    default:
        break;
    }

} 
 */


void task2(void* param) // 10 Hz
{
  //  unsigned int t_start = TMR4;  // deadline debug

    imu_read_mag(&mag);
    imu_read_acc(&accel);
    imu_roll_pitch_yaw(&accel, &mag, &angles);

    char distance[30];
    sprintf(distance, "$MDIST,%d*\r\n", obstacle_cm);
    uart_send_string(distance);

    char msg[50];
    int pos = 0;

    // build $MANGLE message manually to avoid slow sprintf with floats
    const char *header = "$MANGLE,";
    while (*header) msg[pos++] = *header++;

    uart_append_fixed(msg, &pos, angles.roll);
    msg[pos++] = ',';
    uart_append_fixed(msg, &pos, angles.pitch);
    msg[pos++] = ',';
    uart_append_fixed(msg, &pos, angles.yaw);
    msg[pos++] = '*';
    msg[pos++] = '\r';
    msg[pos++] = '\n';
    msg[pos++] = '\0';
    uart_send_string(msg);

    if(button_t3_pressed()) {
        int tx_count = uart_get_tx_count();
        int rx_count = uart_get_rx_count();
        char buf[30];
        sprintf(buf, "$MBUF,%d,%d*\r\n", tx_count, rx_count);
        uart_send_string(buf);
    }

    blink_counter++;
    if (blink_counter >= 10) {
        blink_counter = 0;
    }
    // con if/else
    if (blink_counter < 2) {
        blink = 1;  // acceso solo quando counter vale 0
    } else {
        blink = 0;  // spento per tutti gli altri valori (1,2,3...9)
    }

    current_light = get_light_state();
    switch (current_light) {
    case 0:
        right_lights_set(blink);
        left_lights_set(blink);
        low_intensity_set(0);
        break;
    case 1:
        left_lights_set(0);
        right_lights_set(0);
        low_intensity_set(1);
        break;
    case 2:
        right_lights_set(blink);
        left_lights_set(0);
        low_intensity_set(1);
        break;
    default:
        break;
    }

 //   t2_elapsed = TMR4 - t_start;  // deadline debug
}



void task3(void* param) // 1 Hz
{
     //   unsigned int t_start = TMR4; // deadline debug
    // blink led A0 
    led_toggle_ld1();
    // $MBATT,v_batt*
    battery_volt = Battery_ReadVoltage();
    // commentato, nuova versione con int per risparmiare tempo 
/*     char msg[30]; // controllare grandezza
    sprintf(msg, "$MBATT,%.2f*\r\n", battery_volt);
    uart_send_string(msg); */

    int batt_int = (int)(battery_volt * 100.0f);
    char msg[30];
    sprintf(msg, "$MBATT,%d.%02d*\r\n", batt_int / 100, batt_int % 100);

    // deadline debug
    //   t3_elapsed = TMR4 - t_start;
    // sprintf(msg, "$TTIME,%u,%u,%u*\r\n", t1_elapsed, t2_elapsed, t3_elapsed);
    // uart_send_string(msg);

}