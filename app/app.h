#ifndef __APP_H__
#define __APP_H__

typedef struct project_version
{
    char hardware[8];
    char software[8];
    char buildtime[32];
}project_version_t;

typedef enum {
    COM1 = 0,
    COM2,
    COM3,
    COM4,

    COM_MAX
}COM_NO;

typedef enum {
    COM_ROLE_SLAVE = 0,
    COM_ROLE_MASTER,

    COM_ROLE_MAX
}COM_ROLE;

#define MAC_ADDR_LEN (6)
#define USART_MPORT_MAX (4)
#define IP_BUF_LEN (4)

typedef struct port_config_item
{
    unsigned char com;
    unsigned char role;
    unsigned char peer[IP_BUF_LEN];
    unsigned short port;
}port_config_item_t;

typedef struct port_stat
{
    unsigned char  com;
    unsigned short task_id;

    unsigned int   rcv_bytes;
    unsigned int   rcv_ok_cnt;
    unsigned int   rcv_err_cnt;
    
    unsigned int   snd_bytes;
    unsigned int   snd_ok_cnt;
    unsigned int   snd_err_cnt;
    
    unsigned int   fwd_bytes;
    unsigned int   fwd_ok_cnt;
    unsigned int   fwd_err_cnt;

    unsigned int   rx_int_cnts;
    unsigned int   rx_idle_int_cnts;

    unsigned int   rx_kfifo_in;
    unsigned int   rx_kfifo_in_bytes;
    unsigned int   rx_kfifo_err_in;

    unsigned int   index;
    unsigned int   count;
    unsigned int   en_cnt;
    unsigned int   ok_cnt;
}port_stat_t;

typedef struct network_param
{
    unsigned char mac[MAC_ADDR_LEN];
    unsigned char ip[IP_BUF_LEN];
    unsigned char netmask[IP_BUF_LEN];
    unsigned char gateway[IP_BUF_LEN];
}network_param_t;

int test_cmd(int fd);
int get_version(int fd);
int get_service_config(int fd);
int clear_service_config(int fd);
int modify_service_config(int fd, int argc, char **argv);
int get_port_stat(int fd, int argc, char **argv);
int clear_port_stat(int fd, int argc, char **argv);
int get_network_param(int fd);
int set_network_param(int fd, int argc, char **argv);
int ping_cmd(int fd, int argc, char **argv);

#endif


