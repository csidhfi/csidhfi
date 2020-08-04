
PLATFORM=CWLITEARM
CRYPTO_TARGET = NONE
FUNC_SEL = BENCHMARK
OPT = 3
TARGET = speed
ifndef TARGET
TARGET = speed
endif

CFLAGS += -D$(FUNC_SEL)
CFLAGS += --specs=nosys.specs

ifneq ($(TYPE),)
  CFLAGS += -D$(TYPE)
endif

ifneq ($(TWIST_ATTACK_CM),)
  CFLAGS += -DTWIST_ATTACK_CM 
endif


CSIDH_DIR = ./
CSIDH_SRC = csidh.c constants.c fp.c mont.c uint.c
SRC += speed.c notrandombytes.c systick-standalone.c
SRC += $(addprefix $(CSIDH_DIR),$(CSIDH_SRC))

FIRMWAREPATH = .
include $(FIRMWAREPATH)/Makefile.inc

clean:
	rm -rf *.eep *.elf *.hex *.lss *.map *.sym

