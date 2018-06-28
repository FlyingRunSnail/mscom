#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app.h"
#include "../packet/mscom.h"

static int packet_check(UpdataPacket_t *packet, unsigned int type)
{
    if ((packet->header.type != type) || (packet->header.ErrCode != PACKET_SUCCESS))
    {
        printf("rcved packet check failed, type: %d, errcode: %d.\n", packet->header.type, packet->header.ErrCode);
        return -1;
    }

    return 0;
}

int test_cmd(int fd)
{
    int err;
    int i;
    char *data;
    UpdataPacket_t packet, reply;
#define DATA_LEN (1000)

    data = (char *)malloc(DATA_LEN);
    if (!data)
    {
        printf("malloc failed.\n");
        return -1;
    }

    memset(data, 0, DATA_LEN);

    for (i = 0; i < DATA_LEN; i++)
    {
        data[i] = i;
    }

    err = create_packet(&packet, CPU_ECHO_CMD, data, DATA_LEN);
    if (err != 0)
    {
        printf("create packet for cmd %d failed.\n", CPU_ECHO_CMD);
        return -1;
    }

    dump_packet(&packet);

    err= send_packet(fd, &packet);
    if (err != 0)
    {
        printf("send packet to mcu failed.\n");
        return -1;
    }

    err = rcv_packet(fd, &reply);
    if (err != 0)
    {
        printf("rcv packet failed.\n");
        return -1;
    }
    
    dump_packet(&reply);

    printf("rcv success.\n");
    return 0;
}

int get_version(int fd)
{
    int err;
    project_version_t *ver;

    UpdataPacket_t packet, reply;
    err = create_packet(&packet, GET_VERSION_CMD, NULL, 0);
    if (err != 0)
    {
        printf("create packet for cmd %d failed.\n", GET_VERSION_CMD);
        return -1;
    }

    //dump_packet(&packet);

    err = send_packet(fd, &packet);
    if (err != 0)
    {
        printf("send packet to mcu failed.\n");
        return -1;
    }

    err = rcv_packet(fd, &reply);
    if (err != 0)
    {
        printf("rcv packet failed.\n");
        return -1;
    }

    ver = (project_version_t *)reply.data;

    printf("hardware: %s\n", ver->hardware);
    printf("software: %s\n", ver->software);
    printf("buildtime: %s\n", ver->buildtime);

    return 0;
}

static void show_service_config(port_config_item_t *item)
{
    unsigned int i;

    printf("\r\n\t COM \t role \t  peer \t\t port \r\n");

    for (i = 0; i < COM_MAX; i++)
    {
        printf("\t COM%d \t %s \t %d.%d.%d.%d \t %d\r\n", 
                item[i].com + 1, 
                (item[i].role == COM_ROLE_SLAVE) ? "S" : "M",
                item[i].peer[0], item[i].peer[1], item[i].peer[2], item[i].peer[3],
                item[i].port);
    }
}

int get_service_config(int fd)
{
    int err;
    port_config_item_t *config;
    UpdataPacket_t packet, reply;

    err = create_packet(&packet, GET_SERVICE_CMD, NULL, 0);
    if (err != 0)
    {
        printf("create packet for cmd %d failed.\n", GET_SERVICE_CMD);
        return -1;
    }
    
    err = send_packet(fd, &packet);
    if (err != 0)
    {
        printf("send packet to mcu failed.\n");
        return -1;
    }

    err = rcv_packet(fd, &reply);
    if (err != 0)
    {
        printf("rcv packet failed.\n");
        return -1;
    }

    //dump_packet(&reply);

    config = (port_config_item_t *)reply.data;

    show_service_config(config);
    
    return 0;
}


