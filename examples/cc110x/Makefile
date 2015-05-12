TARGETS = sender.elf receiver.elf tester.elf swap_modem.elf

CPU ?= msp430g2553

all: $(TARGETS)

COMMON_SRCS = ../../utils.c
CFLAGS = -mmcu=$(CPU) -std=c++0x -Os -g -I. -I../common/ -I../.. -I../../drivers  \
	 -fdata-sections -ffunction-sections \
	 -Wl,--gc-sections

%.elf : %.cpp
	msp430-gcc -o $@ $(CFLAGS) $($*_FLAGS) $(COMMON_SRCS) $<
	msp430-objdump -S $@ > $*.lst
	msp430-objcopy --only-section .infod $@ $*_settings.elf
	msp430-objcopy --remove-section .infod $@
	msp430-size --total $@

clean:
	rm -f *.elf *.lst

.PHONY: clean all
