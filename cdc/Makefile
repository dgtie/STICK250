PROG = ../prog/prog
BIN = $(HOME)/.platformio/packages/toolchain-microchippic32/bin
GPP = $(BIN)/pic32-g++
BIN2HEX = $(BIN)/pic32-bin2hex
SIZE = $(BIN)/pic32-size

SRC = $(shell ls *.cpp)
OBJ = $(SRC:.cpp=.o)
TARGET = out

CFLAGS = -mprocessor=32MX250F128D -Iinclude
LDFLAGS = -mprocessor=32MX250F128D -Wl,-Map=$(TARGET).map -T app.ld

all: $(TARGET).hex

$(TARGET).hex: $(TARGET).elf
	$(BIN2HEX) $<

$(TARGET).elf: $(OBJ)
	$(GPP) $(LDFLAGS) -o $@ $^
	$(SIZE) $(TARGET).elf

%.o: %.cpp
	$(GPP) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf *.o $(TARGET).*

flash:
	$(PROG) $(TARGET).hex

