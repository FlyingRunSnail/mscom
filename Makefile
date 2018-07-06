target = myserial
objs = main.o crc/crc.o packet/mscom.o serial/serial.o app/app.o ispload/stm32update.o
#CC=gcc
CC := /opt/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
#CFLAGS = -Werror -Wall -g -O2
CFLAGS = -g -O2
RM := rm -rf

all:$(objs)
	$(CC) -o $(target) $^
	cp $(target) /home/huyong/work/


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY:clean

clean:
	$(RM) *.o
	$(RM) crc/*.o
	$(RM) packet/*.o
	$(RM) serial/*.o
	$(RM) app/*.o
	$(RM) ispload/*.o
	$(RM) $(target)
