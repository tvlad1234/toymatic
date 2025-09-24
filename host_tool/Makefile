CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = toymatic_tool
SRCS = main.c utils.c crc16.c modbus.c plc_commands.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install : $(TARGET)
	cp $(TARGET) /usr/bin/

clean:
	rm -f $(TARGET) $(OBJS)
