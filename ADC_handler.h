#ifndef ADC_HANDLER_H
#define ADC_HANDLER_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h> 

extern volatile uint16_t raw_ir;
extern volatile uint16_t raw_bat;

void ADC_init(void);
float raw_to_voltage(uint16_t raw);
float voltage_to_dist(float voltage);
double read_battery();
int read_IR();

#endif