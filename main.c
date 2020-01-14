#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <windows.h>
#include "ble.h"

#define RX_BUFFER_MAX 0xFF
#define STR_LEN_MAX 0xFF
#define DELAY_ESTABLISH 500
#define DELAY_TE_COMMAND 100

HANDLE openSerial(const char *port) {
    // Open serial port
    HANDLE serialHandle;

    serialHandle = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (!serialHandle) {
        printf("Error CreateFile:%s\n",port);
        return (NULL);
    }

    // Do some basic settings
    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);

    if (!GetCommState(serialHandle, &serialParams)) {
        printf("Error GetCommState:%p\n",serialHandle);
        CloseHandle(serialHandle);
        return (NULL);
    }
    serialParams.BaudRate = CBR_115200;
    serialParams.ByteSize = 8;
    serialParams.StopBits = ONESTOPBIT;
    serialParams.Parity = NOPARITY;
    if (!SetCommState(serialHandle, &serialParams)) {
        printf("Error SetCommState:%p\n",&serialParams);
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
        printf("Error SetCommTimeouts:%p\n",&timeout);
        CloseHandle(serialHandle);
        return (NULL);
    }
    return (serialHandle);
}

void printHciCommandPacket(unsigned char * ptrPacket, unsigned long sizeofPacket) {
    unsigned long i;
    printf("---> HCI packet size: %lu\n",sizeofPacket);
    for (i=0;i!=sizeofPacket;i++) {
        printf("%02X",ptrPacket[i]);
        if (i==0 || i==2 || i==3) printf("|");
        else printf(" ");
        if ((i+1)%0x10==0) printf("\n");
    }
    printf("\n");
    for (i=0;i!=sizeofPacket;i++) {
        if (isprint(ptrPacket[i])) printf("%c",ptrPacket[i]);
        else if (isspace(ptrPacket[i])) printf(" ");
        else printf(".");
    }
    printf("\n");
}

void printHciEventPacket(unsigned char * ptrPacket, unsigned long sizeofPacket) {
    unsigned long i;
    printf("---> HCI packet size: %lu\n",sizeofPacket);
    for (i=0;i!=sizeofPacket;i++) {
        printf("%02X",ptrPacket[i]);
        if (i==0 || i==1 || i==2 || i==4 || i==5) printf("|");
        else printf(" ");
        if ((i+1)%0x10==0) printf("\n");
    }
    printf("\n");
    for (i=0;i!=sizeofPacket;i++) {
        if (isprint(ptrPacket[i])) printf("%c",ptrPacket[i]);
        else if (isspace(ptrPacket[i])) printf(" ");
        else printf(".");
    }
    printf("\n");
}

void readUartHciPacketStdOut(HANDLE serialHandle) {
    unsigned char buffer[0xFF]={0};
    DWORD bytesRead=0;

    if (!ReadFile(serialHandle, buffer, 0xFF, &bytesRead, NULL)) {
        printf("Error ReadFile:\n");
    }
    printf("<Rx> packet:\n");
    printHciEventPacket((unsigned char *) buffer,bytesRead);
}

unsigned long writeUartHciPacket(HANDLE serialHandle, void * packet, unsigned long packetLength) {
    DWORD bytesWritten=0;
    if (!WriteFile(serialHandle, packet, packetLength, &bytesWritten, NULL)) {
        printf("Error WriteFile:\n");
        return (0);
    }
    return bytesWritten;
}

void writeUartHciPacketStdOut(HANDLE serialHandle, void * packet, unsigned long packetLength) {
    DWORD bytesWritten=0;

    printHciCommandPacket((unsigned char *) packet, packetLength);
    getchar();
    if (!WriteFile(serialHandle, packet, packetLength, &bytesWritten, NULL)) {
        printf("Error WriteFile:\n");
    }
    printf("BytesWritten:%lu\n",(unsigned long) bytesWritten);
}

void printHelp(char *arg0) {
    printf("Tool for TE Spider control using Texas Instruments CC2540EMK-USB Dongle\n");
    printf("------------------\n");
    printf("What's up Buddy?\nUsage:\n");
    printf("   %s COM_PORT BLE_MAC_ADRESS COMMANDS...\n",arg0);
    printf("Commands:\n");
    printf("   VOLMAX - sets volume to 100%%\n");
    printf("   LEFT - Enables only left speakers\n");
    printf("   RIGHT - Enables only right speakers\n");
    printf("   BOTH - Enables both left and right speakers\n");
    printf("Examples:\n");
    printf("   %s COM10 E2:17:4D:B8:E0:EE VOLMAX LEFT\n\n",arg0);
    printf("   %s COM10 E2:17:4D:B8:E0:EE VOLMAX BOTH\n\n",arg0);
    printf("   %s COM8 \"F9 98 16 F4 02 24\" VOLMAX\n\n",arg0);
}

