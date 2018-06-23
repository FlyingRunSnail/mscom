#ifndef __SERIAL_HEAD__
#define __SERIAL_HEAD__

int open_serial(const char *dev, int nSpeed, int nBits, char nEvent, int nStop);
void close_serial(int fd);

#endif
