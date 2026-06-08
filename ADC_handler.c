#include "config.h"
#include "ADC_handler.h"

volatile uint16_t raw_ir  = 0;
volatile uint16_t raw_bat = 0;

/**
 * Initializes the ADC1 peripheral for dual-channel sequential scan.
 *
 * Configures ADC1 to sample two analog inputs in scan mode:
 *   - AN15 / RB15: IR distance sensor
 *   - AN11 / RB11: battery monitor
 *
 * Sampling is triggered automatically every time Timer3 overflows (every 2 ms,
 * SSRC=2). After both channels have been converted, the AD1 interrupt fires
 * (SMPI=1) and the results are read inside _AD1Interrupt().
 *
 * Call this function once at startup, after Timer3 has been configured.
 */

void ADC_init(){

    // ADC configuration ----------------------
    AD1CON1bits.ADON = 0;   // turn off ADC before configuring

    AD1CON3bits.ADCS = 8;   // Tad = 8 Tcy (conversion clock period for 10-bit mode)
    AD1CON1bits.FORM = 0;   // output format: unsigned integer
    AD1CON1bits.ASAM = 1;   // automatic sampling start after conversion completes
    AD1CON3bits.SAMC = 16;  // sample time: 16 Tad
    AD1CON1bits.SSRC = 2;   // Timer3 overflow triggers end of sampling and starts conversion
    //AD1CON1bits.SSRC = 7; // (alternative) automatic conversion trigger
    AD1CON1bits.AD12B = 0;  // select 10-bit mode (not 12-bit)

    AD1CON1bits.SIMSAM = 0; // sequential sampling (channels sampled one after another)
    AD1CON2bits.SMPI = 1;   // interrupt after scanning 2 channels (SMPI = N-1 = 1)
    AD1CON2bits.CSCNA = 1;  // enable input scan mode (use AD1CSSL to select channels)

    // Configure analog input pins
    // IR sensor on RB15 / AN15
    ANSELBbits.ANSB15 = 1;  // set RB15 as analog
    TRISBbits.TRISB15 = 1;  // set RB15 as input
    // Battery monitor on RB11 / AN11
    ANSELBbits.ANSB11 = 1;  // set RB11 as analog
    TRISBbits.TRISB11 = 1;  // set RB11 as input

    AD1CSSL = 0;             // clear scan list before setting channels

    AD1CSSLbits.CSS15 = 1;  // include AN15 in scan list (IR sensor)
    AD1CSSLbits.CSS11 = 1;  // include AN11 in scan list (battery monitor)

    // AD1CON2bits.ALTS = 0; // alternate input selection disabled (not used)

    IEC0bits.AD1IE = 0;  // disable ADC interrupt while setting up
    IFS0bits.AD1IF = 0;  // clear any pending interrupt flag
    IEC0bits.AD1IE = 1;  // enable ADC interrupt

    AD1CON1bits.ADON = 1;   // turn on ADC — sampling begins automatically
    // Summary of timing:
    //   ASAM=1 : sampling restarts automatically after each conversion
    //   SSRC=2 : Timer3 overflow (every 2 ms) stops sampling and starts conversion
    //   SMPI=1 : AD1IF fires after both AN15 and AN11 have been converted
}


/**
 *  ADC1 interrupt service routine.
 *
 * Called automatically by hardware when both ADC channels (AN15 and AN11)
 * have been converted. Reads the results from the ADC result buffer and
 * stores them in the shared volatile variables raw_ir and raw_bat.
 *
 * Buffer assignment (scan order, lowest channel index first):
 *   ADC1BUF0 <- AN11 (battery monitor, lower channel index)
 *   ADC1BUF1 <- AN15 (IR sensor, higher channel index)
 *
 * No polling is needed here because the ISR itself is the end-of-conversion
 * notification; AD1CON1bits.DONE is already set when the ISR fires.
 */

 void __attribute__((interrupt, no_auto_psv)) _AD1Interrupt(void) {
    raw_ir  = ADC1BUF1;   
    raw_bat = ADC1BUF0;   
    IFS0bits.AD1IF = 0;   
} 


uint16_t get_raw_IR(){
    return raw_ir;
}

uint16_t get_raw_battery(){
    return raw_bat;
}

/**
 *  Converts a raw 10-bit ADC value to the corresponding voltage.
 *
 * Assumes a 3.3 V reference (Vref+ = 3.3 V, Vref- = 0 V).
 * Formula: V = raw * 3.3 / 1023
 *
 *  raw  Raw ADC sample in the range [0, 1023].
 *  Voltage in volts [0.0, 3.3].
 */

float raw_to_voltage(uint16_t raw){
    return (float)raw * 3.3f / 1023.0f;
}

/**
 * Converts a sensor voltage to an estimated distance in centimetres.
 *
 * Uses a 4th-order polynomial fit to the IR sensor's voltage-distance curve.
 * Valid only within the sensor's specified operating range (verify against
 * the datasheet for the exact model in use).
 *
 *  voltage  Voltage read from the IR sensor [V].
 *          Estimated distance [cm].
 */
   
float voltage_to_dist(float voltage){
    return 2.34f + voltage*(-4.74f + voltage*(4.06f + voltage*(-1.60f + voltage*0.24f)));
}

