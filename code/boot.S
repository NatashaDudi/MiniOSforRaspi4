// Code by https://github.com/isometimes/rpi4-osdev
// Comments by Natasha

// this file is written in Assembly Language
.section ".text.boot"  // A new section is created 

.global _start  // This line is where we start executing. _Start can be referenced by external files now.

_start:
    // Our goal is to run our code on the master core.
    // Therefore we check for the ID of the processor we are currently using.
    mrs     x1, mpidr_el1 // the content of the register mpidr_el1 is written into x1
    and     x1, x1, #3
    cbz     x1, 2f //   Means the same as:
    //                  CMP  x1, #0 -> compare the two operants
    //                  BEQ  2f -> means "branch if equal"

    // If we are in a not-master core, we can start to loop and wait. 
1:  wfe
    b       1b // Branch to 1b
2:  // With 2 we made it and are on the master core!

    // This part is necessary to use a stack soon. 
    ldr     x1, =_start
    mov     sp, x1 // move x1 to sp

    ldr     x1, =__bss_start     // load the bss start from memory into the x1 register
    ldr     w2, =__bss_size      // load the bss size from memory into the w2 register. 
3:  cbz     w2, 4f               // Same as in line 14 but this time with w2. If equal end the loop.
    str     xzr, [x1], #8        // Store xzr into the registers x1. 
    sub     w2, w2, #1           // Subtract w2 from w2 = 0 to compare here:
    cbnz    w2, 3b               // And go to 3d if w2 is not 0.

    // Go to the main function (and do not return in best case)
4:  bl      main
    // End the process on the master core if we every get back to here.
    b       1b
