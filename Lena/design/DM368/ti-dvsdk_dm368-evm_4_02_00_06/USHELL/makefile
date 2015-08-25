TOOLPATH = /opt/arm-2009q1-203/bin
CC = $(TOOLPATH)/arm-none-linux-gnueabi-gcc

ARM9:ushell.c
	$(CC) -g -o ushell ushell.c libreadline_ARM9.a -lpthread -lrt --static
