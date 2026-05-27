#include "config.h"
#include "ADC_handler.h"

volatile uint16_t raw_ir  = 0;
volatile uint16_t raw_bat = 0;

void ADC_init(){

     //configuration ADC----------------------
    
    AD1CON1bits.AD12B = 0; // Select 10-bit mode
    AD1CON3bits.ADCS = 8; //Tad how long is an instant(8Tcy for 10bit adc)
    AD1CON1bits.ASAM = 1; // automatic starting
    AD1CON3bits.SAMC = 16; //sample time 16 Tad
    AD1CON1bits.SSRC = 2;  // Timer1 triggera la fine del campionamento (
    /* AD1CON1bits.SSRC = 7; //automatic ending */
    
    AD1CON1bits.SIMSAM = 0; // Enable Sequential Sampling
    AD1CON2bits.ALTS = 0; // NO!Enable Alternate Input Selection
    AD1CON2bits.CSCNA = 1; // scan mode
    AD1CON2bits.CHPS = 0; // channel 0
    AD1CHS0bits.CH0NA = 0;  // negative = GND
    // L'ADC misura una tensione differenziale tra due ingressi:
    // Positivo(CH0SA) mio segnale (non serve se ho scan)
    // Negativo(CH0NA) terra
    
    // Quali canali scansionare: (sono in fila sullo stesso canale)
    AD1CSSLbits.CSS5   = 1;  // AN5  (sensore IR)
    AD1CSSLbits.CSS11  = 1;  // AN11 (batteria)

    AD1CON2bits.SMPI = 1;  // interrupt dopo aver scansionato 2 canali (2-1=1)

    //in futuro configura interrupts
    
    AD1CON1bits.ADON = 1; //turn on 

    IFS0bits.AD1IF = 0;   // pulisci flag
    IEC0bits.AD1IE = 1;   // abilita interrupt ADC
}

// interrupt ADC. Non c'è bisogno di while (!AD1CON1bits.DONE); 
// Viene chiamato quando il flag del fine conversione diventa 1
void __attribute__((interrupt, no_auto_psv)) _ADC1Interrupt(void) {
    raw_ir  = ADC1BUF0;   // AN5  - sensore IR
    raw_bat = ADC1BUF1;   // AN11 - batteria
    IFS0bits.AD1IF = 0;   // pulisci il flag
}


float raw_to_voltage(uint16_t raw){
    return (float)raw * 3.3f / 1024.0f;
}

float voltage_to_dist(float voltage){
    return 2.34f + voltage*(-4.74f + voltage*(4.06f + voltage*(-1.60f + voltage*0.24f)));
}