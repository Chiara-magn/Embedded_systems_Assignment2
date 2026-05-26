#ifndef LIGHTS_HANDLER_H
#define LIGHTS_HANDLER_H

#include <xc.h>
#include <stdint.h>

void lights_init(void);
void left_lights_toggle(void);
void right_lights_toggle(void);
void low_intensity_toggle(void);
void headlights_toggle(void);

#endif
