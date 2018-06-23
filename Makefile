target = myserial
objs = main.o crc/crc.o mscom_base/mscom.o serial/serial.o
CC=gcc
CFLAGS = -Werror -Wall -g -O2

all:$(objs)
	$(CC) -o $(target) $^


main.o:main.c
	$(CC) -c $^ $(CFLAGS)


crc.o:crc.c
	$(CC) -c $^ $(CFLAGS)

mscom.o:mscom.c
	$(CC) -c $^ $(FLAGS)

serial.o:serial.c
	$(CC) -c $^ $(FLAGS)

.PHONY:clean

clean:
	rm -f *.o
	rm -f crc/*.o
	rm -f mscom_base/*.o
	rm -f serial/*.o
	rm -f $(target)
