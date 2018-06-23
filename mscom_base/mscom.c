#include <string.h>
#include <stdio.h>
#include "mscom.h"
#include "../crc/crc.h"

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

int check_crc(UpdataPacket_t *packet)
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

