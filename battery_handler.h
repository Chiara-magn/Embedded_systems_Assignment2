#ifndef BATTERY_HANDLER_H
#define BATTERY_HANDLER_H

void battery_init(void);

// Legge la tensione della batteria in Volt dall'ADC in scan mode.
// Applica il rapporto del divisore di tensione (3 resistenze uguali → Vbat = 3 * Vadc).
double Battery_ReadVoltage(void);

#endif // BATTERY_HANDLER_H
