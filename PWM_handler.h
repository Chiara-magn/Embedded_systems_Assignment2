#ifndef PWM_HANDLER_H
#define PWM_HANDLER_H

// Inizializza i moduli Output Compare (OC1–OC4) per generare PWM a 10 kHz
void pwm_init(void);

// Imposta il duty cycle (0–100%) sui 4 canali PWM del buggy
void setPWM_A(int duty);
void setPWM_B(int duty);
void setPWM_C(int duty);
void setPWM_D(int duty);

#endif