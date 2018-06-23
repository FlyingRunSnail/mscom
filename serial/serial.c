#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include "serial.h"

static int set_serial(int fd,int nSpeed,int nBits,char nEvent,int nStop)
{
    struct termios newttys1,oldttys1;

     /*±£ŽæÔ­ÓÐŽ®¿ÚÅäÖÃ*/
     if(tcgetattr(fd,&oldttys1)!=0) 
     {
          perror("Setupserial 1");
          return -1;
     }
     bzero(&newttys1,sizeof(newttys1));
     newttys1.c_cflag|=(CLOCAL|CREAD ); /*CREAD ¿ªÆôŽ®ÐÐÊýŸÝœÓÊÕ£¬CLOCAL²¢Žò¿ª±ŸµØÁ¬œÓÄ£Êœ*/

     newttys1.c_cflag &=~CSIZE;/*ÉèÖÃÊýŸÝÎ»*/
     /*ÊýŸÝÎ»Ñ¡Ôñ*/   
     switch(nBits)
     {
         case 7:
             newttys1.c_cflag |=CS7;
             break;
         case 8:
             newttys1.c_cflag |=CS8;
             break;
     }
     /*ÉèÖÃÆæÅŒÐ£ÑéÎ»*/
     switch( nEvent )
     {
         case '0':  /*ÆæÐ£Ñé*/
             newttys1.c_cflag |= PARENB;/*¿ªÆôÆæÅŒÐ£Ñé*/
             newttys1.c_iflag |= (INPCK | ISTRIP);/*INPCKŽò¿ªÊäÈëÆæÅŒÐ£Ñé£»ISTRIPÈ¥³ý×Ö·ûµÄµÚ°Ëžö±ÈÌØ  */
             newttys1.c_cflag |= PARODD;/*ÆôÓÃÆæÐ£Ñé(Ä¬ÈÏÎªÅŒÐ£Ñé)*/
             break;
         case 'E':/*ÅŒÐ£Ñé*/
             newttys1.c_cflag |= PARENB; /*¿ªÆôÆæÅŒÐ£Ñé  */
             newttys1.c_iflag |= ( INPCK | ISTRIP);/*Žò¿ªÊäÈëÆæÅŒÐ£Ñé²¢È¥³ý×Ö·ûµÚ°Ëžö±ÈÌØ*/
             newttys1.c_cflag &= ~PARODD;/*ÆôÓÃÅŒÐ£Ñé*/
             break;
         case 'N': /*ÎÞÆæÅŒÐ£Ñé*/
             newttys1.c_cflag &= ~PARENB;
             break;
     }
     /*ÉèÖÃ²šÌØÂÊ*/
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
     /*ÉèÖÃÍ£Ö¹Î»*/
    if( nStop == 1)/*ÉèÖÃÍ£Ö¹Î»£»ÈôÍ£Ö¹Î»Îª1£¬ÔòÇå³ýCSTOPB£¬ÈôÍ£Ö¹Î»Îª2£¬ÔòŒ€»îCSTOPB*/
    {
        newttys1.c_cflag &= ~CSTOPB;/*Ä¬ÈÏÎªÒ»Î»Í£Ö¹Î»£» */
    }
    else if( nStop == 2)
    {
        newttys1.c_cflag |= CSTOPB;/*CSTOPB±íÊŸËÍÁœÎ»Í£Ö¹Î»*/
    }

    /*ÉèÖÃ×îÉÙ×Ö·ûºÍµÈŽýÊ±Œä£¬¶ÔÓÚœÓÊÕ×Ö·ûºÍµÈŽýÊ±ŒäÃ»ÓÐÌØ±ðµÄÒªÇóÊ±*/
    newttys1.c_cc[VTIME] = 0;/*·Ç¹æ·¶Ä£Êœ¶ÁÈ¡Ê±µÄ³¬Ê±Ê±Œä£»*/
    newttys1.c_cc[VMIN]  = 0; /*·Ç¹æ·¶Ä£Êœ¶ÁÈ¡Ê±µÄ×îÐ¡×Ö·ûÊý*/
    tcflush(fd ,TCIFLUSH);/*tcflushÇå¿ÕÖÕ¶ËÎŽÍê³ÉµÄÊäÈë/Êä³öÇëÇóŒ°ÊýŸÝ£»TCIFLUSH±íÊŸÇå¿ÕÕýÊÕµœµÄÊýŸÝ£¬ÇÒ²»¶ÁÈ¡³öÀŽ */

     /*Œ€»îÅäÖÃÊ¹ÆäÉúÐ§*/
    if((tcsetattr( fd, TCSANOW,&newttys1))!=0)
    {
        perror("com set error");
        return -1;
    }

    return 0;
}

int open_serial(const char *dev, int nSpeed, int nBits, char nEvent, int nStop)
{
    int fd;
    int err;

    fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY);
    if(fd < 0)
    {
        printf("open ttyUSB0 failed.\n");
        return -1;
    }

    if (fcntl(fd, F_SETFL, 0) < 0)
    {
        printf("fcntl failed!\n");
        close(fd);
        return -1;
    }

    err = set_serial(fd, 115200, 8, 'N', 1);
    if (err != 0)
    {
        printf("set serial parameter failed.\n");
        close(fd);
        return -1;
    }

    return fd;
}

void close_serial(int fd)
{
    if (fd >= 0)
    {
        close(fd);
    }
}