int main(int argc, char **argv) {
    char * pComPortStr;
    char * pMacAddrStr;
    unsigned char i;
    long tmpMacElem;
    unsigned char macAddr[BD_ADDR_LEN];

    char serialStr[STR_LEN_MAX];
    HANDLE serialHandle;

    gapDeviceInit_packet pckDevInit=gapDeviceInit_packet_default;
    //why multi init
    gapGetParam_packet pckGetParam[4]={gapGetParam_packet_default,gapGetParam_packet_default,gapGetParam_packet_default,gapGetParam_packet_default};
    gapEstablishLinkRequest_packet pckEstablish=gapEstablishLinkRequest_packet_default;
    attWriteReq_packet pckCharConf=attWriteReq_packet_default;
    gattWriteNoRsp_TE12_packet pckTEFACENA=gattWriteNoRsp_TEFACENA_packet_default;
    gattWriteNoRsp_TE9_packet pckTEVOLMAX=gattWriteNoRsp_TEVOLMAX_packet_default;
    gattWriteNoRsp_TE9_packet pckTELEFTENA=gattWriteNoRsp_TELEFTENA_packet_default;
    gattWriteNoRsp_TE9_packet pckTERIGHTENA=gattWriteNoRsp_TERIGHTENA_packet_default;
    gattWriteNoRsp_TE9_packet pckTEBOTHENA=gattWriteNoRsp_TEBOTHENA_packet_default;
    gattWriteNoRsp_TE8_packet pckTEFACDIS=gattWriteNoRsp_TEFACDIS_packet_default;
    gapTerminateLinkRequest_packet pckTerminate=gapTerminateLinkRequest_packet_default;

    if (argc < 4) {
        printHelp(argv[0]);
        return (1);
    }
    pComPortStr=argv[1];
    pMacAddrStr=argv[2];

    //reference length = mac add length * 2chars + delimiters in between
    if (strlen(pMacAddrStr) != (BD_ADDR_LEN*2)+(BD_ADDR_LEN-1)) {
        printf("Error: wrong length=%d of macAddrStr=%s\n",strlen(pMacAddrStr),pMacAddrStr);
        return (1);
    }
    for (i=0;i!=BD_ADDR_LEN;i++) {
        tmpMacElem=strtol(pMacAddrStr+3*i,NULL,16);
        macAddr[BD_ADDR_LEN-i-1]=(unsigned char) tmpMacElem;
    }

    //SERIAL OPEN
    //TODO string length check
    sprintf(serialStr,"\\\\.\\%s",pComPortStr);
    serialHandle=openSerial(serialStr);
    if (!serialHandle) exit (1);


    //TEXAS INIT
    //TI Init packets
    pckGetParam[0].paramId=0x15;
    pckGetParam[1].paramId=0x16;
    pckGetParam[2].paramId=0x1A;
    pckGetParam[3].paramId=0x19;
    writeUartHciPacket(serialHandle, &pckDevInit, sizeof(pckDevInit));
    readUartHciPacketStdOut(serialHandle);
    writeUartHciPacket(serialHandle, &pckGetParam[0], sizeof(pckGetParam[0]));
    writeUartHciPacket(serialHandle, &pckGetParam[1], sizeof(pckGetParam[1]));
    writeUartHciPacket(serialHandle, &pckGetParam[2], sizeof(pckGetParam[2]));
    writeUartHciPacket(serialHandle, &pckGetParam[3], sizeof(pckGetParam[3]));
    readUartHciPacketStdOut(serialHandle);

    //ESTABLISH CONNECTION
    //MAC address to establish
    memcpy(&pckEstablish.peerAddr,macAddr,BD_ADDR_LEN);
    writeUartHciPacket(serialHandle, &pckEstablish, sizeof(pckEstablish));
    readUartHciPacketStdOut(serialHandle);
    Sleep(DELAY_ESTABLISH);
    readUartHciPacketStdOut(serialHandle);

    //TE STUFF
    //CHARACTERISTIC CONFIGURATION
    writeUartHciPacket(serialHandle, &pckCharConf, sizeof(pckCharConf));
    readUartHciPacketStdOut(serialHandle);
    //FACTORY MODE ENABLE
    writeUartHciPacket(serialHandle, &pckTEFACENA, sizeof(pckTEFACENA));
    Sleep(DELAY_TE_COMMAND);
    readUartHciPacketStdOut(serialHandle);

    /*
    printf("   VOLMAX - sets volume to 100%\n");
    printf("   LEFT - Enables only left speakers\n");
    printf("   RIGHT - Enables only right speakers\n");
    printf("   BOTH - Enables both left and right speakers\n");
    */
    for (i=3;i!=argc;i++) {
        printf("%s\n",argv[i]);
        if (strcmp(argv[i],"VOLMAX")==0) writeUartHciPacket(serialHandle, &pckTEVOLMAX, sizeof(pckTEVOLMAX));
        if (strcmp(argv[i],"LEFT")==0) writeUartHciPacket(serialHandle, &pckTELEFTENA, sizeof(pckTELEFTENA));
        if (strcmp(argv[i],"RIGHT")==0) writeUartHciPacket(serialHandle, &pckTERIGHTENA, sizeof(pckTERIGHTENA));
        if (strcmp(argv[i],"BOTH")==0) writeUartHciPacket(serialHandle, &pckTEBOTHENA, sizeof(pckTEBOTHENA));
        Sleep(DELAY_TE_COMMAND);
        readUartHciPacketStdOut(serialHandle);
    }

    //FACTRORY MODE DISABLE
    writeUartHciPacket(serialHandle, &pckTEFACDIS, sizeof(pckTEFACDIS));
    Sleep(DELAY_TE_COMMAND);
    readUartHciPacketStdOut(serialHandle);

    //TERMINATE CONNECTION
    writeUartHciPacket(serialHandle, &pckTerminate, sizeof(pckTerminate));
    readUartHciPacketStdOut(serialHandle);

    //Close COM
    CloseHandle(serialHandle);

    //main_manual();
    //main_TE_PING(pComPortStr, macAddr);
    //main_TE_auto(pComPortStr, macAddr);

    return (0);
}
