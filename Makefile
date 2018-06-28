target = myserial
objs = main.o crc/crc.o mscom_base/mscom.o serial/serial.o app/app.o
#CC=gcc
CC := /opt/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
#CFLAGS = -Werror -Wall -g -O2
CFLAGS = -g -O2

all:$(objs)
	$(CC) -o $(target) $^
	cp $(target) /home/huyong/work/


main.o:main.c
	$(CC) -c $^ $(CFLAGS)


crc.o:crc.c
	$(CC) -c $^ $(CFLAGS)

mscom.o:mscom.c
	$(CC) -c $^ $(FLAGS)

serial.o:serial.c
	$(CC) -c $^ $(FLAGS)

app.o:app.c
	$(CC) -c $^ $(FLAGS)

.PHONY:clean

clean:
	rm -f *.o
	rm -f crc/*.o
	rm -f mscom_base/*.o
	rm -f serial/*.o
	rm -f app/*.o
	rm -f $(target)
