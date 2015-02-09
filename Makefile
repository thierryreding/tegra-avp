OBJCOPY = $(CROSS_COMPILE)objcopy
AS = $(CROSS_COMPILE)as
AFLAGS = -marm -march=armv4t -mcpu=arm720t -g
CC = $(CROSS_COMPILE)gcc
NOSTDINC_FLAGS = -nostdinc -isystem $(shell $(CC) -print-file-name=include)
CFLAGS = $(NOSTDINC_FLAGS) -DCONFIG_AEABI -I include -marm -march=armv4t -mcpu=arm720t -O2 -g
LD = $(CROSS_COMPILE)ld
LDFLAGS = -nostdlib

OBJS += arch/arm/start.o arch/arm/lib1funcs.o
OBJS += arch/arm/traps.o arch/arm/unwind.o
OBJS += drivers/uart.o avp.o
OBJS += lib/string.o lib/vsnprintf.o

avp.bin: avp.elf
	$(OBJCOPY) -O binary $< $@

avp.elf: $(OBJS) avp.ld
	$(LD) $(LDFLAGS) -T avp.ld $(OBJS) -o $@

%.o: %.S
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJS) avp.elf avp.bin
