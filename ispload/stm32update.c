#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stm32update.h"

#define STM32_COMMAND_BUF_LEN (256)

int stm32_init()
{
    return STM32_INIT_OK;
}

int stm32_update(const char *dev, const char *file, unsigned int start_addr)
{
    int err;
    char command[STM32_COMMAND_BUF_LEN] = {0};

    if (!dev)
    {
        printf("dev is null.\n");
        return STM32_UPDATE_NODEV;
    }

    if (!file)
    {
        printf("file is null.\n");
        return STM32_UPDATE_NOFILE;
    }

    snprintf(command, STM32_COMMAND_BUF_LEN, "./stm32flash-origin %s -b 115200 -w %s -v -g 0x%08x", dev, file, start_addr);
#ifdef STM32_UPDATE_DEBUG    
    printf(command);
    printf("\n");
#endif

    err = system(command);
    if (err < 0)
    {
        printf("excute %s failed.\n", command);
        return STM32_UPDATE_FAIL;
    }

#ifdef STM32_UPDATE_DEBUG
    printf("stm32 update success.\n");
#endif

    return STM32_UPDATE_OK;
}

void stm32_exit()
{
    return ;
}
