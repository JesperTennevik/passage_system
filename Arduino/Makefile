MCU     = atmega328p
F_CPU   = 16000000UL
TARGET  = app
BUILD_DIR = build

CC      = avr-gcc
OBJCOPY = avr-objcopy
SIZE    = avr-size
AVRDUDE = avrdude

PROGRAMMER = arduino
PORT       = COM3
BAUD       = 115200

CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -Wextra -std=c11 \
        -Idrivers/millis -Idrivers/spi -Idrivers/uart -Iconfig -Idrivers/uart_protocol \
		-Idrivers/mfrc522rfid -Idrivers/servo -Idrivers/shift_register \
		-Idrivers/rgb -Idrivers/keypad -Istate_machine -Istate_machine/states 

SRCS = \
	src/main.c \
	drivers/millis/millis.c \
	drivers/uart/uart.c \
	drivers/uart/ring_buffer.c \
	drivers/uart_protocol/uart_protocol.c \
	drivers/mfrc522rfid/mfrc522.c \
	drivers/servo/SG90.c \
	drivers/shift_register/74HC595.c \
	drivers/keypad/keypad.c \
	drivers/rgb/rgb.c \
	state_machine/state_machine.c \
	state_machine/states/state_input_await.c \
	state_machine/states/state_error_blink.c \
	state_machine/states/state_idle.c \
	state_machine/states/state_request_sid.c \
	state_machine/states/state_send_pin.c \
	state_machine/states/state_access_denied.c \
	state_machine/states/state_access_granted.c \
	drivers/spi/spi.c

OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)
ELF  = $(BUILD_DIR)/$(TARGET).elf
HEX  = $(BUILD_DIR)/$(TARGET).hex
MAP  = $(BUILD_DIR)/$(TARGET).map

all: $(HEX)

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	$(SIZE) $<

$(ELF): $(OBJS)
	$(CC) $(CFLAGS) -Wl,-Map=$(MAP) -o $@ $^

$(BUILD_DIR)/%.o: %.c
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	$(CC) $(CFLAGS) -c $< -o $@

flash: $(HEX)
	$(AVRDUDE) -c $(PROGRAMMER) -p m328p -P $(PORT) -b $(BAUD) -U flash:w:$(HEX):i

clean:
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)

.PHONY: all clean flash