
TOOLPATH = /opt/arm-2009q1/bin
CC = $(TOOLPATH)/arm-none-linux-gnueabi-gcc
AR = $(TOOLPATH)/arm-none-linux-gnueabi-ar

LIB = ushellagent.a 
OBJECTS = ushellagent.o elf.o

LENA:
	$(CC) -g -o LENA.EXE main.c $(LIB) -lpthread -lrt 

lib:$(OBJECTS)
	$(AR) r $(LIB) $(OBJECTS)
	rm -f *.o
	
ushellagent.o:ushellagent.c typedef.h ushellagent.h 
	$(CC) -c ushellagent.c

elf.o:elf.c 
	$(CC) -c elf.c 

clean:
	rm LENA.EXE $(LIB) 
	
	