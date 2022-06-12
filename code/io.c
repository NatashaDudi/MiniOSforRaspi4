// Code by https://github.com/isometimes/rpi4-osdev
// Comments by Adrian, Natasha, (isometimes)

#include "io.h"

// Here starts the GPIO section
enum {
    // Create different values to use places in memory (all as offsets to the PERIPHERAL_BASE which is where the memory addresses start.)
    GPFSEL0         = PERIPHERAL_BASE + 0x200000,
    GPSET0          = PERIPHERAL_BASE + 0x20001C,
    GPCLR0          = PERIPHERAL_BASE + 0x200028,
    GPPUPPDN0       = PERIPHERAL_BASE + 0x2000E4
};

enum {
    GPIO_MAX_PIN       = 53, // Number of pins available on the Raspberry Pi 4
    GPIO_FUNCTION_OUT  = 1,
    GPIO_FUNCTION_ALT5 = 2,
    GPIO_FUNCTION_ALT3 = 7
};

enum {
    // Three different pull states are possible: 
    Pull_None = 0, // This state is needed for our UART.
    Pull_Down = 1,
    Pull_Up = 2
};

// Method to write a value into one of the registers that can be found at the memory addresses listed above. (line 18 + 24) 
void mmio_write(long reg, unsigned int val) { *(volatile unsigned int *)reg = val; }
// Method to read a value from the registers mentioned above. (line 18 + 24)
unsigned int mmio_read(long reg) { return *(volatile unsigned int *)reg; }

unsigned int gpio_call(unsigned int pin_number, unsigned int value, unsigned int base, unsigned int field_size, unsigned int field_max) {
    // Move '1' with shift left 'field_size' times than substract 1
    unsigned int field_mask = (1 << field_size) - 1;
  
    // Like error handling: if the pin number is bigger than the pin numbers available, gpio_call cannot proceed.
    if (pin_number > field_max) return 0;
    // Like error handling: if the value is bigger than field_mask, gpio_call cannot proceed.
    if (value > field_mask) return 0; 

    unsigned int num_fields = 32 / field_size;
    // Find register that shall be changed
    unsigned int reg = base + ((pin_number / num_fields) * 4);
    // Calculate where a bit has to be changed
    unsigned int shift = (pin_number % num_fields) * field_size;

    // Get value from register 'reg'
    unsigned int curval = mmio_read(reg);
    // change the bit from curval at place "field_mask << shift" to 0
    curval &= ~(field_mask << shift);
    // change the bit from curval at place "value << shift" to 1
    curval |= value << shift;
    // write the newly changed value curval to the register.
    mmio_write(reg, curval);

    return 1;
}

// set a gpio or clear it means using 1 or 0.
unsigned int gpio_set     (unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPSET0, 1, GPIO_MAX_PIN); }
unsigned int gpio_clear   (unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPCLR0, 1, GPIO_MAX_PIN); }
// Three pull states exist. The "gpio_pull" method changes the pull state of a pin.
unsigned int gpio_pull    (unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPPUPPDN0, 2, GPIO_MAX_PIN); }
// Since there are more methods than pins, this method changes between Alt 3 and Alt 5 respectively. 
unsigned int gpio_function(unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPFSEL0, 3, GPIO_MAX_PIN); }

// Using two methods (Alt 3 and Alt 5) with the same pin. Here: Change the pin to use the Alt 3 function.
void gpio_useAsAlt3(unsigned int pin_number) {
    gpio_pull(pin_number, Pull_None);
    gpio_function(pin_number, GPIO_FUNCTION_ALT3);
}

// Using two methods (Alt 3 and Alt 5) with the same pin. Here: Change the pin to use the Alt 5 function.
void gpio_useAsAlt5(unsigned int pin_number) {
    gpio_pull(pin_number, Pull_None);
    gpio_function(pin_number, GPIO_FUNCTION_ALT5);
}

// This method inizialises the pin to a neutral value (neither Alt 3 nor Alt 5 that can be seen above at line 82-91).
void gpio_initOutputPinWithPullNone(unsigned int pin_number) {
    gpio_pull(pin_number, Pull_None);
    gpio_function(pin_number, GPIO_FUNCTION_OUT);
}

// This method either sets or clears a pin depending of the values given.
void gpio_setPinOutputBool(unsigned int pin_number, unsigned int onOrOff) {
    if (onOrOff) {
        gpio_set(pin_number, 1);
    } else {
        gpio_clear(pin_number, 1);
    }
}

// Here starts the UART section


/*
To get I/O here, you need to download PuTTY (only available for Windows)
and use a serial TTL to USB cable to connect your Raspberry Pi 4 to your laptop.

Basic options in PuTTY have to be changed in the following manner:
On the left side are the categories. 
In the category "Session" the "Connection Type" has to be changed to "Serial"
In the category "Session" the "Serial line" is COM and the number that shows up in the settings of connected devices (e. g. "COM3").
In the category "Connection < Serial" change "Speed (baud)" to 115200, "Data bits" to 8, "Stop bits" to 1 and "Parity" and "Flow control" to "None".
*/ 


