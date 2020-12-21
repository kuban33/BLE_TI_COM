CC=mingw32-gcc
CFLAGS=-I.

ble_ti_com: ble.o uart.o te.o main.o
	$(CC) -o TE_SPIDER_TIBLE_TOOL.EXE ble.o uart.o te.o main.o
