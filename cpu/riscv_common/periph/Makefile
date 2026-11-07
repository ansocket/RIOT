MODULE = riscv_common_periph

# QingKe cores (PFIC) have no CLINT and use a CPU-specific timer driver.
ifneq (,$(filter periph_pfic,$(USEMODULE)))
  SUBMODULES_NO_SRC += coretimer.c
endif

include $(RIOTMAKE)/periph.mk
