#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "utils.h"

static int serial_fd = -1;

static struct timespec start_time;

void utils_init(void)
{
    clock_gettime(CLOCK_MONOTONIC, &start_time);
}

uint32_t micros(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    uint64_t now_us = (uint64_t)ts.tv_sec * 1000000ULL + ts.tv_nsec / 1000ULL;
    uint64_t start_us = (uint64_t)start_time.tv_sec * 1000000ULL + start_time.tv_nsec / 1000ULL;

    return (uint32_t)(now_us - start_us);
}

int rs485_open(const char *dev, speed_t baudrate)
{
    struct termios tty;

    serial_fd = open(dev, O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_fd < 0)
    {
        perror("open");
        return -1;
    }

    if (tcgetattr(serial_fd, &tty) != 0)
    {
        perror("tcgetattr");
        return -1;
    }

    cfsetospeed(&tty, baudrate);
    cfsetispeed(&tty, baudrate);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 1; // block until 1 byte
    tty.c_cc[VTIME] = 0;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    cfmakeraw(&tty);
    tty.c_cflag |= CLOCAL | CREAD;

    if (tcsetattr(serial_fd, TCSANOW, &tty) != 0)
    {
        perror("tcsetattr");
        return -1;
    }
    tcflush(serial_fd, TCIOFLUSH);

    return 0;
}

void rs485_close(void)
{
    close(serial_fd);
}

void rs485_write(uint8_t *buf, int len)
{
    if (serial_fd >= 0)
        write(serial_fd, buf, len);
}

int rs485_read_byte(uint8_t *b)
{
    if (serial_fd < 0)
        return -1;
    return read(serial_fd, b, 1); // blocks until 1 byte
}
