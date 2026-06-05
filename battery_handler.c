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
    
    // Convert ADC value to battery voltage (in volts)
    double battery_voltage =raw_to_voltage(get_raw_battery()) * 3.0;

    return battery_voltage;
}