CC=mingw32-gcc
CFLAGS=-I.

ble_ti_com: ble.o main.o
	$(CC) -o ble_ti_com ble.o main.o
