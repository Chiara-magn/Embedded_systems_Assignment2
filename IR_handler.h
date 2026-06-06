#ifndef IR_HANDLER_H
#define IR_HANDLER_H

void IR_init(void);

// Legge la distanza dal sensore IR in centimetri dall'ADC in scan mode.
// Il sensore ha range valido 10 cm – 150 cm (slide 23).
// La rispositionta non lineare è approssimata con un polinomio di 4° ordine (slide 24).
int IR_ReadDistance_cm(void);

#endif // IR_HANDLER_H
