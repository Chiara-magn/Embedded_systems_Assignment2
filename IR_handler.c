// IR_handler.c
#include "IR_handler.h"
#include "ADC_handler.h"

void IR_init(void){
    // IR sensor
    TRISBbits.TRISB5 = 1;
    ANSELBbits.ANSB5 = 1;
    TRISBbits.TRISB4 = 0;
    LATBbits.LATB4   = 1;
}

float IR_ReadDistance_cm(void) {
    float V = raw_to_voltage(raw_ir);
    return voltage_to_dist(V) * 100.0f;  // metri → cm
}