########################################################
#	PROJECT SETTINGS
########################################################
PROJECT_NAME 	= OS
OUTPUT_NAME 	= os-image
SRC_BASE 		= .
DEFINES 		= 

########################################################
#	DIRECTORIES
########################################################
BUILD_DIR = build/bin
TEMP_DIR = build/temp
KERNEL_SRC = src/Kernel
C_LIBS_SRC = src/Libc/src
INCLUDE_DIR = -I./src\
	-I./src/Libc/include

########################################################
#	SOURCE FILES
########################################################
C_SRCS = $(wildcard *.c)
C_SRCS += $(wildcard $(KERNEL_SRC)/*.c)
C_SRCS += $(wildcard $(KERNEL_SRC)/*/*.c)
C_SRCS += $(wildcard $(KERNEL_SRC)/*/*/*.c)
C_SRCS += $(wildcard $(KERNEL_SRC)/*/*/*/*.c)
C_SRCS += $(wildcard $(KERNEL_SRC)/*/*/*/*/*.c)
C_SRCS += $(wildcard $(KERNEL_SRC)/*/*/*/*/*/*.c)
C_SRCS += $(wildcard $(KERNEL_SRC)/*/*/*/*/*/*/*.c)
C_SRCS += $(wildcard $(KERNEL_SRC)/*/*/*/*/*/*/*/*.c)
C_SRCS += $(wildcard $(KERNEL_SRC)/*/*/*/*/*/*/*/*/*.c)
C_SRCS += $(wildcard $(KERNEL_SRC)/*/*/*/*/*/*/*/*/*/*.c)

C_SRCS += $(wildcard $(C_LIBS_SRC)/*.c)
C_SRCS += $(wildcard $(C_LIBS_SRC)/*/*.c)
C_SRCS += $(wildcard $(C_LIBS_SRC)/*/*/*.c)
C_SRCS += $(wildcard $(C_LIBS_SRC)/*/*/*/*.c)
C_SRCS += $(wildcard $(C_LIBS_SRC)/*/*/*/*/*.c)
C_SRCS += $(wildcard $(C_LIBS_SRC)/*/*/*/*/*/*.c)
C_SRCS += $(wildcard $(C_LIBS_SRC)/*/*/*/*/*/*/*.c)
C_SRCS += $(wildcard $(C_LIBS_SRC)/*/*/*/*/*/*/*/*.c)
C_SRCS += $(wildcard $(C_LIBS_SRC)/*/*/*/*/*/*/*/*/*.c)
C_SRCS += $(wildcard $(C_LIBS_SRC)/*/*/*/*/*/*/*/*/*/*.c)

COBJECTS64		:= $(patsubst %.c, $(TEMP_DIR)/obj64/%.o, $(C_SRCS))
ALL_OBJECTS64	:= $(sort $(COBJECTS64))

########################################################
#	COMPILER
########################################################
CC= ccache gcc

########################################################
#	COMPILER OPTIONS
########################################################
OPTIMIZATION = 

########################################################
#	COMPILER FLAGS
########################################################
CFLAGS = $(DEFINES) $(INCLUDE_DIR) -ffreestanding

########################################################
#	LINKER
########################################################
LD= ld

########################################################
#	LINKER OPTIONS
########################################################
LD_OPTIMIZATION = -flto

########################################################
#	GENERATE OBJECT FILES
########################################################
OBJECTS = $(C_SRCS:.c=.o)

bootloader:
	mkdir -p $(BUILD_DIR)
	nasm -fbin src/Bootloader/start.asm -o build/bin/Bootloader

kernel: $(OBJECTS)
	mkdir -p $(BUILD_DIR)
	nasm -f elf64 src/Bootloader/KernelEntry/kernel_entry.asm -o build/temp/kernel_entry.o
	ld -o $(BUILD_DIR)/kernel.bin -T LinkerScript/Kernel.ld build/temp/kernel_entry.o $(ALL_OBJECTS64) -flto --oformat binary

os:
	mkdir -p build/os
	cat build/bin/Bootloader $(BUILD_DIR)/kernel.bin > build/os/os-image

run:
	qemu-system-x86_64 build/os/os-image

iso:
	truncate build/exec/Bootloader -s 1200k
	mkisofs -o build/iso/OS -b build/exec/Bootloader ./build/exec/

########################################################
#	GENERAL COMPILATION RULES
########################################################
.c.o :
	mkdir -p $(TEMP_DIR)/obj64/$(dir $<)
	$(CC) $(CFLAGS) $(OPTIMIZATION) -c $< -o $(TEMP_DIR)/obj64/$(<:.c=.o)

########################################################
#	CLEAN
########################################################
clean:
	$(REMOVE) build/