#include "config.h"
#include "UART_handler.h"
#include "IMU_handler.h"

volatile int total_chars = 0; // Total characters received (for debugging)

// arrays for RX and TX circular buffers (physical memory storage)
// defined as volatile since they are accessed in ISRs
static volatile char rx_storage[RX_BUFFER_SIZE];
static volatile char tx_storage[TX_BUFFER_SIZE];

// Circular buffer structures for RX and TX, containing pointers 
// to the storage arrays and head/tail indices
static circular_buffer_t rx_buf = {
    .buf = rx_storage,
    .head = 0,
    .tail = 0,
    .size = RX_BUFFER_SIZE
};

static circular_buffer_t tx_buf = {
    .buf = tx_storage,
    .head = 0,
    .tail = 0,
    .size = TX_BUFFER_SIZE
};


// command buffer for parsing
static char command_buffer[UART_COMMAND_BUFFER_SZ]; //stores incoming command
static uint8_t i = 0;       // Index into command_buffer
static int current_speed = 0; // Current speed
static int current_yawrate = 0; // current yaw

// static int current_hz = 10; // Current $ACC send frequency (default 10 Hz)
// static int current_bw = 15; // Current accelerometer bandwidth (default 15 = 1000 Hz) 


// Parser states for incoming UART commands
typedef enum { 
    STATE_WAIT_START,   // Waiting for '$'
    STATE_MSG,          // PCREF check
    STATE_COMMA1,       // Waiting for first ','
    STATE_COMMA2,       // Waiting for speed and second ','
} parser_state_t;

// initialize parser state to wait for start of command
static parser_state_t state = STATE_WAIT_START;



// UART initialization function
void uart_init(void){
	
    UART_TRIS_TX = 0;               // output D0 -> TX
    UART_TRIS_RX = 1;              // input D11 -> RX
    
    RPINR18bits.U1RXR = UART1_RX_RPIN;  // Map UART1 RX to RPI75 (RD11)
    RPOR0bits.RP64R   = UART1_TX_RPIN;              // Map UART1 TX to RP64  (RD0)
    
    U1STA = 0x00;                       // reset control and status register
    U1MODE = 0x00;                      // reset mode register
    U1BRG = 468 ;                       // Baud rate setting (72000000/16*9600)-1
   
    U1MODEbits.UARTEN = 1;              // Enable UART
    U1STAbits.UTXEN = 1;                // Enable TX
    
    U1STAbits.URXISEL = 0;              //Interrupt is set on every received character
    IFS0bits.U1RXIF   = 0;              // Clear RX interrupt flag
    IEC0bits.U1RXIE   = 1;              // Enable RX interrupt
	
}
  
/* 
UART1 RX interrupt — called on every received character, 
saves it in the circular buffer if there's space, otherwise discards it

*/

void __attribute__((interrupt, no_auto_psv))
_U1RXInterrupt(void) {
    IFS0bits.U1RXIF = 0;              // Clear interrupt flag

    // Drain all characters currently in the UART FIFO
    while (U1STAbits.URXDA) {
        char c = U1RXREG;             // Read next received character
        total_chars++;

        int next = (rx_buf.head + 1) % rx_buf.size;  // Next write position
        if (next != rx_buf.tail) {    // If buffer is not full
            rx_buf.buf[rx_buf.head] = c;  // Store character
            rx_buf.head = next;            // Advance write index
        }
        // If buffer is full: discard character silently
    }
}

/*
  UART1 TX interrupt — called when U1TXREG is empty, sends next byte from TX circular buffer
  Disables itself when buffer is empty. 
  Replaces while(U1STAbits.UTXBF) that waited for the buffer to be empty,
  allowing non-blocking transmission.

 */

void __attribute__((interrupt, no_auto_psv))
_U1TXInterrupt(void) {
    IFS0bits.U1TXIF = 0;              // Clear interrupt flag

    while (!U1STAbits.UTXBF && tx_buf.tail != tx_buf.head) {   // If TX buffer has data
        U1TXREG = tx_buf.buf[tx_buf.tail]; // Send next character
        tx_buf.tail = (tx_buf.tail + 1) % tx_buf.size;  // Advance read index
    } 
    if (tx_buf.tail == tx_buf.head) {
        IEC0bits.U1TXIE = 0;  // Buffer empty: disable TX interrupt
    }
}

