// battery_handler.c
#include "config.h"
#include "battery_handler.h"
#include "ADC_handler.h"

void battery_init(void){
    // Battery
    TRISBbits.TRISB11 = 1;
    ANSELBbits.ANSB11 = 1;
}

double Battery_ReadVoltage(){
    AD1CON1bits.SAMP = 0;   // Stop sampling and start conversion
    while (!AD1CON1bits.DONE);  // // Wait until conversion is complete
    unsigned int adc_val = ADC1BUF0; // Read raw ADC value
    AD1CON1bits.SAMP = 1;
    
    // Convert ADC value to battery voltage (in volts)
    double battery_voltage =raw_to_voltage(adc_val) * 3.0;
    
    return battery_voltage;
}