#ifndef UART_HANDLER_H
#define UART_HANDLER_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h> 

extern volatile int total_chars; // Total characters received (for debugging)

void uart_init(void);

void uart_send_char(char c);
void uart_send_string(const char *s);

int uart_available(void);
char uart_read_char(void);

bool uart_command_buffer(void);

int uart_get_speed(void);   
int uart_get_yawrate(void); 

int uart_get_rx_count(void);
int uart_get_tx_count(void);

// useful to save some time for deadlines
void uart_append_fixed(char *buf, int *position, float val);

// Circular buffer to get user commands
#define RX_BUFFER_SIZE 32 // emptied by command buffer max ~10 char/ciclo for 9600 baud

#define UART_COMMAND_BUFFER_SZ 8 // max 3 characters 
// contains only correctly formatted commands, emptied by command processor

#define TX_BUFFER_SIZE   128  // contains data from imu to UART, emptied by TX ISR
// worst case scenario task2 + task3 messages, total bytes in the buffer 60/63. with some margin 128

// Circular buffer struct — used for both RX and TX buffers.
// head: index where next character will be written by ISR (RX) or main code (TX)
// tail: index where next character will be read by main code (RX) or ISR (TX)
// size: actual buffer size

typedef struct {
    volatile char *buf;  //max sixe
    volatile int  head;     
    volatile int  tail;     
    int           size;     // dimention used at runtime (RX_BUFFER_SIZE or TX_BUFFER_SIZE)
} circular_buffer_t;

#endif