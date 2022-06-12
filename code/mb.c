// Code by https://github.com/isometimes/rpi4-osdev
// Comments by Adrian, Natasha, (isometimes)

#include "io.h"
// io.h is included to use PERIPHERAL_BASE, mmio_read and mmio_write.

// The buffer must be 16-byte aligned as only the upper 28 bits of the address can be passed via the mailbox (comment from isometimes)
// This value is used for the mailbox.
volatile unsigned int __attribute__((aligned(16))) mbox[36];

enum {
    // Define values that can be used for addresses, using the PERIPHERAL_BASE (start of addresses) and adding additional values to it.
    VIDEOCORE_MBOX = (PERIPHERAL_BASE + 0x0000B880),
    MBOX_READ      = (VIDEOCORE_MBOX + 0x0),
    MBOX_POLL      = (VIDEOCORE_MBOX + 0x10),
    MBOX_SENDER    = (VIDEOCORE_MBOX + 0x14),
    MBOX_STATUS    = (VIDEOCORE_MBOX + 0x18),
    MBOX_CONFIG    = (VIDEOCORE_MBOX + 0x1C),
    MBOX_WRITE     = (VIDEOCORE_MBOX + 0x20),
    MBOX_RESPONSE  = 0x80000000,
    MBOX_FULL      = 0x80000000,
    MBOX_EMPTY     = 0x40000000
};

// Method to wait for a new message 
unsigned int mbox_call(unsigned char ch)
{
    // 28-bit address (MSB) and 4-bit value (LSB) (comment from isometimes)
    unsigned int r = ((unsigned int)((long) &mbox) &~ 0xF) | (ch & 0xF);

    // Stay stuck in a loop until there is an option to write.
    while (mmio_read(MBOX_STATUS) & MBOX_FULL);
    
    // Write the address of our buffer to the mailbox with the channel appended (comment from isometimes)
    mmio_write(MBOX_WRITE, r);

    while (1) {
        // Waiting for a reply
        while (mmio_read(MBOX_STATUS) & MBOX_EMPTY);

        // Is it a reply to our message? (comment from isometimes)
        if (r == mmio_read(MBOX_READ)) return mbox[1]==MBOX_RESPONSE; // Is it successful? (comment from isometimes)
           
    }
    return 0;
}
