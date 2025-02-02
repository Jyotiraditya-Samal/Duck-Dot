# Duck & Dot GBA Makefile
# Requires DevKitPro (https://devkitpro.org)

# Toolchain paths (auto-detected if DevKitPro is installed properly)
DEVKITARM = $(DEVKITPRO)/devkitARM
CC = $(DEVKITARM)/bin/arm-none-eabi-gcc
OBJCOPY = $(DEVKITARM)/bin/arm-none-eabi-objcopy

# Project name
TARGET = duck_dot

# Compiler flags
ARCH = -mthumb -mthumb-interwork
CFLAGS = $(ARCH) -Wall -O2 -fno-strict-aliasing -D_GBABS_
LDFLAGS = $(ARCH) -specs=gba.specs

# Source files
SOURCES = duck_dot.c

all: $(TARGET).gba

$(TARGET).elf: $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET).elf $(LDFLAGS)

$(TARGET).gba: $(TARGET).elf
	$(OBJCOPY) -v -O binary $(TARGET).elf $(TARGET).gba
	-@gbafix $(TARGET).gba

clean:
	@rm -fv *.elf *.gba *.o

.PHONY: clean all
