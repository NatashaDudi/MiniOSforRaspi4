// Code by https://github.com/isometimes/rpi4-osdev
// Comments by Adrian, Natasha

#define PERIPHERAL_BASE 0xFE000000 // This is where the memory addresses start

// All these methods are defined and described at io.c
unsigned int gpio_call();
unsigned int gpio_set();
unsigned int gpio_clear();
unsigned int gpio_pull();
unsigned int gpio_function();
void uart_init();
void uart_writeText(char *buffer);
void uart_loadOutputFifo();
unsigned char uart_readByte();
unsigned int uart_isReadByteReady();
void uart_writeByteBlocking(unsigned char ch);
void uart_update();
void mmio_write(long reg, unsigned int val);
unsigned int mmio_read(long reg);
