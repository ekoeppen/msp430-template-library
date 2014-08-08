CPU ?= msp430g2553

HEADERS := $(wildcard *.h) $(wildcard ../../*.h)
SRCS := $(wildcard *.c) $(wildcard *.cpp) $(wildcard ../../*.c)

CFLAGS = -std=c++0x -Os -g -I../.. \
	 -fdata-sections -ffunction-sections \
	 -Wl,--gc-sections

prog.elf: $(SRCS) $(HEADERS) Makefile
	msp430-gcc -o $@ -mmcu=$(CPU) $(CFLAGS) $(SRCS)
	msp430-objdump -S $@ >prog.lst
	msp430-size --total $@

clean:
	rm -f prog.elf prog.lst

flash: prog.elf
	mspdebug rf2500 "prog prog.elf"

all: prog.elf

