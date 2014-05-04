#CPU = msp430f2013
CPU = msp430g2553

SRCS := $(wildcard *.c) $(wildcard *.cpp) \
	#../spi/spi.c \
	#../usci_uart/usci_uart.c \


CFLAGS = -std=c++0x -Os -g -I. \
	 -fdata-sections -ffunction-sections \
	 -Wl,--gc-sections

prog.elf: $(SRCS) Makefile
	msp430-gcc -o $@ -mmcu=$(CPU) $(CFLAGS) $(SRCS)
	msp430-objdump -S $@ >prog.lst
	msp430-size --total $@

clean:
	rm -f prog.elf prog.lst

flash: prog.elf
	mspdebug rf2500 "prog prog.elf"

all: prog.elf

