bootloader.bin: bootloader.elf
	$(CROSS_COMPILE)objcopy -O binary $< $@

bootloader.elf: start.o
	$(CROSS_COMPILE)ld -nostdlib -T bootloader.lds $< -o $@

%.o: %.S
	$(CROSS_COMPILE)gcc -o $@ -c $<
