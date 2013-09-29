OBJCOPY = $(CROSS_COMPILE)objcopy
AS = $(CROSS_COMPILE)as
AFLAGS = -march=armv4t -mcpu=arm720t -g
CC = $(CROSS_COMPILE)gcc
CFLAGS = -nostdinc -I include -march=armv4t -mcpu=arm720t -O2 -g
LD = $(CROSS_COMPILE)ld
LDFLAGS = -nostdlib

OBJS = start.o drivers/uart.o avp.o

avp.bin: avp.elf
	$(OBJCOPY) -O binary $< $@

avp.elf: $(OBJS) avp.ld
	$(LD) $(LDFLAGS) -T avp.ld $(OBJS) -o $@

%.o: %.S
	$(AS) $(AFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJS) avp.elf avp.bin
