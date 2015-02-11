OBJCOPY = $(CROSS_COMPILE)objcopy
AS = $(CROSS_COMPILE)as
AFLAGS = -marm -march=armv4t -mcpu=arm720t -g
CC = $(CROSS_COMPILE)gcc
NOSTDINC_FLAGS = -nostdinc -isystem $(shell $(CC) -print-file-name=include)
CFLAGS = $(NOSTDINC_FLAGS) -DCONFIG_AEABI -I include -marm -march=armv4t -mcpu=arm720t -O0 -ggdb -Wall -Werror
LD = $(CROSS_COMPILE)ld
LDFLAGS = -nostdlib

export AS AFLAGS CC CFLAGS LD LDFLAGS

obj-y += arch/
obj-y += drivers/
obj-y += lib/
obj-y += avp.o

subdirs = $(filter %/,$(obj-y))
objs += $(addsuffix built-in.o,$(subdirs))
objs += $(filter-out %/,$(obj-y))

avp.bin: avp.elf
	@echo "  OBJCOPY $@"
	@$(OBJCOPY) -O binary $< $@

avp.elf: $(objs) avp.ld
	@echo "  LD      $@"
	@$(LD) $(LDFLAGS) -T avp.ld $(objs) -o $@

%.o: %.c
	@echo "  CC      $@"
	@$(CC) $(CFLAGS) -o $@ -c $<

MAKEFLAGS += --no-print-directory -r -R

%/built-in.o: FORCE
	@$(MAKE) -f scripts/Makefile.build obj=$*/

$(addprefix clean-,$(subdirs)): clean-%:
	@$(MAKE) -f scripts/Makefile.build obj=$* clean

clean: $(addprefix clean-,$(subdirs))
	rm -f $(OBJS) avp.elf avp.bin

FORCE: ;
.PHONY: FORCE
