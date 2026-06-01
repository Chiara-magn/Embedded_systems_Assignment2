// IR_handler.c
#include "IR_handler.h"
#include "ADC_handler.h"

void IR_init(void){
    // IR sensor
    TRISBbits.TRISB14 = 1; // analog
    ANSELBbits.ANSB14 = 1; // analog
    TRISBbits.TRISB9 = 0; // enable
    LATBbits.LATB9   = 1; // enable
}

float IR_ReadDistance_cm(void) {
    float V = raw_to_voltage(raw_ir);
    return voltage_to_dist(V) * 100.0f;  // metri → cm
}