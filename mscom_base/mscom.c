#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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
                dump_packet(packet);
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
            if (size == 0)
            {
               usleep(4000); 
               size = read(fd, rcv_buf + length, RCV_BUF_LEN - length);
            }
            length += size;
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

#define __MSCOM_DEBUG__
#ifdef __MSCOM_DEBUG__
static void dump_payload(char *buf, unsigned int len)
{
    unsigned int i;

    printf("payload: ");
    for (i = 0; i < len; i++)
    {
        printf("%02x ", buf[i]);
    }

    printf("\n");
}
#endif

int send_packet(int fd, UpdataPacket_t *packet)
{
    int ret;
    char *send_buf = NULL;
    int header_len = sizeof(UpdataPacketHeader_t);
    int payload_len = packet->header.size;
    int buf_len = header_len + payload_len;

    if (!packet)
    {
        printf("invalid packet.\n");
        return -1;
    }

    send_buf = (char *)malloc(buf_len);
    if (!send_buf)
    {
        printf("alloc send_buf failed.\n");
        return -1;
    }

    memset(send_buf, 0, buf_len);
    memcpy(send_buf, (char *)&(packet->header), header_len);
    memcpy(send_buf + header_len, (char *)packet->data, payload_len);

    //dump_packet(packet);
    //dump_payload(send_buf, buf_len);

    ret = write(fd, (char *)send_buf, buf_len);
    if (ret < 0)
    {
        free(send_buf);
        printf("send packet payload failed.\n");
        tcflush(fd,TCOFLUSH); 
        return -1;
    }
    
    free(send_buf);

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

	printf("\n");
    }
}





