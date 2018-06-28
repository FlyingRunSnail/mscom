#ifndef __MSCOM_H__
#define __MSCOM_H__

#define MAGIC (0x616E6469)

enum UpdataType
{
    UPDATA_MCU_CMD = 0,
    ERASE_FPGA_FLASH_CMD,
    WRITE_FPGA_FLASH_CMD,
    READ_FPGA_FLASH_CMD,
    MCU_JUMP_TO_FLASH_CMD,
    CPU_ECHO_CMD,
    MCU_REPLY_CMD,
    GET_VERSION_CMD,
    GET_NETPARAM_CMD,
    SET_NETPARAM_CMD,
    DO_PING_CMD,
    GET_SERVICE_CMD,
    CLEAR_SERVICE_CMD,
    SET_SERVICE_CMD,
    GET_PORT_STAT_CMD,
    CLEAR_PORT_STAT_CMD,
};

enum UpdataErrCode
{
    PACKET_SUCCESS = 0,
    PACKET_FAIL,
    PACKET_MAGIC,
    PACKET_HEAD_CRC_ERR,
    PACKET_DATA_CRC_ERR,
    PACKET_UNKNOW
};

typedef struct UpdataPacketHeader
{
    unsigned int magic;
    unsigned int type;
    unsigned int ErrCode;
    unsigned int size;
    unsigned int dcrc;
    unsigned int hcrc;

}UpdataPacketHeader_t;


typedef struct UpdataPacket
{
    UpdataPacketHeader_t header;
    void *data;

}UpdataPacket_t;


int create_packet(UpdataPacket_t *packet, int type, char *data, int size);
int check_packet(UpdataPacket_t *packet);
void clear_rcv_buf(int fd);
int rcv_packet(int fd, UpdataPacket_t *packet);
int send_packet(int fd, UpdataPacket_t *packet);
void dump_packet(UpdataPacket_t *packet);

#endif


