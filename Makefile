CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -std=c99
TARGET  = mysh
SRCS    = main.c parse.c execute.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
