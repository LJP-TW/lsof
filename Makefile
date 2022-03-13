TARGET := hw1

HW1_C_FILES = $(wildcard *.c)
HW1_O_FILES = $(HW1_C_FILES:%.c=%.o)

all: $(TARGET)

$(TARGET): $(HW1_O_FILES)
	gcc $^ -o $@

%.o: %.c
	gcc -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o $(TARGET)