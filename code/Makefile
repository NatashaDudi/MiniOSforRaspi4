# Code from: https://github.com/isometimes/rpi4-osdev
# Explanations for flags from: https://gcc.gnu.org/onlinedocs/gcc and https://stackoverflow.com/questions/17692428/what-is-ffreestanding-option-in-gcc
# Comments by Natasha
CFILES = $(wildcard *.c) # This is used for all files that end with ".c" meaning that they are files written in the C language.
OFILES = $(CFILES:.c=.o) # Changing all the files written in the C language to a binary form (ending with ".o").
LLVMPATH = /usr/local/opt/llvm/bin/
# path from isomtimes (https://github.com/isometimes/rpi4-osdev) "/opt/homebrew/opt/llvm/bin" that was changed to "/usr/local/opt/llvm/bin/" to give the correct path way to the compiler for mac users.
CLANGFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a72+nosimd
# "-Wall" turns on warning flags. In combination with "-2", "-Warray-bounds=1" is turned on.
# "-ffreestanding" is to tell the compiler that standard functions might not have their normal definition. (This is probably used because we are working on a kernel.)
# "-nostdinc" to not look for ".h" files in the directories of the standard system but only in the current folder.
# "-nostdlib" same as with "-nostdinc" but with no startup files or libraries instead of ".h" files.
# "-mcpu=cortex-a72+nosimd" is to specify the processor used with "AArch64" which is "cortex-a72" in this case + no "simd". SIMD are advanced instructions that are turned off.

all: clean kernel8.img
# Typing in "make all" will be the same as using "clean", than executing the following lines:

boot.o: boot.S
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c boot.S -o boot.o
# Creating "boot.o" with the compiler and boot.S

%.o: %.c
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c $< -o $@
# Creating all ".o" files by compiling all files with ".c". Here "$<" stands for the ".c" filename whereas "$@" stands for the ".o" file name.

kernel8.img: boot.o $(OFILES)
	$(LLVMPATH)/ld.lld -m aarch64elf -nostdlib boot.o $(OFILES) -T link.ld -o kernel8.elf
	$(LLVMPATH)/llvm-objcopy -O binary kernel8.elf kernel8.img
# Get all recently created ".o" files, "boot.o" and the linker file to create "kernel8.elf".
# Then use the "kernel8.elf" to create the kernel image "kernel8.img".

clean:
# "clean" stands for deleting everything that this make file does,
# namely the kernel8.elf, and all files that end with ".o" or ".img"
	/bin/rm kernel8.elf *.o *.img > /dev/null 2> /dev/null || true
