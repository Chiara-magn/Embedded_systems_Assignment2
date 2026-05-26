#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <xc.h>
#include <stdint.h>

extern volatile int btn_t2_pressed;
extern volatile int btn_t3_pressed;

void buttons_init(void);
int  button_t2_pressed(void);
int  button_t3_pressed(void);

#endif