int clear_service_config(int fd)
{
    int err;
    UpdataPacket_t packet, reply;

    err = create_packet(&packet, CLEAR_SERVICE_CMD, NULL, 0);
    if (err != 0)
    {
        printf("create packet for cmd %d failed.\n", GET_SERVICE_CMD);
        return -1;
    }

    err = send_packet(fd, &packet);
    if (err != 0)
    {
        printf("send packet to mcu failed.\n");
        return -1;
    }

    err = rcv_packet(fd, &reply);
    if (err != 0)
    {
        printf("rcv packet failed.\n");
        return -1;
    }

    printf("clear service config success.\n");
     
    return 0;
}


int modify_service_config(int fd, int argc, char **argv)
{
    int err;
    port_config_item_t config;
    UpdataPacket_t packet, reply;

    if (argc != 4)
    {
        printf("illegal argc.\n");
        return -1;
    }

#if 1
    config.com = (unsigned char)atoi(argv[0]);
    config.role = (unsigned char)atoi(argv[1]);
    sscanf(argv[2], "%d.%d.%d.%d", (int *)&config.peer[0], (int *)&config.peer[1], (int *)&config.peer[2], (int *)&config.peer[3]);
    config.port = (unsigned short)atoi(argv[3]);
    printf("parse result:\n");
    printf("com: %d\n", config.com);
    printf("role:%d\n", config.role);
    printf("peer:%d.%d.%d.%d\n", config.peer[0], config.peer[1], config.peer[2], config.peer[3]);
    printf("port:%u\n", config.port);
#else
    config.com = 0;
    config.role = 0;
    config.peer[0] = 127;
    config.peer[1] = 0;
    config.peer[2] = 0;
    config.peer[3] = 1;
    config.port = (unsigned short)(8888);
#endif
    
    err = create_packet(&packet, SET_SERVICE_CMD, (char *)&config, sizeof(config));
    if (err != 0)
    {
        printf("create packet for cmd %d failed.\n", SET_SERVICE_CMD);
        return -1;
    }
    
    dump_packet(&packet);

    err = send_packet(fd, &packet);
    if (err != 0)
    {
        printf("send packet to mcu failed.\n");
        return -1;
    }
#if 1
    err = rcv_packet(fd, &reply);
    if (err != 0)
    {
        printf("rcv packet failed.\n");
        return -1;
    }
    dump_packet(&reply);
#endif
    return 0;
}

static int do_get_port_stat(int fd, unsigned int i, port_stat_t *stat)
{
    int err;
    UpdataPacket_t packet, reply;

    err = create_packet(&packet, GET_PORT_STAT_CMD, (char *)&i, sizeof(i));
    if (err != 0)
    {
        printf("create packet for cmd %d failed.\n", GET_PORT_STAT_CMD);
        return -1;
    }

    //dump_packet(&packet);
    
    err = send_packet(fd, &packet);
    if (err != 0)
    {
        printf("send packet to mcu failed.\n");
        return -1;
    }
    
    err = rcv_packet(fd, &reply);
    if (err != 0)
    {
        printf("rcv packet failed.\n");
        return -1;
    }
    
    //dump_packet(&reply);
    memcpy((char *)stat, (char *)reply.data, reply.header.size);

    return 0;
}

static void show_port_stat(port_stat_t *stat)
{
    if (!stat)
    {
        printf("stat is illegal.\n");
        return;
    }

    printf("\nCOM%d     task id:%d\n", stat->com + 1, stat->task_id);
    printf("----------------------------------------------------------------------------------------------------\n");
    printf("rcv_bytes:    %8u     rcv_ok_cnt:     %8u    rcv_err_cnt:    %8u\n", stat->rcv_bytes, stat->rcv_ok_cnt, stat->rcv_err_cnt);
    printf("snd_bytes:    %8u     snd_ok_cnt:     %8u    snd_err_cnt:    %8u\n", stat->snd_bytes, stat->snd_ok_cnt, stat->snd_err_cnt);
    printf("fwd_bytes:    %8u     fwd_ok_cnt:     %8u    fwd_err_cnt:    %8u\n", stat->fwd_bytes, stat->fwd_ok_cnt, stat->fwd_err_cnt);
    printf("\ninterrupt stat:\n");
    printf("----------------------------------------------------------------------------------------------------\n");
    printf("rx_int_cnts:  %8u     rx_idle_int_cnts: %8u\n", stat->rx_int_cnts, stat->rx_idle_int_cnts);
    printf("\nreceive stat:\n");
    printf("----------------------------------------------------------------------------------------------------\n");
    printf("rx_kfifo_in:  %8u     rx_kfifo_in_bytes:      %8u    rx_kfifo_err_in:    %8u\n", stat->rx_kfifo_in, stat->rx_kfifo_in_bytes, stat->rx_kfifo_err_in);
    printf("\ntx dma stat:\n");
    printf("----------------------------------------------------------------------------------------------------\n");
    printf("index:        %8u     count:    %8u          en_cnt:    %8u    ok_cnt:    %8u\n", stat->index, stat->count, stat->en_cnt, stat->ok_cnt);
}

