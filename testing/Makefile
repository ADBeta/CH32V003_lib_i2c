### Project Specific Variables ################################################
TOOLKIT_DIR  := ../toolkit
EXTRALIB_DIR := $(TOOLKIT_DIR)/extralibs

# Project name
TARGET := i2c-test

LIB_SRC := ../lib_i2c.c

# Change this to specify your MCU model, for compilation
TARGET_MCU := CH32V003

### System Variables ##########################################################
# Cross-compiler prefix
PREFIX := riscv64-unknown-elf

# Set the minichlink executable
# MINICHLINK ?= minichlink      # Preinstalled system Exec
# MINICHLINK ?= minichlink.exe  # Windows Exec
MINICHLINK := $(TOOLKIT_DIR)/minichlink

MCU_C := $(TOOLKIT_DIR)/ch32v003fun.c

GENERATED_LD_FILE := ./generated_ch32v003.ld
#TARGET_MCU_LD := 0
LDFLAGS := -L$(TOOLKIT_DIR) -lgcc -T $(GENERATED_LD_FILE) -Wl,--gc-sections

# All .c files to compile, just adds ch32v003_fun, target and extra files
FILES_TO_COMPILE := $(MCU_C) $(TARGET).c $(LIB_SRC)

# Specify the MCU LD, for the CH32V003
TARGET_MCU_LD:=0

# Architecutre Compile Flags. Change these if using a different Chip 
CFLAGS_ARCH += -march=rv32ec -mabi=ilp32e -DCH32V003=1

# Compiler flags, warnings, dirs etc
CFLAGS := \
-g -Os -flto -ffunction-sections -fdata-sections -fmessage-length=0 -msmall-data-limit=8 \
$(CFLAGS_ARCH) -static-libgcc \
-I/usr/riscv64-unknown-elf/include/ \
-I$(EXTRALIB_DIR) \
-I$(TOOLKIT_DIR) \
-I../ \
-I./  \
-nostdlib \
-Wall $(EXTRA_CFLAGS)

### Makefile dependencies #####################################################
.PHONY: all build flash monitor unbrick clean
all: build

# In order to 'build', work through until .bin exists
build: $(TARGET).bin

# Create the LD file needed - requires the build folder
$(GENERATED_LD_FILE):
	$(PREFIX)-gcc -E -P -x c -DTARGET_MCU=$(TARGET_MCU) -DMCU_PACKAGE=$(MCU_PACKAGE) -DTARGET_MCU_LD=$(TARGET_MCU_LD) $(TOOLKIT_DIR)/ch32v003fun.ld > $(GENERATED_LD_FILE)
	
# Compile the .elf file - requires the compiled ld file, .c files and other depends
$(TARGET).elf: $(FILES_TO_COMPILE) $(GENERATED_LD_FILE) $(EXTRA_ELF_DEPENDENCIES)
	$(PREFIX)-gcc -o $@ $(FILES_TO_COMPILE) $(CFLAGS) $(LDFLAGS)
	
# Create the binary file and hex from the .elf file
$(TARGET).bin: $(TARGET).elf
	$(PREFIX)-size $(TARGET).elf
	$(PREFIX)-objdump -S $^ > $(TARGET).lst
	$(PREFIX)-objdump -t $^ > $(TARGET).map
	$(PREFIX)-objcopy -O binary $< $(TARGET).bin
	$(PREFIX)-objcopy -O ihex $< $(TARGET).hex

terminal: monitor

gdbserver : 
	-$(MINICHLINK)/minichlink -baG

clangd :
	make clean
	bear -- make build
	@echo "CompileFlags:" > .clangd
	@echo "  Remove: [-march=*, -mabi=*]" >> .clangd

clangd_clean :
	rm -f compile_commands.json .clangd
	rm -rf .cache

closechlink:
	-killall minichlink

monitor:
	$(MINICHLINK) -T

unbrick:
	$(MINICHLINK) -u

flash: $(TARGET).bin
	$(MINICHLINK) -w $< flash -b

clean:
	rm $(filter-out $(TARGET).c, $(wildcard $(TARGET).*)) $(GENERATED_LD_FILE)
