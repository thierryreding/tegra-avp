OBJCOPY = $(CROSS_COMPILE)objcopy
AS = $(CROSS_COMPILE)as
AFLAGS = -march=armv4t -mcpu=arm720t -g
LD = $(CROSS_COMPILE)ld

avp.bin: avp.elf
	$(OBJCOPY) -O binary $< $@

avp.elf: avp.o avp.ld
	$(LD) -T avp.ld $< -o $@

avp.o: avp.S
	$(AS) $(AFLAGS) -o $@ $<

clean:
	rm -f avp.o avp.elf avp.bin
