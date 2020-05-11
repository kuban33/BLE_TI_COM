/*******************************************************************************
 \file  uart.c
 \brief sources for uart communication with TI BLE Dongle
 \author SARA
 \date 20200227
*******************************************************************************/
#include "uart.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

HANDLE openSerial(const char *port) {
    // Open serial port
    HANDLE serialHandle;

    serialHandle = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (!serialHandle) {
        fprintf(stderr,"Error CreateFile:%s\n",port);
        return (NULL);
    }

    // Do some basic settings
    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);

    if (!GetCommState(serialHandle, &serialParams)) {
        fprintf(stderr,"Error GetCommState:%p\n",serialHandle);
        CloseHandle(serialHandle);
        return (NULL);
    }
    serialParams.BaudRate = CBR_115200;
    serialParams.ByteSize = 8;
    serialParams.StopBits = ONESTOPBIT;
    serialParams.Parity = NOPARITY;
    if (!SetCommState(serialHandle, &serialParams)) {
        fprintf(stderr,"Error SetCommState:%p\n",&serialParams);
        CloseHandle(serialHandle);
        return (NULL);
    }

    // Set timeouts
    COMMTIMEOUTS timeout = { 0 };

    timeout.ReadIntervalTimeout = 50;
    timeout.ReadTotalTimeoutConstant = 50;
    timeout.ReadTotalTimeoutMultiplier = 50;
    timeout.WriteTotalTimeoutConstant = 50;
    timeout.WriteTotalTimeoutMultiplier = 10;
    /*
    timeout.ReadIntervalTimeout = 10;
    timeout.ReadTotalTimeoutConstant = 10;
    timeout.ReadTotalTimeoutMultiplier = 10;
    timeout.WriteTotalTimeoutConstant = 10;
    timeout.WriteTotalTimeoutMultiplier = 5;
    */
    if (!SetCommTimeouts(serialHandle, &timeout)) {
        fprintf(stderr,"Error SetCommTimeouts:%p\n",&timeout);
        CloseHandle(serialHandle);
        return (NULL);
    }
    return (serialHandle);
}
/*
HANDLE openSerialEventMode(const char *port) {
    // Open serial port
    HANDLE serialHandle;
    //OVERLAPPED o;

    serialHandle = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (!serialHandle) {
        fprintf(stderr,"Error CreateFile:%s Error:%lu\n",port,GetLastError());
        return (NULL);
    }

    // Do some basic settings
    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);

    if (!GetCommState(serialHandle, &serialParams)) {
        fprintf(stderr,"Error GetCommState:%p Error:%lu\n",serialHandle,GetLastError());
        CloseHandle(serialHandle);
        return (NULL);
    }
    serialParams.BaudRate = CBR_115200;
    serialParams.ByteSize = 8;
    serialParams.StopBits = ONESTOPBIT;
    serialParams.Parity = NOPARITY;
    if (!SetCommState(serialHandle, &serialParams)) {
        fprintf(stderr,"Error SetCommState:%p Error:%lu\n",&serialParams,GetLastError());
        CloseHandle(serialHandle);
        return (NULL);
    }

    if (!SetCommMask(serialHandle,EV_TXEMPTY|EV_RXCHAR)) {
        fprintf(stderr,"Error SetCommMask:%p Error:%lu\n",serialHandle,GetLastError());
        CloseHandle(serialHandle);
        return (NULL);
    }

    // Set timeouts
    COMMTIMEOUTS timeout = { 0 };

    timeout.ReadIntervalTimeout = 50;
    timeout.ReadTotalTimeoutConstant = 50;
    timeout.ReadTotalTimeoutMultiplier = 50;
    timeout.WriteTotalTimeoutConstant = 50;
    timeout.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(serialHandle, &timeout)) {
        fprintf(stderr,"Error SetCommTimeouts:%p Error:%lu\n",&timeout,GetLastError());
        CloseHandle(serialHandle);
        return (NULL);
    }
    return (serialHandle);
}
*/
unsigned char * readUart(HANDLE serialHandle, unsigned char ** bytes, unsigned long * bytesRead) {
    unsigned char buffer[RX_BUFFER_MAX];

    if (!ReadFile(serialHandle, buffer, RX_BUFFER_MAX, bytesRead, NULL)) {
        fprintf(stderr,"Error ReadFile:\n");
        *bytes=NULL;
        return NULL;
    }
    if (*bytesRead>0) {
        if (*bytes!=NULL) { free(*bytes); *bytes=NULL; }
        *bytes=memcpy(realloc(*bytes,sizeof(unsigned char) * *bytesRead),buffer,*bytesRead);
        return *bytes;
    }
    else {
        *bytes=NULL;
        return NULL;
    }
}

unsigned long writeUart(HANDLE serialHandle, void * packet, unsigned long packetLength) {
    DWORD bytesWritten=0;
    if (!WriteFile(serialHandle, packet, packetLength, &bytesWritten, NULL)) {
        fprintf(stderr,"Error WriteFile:\n");
        return (0);
    }
    return bytesWritten;
}
