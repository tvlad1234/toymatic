#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "modbus.h"
#include "utils.h"
#include "plc_commands.h"

// Modbus context
struct modbus_ctx ctx;

// RS485 reader thread
void *reader_thread(void *arg)
{
    uint8_t b;
    while (1)
    {
        int n = rs485_read_byte(&b);
        if (n == 1)
            modbus_receive_byte(&ctx, b, micros());
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "usage: %s [serial port] [commands]\n", argv[0]);
        return -1;
    }

    utils_init();

    if (rs485_open(argv[1], B115200) < 0)
    {
        fprintf(stderr, "Failed to open serial port %s\n", argv[1]);
        return 1;
    }
    printf("Opened %s\n", argv[1]);

    // Init Modbus context
    ctx.state = MB_IDLE;
    ctx.adu_len = 0;
    ctx.byte_period = 87;

    // Create RS485 receiver thread
    pthread_t th;
    pthread_create(&th, NULL, reader_thread, NULL);

    for (int i = 2; i < argc; i++)
    {
        if (argv[i][0] == '-' && argv[i][1])
        {
            int ret;
            char c = argv[i][1];

            if (c == 'f') // get fault code
            {
                ret = get_plc_status(&ctx);
                if (ret != PLC_COMMS_TIMEOUT)
                    printf("PLC status: %s\n", PLC_err_msg(ret));
            }

            else if (c == 'r') // run
                ret = PLC_run(&ctx);

            else if (c == 's') // stop
                ret = PLC_stop(&ctx);

            else if (c == 'c') // clear fault code
                ret = PLC_clear_fault(&ctx);

            else if (c == 'e') // erase program
                ret = PLC_erase(&ctx);

            else if (c == 'p') // upload program
            {
                i++;
                if (i < argc)
                {
                    char *filename = argv[i];
                    printf("Uploading program %s\n", filename);
                    ret = PLC_upload(&ctx, filename);
                    if (ret == PLC_OK)
                        printf("Upload OK!\n");
                    else if (ret == PLC_ERR_BIN_HEADER)
                        printf("Error uploading file\n");
                }
                else
                {
                    printf("Missing filename!\n");
                    break;
                }
            }

            else
            {
                printf("Unknown command -%c\n", c);
                break;
            }

            if (ret == PLC_COMMS_TIMEOUT)
            {
                printf("PLC query timeout!\n");
                rs485_close();
                return 1;
            }
        }
        else
        {
            printf("Invalid syntax\n");
            break;
        }

        // usleep(250000); // small sleep
    }

    rs485_close();
    return 0;
}
