CC      := g++
CFLAGS  := -std=c++11 -Wall -Wextra -pedantic -O3 -march=native
LDFLAGS := -lraylib -lm

TARGET  := knob
SRC     := knob.cpp

# -------- RULES --------
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