// Send a null-terminated string over UART one character at a time
void uart_send_string(const char *s) {
    while (*s) {          // Loop until null terminator '\0'
        uart_send_char(*s);  // Send one character at a time
        s++;                 // Advance pointer to next character
    }
}

// Check if RX buffer contains unread characters (returns 1 if available, 0 if empty)
int uart_available(void) {
    IEC0bits.U1RXIE = 0;   // Disable RX interrupt to safely read shared variable
    int head = rx_buf.head;   // Local copy of head
    IEC0bits.U1RXIE = 1;   // Re-enable RX interrupt
    return (head != rx_buf.tail);
}

// Read one character from RX buffer, returns 0 if buffer is empty
char uart_read_char(void) {
    IEC0bits.U1RXIE = 0;    // Disable RX interrupt to safely read shared variable (could use uart_available here )
    int head = rx_buf.head;    // Local copy to avoid race conditions
    IEC0bits.U1RXIE = 1;

    if (head == rx_buf.tail) return 0;  // Buffer empty

    char c = rx_buf.buf[rx_buf.tail];  // Read oldest character
    
    rx_buf.tail = (rx_buf.tail + 1) % rx_buf.size; 
    // Advance read index 
    //and come back to 0 if we reach the end of the buffer 
    //(ex: rx_buf.tail = 7 → (7+1) % 8 = 0 ) back to the start
    
    return c;
}

/*sends a single character over UART, non-blocking,
 using the TX circular buffer and enabling the TX interrupt*/  

void uart_send_char(char c) {
    int next = (tx_buf.head + 1) % tx_buf.size; // Calculate next bb  position
    while (next == tx_buf.tail);   // Wait if TX buffer is full 

    tx_buf.buf[tx_buf.head] = c;    // Store character in TX buffer
    tx_buf.head = next;            // Advance write index          
    
    IEC0bits.U1TXIE = 0;          // Disable TX interrupt to safely check if U1TXREG is empty
    if (!U1STAbits.UTXBF) {       // If hardware TX buffer is empty, 
        // kickstart transmission by writing first character
        
        if (tx_buf.tail != tx_buf.head) { // if buffer has data
            U1TXREG = tx_buf.buf[tx_buf.tail];  //send first byte
            tx_buf.tail = (tx_buf.tail + 1) % tx_buf.size; // Advance read index
        }
    }
    IEC0bits.U1TXIE = 1;          // Re-enable TX interrupt — ISR will send the rest
}

/*
    uart_command_buffer - non-blocking parser for incoming UART commands in the format $PCREF,speed,yawrate*
    It uses a state machine to parse the command character by character as they arrive in the RX buffer.
    Returns true if a valid command was fully received and parsed, false otherwise.
    On valid command, updates global variables current_speed and current_yawrate with parsed values.
    On invalid command, sends $ERR,1* or $ERR,2* over UART depending on which value was invalid.
 */

