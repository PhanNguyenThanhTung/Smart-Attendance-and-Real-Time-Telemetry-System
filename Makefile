vpath %.c Src
vpath %.h Inc

CC = arm-none-eabi-gcc
CP = arm-none-eabi-objcopy
SZ = arm-none-eabi-size

TARGET = main
SRCS = main.c MFRC522.c startup.c
OBJS = main.o MFRC522.o startup.o
LDSCRIPT = stm32f103c8t6.ld

MCU = -mcpu=cortex-m3 -mthumb
CFLAGS = $(MCU) -Wall -g -O0 -IInc
LDFLAGS = $(MCU) -T$(LDSCRIPT) -nostdlib -Wl,-Map=$(TARGET).map


.PHONY: all clean

all: $(TARGET).bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


$(TARGET).elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	$(SZ) $@

$(TARGET).bin: $(TARGET).elf 
	$(CP) -O binary $< $@

clean:
	del /f /q *.o $(TARGET).elf $(TARGET).bin $(TARGET).map