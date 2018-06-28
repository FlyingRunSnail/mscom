#include <stdio.h>
#include <errno.h>
#include "crc/crc.h"
#include "mscom_base/mscom.h"
#include "serial/serial.h"
#include "app/app.h"

#if 0
static void help(void)
{
    printf("usage: ./myserial /dev/ttyUSBX 0 1 127.0.0.1 1088\n");
}
#endif

int main(int argc, char *argv[])
{
    int fd = 0;
    int err;
    char *dev_name = NULL;
    
#if  0  
    int i = 0;
    UpdataPacket_t request;
    UpdataPacket_t reply;

    if (argc != 2)
    {
        help();
        return -1;
    }
#endif    

    dev_name = argv[1];
    fd = open_serial(dev_name, 115200, 8, 'N', 1);
    if (fd < 0)
    {
        printf("open or set param for %s failed.\n", dev_name);
        return -1;
    }

#if 0
    while(i < 2){
    // write and read from fd
    err = create_packet(&request, CPU_ECHO_CMD, NULL, 0);
    if (err != 0)
    {
        printf("create packet failed.\r\n");
        close_serial(fd);
        return -1;
    }

    clear_rcv_buf(fd);

    printf("send %d times.\n", i+1);
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
    printf("rcv %d times.\n", i+1);
    i++;
	
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
    }
#endif
    //err = get_version(fd);
    //err = clear_service_config(fd);
    //printf("ready to test cmd.\n");
    //err = modify_service_config(fd, argc - 2, &argv[2]);
    //err = test_cmd(fd);
#if 0
    err = get_port_stat(fd, argc - 2, &argv[2]);
    if (err != 0)
    {
        printf("modify service config failed.\n");
    }

    err = clear_port_stat(fd, argc - 2, &argv[2]);
    if (err != 0)
    {
        printf("modify service config failed.\n");
    }

    err = get_port_stat(fd, argc - 2, &argv[2]);
    if (err != 0)
    {
        printf("modify service config failed.\n");
    }
    printf("ready to get service config.\n");
    //err = get_service_config(fd);
    err = test_cmd(fd);
    if (err != 0)
    {
        printf("call get_version failed.\n");
    }
    err = set_network_param(fd, argc - 2, &argv[2]);
    if (err != 0)
    {
        printf("set network param failed.\n");
    }

    err = get_network_param(fd);
    if (err != 0)
    {
        printf("get network param failed.\n");
    }
#endif

    err = ping_cmd(fd, argc - 2, &argv[2]);
    if (err != 0)
    {
        printf("do ping failed.\n");
    }

    close_serial(fd);

    return 0;
}


