#include <stdio.h>
#include <errno.h>
#include "crc/crc.h"
#include "mscom_base/mscom.h"
#include "serial/serial.h"

static void help(void)
{
    printf("usage: ./myserial /dev/ttyUSBX\n");
}

int main(int argc, char *argv[])
{
    int fd = 0;
    int err;
    char *dev_name = NULL;
    UpdataPacket_t request;
    UpdataPacket_t reply;
    
    if (argc != 2)
    {
        help();
        return -1;
    }

    dev_name = argv[1];
    fd = open_serial(dev_name, 115200, 8, 'N', 1);
    if (fd < 0)
    {
        printf("open or set param for %s failed.\n", dev_name);
        return -1;
    }

    // write and read from fd
    err = create_packet(&request, CPU_ECHO_CMD, NULL, 0);
    if (err != 0)
    {
        printf("create packet failed.\r\n");
        close_serial(fd);
        return -1;
    }

    clear_rcv_buf(fd);
    err = send_packet(fd, &request);
    if (err != 0)
    {
        printf("send packet to mcu failed.\n");
        close_serial(fd);
        return -1;
    }

    err = rcv_packet(fd, &reply);
    if (err != 0)
    {
	printf("rcv packet failed.\n");
	close_serial(fd);
	return -1;
    }
	
    if (reply.header.type == CPU_ECHO_CMD)
    {
	if (reply.header.ErrCode == PACKET_SUCCESS)
	{
            dump_packet(&reply);
	    printf("rcv success.\n");
	}
	else
	{
	    printf("rcv errcode: %d is not the expected: %d.\n", reply.header.ErrCode, PACKET_SUCCESS);
	}
    }
    else
    {
        printf("rcv type: %d is not the expected: %d.\n", reply.header.type, MCU_REPLY_CMD);
    }

    close_serial(fd);

    return 0;
}


