#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>
#include "crc/crc.h"
#include "mscom_base/mscom.h"
//#include "serial/serial.h"


#define RCV_BUF_LEN (1<<10)

int set_serial(int fd,int nSpeed,int nBits,char nEvent,int nStop)
{
    struct termios newttys1,oldttys1;

     /*保存原有串口配置*/
     if(tcgetattr(fd,&oldttys1)!=0) 
     {
          perror("Setupserial 1");
          return -1;
     }
     bzero(&newttys1,sizeof(newttys1));
     newttys1.c_cflag|=(CLOCAL|CREAD ); /*CREAD 开启串行数据接收，CLOCAL并打开本地连接模式*/

     newttys1.c_cflag &=~CSIZE;/*设置数据位*/
     /*数据位选择*/   
     switch(nBits)
     {
         case 7:
             newttys1.c_cflag |=CS7;
             break;
         case 8:
             newttys1.c_cflag |=CS8;
             break;
     }
     /*设置奇偶校验位*/
     switch( nEvent )
     {
         case '0':  /*奇校验*/
             newttys1.c_cflag |= PARENB;/*开启奇偶校验*/
             newttys1.c_iflag |= (INPCK | ISTRIP);/*INPCK打开输入奇偶校验；ISTRIP去除字符的第八个比特  */
             newttys1.c_cflag |= PARODD;/*启用奇校验(默认为偶校验)*/
             break;
         case 'E':/*偶校验*/
             newttys1.c_cflag |= PARENB; /*开启奇偶校验  */
             newttys1.c_iflag |= ( INPCK | ISTRIP);/*打开输入奇偶校验并去除字符第八个比特*/
             newttys1.c_cflag &= ~PARODD;/*启用偶校验*/
             break;
         case 'N': /*无奇偶校验*/
             newttys1.c_cflag &= ~PARENB;
             break;
     }
     /*设置波特率*/
    switch( nSpeed )  
    {
        case 2400:
            cfsetispeed(&newttys1, B2400);
            cfsetospeed(&newttys1, B2400);
            break;
        case 4800:
            cfsetispeed(&newttys1, B4800);
            cfsetospeed(&newttys1, B4800);
            break;
        case 9600:
            cfsetispeed(&newttys1, B9600);
            cfsetospeed(&newttys1, B9600);
            break;
        case 115200:
            cfsetispeed(&newttys1, B115200);
            cfsetospeed(&newttys1, B115200);
            break;
        default:
            cfsetispeed(&newttys1, B9600);
            cfsetospeed(&newttys1, B9600);
            break;
    }
     /*设置停止位*/
    if( nStop == 1)/*设置停止位；若停止位为1，则清除CSTOPB，若停止位为2，则激活CSTOPB*/
    {
        newttys1.c_cflag &= ~CSTOPB;/*默认为一位停止位； */
    }
    else if( nStop == 2)
    {
        newttys1.c_cflag |= CSTOPB;/*CSTOPB表示送两位停止位*/
    }

    /*设置最少字符和等待时间，对于接收字符和等待时间没有特别的要求时*/
    newttys1.c_cc[VTIME] = 0;/*非规范模式读取时的超时时间；*/
    newttys1.c_cc[VMIN]  = 0; /*非规范模式读取时的最小字符数*/
    tcflush(fd ,TCIFLUSH);/*tcflush清空终端未完成的输入/输出请求及数据；TCIFLUSH表示清空正收到的数据，且不读取出来 */

     /*激活配置使其生效*/
    if((tcsetattr( fd, TCSANOW,&newttys1))!=0)
    {
        perror("com set error");
        return -1;
    }

    return 0;
}

char rcv_buf[RCV_BUF_LEN] = {0};

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

    tv.tv_sec  = 60;
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

        if (check_crc(packet) != 0)
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

int main(int argc, char *argv[])
{
    int fd = 0;
    int err = 0;
    char *dev_name = NULL;
    UpdataPacket_t request;
    UpdataPacket_t reply;
    
    if (argc != 2)
    {
        return -1;
    }

    dev_name = argv[1];
    fd = open(dev_name, O_RDWR|O_NOCTTY|O_NDELAY);
    if(fd < 0)
    {
        printf("open ttyUSB0 failed.\n");
        return -1;
    }

    if (fcntl(fd, F_SETFL, 0) < 0)
    {
        printf("fcntl failed!\n");
        return -1;
    }

    err = set_serial(fd, 115200, 8, 'N', 1);
    if (err != 0)
    {
        close(fd);
        return -1;
    }
    
    //serialOpen();

    // write and read from fd
    err = create_packet(&request, CPU_ECHO_CMD, NULL, 0);
    if (err != 0)
    {
        printf("create packet failed.\r\n");
        close(fd);
        return -1;
    }

    clear_rcv_buf(fd);
    //err = write(fd, &request, sizeof(request));
    err = send_packet(fd, &request);
    if (err != 0)
    {
        printf("send packet to mcu failed.\n");
        close(fd);
        return -1;
    }

    err = rcv_packet(fd, &reply);
    if (err != 0)
    {
	printf("rcv packet failed.\n");
	close(fd);
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

    close(fd);

    return 0;
}


