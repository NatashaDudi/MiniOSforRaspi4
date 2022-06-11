SECTIONS
{
    /* Code by https://github.com/isometimes/rpi4-osdev */
    /* Explanation regarding linker files from jsandler: https://jsandler18.github.io/explanations/linker_ld.html and from https://sourceware.org/binutils/docs/ld/Output-Section-Discarding.html */
    /* Comments by Natasha */

    . = 0x80000;     /* Instructions start at this address which is where our boot-code is.*/
    .text : { KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*) } 
    /* ".text" is for executable code */
    /* the 'KEEP' method is used for the linker to know that this section has to be kept even in a case where nothing references it. */
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
    /* ".rodata" is for global constants and stands for "read only data". */
    PROVIDE(_data = .);
    .data : { *(.data .data.* .gnu.linkonce.d*) } 
    /* ".data" is for global variables that will be loaded at compile time. */
    .bss (NOLOAD) : {
    /* ".bss" is for uninitialised global variables. */
        . = ALIGN(16); /* Align the current address to a page size of 16 */
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    _end = .;

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
   /* "/DISCARD/" stands for sections that will not be used for the output file. */
}
__bss_size = (__bss_end - __bss_start)>>3;
