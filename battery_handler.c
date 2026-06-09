// battery_handler.c
#include "config.h"
#include "battery_handler.h"
#include "ADC_handler.h"

void battery_init(void){
    // Battery
    BATTERY_TRIS = 1; // Set RB11 as input
    BATTERY_ANSEL = 1; // Set RB11 as analog
}

double Battery_ReadVoltage(){
    
    // Convert ADC value to battery voltage (in volts)
    // The battery voltage is scaled by a factor of 3, so we need 
    // to multiply the measured voltage to get the actual battery one.

    double battery_voltage =raw_to_voltage(get_raw_battery()) * BATTERY_DIVIDER_RATIO;

    return battery_voltage;
}