#include "PWM_handler.h"
#include "config.h"
#include <xc.h>


// There is an internal counter OCxTMR that increases 
// every peripheral clock cycle (FP = 72 MHz) and resets to 0 
// when it reaches OCxRS (PWM_PERIOD) that in this case is fixed 
// at 10 kHZ. OCxR defines the duty cycle: 
// the output pin goes LOW when OCxTMR == OCxR and then reset to 
// HIGH when OCxTMR == OCxRS (PWM_PERIOD).

// to move the car duty cycle should be at least 40%


// function to convert duty cycle percentage (0-100%) to OCxR value
static inline uint16_t duty_to_OCxR(int duty) {
    if(duty < 0) duty = 0;
    if(duty > 100) duty = 100;
    return (uint16_t)((duty * PWM_PERIOD) / 100);
}
// inline= suggests to the compiler to replace the function call with the actual code.




void pwm_init(void) {

    // Configuration of PWM output pins as digital outputs
    PWM_A_TRIS = 0;   // PWM-A
    PWM_B_TRIS = 0;   // PWM-B
    PWM_C_TRIS = 0;   // PWM-C
    PWM_D_TRIS = 0;   // PWM-D
    
    // pin remapping
    RPOR0bits.RP65R  = OC1_RPIN;  // RD1 -> OC1 (PWM-A)
    RPOR1bits.RP66R  = OC2_RPIN;  // RD2 -> OC2 (PWM-B)
    RPOR1bits.RP67R  = OC3_RPIN;  // RD3 -> OC3 (PWM-C)
    RPOR2bits.RP68R =  OC4_RPIN;  // RD4 -> OC4 (PWM-D)

    // configuration OC1
    OC1CON1bits.OCTSEL = 7;     // Clock = Peripheral Clock (FP)
    OC1CON1bits.OCM = 0b110;    // PWM edge-aligned
    OC1CON2bits.SYNCSEL = 0x1F; // No sync source

    OC1RS  = PWM_PERIOD;
    OC1R = 0;


    // configuration OC2
    OC2CON1bits.OCTSEL = 7;
    OC2CON1bits.OCM = 0b110;
    OC2CON2bits.SYNCSEL = 0x1F;

    // period
    OC2RS  = PWM_PERIOD;
    // duty cycle (motor off at start)
    OC2R = 0;


    // configuration OC3
    OC3CON1bits.OCTSEL = 7;
    OC3CON1bits.OCM = 0b110;
    OC3CON2bits.SYNCSEL = 0x1F;

    OC3RS  = PWM_PERIOD;
    OC3R = 0;


    // configuration OC4
    OC4CON1bits.OCTSEL = 7;
    OC4CON1bits.OCM = 0b110;
    OC4CON2bits.SYNCSEL = 0x1F;

    OC4RS  = PWM_PERIOD;
    OC4R = 0;

}

// Set PWM-A (OC1) duty cycle (0-100%)
void setPWM_A(int duty) {
    OC1R = duty_to_OCxR(duty);
}

// Set PWM-B (OC2) duty cycle (0-100%)
void setPWM_B(int duty) {
    OC2R = duty_to_OCxR(duty);
}

// Set PWM-C (OC3) duty cycle (0-100%)          
void setPWM_C(int duty) {
    OC3R = duty_to_OCxR(duty);
}

// Set PWM-D (OC4) duty cycle (0-100%)
void setPWM_D(int duty) {
    OC4R = duty_to_OCxR(duty);
}

