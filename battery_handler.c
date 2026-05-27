// battery_handler.c
#include "config.h"
#include "battery_handler.h"
#include "ADC_handler.h"

void battery_init(void){
    // Battery
    TRISBbits.TRISB11 = 1;
    ANSELBbits.ANSB11 = 1;
}

float Battery_ReadVoltage(void) {
    return raw_to_voltage(raw_bat) * BATTERY_DIVIDER_RATIO;
}