enum {
    // This uses the same principles as used for GPIO meaning that we start with a base address (=AUX_BASE) and add offsets to use other addresses closeby.
    AUX_BASE        = PERIPHERAL_BASE + 0x215000,
    AUX_IRQ         = AUX_BASE,
    AUX_ENABLES     = AUX_BASE + 4,
    AUX_MU_IO_REG   = AUX_BASE + 64,
    AUX_MU_IER_REG  = AUX_BASE + 68,
    AUX_MU_IIR_REG  = AUX_BASE + 72,
    AUX_MU_LCR_REG  = AUX_BASE + 76,
    AUX_MU_MCR_REG  = AUX_BASE + 80,
    AUX_MU_LSR_REG  = AUX_BASE + 84,
    AUX_MU_MSR_REG  = AUX_BASE + 88,
    AUX_MU_SCRATCH  = AUX_BASE + 92,
    AUX_MU_CNTL_REG = AUX_BASE + 96,
    AUX_MU_STAT_REG = AUX_BASE + 100,
    AUX_MU_BAUD_REG = AUX_BASE + 104,
    // clock speep of 500Mhz
    AUX_UART_CLOCK  = 500000000,
    UART_MAX_QUEUE  = 16 * 1024
};

#define AUX_MU_BAUD(baud) ((AUX_UART_CLOCK/(baud*8))-1)

// Queue to store the char values that will be send.
unsigned char uart_output_queue[UART_MAX_QUEUE];
unsigned int uart_output_queue_write = 0; // Index for the UART queue
unsigned int uart_output_queue_read = 0; // Index for the UART queue

void uart_init() {
    // this method is used at kernel.c in the main method!
    // Here we write different values into different registers to set up the UART.
    mmio_write(AUX_ENABLES, 1); //enable UART1 (comment from isometimes)
    mmio_write(AUX_MU_IER_REG, 0);
    mmio_write(AUX_MU_CNTL_REG, 0);
    mmio_write(AUX_MU_LCR_REG, 3); //8 bits (comment from isometimes)
    mmio_write(AUX_MU_MCR_REG, 0);
    mmio_write(AUX_MU_IER_REG, 0);
    mmio_write(AUX_MU_IIR_REG, 0xC6); //disable interrupts (comment from isometimes)
    mmio_write(AUX_MU_BAUD_REG, AUX_MU_BAUD(115200)); // The baude rate has to be the same as the value we entered with the PuTTY App (which is 115200).
    gpio_useAsAlt5(14); // NOTE: is this a typo? Should this be "gpio_useAsAlt3(14);"?
    gpio_useAsAlt5(15); 
    mmio_write(AUX_MU_CNTL_REG, 3); //enable RX/TX (comment from isometimes)
}

// Checks if there are entries in the queue by checking if anything was written into the queue or not.
unsigned int uart_isOutputQueueEmpty() {
    return uart_output_queue_read == uart_output_queue_write;
}

unsigned int uart_isReadByteReady()  { return mmio_read(AUX_MU_LSR_REG) & 0x01; }
// Checks if the UART line status is good to send something.
unsigned int uart_isWriteByteReady() { return mmio_read(AUX_MU_LSR_REG) & 0x20; }

// Write a single byte.
unsigned char uart_readByte() {
    while (!uart_isReadByteReady());
    return (unsigned char)mmio_read(AUX_MU_IO_REG);
}
// Waits until there is the possibility to send something then writes.
void uart_writeByteBlockingActual(unsigned char ch) {
    while (!uart_isWriteByteReady()); 
    mmio_write(AUX_MU_IO_REG, (unsigned int)ch);
}

// Get Output from the First-In-First-Out Queue
void uart_loadOutputFifo() {
    // If there are entries in the queue and the UART is ready
    while (!uart_isOutputQueueEmpty() && uart_isWriteByteReady()) {
        // write every entry from the list
        uart_writeByteBlockingActual(uart_output_queue[uart_output_queue_read]);
        uart_output_queue_read = (uart_output_queue_read + 1) & (UART_MAX_QUEUE - 1);
    }
}
// Writing a char into the queue to write from the Raspberry Pi 4 to your laptop.
void uart_writeByteBlocking(unsigned char ch) {
    unsigned int next = (uart_output_queue_write + 1) & (UART_MAX_QUEUE - 1);

    while (next == uart_output_queue_read) uart_loadOutputFifo();
    // Go through the FIFO-queue and put value into the output queue.
    uart_output_queue[uart_output_queue_write] = ch;
    uart_output_queue_write = next;
}

// Sends a list of chars which is a string.
void uart_writeText(char *buffer) {
    while (*buffer) {
       if (*buffer == '\n') uart_writeByteBlockingActual('\r');
       uart_writeByteBlockingActual(*buffer++);
    }
}
// Get all the values out of the queue
void uart_drainOutputQueue() {
    while (!uart_isOutputQueueEmpty()) uart_loadOutputFifo();
}
// This method is used in the main function to get the current UART input meaning that it reads values that were sent from your laptop to the Raspberry Pi 4.
void uart_update() {
    uart_loadOutputFifo();

    if (uart_isReadByteReady()) {
       unsigned char ch = uart_readByte();
       // if a '\r' is send than this should be interpreted as a new line. In every other case the char has to be analyzed.
       if (ch == '\r') uart_writeText("\n"); else uart_writeByteBlocking(ch);
    }
}
