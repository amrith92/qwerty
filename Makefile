CC		= gcc
CFLAGS	= -Wall -c -g

OUT		= bin/qwerty

SOURCE	= source/
BUILD	= obj/

OBJECTS := $(patsubst $(SOURCE)%.c,$(BUILD)%.o,$(wildcard $(SOURCE)*.c))

all: $(OBJECTS) $(OUT)

binary: remout all

$(OUT):
	gcc $(OBJECTS) -o $(OUT)

$(BUILD)%.o: $(SOURCE)%.c
	$(CC) $(CFLAGS) -I $(SOURCE) $< -o $@

remout:
	-rm $(OUT)

clean:
	-rm -f $(BUILD)*.o
	-rm $(OUT)
