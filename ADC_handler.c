#include "config.h"
#include "ADC_handler.h"

volatile uint16_t raw_ir  = 0;
volatile uint16_t raw_bat = 0;

void ADC_init(){

     //configuration ADC----------------------
    AD1CON1bits.ADON = 0;   // turn off
    AD1CON3bits.ADCS = 8; //Tad how long is an instant(8Tcy for 10bit adc)
    AD1CON1bits.FORM = 0; // int
    AD1CON1bits.ASAM = 1; // automatic starting
    AD1CON3bits.SAMC = 16; //sample time 16 Tad
    AD1CON1bits.SSRC = 2;  // Timer3 triggera la fine del campionamento (
    //AD1CON1bits.SSRC = 7; //automatic ending 
    AD1CON1bits.AD12B = 0; // Select 10-bit mode
    
    AD1CON1bits.SIMSAM = 0; // Enable Sequential Sampling
    AD1CON2bits.SMPI = 1;  // interrupt dopo aver scansionato 2 canali (2-1=1)
    AD1CON2bits.CSCNA = 1; // scan mode

    // Set analog
    // IR
    ANSELBbits.ANSB15 = 1;
    TRISBbits.TRISB15 = 1;
    // Battery
    ANSELBbits.ANSB11 = 1;
    TRISBbits.TRISB11 = 1;

    AD1CSSL = 0;

    AD1CSSLbits.CSS15   = 1;  // AN15 (sensore IR)
    AD1CSSLbits.CSS11  = 1;  // AN11 (batteria)

    // AD1CON2bits.ALTS = 0; // NO!Enable Alternate Input Selection

    IEC0bits.AD1IE = 0;  // disabilita interrupt
    IFS0bits.AD1IF = 0;   // pulisci flag
    IEC0bits.AD1IE = 1;   // abilita interrupt ADC 

    AD1CON1bits.ADON = 1; //turn on 
   // AD1CON1bits.SAMP = 1; 
   // ASAM=1: sampling continuo automatico; Timer3 overflow (ogni 2ms) stoppa il sampling
    // e avvia la conversione (SSRC=2); al termine dei 2 canali scatta AD1IF (SMPI=1)
}

// interrupt ADC. Non c'è bisogno di while (!AD1CON1bits.DONE); 
// Viene chiamato quando il flag del fine conversione diventa 1
 void __attribute__((interrupt, no_auto_psv)) _AD1Interrupt(void) {
    raw_ir  = ADC1BUF1;   // AN15  - sensore IR
    raw_bat = ADC1BUF0;   // AN11 - batteria
    IFS0bits.AD1IF = 0;   // pulisci il flag
} 


uint16_t get_raw_IR(){
    return raw_ir;
}

uint16_t get_raw_battery(){
    return raw_bat;
}

float raw_to_voltage(uint16_t raw){
    return (float)raw * 3.3f / 1023.0f;
}

float voltage_to_dist(float voltage){
    return 2.34f + voltage*(-4.74f + voltage*(4.06f + voltage*(-1.60f + voltage*0.24f)));
}

