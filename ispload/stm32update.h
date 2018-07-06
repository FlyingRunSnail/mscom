#ifndef STM32_UPDATE_H
#define STM32_UPDATE_H

#define STM32_INIT_OK       (0)
#define STM32_INIT_FAIL     (-1)

#define STM32_UPDATE_OK     (0)
#define STM32_UPDATE_FAIL   (-1)
#define STM32_UPDATE_NODEV  (-2)
#define STM32_UPDATE_NOFILE (-3)

//#define STM32_UPDATE_DEBUG

int stm32_init(void);
int stm32_update(const char *dev, const char *file, unsigned int start_addr);
void stm32_exit(void);

#endif
