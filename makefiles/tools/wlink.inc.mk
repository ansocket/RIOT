# wlink: flash tool for WCH-Link probes and WCH RISC-V MCUs
# https://github.com/ch32-rs/wlink

FLASHER ?= wlink
RESET ?= wlink

# The firmware is linked to the boot alias at address 0, so flash the plain
# binary to the physical code flash address instead of the ELF file
FLASHFILE ?= $(BINFILE)

# Physical start address of the code flash
WLINK_FLASH_BASE ?= 0x08000000
WLINK_FLASH_ADDR ?= $(shell printf "0x%08x" $$(($(WLINK_FLASH_BASE) + $(or $(IMAGE_OFFSET),0))))

# Chip family (e.g. CH32V30X), detected by wlink if empty
WLINK_CHIP ?=

WLINK_FLAGS ?= $(if $(WLINK_CHIP),--chip $(WLINK_CHIP))

FFLAGS ?= $(WLINK_FLAGS) flash --address $(WLINK_FLASH_ADDR) $(FLASHFILE)
RESET_FLAGS ?= $(WLINK_FLAGS) reset
