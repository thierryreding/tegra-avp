MAKEFLAGS += --no-print-directory -r -R

ifeq ("$(origin V)", "command line")
  KBUILD_VERBOSE = $(V)
endif

ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

ifeq ($(KBUILD_VERBOSE),1)
  quiet =
  Q =
else
  quiet = quiet_
  Q = @
endif

export KBUILD_VERBOSE quiet Q

# we do not support out-of-tree builds yet
srctree := .
objtree := .
src := $(srctree)
obj := $(objtree)

export srctree objtree

PHONY := _all
_all:

# We need some generic definitions (do not try to remake the file).
include $(srctree)/scripts/Kbuild.include
$(srctree)/scripts/Kbuild.include: ;

-include $(objtree)/config.mk

quiet_cmd_configure = GEN     $@
      cmd_configure = $(srctree)/configure

$(objtree)/config.mk $(objtree)/config.h: $(srctree)/configure
	$(call cmd,configure)

OBJCOPY = $(CROSS_COMPILE)objcopy
AS = $(CROSS_COMPILE)as
AFLAGS = -marm -march=armv4t -mcpu=arm720t -g
CC = $(CROSS_COMPILE)gcc
CPP = $(CC) -E
NOSTDINC_FLAGS = -nostdinc -isystem $(shell $(CC) -print-file-name=include)
CPPFLAGS = $(NOSTDINC_FLAGS) -include config.h
CFLAGS = $(CPPFLAGS) -I include -marm -march=armv4t -mcpu=arm720t
LD = $(CROSS_COMPILE)ld
LDFLAGS = -nostdlib

ifneq ($(CONFIG_DEBUG),)
  CFLAGS += -O0 -ggdb
else
  CFLAGS += -O2 -g
endif

CFLAGS += -Wall -Werror

export AS AFLAGS CC CFLAGS LD LDFLAGS

obj-y += arch/
obj-y += core/
obj-y += drivers/
obj-y += lib/
obj-y += init/

subdirs = $(patsubst %/,%,$(filter %/,$(obj-y)))
objs = $(patsubst %,%/built-in.o,$(subdirs))
targets :=

PHONY += all
_all: all

quiet_cmd_cpp_lds_S = LDS     $@
      cmd_cpp_lds_S = $(CPP) $(CPPFLAGS) -P -C -MD -MF $@.d -MT $@ -o $@ $<

%.lds: %.lds.S
	$(call cmd,cpp_lds_S)

targets += avp.lds
deps += avp.lds.d

quiet_cmd_link_avp = LD      $@
      cmd_link_avp = $(LD) $(LDFLAGS) -T avp.lds $(objs) -o $@

avp.elf: $(objs) avp.lds
	$(call cmd,link_avp)

targets += avp.elf

quiet_cmd_objcopy = OBJCOPY $@
      cmd_objcopy = $(OBJCOPY) -O binary $< $@

avp.bin: avp.elf
	$(call cmd,objcopy)

targets += avp.bin

all: $(targets)

%/built-in.o: FORCE
	$(Q)$(MAKE) $(build)=$*

subdirs = $(patsubst %/,%,$(filter %/, $(obj-y)))
clean-dirs := $(addprefix _clean_, $(subdirs))

$(clean-dirs): _clean_%:
	$(Q)$(MAKE) $(clean)=$*

quiet_cmd_rmfiles = CLEAN $(rm-files)
      cmd_rmfiles = rm -f $(rm-files)

clean: rm-files = $(strip $(targets))
clean: $(clean-dirs)
	$(call cmd,rmfiles)

ifneq ($(deps),)
  -include $(deps)
endif

PHONY += FORCE
FORCE:

.PHONY: $(PHONY)
