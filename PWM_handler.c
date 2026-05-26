#include "PWM_handler.h"
#include "config.h"
#include <xc.h>

// Parametri PWM

#define FCY     72000000UL      // 72 MHz
#define FPWM    10000           // 10 kHz
#define OCxRS ((FCY / FPWM) - 1)   
//period, timer resets when OCxTMR == OCxRS 

// Funzione interna: converte duty in OCxR
static inline uint16_t duty_to_OCxR(int duty) {
    if(duty < 0) duty = 0;
    if(duty > 100) duty = 100;
    return (uint16_t)((duty * OCxRS) / 100);
}
//OCxR = definisce il duty cycle (il pin va LOW quando OCxTMR == OCxR)
// il range utile va da 40 a 100%

// Inizializzazione PWM (OC1�??OC4)
void pwm_init(void) {

    // Configurazione pin RD1-RD4 come output
    PWM_A_TRIS = 0;   // PWM-A
    PWM_B_TRIS = 0;   // PWM-B
    PWM_C_TRIS = 0;   // PWM-C
    PWM_D_TRIS = 0;   // PWM-D
    
    RPOR0bits.RP65R  = OC1_RPIN;  // RD1 -> OC1 (PWM-A)
    RPOR1bits.RP66R  = OC2_RPIN;  // RD2 -> OC2 (PWM-B)
    RPOR1bits.RP67R  = OC3_RPIN;  // RD3 -> OC3 (PWM-C)
    RPOR2bits.RP68R =  OC4_RPIN;  // RD4 -> OC4 (PWM-D)

    // Configurazione OC1
    OC1CON1bits.OCTSEL = 7;     // Clock = Peripheral Clock (FP)
    OC1CON1bits.OCM = 0b110;    // PWM edge-aligned
    OC1CON2bits.SYNCSEL = 0x1F; // No sync source

    OC1R  = 0;
    OC1RS = OCxRS;


    // Configurazione OC2
    OC2CON1bits.OCTSEL = 7;
    OC2CON1bits.OCM = 0b110;
    OC2CON2bits.SYNCSEL = 0x1F;

    OC2R  = 0;
    OC2RS = OCxRS;


    // Configurazione OC3
    OC3CON1bits.OCTSEL = 7;
    OC3CON1bits.OCM = 0b110;
    OC3CON2bits.SYNCSEL = 0x1F;

    OC3R  = 0;
    OC3RS = OCxRS;


    // Configurazione OC4
    OC4CON1bits.OCTSEL = 7;
    OC4CON1bits.OCM = 0b110;
    OC4CON2bits.SYNCSEL = 0x1F;

    OC4R  = 0;
    OC4RS = OCxRS;

    

}

// Set PWM-A (OC1)
void setPWM_A(int duty) {
    OC1R = duty_to_OCxR(duty);
}


// Set PWM-B (OC2)
void setPWM_B(int duty) {
    OC2R = duty_to_OCxR(duty);
}

// Set PWM-C (OC3)
void setPWM_C(int duty) {
    OC3R = duty_to_OCxR(duty);
}


// Set PWM-D (OC4)
void setPWM_D(int duty) {
    OC4R = duty_to_OCxR(duty);
}

