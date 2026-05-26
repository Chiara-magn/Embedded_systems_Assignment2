#include "motor.h"
#include "PWM_handler.h"


// left - A=RD1/B=RD2
// right - C=RD3/D=RD4

//RD2/4 forward
//RD1/3 backward

// Inizializzazione generale del modulo motori
void motor_init(void) {
    // Qui non serve nulla se PWM gia' inizializzato in pwm_init()
    // Manteniamo la funzione per coerenza architetturale
   motor_stop();   // <-- forza stato fermo all'avvio
}

// Ferma tutti i motori
void motor_stop(void) {
    setPWM_A(0);
    setPWM_B(0);
    setPWM_C(0);
    setPWM_D(0);
}

// Movimento in avanti (Assignment 1)
// duty [40..100]%
void motor_forward(int duty) {   
    setPWM_A(0);
    setPWM_B(duty);
    setPWM_C(0);
    setPWM_D(duty);
}

// Movimento in avanti con rotazione oraria (Assignment 2)
// duty_left > duty_right
void motor_forward_clockwise(int duty_left, int duty_right) {

    // Lato sinistro piu veloce per girare a destra
    setPWM_A(0);
    setPWM_B(duty_left);

    // Lato destro pi� lento
    setPWM_C(0);
    setPWM_D(duty_right);
}

// Funzione generale speed + yaw rate (Assignment 3)
// speed  [-100..100]
// yaw    [-100..100]
void motor_speed_yaw(int speed, int yaw) {

    // Calcolo velocit� differenziale
    int L = speed + yaw;
    int R = speed - yaw;

    // Saturazione
    if(L > 100) L = 100;
    if(L < -100) L = -100;
    if(R > 100) R = 100;
    if(R < -100) R = -100;

    
    if(L > 0) {          // avanti
        setPWM_A(0);
        setPWM_B(L);
    }
    else if(L < 0) {    // indietro
        setPWM_A(-L);
        setPWM_B(0);
    }
    else {                // fermo
        setPWM_A(0);
        setPWM_B(0);
    }

    // LATO DESTRO
    if(R > 0) {          // avanti
        setPWM_C(0);
        setPWM_D(R);
    }
    else if(R < 0) {    // indietro
        setPWM_C(-R);
        setPWM_D(0);
    }
    else {                // fermo
        setPWM_C(0);
        setPWM_D(0);
    }
}
