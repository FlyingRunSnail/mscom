#ifndef __STM32ISP_H__
#define __STM32ISP_H__

#include "stm32.h"

int stm32isp_init(const char *port, unsigned int baud, unsigned int bootp, unsigned int reset);
int stm32isp_load(const char *filename);
void stm32isp_exit(void);

#endif
