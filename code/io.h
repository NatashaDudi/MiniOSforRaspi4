#define PERIPHERAL_BASE 0xFE000000

// N: not needed anymore
// #define SAFE_ADDRESS    0x00210000 // Somewhere safe to store a lot of data

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