int get_port_stat(int fd, int argc, char **argv)
{
    unsigned int i;
    port_stat_t stat;

    if (argc != 1)
    {
        printf("usage: get_port_stat [all|0~3]\n");
	return -1;
    }

    if (strcmp(argv[0], "all") == 0) 
    {
        for (i = 0; i < USART_MPORT_MAX; i++)
	{
            do_get_port_stat(fd, i, &stat);	    
	    show_port_stat(&stat);
	}
    }
    else
    {
        i = atoi(argv[0]);
	if (i >= COM_MAX)
	{
            printf("usage: get_port_stat [all|0~3]\n");
	    return -1;
	}

        do_get_port_stat(fd, i, &stat);	    
	show_port_stat(&stat);
    }

    return 0;
}

static int do_clear_port_stat(int fd, unsigned int i)
{
    int err;
    UpdataPacket_t packet, reply;

    err = create_packet(&packet, CLEAR_PORT_STAT_CMD, (char *)&i, sizeof(i));
    if (err != 0)
    {
        printf("create packet for cmd %d failed.\n", GET_PORT_STAT_CMD);
        return -1;
    }

    //dump_packet(&packet);
    
    err = send_packet(fd, &packet);
    if (err != 0)
    {
        printf("send packet to mcu failed.\n");
        return -1;
    }
    
    err = rcv_packet(fd, &reply);
    if (err != 0)
    {
        printf("rcv packet failed.\n");
        return -1;
    }
    
    dump_packet(&reply);

    if ((reply.header.type != CLEAR_PORT_STAT_CMD) || (reply.header.ErrCode != PACKET_SUCCESS))
    {
        printf("rcved packet check failed.\n");
        return -1;
    }

    return 0;
}

int clear_port_stat(int fd, int argc, char **argv)
{
    int err;
    unsigned int i;
    
    if (argc != 1)
    {
        printf("usage: clear_port_stat [all|0~3]\n");
	return -1;
    }

    if (strcmp(argv[0], "all") == 0) 
    {
        for (i = 0; i < USART_MPORT_MAX; i++)
	{
	    err = do_clear_port_stat(fd, i); 
	    if (err != 0)
	    {
	        printf("clear port %d failed.\n", i + 1);
	    }
	}
    }
    else
    {
        i = atoi(argv[0]);
	if (i >= COM_MAX)
	{
            printf("usage: get_port_stat [all|0~3]\n");
	    return -1;
	}

	err = do_clear_port_stat(fd, i);
	if (err != 0)
	{
	    printf("clear port %d failed.\n", i + 1);
	}
    }

    return 0;
}

