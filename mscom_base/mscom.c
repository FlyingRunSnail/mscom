#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "mscom.h"
#include "../crc/crc.h"

#define RCV_BUF_LEN (1<<10)
char rcv_buf[RCV_BUF_LEN] = {0};

int create_packet(UpdataPacket_t *packet, int type, char *data, int size)
{
    memset(packet, 0, sizeof(UpdataPacketHeader_t));

    packet->header.magic = MAGIC;
    packet->header.type = type;
    packet->header.size = size;
    if (data != NULL)
    {
        packet->header.dcrc = crc32(data, size);
    }

    packet->header.hcrc = crc32((char *)packet, sizeof(UpdataPacketHeader_t) - sizeof(int));
    packet->data = data;
    
    return 0;
}

int check_packet(UpdataPacket_t *packet)
{
    int hcrc = 0, dcrc = 0;

    if (packet->header.magic != MAGIC)
    {
	printf("magic num is invalid, rcv: 0x%08x, expect: 0x%08x\n", packet->header.magic, MAGIC);
        return -1;
    }

    hcrc = crc32((char *)packet, sizeof(UpdataPacketHeader_t) - sizeof(int));

    if (hcrc != packet->header.hcrc)
    {
	printf("hcrc checksum is invalid, rcv: 0x%08x, expect: 0x%08x\n", packet->header.hcrc, hcrc);
        return -1;
    }

    if (packet->data != NULL)
    {
	dcrc = crc32((char *)(packet->data), packet->header.size);
	if (dcrc != packet->header.dcrc)
	{
		printf("dcrc checksum is invalid, rcv: 0x%08x, expect: 0x%08x\n", packet->header.dcrc, dcrc);
		return -1;
	}
    }

    return 0;
}

void clear_rcv_buf(int fd)
{
    int length = 0;
    
    do
    {
        length = read(fd, rcv_buf, RCV_BUF_LEN);
    }
    while(length > 0);
}

int rcv_packet(int fd, UpdataPacket_t *packet)
{
    int length = 0;
    int size = 0;
    int retval;
    fd_set fs_read;
    struct timeval tv;

    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);

    tv.tv_sec  = 5;
    tv.tv_usec = 0;

    retval = select(fd + 1, &fs_read, NULL, NULL, &tv);

    if (retval == -1)
    {
        perror("select()");
        return -1;
    }
    else if (retval)
    {
        printf("Data is available now.\n");
        /* FD_ISSET(0, &rfds) will be true. */
    }
    else
    {
        printf("No data within five seconds.\n");
        return -1;
    }

    if (FD_ISSET(fd, &fs_read))
    {
	memset(rcv_buf, 0, sizeof(rcv_buf));

        do
        {
            size = read(fd, rcv_buf + length, RCV_BUF_LEN - length);
            length += size;
            //usleep(1000);

        }while(size > 0);

        if (length < sizeof(UpdataPacketHeader_t))
        {
            printf("length: %d is not equal %d\n", length, (int)sizeof(UpdataPacketHeader_t));
            return -1;
        }

        memcpy(&packet->header, rcv_buf, sizeof(UpdataPacketHeader_t));
        packet->data = rcv_buf + sizeof(UpdataPacketHeader_t);

        if (check_packet(packet) != 0)
        {
            printf("crc check sum failed.\n");
            return -1;
        }

        return 0;
    }
    
    printf("poll failed.\n");
    return -1;
}

int send_packet(int fd, UpdataPacket_t *packet)
{
    int ret;

    if (!packet)
    {
        printf("invalid packet.\n");
        return -1;
    }

    ret = write(fd, (char *)(&packet->header), sizeof(UpdataPacketHeader_t));
    if (ret < 0)
    {
        printf("send packet header failed.\n");
        tcflush(fd,TCOFLUSH); 
        return -1;
    }

    ret = write(fd, (char *)(packet->data), packet->header.size);
    if (ret < 0)
    {
        printf("send packet payload failed.\n");
        tcflush(fd,TCOFLUSH); 
        return -1;
    }

    return 0;
}

void dump_packet(UpdataPacket_t *packet)
{
    int i;
    char *buf;

    if (packet)
    {
        printf("magic:   0x%08x\n", packet->header.magic);
        printf("type:    %u\n", packet->header.type);
        printf("errcode: %u\n", packet->header.ErrCode);
        printf("size:    %u\n", packet->header.size);
        printf("dcrc:    0x%08x\n", packet->header.dcrc);
        printf("hcrc:    0x%08x\n", packet->header.hcrc);

        if (packet->header.size)
        {
            buf = (char *)packet->data;
            printf("payload: ");
        }

        for (i = 0; i < packet->header.size; i++)
        {
            printf("%02x ", buf[i]);
        }
    }
}





