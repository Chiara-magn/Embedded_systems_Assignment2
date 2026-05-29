#include "config.h"
#include "UART_handler.h"
#include "IMU_handler.h"

volatile int total_chars = 0; // Total characters received (for debugging)

// Circular buffer instances — rx uses RX_BUFFER_SIZE, tx uses TX_BUFFER_SIZE
static volatile char rx_storage[RX_BUFFER_SIZE];
static volatile char tx_storage[TX_BUFFER_SIZE];

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
/* static int current_hz = 10; // Current $ACC send frequency (default 10 Hz)
static int current_bw = 15; // Current accelerometer bandwidth (default 15 = 1000 Hz) */
static int current_speed = 0; // Current speed
static int current_yawrate = 0; // current yaw

/* Parser states for incoming UART commands
Splits $BW and $HZ cases to avoid accepting invalid values
 */
typedef enum { 
    STATE_WAIT_START,   // Waiting for '$'
    STATE_MSG,          //  PCREF check
    STATE_COMMA1,       // Waiting for first ','
    STATE_COMMA2,       // Waiting for speed and second ','
    STATE_END           // Waiting for yaw and '*'
} parser_state_t;
static parser_state_t state = STATE_WAIT_START;



// UART initialization function
void uart_init(void){
	
    TRISDbits.TRISD0 = 0;               // output D0 -> TX
    TRISDbits.TRISD11 = 1;              // input D11 -> RX
    
    RPINR18bits.U1RXR = UART1_RX_RPIN;  // Map UART1 RX to RPI75 (RD11)
    RPOR0bits.RP64R   = 1;              // Map UART1 TX to RP64  (RD0)
    
    U1STA = 0x00;                       // reset control and status register
    U1MODE = 0x00;                      // reset mode register
    U1BRG = 468 ;                       // Baud rate setting (72000000/16*9600)-1
   // U1BRG = 38; //BAUDrate 115200
    
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
    int next = (tx_buf.head + 1) % tx_buf.size; // Calculate next write position
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
  State machine parser — reads RX buffer and detects complete commands
  Valid commands: $BW,xx* and $HZ,yy* (and also values accepted for each)
  return true if a complete valid-format command was received, false otherwise
 */

bool uart_command_buffer(void) {
    bool string_ready = false;

    while (uart_available()) {
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
                    if (i == 5) {
                        i = 0;
                        state = STATE_COMMA1;
                    }
                } else {
                    i = 0;
                    state = STATE_WAIT_START;
                }
                break;

            case STATE_COMMA1:
                if (c == ',') {
                    command_buffer[i] = '\0';
                    int val = atoi(command_buffer);
                    if (val < -100 || val > 100) {
                        uart_send_string("$ERR,1*");
                        i = 0;
                        state = STATE_WAIT_START;
                    } else {
                        current_speed = val;
                        i = 0;
                        state = STATE_COMMA2;
                    }
                } else if (c == '-' || (c >= '0' && c <= '9')) {
                    command_buffer[i] = c;
                    i++;
                } else {
                    i = 0;
                    state = STATE_WAIT_START;
                }
                break;

            case STATE_COMMA2:
                if (c == '*') {
                    command_buffer[i] = '\0';
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


/*
  Validate and apply a parsed command from command_buffer
  Sends $ERR,1* on invalid values and return true if command was valid and applied, false otherwise.
  Validates $BW,xx* (8 ≤ xx ≤ 15) and $HZ,yy* (yy ∈ {0,1,2,5,10})
 */

/* bool uart_validate_command(void) { 

    // Convert ASCII digits to integer value
    // e.g. '1','5' → (1*10)+5 = 15
    // '0' = 48 in ASCII, so '5'-'0' = 5

    int tens  = command_buffer[2] - '0'; // tens digit
    int units = command_buffer[3] - '0'; // units digit

    uint8_t data = tens * 10 + units; 

    if (command_buffer[0] == 'B'){     // $BW command: set bandwidth
        if (data <8 || data > 15){     // Valid range: 8 to 15
            uart_send_string("$ERR,1*");
            return false;}

        current_bw = data;       // Update global variable with new bandwidth setting           
        imu_set_bandwidth(data); // Apply to IMU register
    }
    if (command_buffer[0] == 'H'){     // $HZ command: set frequency
        if (data != 0 && data != 1 && data != 2 && data != 5 && data != 10){
            uart_send_string("$ERR,2*"); // Valid values: 0,1,2,5,10 only
            return false;}
            current_hz = data;
    }
    return true;
}
 */
/*
  Get the current $ACC send frequency
  return Current frequency in Hz (0 means disabled)
  It will be used in main loop to determine how often to send $ACC messages based on user command
 */
/* int uart_get_hz(void) {
    return current_hz;
}
 */
