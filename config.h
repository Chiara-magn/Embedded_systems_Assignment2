#ifndef CONFIG_H
#define CONFIG_H

#include "xc.h"
#include "stdlib.h"
#include "stdio.h"

// UART1 pin remapping
#define UART1_RX_RPIN   75   // UART1 RX mapped to RPI75 (RD11, input)
#define UART1_TX_RPIN   64   // UART1 TX mapped to RP64  (RD0,  output)

// SPI1 pin remapping
#define SPI1_MISO_RPIN   17   // MISO        mapped to RPI17 (RA1,  input)
#define SPI1_MOSI_RPIN   109  // MOSI        mapped to RP109 (RF13, output)
#define SPI1_SCK_RPIN    108  // SCK1        mapped to RP108 (RF12, output)

// IMU Chip Select

#define ACC_CS_LAT     LATBbits.LATB3   // Accelerometer Chip Select → output latch  (RB3)
#define ACC_CS_TRIS    TRISBbits.TRISB3 // Accelerometer Chip Select → direction register
#define ACC_CHIP_ID    0xFA             // Expected chip ID 

#define GYR_CS_LAT     LATBbits.LATB4   // Gyroscope Chip Select → output latch  (RB4)
#define GYR_CS_TRIS    TRISBbits.TRISB4 // Gyroscope Chip Select → direction register
#define GYR_CHIP_ID    0x0F             // Expected chip ID 

#define MAG_CS_LAT     LATDbits.LATD6   // Magnetometer Chip Select → output latch  (RD6)
#define MAG_CS_TRIS    TRISDbits.TRISD6 // Magnetometer Chip Select → direction register
#define MAG_CHIP_ID    0x32             // Expected chip ID 


// LED
#define LD1_LAT     LATAbits.LATA0       // LD1 connected to RA0       
#define LD1_TRIS    TRISAbits.TRISA0

#define LD2_LAT     LATGbits.LATG9       // LD2 connected to RG9        
#define LD2_TRIS    TRISGbits.TRISG9


// LIGHTS
#define R_LIGHTS_LAT  LATFbits.LATF1     // Right Lights connected to RF1
#define R_LIGHTS_TRIS TRISFbits.TRISF1

#define L_LIGHTS_LAT LATBbits.LATB8      // Left Lights connected to RB8
#define L_LIGHTS_TRIS TRISBbits.TRISB8 

#define LOW_LIGHTS_LAT LATGbits.LATG1    // Low intensity connected to RG1
#define LOW_LIGHTS_TRIS TRISGbits.TRISG1

#define BEAM_LAT   LATAbits.LATA7        // Beam headlights connected to RA7
#define BEAM_TRIS  TRISAbits.TRISA7


// BUTTONS (INT1 e INT2)
#define BTN_T3_TRIS   TRISEbits.TRISE9
#define BTN_T3_RPIN   89

#define BTN_T2_TRIS   TRISEbits.TRISE8
#define BTN_T2_RPIN   88


// PWM OUTPUTS (OC1–OC4)
#define PWM_A_TRIS    TRISDbits.TRISD1 // PWM-A → RD1 → OC1
#define OC1_RPIN      16

#define PWM_B_TRIS    TRISDbits.TRISD2 // PWM-B → RD2 → OC2
#define OC2_RPIN      17

#define PWM_C_TRIS    TRISDbits.TRISD3 // PWM-C → RD3 → OC3
#define OC3_RPIN      18

#define PWM_D_TRIS    TRISDbits.TRISD4 // PWM-D → RD4 → OC4
#define OC4_RPIN      19

// ADC channel assignments ---
#define ADC_CH_BATTERY      AD1CSSLbits.CSS11      // AN11  → BAT-VSENSE line (slide 26)
#define ADC_CH_IR           AD1CSSLbits.CSS15      // AN15   → IR Distance Sensor 

// BATTERY
 
#define BATTERY_DIVIDER_RATIO   3.0f       // 3 resistors (R49, R51, R54): Vbat = 3 * Vadc 
#define BATTERY_TRIS    TRISBbits.TRISB11  // RB11 as input
#define BATTERY_ANSEL   ANSELBbits.ANSB11  // RB11 as analog input

// IR infrared sensor
#define OBSTACLE_DETECTED_THRESHOLD 15 // 15 cm
#define IR_AN_TRIS   TRISBbits.TRISB15
#define IR_AN_ANSEL  ANSELBbits.ANSB15

#define IR_EN_TRIS   TRISBbits.TRISB9
#define IR_EN_LAT    LATBbits.LATB9

#endif

