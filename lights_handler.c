#include "config.h"
#include "lights_handler.h"

//Initialize Left, right and low intensity lights as digital outputs, both off
// for definitions of R_LIGHTS_TRIS, R_LIGHTS_LAT, R_LIGHTS_TRIS, L_LIGHTS_LAT,
// LOW_LIGHTS_TRIS and LOW_LIGHTS_LAT see config.h
void lights_init(void) {

    R_LIGHTS_TRIS = 0;     // output right lights
    R_LIGHTS_LAT = 0;      // lights turned off
	
	L_LIGHTS_TRIS = 0;     // output left lights
    L_LIGHTS_LAT = 0;      // lights turned off

	LOW_LIGHTS_TRIS = 0;   // output low intensity lights
    LOW_LIGHTS_LAT = 0;    // lights turned off

    BEAM_TRIS = 0;         // output beam headlights
    BEAM_LAT = 0;          // lights turned off
}

void left_lights_toggle(void){
    L_LIGHTS_LAT = !L_LIGHTS_LAT; 
}
void right_lights_toggle(void){
    R_LIGHTS_LAT = !R_LIGHTS_LAT; 
}
void low_intensity_toggle(void){
    LOW_LIGHTS_LAT = !LOW_LIGHTS_LAT;
}
void headlights_toggle(void){
    BEAM_LAT = !BEAM_LAT;
}