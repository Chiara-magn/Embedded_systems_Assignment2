// IR_handler.c
#include "IR_handler.h"
#include "ADC_handler.h"
#include "config.h"

void IR_init(void){
    // IR sensor

    IR_AN_TRIS  = 1;   // input
    IR_AN_ANSEL = 1;   // analog mode

    IR_EN_TRIS = 0;    // output
    IR_EN_LAT  = 1;    // enable high
    
}

/* float IR_ReadDistance_cm(void) {
    float V = raw_to_voltage(raw_ir);
    return voltage_to_dist(V) * 100.0f;  // metri → cm
}
 */
int IR_ReadDistance_cm(){    

    double voltage = raw_to_voltage(get_raw_IR());
    
    double raw_distance = voltage_to_dist(voltage) * 100;
    int distance = (int)(raw_distance);
       
    return distance;
}