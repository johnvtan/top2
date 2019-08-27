TARGET := top2
CC := gcc
CFLAGS := -Wall -Wextra
SRCS := src/$(TARGET).c

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -rf top2