bool uart_command_buffer(void) {
    
    // variable to keep track of parsing state across function calls
    bool string_ready = false;

    while (uart_available()) { //if there are characters in the RX buffer, read and parse them one by one
        
        char c = uart_read_char();

        switch (state) {

            case STATE_WAIT_START:
                if (c == '$') {
                    state = STATE_MSG;
                    i = 0;
                }
                break;

            case STATE_MSG:
            
                if (c == "PCREF"[i]) {
                    i++;
                } else if (i == 5 && c == ',') {
                    // all 5 characters of "PCREF" matched, now waiting for ',' before speed value
                    i = 0;
                    state = STATE_COMMA1;
                } else {
                    i = 0;
                    state = STATE_WAIT_START;
                }
                break;

            case STATE_COMMA1:
                // we have reached next comma, check if the speed value is valid, 
                // if so move to next state to parse yawrate, if not send error and reset parser    
                if (c == ',') {
                    command_buffer[i] = '\0';
                    // convert speed string to integer 
                    int val = atoi(command_buffer);
                    // check if speed is in valid range -100 to 100, if not send error message and reset parser
                    if (val < -100 || val > 100) {
                        uart_send_string("$ERR,1*");
                        i = 0;
                        state = STATE_WAIT_START;
                    } else {
                        current_speed = val;
                        i = 0;
                        state = STATE_COMMA2;
                    }

                // if we receive a valid character for speed value (digit or minus sign), 
                //store it in command_buffer for later parsing    
                } else if (c == '-' || (c >= '0' && c <= '9')) {
                    command_buffer[i] = c;
                    i++;

                } else {
                    i = 0;
                    state = STATE_WAIT_START;
                }
                break;

            case STATE_COMMA2:

                // we have reached the end of the command, check if yawrate value is valid,
                // if so we have a complete valid command and can return true, if not send error
                if (c == '*') {
                    command_buffer[i] = '\0';
                    // convert yawrate string to integer
                    int val = atoi(command_buffer);
                    if (val < -100 || val > 100) {
                        uart_send_string("$ERR,2*");
                        i = 0;
                        state = STATE_WAIT_START;
                    } else {
                        current_yawrate = val;
                        i = 0;
                        string_ready = true;
                        state = STATE_WAIT_START;
                    }
                } else if (c == '-' || (c >= '0' && c <= '9')) {
                    command_buffer[i] = c;
                    i++;
                } else {
                    i = 0;
                    state = STATE_WAIT_START;
                }
                break;
        }

        // safety check to avoid overflow of command_buffer
        if (i >= UART_COMMAND_BUFFER_SZ - 1) {
            i = 0;
            state = STATE_WAIT_START;
        }

        if (string_ready) break;
    }

    return string_ready;
}

int uart_get_speed(void)   { return current_speed; }
int uart_get_yawrate(void) { return current_yawrate; }

int uart_get_rx_count(void) {
    return (rx_buf.head - rx_buf.tail + rx_buf.size) % rx_buf.size;
}

int uart_get_tx_count(void) {
    return (tx_buf.head - tx_buf.tail + tx_buf.size) % tx_buf.size;
}



/*
 * append_fixed - writes a float as a fixed-point string into a buffer
 * without using sprintf, to avoid slow float formatting on dsPIC.
 *
 * Example: -3.15 → writes '-', '3', '.', '1', '5' into buf
 *
 * buf  - output character buffer
 * position  - current write position in buffer (updated after each character)
 * val  - float value to write (max 2 decimal places)
 */

 
void uart_append_fixed(char *buf, int *position, float val) {
    
    // multiply by 100 to keep 2 decimal places as an integer
    // example: -3.15 * 100 = -315
    int v = (int)(val * 100.0f);
    
    // write the sign and work with the absolute value
    if (v < 0) { buf[(*position)++] = '-'; v = -v; }
    
    // split integer and decimal parts
    // example: 315 / 100 = 3 (integer part)
    //          315 % 100 = 15 (decimal part)
    int integer_part = v / 100;
    int decimal_part = v % 100;
    
    // write integer part digit by digit, without leading zeros
    // example: 3   → writes '3'
    //          39  → writes '3', '9'
    //          123 → writes '1', '2', '3'
    if (integer_part >= 100) {
        buf[(*position)++] = '0' + (integer_part / 100);        // hundreds
        buf[(*position)++] = '0' + (integer_part / 10 % 10);   // tens
        buf[(*position)++] = '0' + (integer_part % 10);         // units
    } else if (integer_part >= 10) {
        buf[(*position)++] = '0' + (integer_part / 10);         // tens
        buf[(*position)++] = '0' + (integer_part % 10);         // units
    } else {
        buf[(*position)++] = '0' + integer_part;                // units only
    }
    
    // write decimal point and always 2 decimal digits
    // example: 15 → writes '.', '1', '5'
    //          05 → writes '.', '0', '5'
    buf[(*position)++] = '.';
    buf[(*position)++] = '0' + (decimal_part / 10);   // first decimal digit
    buf[(*position)++] = '0' + (decimal_part % 10);   // second decimal digit
}