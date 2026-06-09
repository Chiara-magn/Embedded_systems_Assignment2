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


// Struct to hold raw accelerometer (int16_t) and magnetometer data (float)
sensor_data_t accel = {0, 0, 0};
sensor_data_t mag = {0, 0, 0};

// Struct to hold computed angles (roll, pitch and yaw as float)
angle_data_t angles = {0.0f, 0.0f, 0.0f};

int obstacle_cm;         //obstacle distance in cm read from IR sensor
int current_speed = 0;   //current speed command received from UART
int current_yaw = 0;     //current yaw rate command received from UART
float battery_volt = 0;  // battery voltage read from ADC
int current_light = 0;   // current light state from FSM

// variables for blinking logic
static int blink_counter = 0;
static int blink = 0;

// debug variables for testing deadline
unsigned int t1_elapsed = 0;
unsigned int t2_elapsed = 0;
unsigned int t3_elapsed = 0;


// TASK 1 500 Hz: control loop -> FSM update, PWM, read IR
void task1(void* param)
{
     //   unsigned int t_start = TMR4; // deadline debug

    obstacle_cm = IR_ReadDistance_cm(); // IR sensor read 

    // if a command is available in the UART buffer, read it 
    // and update current_speed and current_yaw
    if(uart_command_buffer()){ 
        current_speed = uart_get_speed();   
        current_yaw = uart_get_yawrate(); 
    }
    
    // update FSM state based on obstacle distance 
    // and set motor commands accordingly
    fsm_update_state(obstacle_cm);

    switch(get_current_state()) {
        case MOVING:
            if(current_speed == 0 && current_yaw == 0) //no command received, just move forward at fixed speed
                motor_forward(60);
            else
                motor_speed_yaw(current_speed, current_yaw);
            break;

        case OBSTACLE_AVOIDANCE:
            // motors are handled in FSM.
            break;

        case HALTED:
        default:
            motor_stop();
            break;
    } 
      //  t1_elapsed = TMR4 - t_start; // deadline debug
}   



// TASK 2 10 Hz: read IMU, send UART messages, update lights (blinking in OBSTACLE_AVOIDANCE)
void task2(void* param) // 10 Hz
{
    //  unsigned int t_start = TMR4;  // deadline debug

    // read IMU data and compute angles
    imu_read_mag(&mag);
    imu_read_acc(&accel);
    imu_roll_pitch_yaw(&accel, &mag, &angles);

    // send distance over UART in $MDIST,distance* format
    char distance[30];
    sprintf(distance, "$MDIST,%d*\r\n", obstacle_cm);
    uart_send_string(distance);

    // send angles over UART in $MANGLE,roll,pitch,yaw* format
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

    // if button T3 is pressed, send UART message with current TX and RX counts
    // we decided to put this in task2 because it is not time critical and we want 
    // to avoid slowing down the control loop in task1 with sprintf and uart_send_string
    
    if(button_t3_pressed()) {
        int tx_count = uart_get_tx_count();
        int rx_count = uart_get_rx_count();
        char buf[30];
        sprintf(buf, "$MBUF,%d,%d*\r\n", tx_count, rx_count);
        uart_send_string(buf);
    }

    // generation of 1HZ blinking signal (on/off at 1Hz) for lights in OBSTACLE_AVOIDANCE state
    blink_counter++;
    if (blink_counter >= 10) {
        blink_counter = 0;
    }
    // blink is 1 for 2 cycles (0 and 1) and 0 for the next 8 cycles (2-9),
    // creating a 20% duty cycle blink at 1Hz
    if (blink_counter < 2) {
        blink = 1;  
    } else {
        blink = 0;  
    }

    // update lights based on current FSM light state
    current_light = get_light_state();

    switch (current_light) {

    case 0: //HALTED: left and right blink, low intensity off
        right_lights_set(blink);
        left_lights_set(blink);
        low_intensity_set(0);
        break;
    case 1: //MOVING: left and right off, low intensity on
        left_lights_set(0);
        right_lights_set(0);
        low_intensity_set(1);
        break;
    case 2: //OBSTACLE_AVOIDANCE: left off, right blink, low intensity on

        right_lights_set(blink);
        left_lights_set(0);
        low_intensity_set(1);
        break;
    default:
        break;
    }

 //   t2_elapsed = TMR4 - t_start;  // deadline debug
}


// TASK 3 1 Hz: read battery voltage, send UART message, toggle LED
void task3(void* param) 
{
    //   unsigned int t_start = TMR4; // deadline debug

    // blink led A0 
    led_toggle_ld1();
    // read battery voltage and send UART message in $MBATT,v_batt* format
    battery_volt = Battery_ReadVoltage();
   
    // convert battery voltage to an integer representation 
    // with 2 decimal places to avoid using slow sprintf with floats.
    int batt_int = (int)(battery_volt * 100.0f);
    char msg[30];
    sprintf(msg, "$MBATT,%d.%02d*\r\n", batt_int / 100, batt_int % 100);
    uart_send_string(msg);


    // deadline debug
    //  t3_elapsed = TMR4 - t_start;
    //  sprintf(msg, "$TTIME,%u,%u,%u*\r\n", t1_elapsed, t2_elapsed, t3_elapsed);
    //  uart_send_string(msg);

}