MODULE = riscv_common_periph

# QingKe cores (PFIC) have no CLINT, their core timer is the SysTick
ifneq (,$(filter periph_pfic,$(USEMODULE)))
  SUBMODULES_NO_SRC += coretimer.c
  ifneq (,$(filter periph_coretimer,$(USEMODULE)))
    SRC += coretimer_qingke.c
  endif
endif

include $(RIOTMAKE)/periph.mk
