# ====================================================
# GBA Toolchain Paths (Edit these based on your OS!)
# ====================================================
DEVKITPRO = /opt/devkitpro
DEVKITARM = $(DEVKITPRO)/devkitARM

# ====================================================
# Compiler and Flags
# ====================================================
CC = $(DEVKITARM)/bin/arm-none-eabi-gcc
CFLAGS = -Wall -Wextra -O2 -mthumb -mthumb-interwork -mlong-calls
LDFLAGS = -specs=gba.specs

# ====================================================
# Build Targets
# ====================================================
TARGET = duck_dot
OBJS = duck_dot.o

# ====================================================
# Rules
# ====================================================
all: $(TARGET).gba

# Compile .c to .o
$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link .o files to .elf
$(TARGET).elf: $(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@

# Convert .elf to .gba
$(TARGET).gba: $(TARGET).elf
	$(DEVKITARM)/bin/arm-none-eabi-objcopy -O binary $< $@
	$(DEVKITPRO)/tools/bin/gbafix $@

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).gba