int get_network_param(int fd)
{
    int err;
    UpdataPacket_t packet, reply;
    network_param_t *network;

    err = create_packet(&packet, GET_NETPARAM_CMD, NULL, 0);
    if (err != 0)
    {
        printf("create packet for cmd %d failed.\n", GET_NETPARAM_CMD);
        return -1;
    }

    err = send_packet(fd, &packet);
    if (err != 0)
    {
        printf("send packet to mcu failed.\n");
        return -1;
    }
    
    err = rcv_packet(fd, &reply);
    if (err != 0)
    {
        printf("rcv packet failed.\n");
        return -1;
    }

    err =packet_check(&reply, GET_NETPARAM_CMD);
    if (err != 0)
    {
        printf("packet check failed.\n");
	return -1;
    }

    network = (network_param_t *)(reply.data);
    printf("mac     address: %02x:%02x:%02x:%02x:%02x:%02x\n", network->mac[0], network->mac[1], network->mac[2], network->mac[3], network->mac[4], network->mac[5]);
    printf("ip      address: %d.%d.%d.%d\n", network->ip[0], network->ip[1], network->ip[2], network->ip[3]);
    printf("netmask address: %d.%d.%d.%d\n", network->netmask[0], network->netmask[1], network->netmask[2], network->netmask[3]);
    printf("getway  address: %d.%d.%d.%d\n", network->gateway[0], network->gateway[1], network->gateway[2], network->gateway[3]);

    return 0;
}

int set_network_param(int fd, int argc, char **argv)
{
    int err;
    network_param_t network;
    UpdataPacket_t packet, reply;

    if (argc != 3)
    {
        printf("usage: set_network_param 192.168.0.100 255.255.255.0 192.168.0.1\n");
        return -1;
    }

    sscanf(argv[0], "%d.%d.%d.%d", (int *)&network.ip[0], (int *)&network.ip[1], (int *)&network.ip[2], (int *)&network.ip[3]);
    sscanf(argv[1], "%d.%d.%d.%d", (int *)&network.netmask[0], (int *)&network.netmask[1], (int *)&network.netmask[2], (int *)&network.netmask[3]);
    sscanf(argv[2], "%d.%d.%d.%d", (int *)&network.gateway[0], (int *)&network.gateway[1], (int *)&network.gateway[2], (int *)&network.gateway[3]);

    err = create_packet(&packet, SET_NETPARAM_CMD, (void *)&network, sizeof(network));
    if (err != 0)
    {
        printf("create packet for cmd %d failed.\n", GET_NETPARAM_CMD);
        return -1;
    }
    
    err = send_packet(fd, &packet);
    if (err != 0)
    {
        printf("send packet to mcu failed.\n");
        return -1;
    }
    
    err = rcv_packet(fd, &reply);
    if (err != 0)
    {
        printf("rcv packet failed.\n");
        return -1;
    }

    err =packet_check(&reply, SET_NETPARAM_CMD);
    if (err != 0)
    {
        printf("packet check failed.\n");
	return -1;
    }

    return 0;
}

int ping_cmd(int fd, int argc, char **argv)
{
    int err;
    char buf[4] = {0};
    UpdataPacket_t packet, reply;

    if (argc != 1)
    {
        printf("usage:ping 192.168.1.111\n");
        return -1;
    }

    sscanf(argv[0], "%d.%d.%d.%d", (int *)&buf[0], (int *)&buf[1], (int *)&buf[2], (int *)&buf[3]);
    printf("ip:%d.%d.%d.%d\n", buf[0], buf[1], buf[2], buf[3]);

    err = create_packet(&packet, DO_PING_CMD, (void *)argv[0], strlen(argv[0]) + 1);
    if (err != 0)
    {
        printf("create packet for cmd %d failed.\n", DO_PING_CMD);
        return -1;
    }
    
    err = send_packet(fd, &packet);
    if (err != 0)
    {
        printf("send packet to mcu failed.\n");
        return -1;
    }
    
    err = rcv_packet(fd, &reply);
    if (err != 0)
    {
        printf("rcv packet failed.\n");
        return -1;
    }

    err =packet_check(&reply, DO_PING_CMD);
    if (err != 0)
    {
        printf("packet check failed.\n");
	return -1;
    }

    printf("rcv from %s, time=%lu\n", argv[0], *((unsigned int *)(reply.data)));
    return 0;
}


