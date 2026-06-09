// IR_handler.c
#include "IR_handler.h"
#include "ADC_handler.h"
#include "config.h"



void IR_init(void){
  
    IR_AN_TRIS  = 1;   // input
    IR_AN_ANSEL = 1;   // analog mode

    IR_EN_TRIS = 0;    // output
    IR_EN_LAT  = 1;    // enable high
    
}

/* 
Reads the raw ADC value from the IR sensor (AN15) and converts it to 
an estimated distance in centimetres.
    
The conversion is done in two steps:
    1. Convert the raw ADC value to voltage using the formula: V = raw * 3.3 / 1023
    2. Convert the voltage to distance using a 4th-order polynomial fit to the sensor's curve.
    
The returned distance is an integer in centimetres, truncated from the raw floating-point result.
 */
int IR_ReadDistance_cm(){    

    double voltage = raw_to_voltage(get_raw_IR());
    double raw_distance = voltage_to_dist(voltage) * 100;
    int distance = (int)(raw_distance);
       
    return distance;
}