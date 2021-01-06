/*******************************************************************************
 \file  uart.c
 \brief definitions for uart communication with TI BLE Dongle
 \author SARA
 \date 20200227
*******************************************************************************/
#ifndef __UART_H__
#define __UART_H__

#include <windows.h>

#define RX_BUFFER_MAX 0x400

extern HANDLE openSerial(const char *port);
extern unsigned char * readUart(HANDLE serialHandle, unsigned char ** bytes, unsigned long * bytesRead);
extern unsigned long writeUart(HANDLE serialHandle, void * packet, unsigned long packetLength);

#endif // __UART_H__
