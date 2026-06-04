#include "config.h"
#include "button_handler.h"

volatile int btn_t2_pressed = 0;
volatile int btn_t3_pressed = 0;


void buttons_init(void){
	
	// INT1 button 3
    BTN_T3_TRIS = 1;                 // input
    RPINR0bits.INT1R = BTN_T3_RPIN;  // remapping
    IEC1bits.INT1IE = 0; 
    IFS1bits.INT1IF = 0;             // clear flag
    IEC1bits.INT1IE = 1;             // abilita interrupt

	// INT2 button 2
    BTN_T2_TRIS = 1;                 // input
    RPINR1bits.INT2R = BTN_T2_RPIN;  // remapping
    IEC1bits.INT2IE = 0; 
    IFS1bits.INT2IF = 0;             // clear flag
    IEC1bits.INT2IE = 1;             // abilita interrupt
	
}

// --- ISR BUTTON ---
void __attribute__((interrupt, no_auto_psv)) 
_INT1Interrupt(void) {
    IFS1bits.INT1IF = 0;
    btn_t3_pressed = 1;
}

void __attribute__((interrupt, no_auto_psv)) 
_INT2Interrupt(void) {
    IFS1bits.INT2IF = 0;
    btn_t2_pressed = 1;
}
// --- ---


int button_t2_pressed(void) {
    if (btn_t2_pressed) {
        btn_t2_pressed = 0;
        return 1;
    }
    return 0;
}

int button_t3_pressed(void) {
    if (btn_t3_pressed) {
        btn_t3_pressed = 0;
        return 1;
    }
    return 0;
}


