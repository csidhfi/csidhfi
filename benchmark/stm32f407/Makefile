CC = gcc

PREFIX	?= arm-none-eabi
ARM_CC		= $(PREFIX)-gcc
ARM_LD		= $(PREFIX)-gcc
ARM_OBJCOPY	= $(PREFIX)-objcopy
OPENCM3_DIR = libopencm3
OPENCM3NAME = opencm3_stm32f4
OPENCM3FILE = $(OPENCM3_DIR)/lib/lib$(OPENCM3NAME).a

LDSCRIPT   = stm32f405x6.ld
LIBNAME    = opencm3_stm32f4
ARCH_FLAGS = -mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
DEFINES    = -DSTM32F4

CSIDH_DIR = csidh/
CSIDH_SRC = csidh.c constants.c fp.c mont.c uint.c 
SRCS  = hal-stm32f4.c speed.c randombytes.c
SRCS += $(addprefix $(CSIDH_DIR),$(CSIDH_SRC)) 



ARM_CFLAGS	+= -O3 \
		   -Wall -Wextra -Wimplicit-function-declaration \
		   -Wredundant-decls \
		   -Wundef -Wshadow \
		   -I$(OPENCM3_DIR)/include \
		   -fno-common $(ARCH_FLAGS) -MD $(DEFINES)
ARM_LDFLAGS	+= --static -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group \
		   -T$(LDSCRIPT) -nostartfiles -Wl,--gc-sections,--print-gc-sections \
		   $(ARCH_FLAGS) \
		   -L$(OPENCM3_DIR)/lib

all: speed_nocountermeasures.bin speed_countermeasures_notwistcm.bin speed_countermeasures_twistcm.bin

%.bin: %.elf
	$(ARM_OBJCOPY) -Obinary $(*).elf $(*).bin

speed_nocountermeasures.elf: $(SRCS) $(LDSCRIPT) $(OPENCM3FILE)
	$(ARM_CC) $(ARM_CFLAGS) -o $@ $(SRCS) $(ARM_LDFLAGS) -l$(LIBNAME) -I$(CSIDH_DIR) -I.

speed_countermeasures_notwistcm.elf: $(SRCS) $(LDSCRIPT) $(OPENCM3FILE)
	$(ARM_CC) -DCM $(ARM_CFLAGS) -o $@ $(SRCS) $(ARM_LDFLAGS) -l$(LIBNAME) -I$(CSIDH_DIR) -I.

speed_countermeasures_twistcm.elf: $(SRCS) $(LDSCRIPT) $(OPENCM3FILE)
	$(ARM_CC) -DCM -DTWIST_ATTACK_CM $(ARM_CFLAGS) -o $@ $(SRCS) $(ARM_LDFLAGS) -l$(LIBNAME) -I$(CSIDH_DIR) -I.

$(OPENCM3FILE):
	make -C $(OPENCM3_DIR)

clean:
	find . -name \*.o -type f -exec rm -f {} \;
	find . -name \*.d -type f -exec rm -f {} \;
	rm -f *.elf
	rm -f *.bin




