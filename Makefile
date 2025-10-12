# Default path
BOOT_PATH ?= boot.elf

EE_BIN = proverb_unc.elf
EE_BIN_PKD = proverb.elf
BOOT_BIN = boot.bin

EE_INCS = -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include -I$(PS2SDK)/sbv/include -Iinclude -I$(PS2SDK)/ports/include
EE_LIBS = -liopreboot -lpatches
EE_OBJS = main.o
ELF_FILES += loader.elf

# C compiler flags
EE_CFLAGS = -D_EE -Os -G0 -Wall -Werror -fdata-sections -ffunction-sections $(EE_INCS)
EE_LDFLAGS += -Wl,-zmax-page-size=128 -Wl,--gc-sections -s

IOPRP_BIN = ioprp.img
IOPRP_CONTENTS = fileio.irx
vpath %.irx $(PS2SDK)/iop/irx/

ifneq ($(BOOT_PATH),LOADCONF)
 EE_CFLAGS += -DBOOT_PATH=\"$(BOOT_PATH)\"
endif

# Reduce binary size by using newlib-nano
EE_NEWLIB_NANO = 1
NEWLIB_NANO = 1

EE_OBJS_DIR = obj/
EE_ASM_DIR = asm/
EE_SRC_DIR = src/

EE_OBJS += $(IRX_FILES:.irx=_irx.o)
EE_OBJS += $(IOPRP_BIN:.img=_img.o)
EE_OBJS += $(ELF_FILES:.elf=_elf.o)
EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

.PHONY: all clean

all: $(EE_BIN_PKD)

$(EE_BIN_PKD): $(EE_BIN)
	ps2-packer $(EE_BIN) $(EE_BIN_PKD)

clean:
	$(MAKE) -C loader clean
	rm -rf $(EE_OBJS_DIR) $(EE_BIN) $(EE_BIN_RAW) $(OUT_DIR)

BIN2C = $(PS2SDK)/bin/bin2c

# ELF loader
loader.elf:
	$(MAKE) -C loader/$<

%loader_elf.c: loader.elf
	$(BIN2C) $(*:$(EE_SRC_DIR)%=loader/%)loader.elf $@ $(*:$(EE_SRC_DIR)%=%)loader_elf

$(EE_OBJS_DIR)ioprp_img.c: $(IOPRP_BIN)
	$(BIN2C) $< $@ ioprp_img

$(EE_ASM_DIR):
	@mkdir -p $@

$(EE_OBJS_DIR):
	@mkdir -p $@

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.c | $(EE_OBJS_DIR)
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)%.o : $(EE_SRC_DIR)%.S
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_BIN_RAW): $(EE_BIN)
	$(EE_OBJCOPY) -O binary -v $< $@

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
