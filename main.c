#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <windows.h>
#include "ble.h"

#define VERSION "beta3"

#define RX_BUFFER_MAX 0xFF
#define STR_LEN_MAX 0xFF
#define DELAY_ESTABLISH 500
#define DELAY_TE_COMMAND 100
#define MAC_STR_WDELIM (BD_ADDR_LEN*2)+(BD_ADDR_LEN-1)

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
    /*
    timeout.ReadIntervalTimeout = 10;
    timeout.ReadTotalTimeoutConstant = 10;
    timeout.ReadTotalTimeoutMultiplier = 10;
    timeout.WriteTotalTimeoutConstant = 10;
    timeout.WriteTotalTimeoutMultiplier = 5;
    */
    if (!SetCommTimeouts(serialHandle, &timeout)) {
        printf("Error SetCommTimeouts:%p\n",&timeout);
        CloseHandle(serialHandle);
        return (NULL);
    }
    return (serialHandle);
}

void printHciCommandPacket(void * ptrPacket, unsigned long sizeofPacket) {
    unsigned long i;
    unsigned char * packet=(unsigned char *) ptrPacket;

    printf("<Tx> packet:\n");
    printf("---> HCI packet size: %lu\n",sizeofPacket);
    for (i=0;i!=sizeofPacket;i++) {
        printf("%02X",packet[i]);
        if (i==0 || i==2 || i==3) printf("|");
        else printf(" ");
        if ((i+1)%0x10==0) printf("\n");
    }
    printf("\n");
    for (i=0;i!=sizeofPacket;i++) {
        if (isprint(packet[i])) printf("%c",packet[i]);
        else if (isspace(packet[i])) printf(" ");
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

unsigned char * readUartHciPacket(HANDLE serialHandle, unsigned char ** bytes, unsigned long * bytesRead) {
    unsigned char buffer[RX_BUFFER_MAX];

    if (!ReadFile(serialHandle, buffer, RX_BUFFER_MAX, bytesRead, NULL)) {
        fprintf(stderr,"Error ReadFile BLE:\n");
        *bytes=NULL;
        return NULL;
    }
    if (*bytesRead>0) {
        *bytes=memcpy(malloc(sizeof(unsigned char) * *bytesRead),buffer,*bytesRead);
        return *bytes;
    }
    else {
        *bytes=NULL;
        return NULL;
    }
}

void readUartHciPacketStdOut(HANDLE serialHandle) {
    unsigned char buffer[RX_BUFFER_MAX]={0};
    DWORD bytesRead=0;

    if (!ReadFile(serialHandle, buffer, RX_BUFFER_MAX, &bytesRead, NULL)) {
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

unsigned char convMacAddr(unsigned char * macAddr, unsigned char macAddrLen, const char delim, char * macStr, unsigned char macStrLen) {
    //returns length of converted string
    unsigned char i;
    char * iStr;

    if (macAddr==NULL || macAddrLen<1 || macStr==NULL || macStrLen<2*macAddrLen+macAddrLen) return (0);

    iStr=macStr;
    for (i=0;i!=macAddrLen-1;i++,iStr+=3) {
        sprintf(iStr,"%02X%c",macAddr[macAddrLen-i-1],delim);
    }
    sprintf(iStr,"%02X",macAddr[0]);
    iStr+=2;

    return (iStr-macStr);
}

void printVersion(char *arg0) {
    printf("%s: Version=%s\n",arg0,VERSION);
}

void printTryHelp(char *arg0) {
    fprintf(stderr,"%s: wrong input arguments\n",arg0);
    fprintf(stderr,"Try '%s --help' for more information.\n",arg0);
}

void printHelp(char *arg0) {
    printf("Tool for TE Spider control using Texas Instruments CC2540EMK-USB Dongle\n");
    printf("------------------\n");
    printf("What's up Buddy?\nUsage:\n");
    printf("   %s COM_PORT BLE_MAC_ADRESS COMMAND1 [COMMAND2 [COMMAND3]]\n",arg0);
    printf("   %s --help\n",arg0);
    printf("   %s --version\n",arg0);
    printf("Commands:\n");
    printf("   VOLMAX - sets volume to 100%%\n");
    printf("   LEFT - Enables only left speakers\n");
    printf("   RIGHT - Enables only right speakers\n");
    printf("   BOTH - Enables both left and right speakers\n");
    printf("   RESET - Out of factory mode, included (VOLMAX + BOTH)\n");
    printf("Examples:\n");
    printf("   %s COM10 E2:17:4D:B8:E0:EE VOLMAX LEFT\n",arg0);
    printf("   %s COM10 E2:17:4D:B8:E0:EE VOLMAX BOTH\n",arg0);
    printf("   %s COM8 \"F9 98 16 F4 02 24\" VOLMAX\n",arg0);
    printf("\n");
}

int main_PROD(int argc, char **argv) {
    char * pComPortStr;
    char * pMacAddrStr;
    unsigned char i;
    long tmpMacElem;
    unsigned char macAddr[BD_ADDR_LEN];
    char macAddrStr[MAC_STR_WDELIM+1];
    char serialStr[STR_LEN_MAX];
    HANDLE serialHandle;
    unsigned char * buffer;
    unsigned long bufferLen;

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
    gapTerminateLinkRequest_packet pckTerminateInit=gapTerminateLinkRequest_packet_init;

    if (argc==2 && strcmp(argv[1],"--help")==0) {
        printHelp(argv[0]);
        return (0);
    }
    if (argc==2 && strcmp(argv[1],"--version")==0) {
        printVersion(argv[0]);
        return (0);
    }
    if (argc<4 || argc>6) {
        printTryHelp(argv[0]);
        return (1);
    }
    pComPortStr=argv[1];
    pMacAddrStr=argv[2];

    //reference length = mac add length * 2chars + delimiters in between
    if (strlen(pMacAddrStr) != MAC_STR_WDELIM) {
        fprintf(stderr,"ERROR: wrong length=%d of macAddrStr=%s\n",strlen(pMacAddrStr),pMacAddrStr);
        printf("HINT: Double check entered bluetooth address? Is it in delimited format e.g.: E2:17:4D:B8:E0:EE\n");
        return (1);
    }
    for (i=0;i!=BD_ADDR_LEN;i++) {
        tmpMacElem=strtol(pMacAddrStr+3*i,NULL,16);
        macAddr[BD_ADDR_LEN-i-1]=(unsigned char) tmpMacElem;
    }
    if (!convMacAddr(macAddr,BD_ADDR_LEN,':',macAddrStr,MAC_STR_WDELIM+1)) {
        fprintf(stderr,"ERROR: cannot convert MAC back to string\n");
        return (1);
    }

    //SERIAL OPEN
    //TODO string length check
    sprintf(serialStr,"\\\\.\\%s",pComPortStr);
    printf(" ... opening serial connection to TI BLE on port=%s\n",serialStr);
    serialHandle=openSerial(serialStr);
    if (!serialHandle) {
        fprintf(stderr,"ERROR: BLE not able to open serial on port=%s\n",serialStr);
        printf("HINT: Check on which COM port of computer is TI CC2540 USB Bluetooth Dongle connected? Or isn't COM port occupied by other process?\n");
        return (1);
    }
    printf(" Serial connection opened.\n");

    //TEXAS INIT
    //TI Init packets
    pckGetParam[0].paramId=0x15;
    pckGetParam[1].paramId=0x16;
    pckGetParam[2].paramId=0x1A;
    pckGetParam[3].paramId=0x19;
    printf(" ... initializing Bluetooth Texas Instruments CC2540EMK-USB Dongle\n");
    printHciCommandPacket(&pckDevInit,sizeof(pckDevInit));
    writeUartHciPacket(serialHandle, &pckDevInit, sizeof(pckDevInit));
    readUartHciPacketStdOut(serialHandle);
    printHciCommandPacket(&pckGetParam[0],sizeof(pckGetParam[0]));
    writeUartHciPacket(serialHandle, &pckGetParam[0], sizeof(pckGetParam[0]));
    printHciCommandPacket(&pckGetParam[1],sizeof(pckGetParam[1]));
    writeUartHciPacket(serialHandle, &pckGetParam[1], sizeof(pckGetParam[1]));
    printHciCommandPacket(&pckGetParam[2],sizeof(pckGetParam[2]));
    writeUartHciPacket(serialHandle, &pckGetParam[2], sizeof(pckGetParam[2]));
    printHciCommandPacket(&pckGetParam[3],sizeof(pckGetParam[3]));
    writeUartHciPacket(serialHandle, &pckGetParam[3], sizeof(pckGetParam[3]));
    readUartHciPacketStdOut(serialHandle);
    // HANDLE rx packets?
    //printf(" Initialize completed.\n");

    //ESTABLISH CONNECTION
    //MAC address to establish
    memcpy(&pckEstablish.peerAddr,macAddr,BD_ADDR_LEN);
    printf(" ... establishing connection to MAC address=%s \n",macAddrStr);
    printHciCommandPacket(&pckEstablish,sizeof(pckEstablish));
    writeUartHciPacket(serialHandle, &pckEstablish, sizeof(pckEstablish));
    readUartHciPacketStdOut(serialHandle);
    Sleep(DELAY_ESTABLISH);
    if (!readUartHciPacket(serialHandle,&buffer,&bufferLen)) {
        fprintf(stderr,"ERROR: not able to establish link with MAC address=%s\n",macAddrStr);
        printf("HINT: Double check MAC address. Is spider discoverable by other bluetooth device?\n");
        //TI Init termination
        printHciCommandPacket(&pckTerminateInit,sizeof(pckTerminateInit));
        writeUartHciPacket(serialHandle, &pckTerminateInit, sizeof(pckTerminateInit));
        readUartHciPacketStdOut(serialHandle);
        CloseHandle(serialHandle);
        return (1);
    }
    free(buffer);
    printf(" Connection established.\n");

    //TE STUFF
    //CHARACTERISTIC CONFIGURATION
    printf(" ... configuring bluetooth characteristics, handle=0x%04X\n",pckCharConf.handle);
    printHciCommandPacket(&pckCharConf,sizeof(pckCharConf));
    writeUartHciPacket(serialHandle, &pckCharConf, sizeof(pckCharConf));
    readUartHciPacketStdOut(serialHandle);
    //FACTORY MODE ENABLE
    printf("TE command=FACTORYENABLE\n");
    printHciCommandPacket(&pckTEFACENA,sizeof(pckTEFACENA));
    writeUartHciPacket(serialHandle, &pckTEFACENA, sizeof(pckTEFACENA));
    Sleep(DELAY_TE_COMMAND);
    readUartHciPacketStdOut(serialHandle);

    /*
    printf("   VOLMAX - sets volume to 100%\n");
    printf("   LEFT - Enables only left speakers\n");
    printf("   RIGHT - Enables only right speakers\n");
    printf("   BOTH - Enables both left and right speakers\n");
    printf("   RESET - Out of factory mode, included (VOLMAX + BOTH)\n");
    */
    for (i=3;i!=argc;i++) {
        printf("TE command=%s\n",argv[i]);
        if (strcmp(argv[i],"VOLMAX")==0) {
            printHciCommandPacket(&pckTEVOLMAX,sizeof(pckTEVOLMAX));
            writeUartHciPacket(serialHandle, &pckTEVOLMAX, sizeof(pckTEVOLMAX));
        }
        if (strcmp(argv[i],"LEFT")==0) {
            printHciCommandPacket(&pckTELEFTENA,sizeof(pckTELEFTENA));
            writeUartHciPacket(serialHandle, &pckTELEFTENA, sizeof(pckTELEFTENA));
        }
        if (strcmp(argv[i],"RIGHT")==0) {
            printHciCommandPacket(&pckTERIGHTENA,sizeof(pckTERIGHTENA));
            writeUartHciPacket(serialHandle, &pckTERIGHTENA, sizeof(pckTERIGHTENA));
        }
        if (strcmp(argv[i],"BOTH")==0){
            printHciCommandPacket(&pckTEBOTHENA,sizeof(pckTEBOTHENA));
            writeUartHciPacket(serialHandle, &pckTEBOTHENA, sizeof(pckTEBOTHENA));
        }
        if (strcmp(argv[i],"RESET")==0) {
            printHciCommandPacket(&pckTEVOLMAX,sizeof(pckTEVOLMAX));
            writeUartHciPacket(serialHandle, &pckTEVOLMAX, sizeof(pckTEVOLMAX));
            Sleep(DELAY_TE_COMMAND);
            readUartHciPacketStdOut(serialHandle);
            printHciCommandPacket(&pckTEBOTHENA,sizeof(pckTEBOTHENA));
            writeUartHciPacket(serialHandle, &pckTEBOTHENA, sizeof(pckTEBOTHENA));
            Sleep(DELAY_TE_COMMAND);
            readUartHciPacketStdOut(serialHandle);
            //FACTRORY MODE DISABLE
            printHciCommandPacket(&pckTEFACDIS,sizeof(pckTEFACDIS));
            writeUartHciPacket(serialHandle, &pckTEFACDIS, sizeof(pckTEFACDIS));
        }
        Sleep(DELAY_TE_COMMAND);
        readUartHciPacketStdOut(serialHandle);
    }

    //TERMINATE CONNECTION
    printf(" ... terminating bluetooth connection\n");
    printHciCommandPacket(&pckTerminate,sizeof(pckTerminate));
    writeUartHciPacket(serialHandle, &pckTerminate, sizeof(pckTerminate));
    readUartHciPacketStdOut(serialHandle);

    //Close COM
    printf(" ... closing serial connection\n");
    CloseHandle(serialHandle);

    return (0);
}
/*
int main_DEVL(int argc, char **argv) {
    char * pComPortStr;
    char * pMacAddrStr;
    unsigned char i,j;
    long tmpMacElem;
    unsigned char macAddr[BD_ADDR_LEN];

    char serialStr[STR_LEN_MAX];
    HANDLE serialHandle;

    gapDeviceInit_packet pckDevInit=gapDeviceInit_packet_default;
    //why multi init
    gapGetParam_packet pckGetParam[4]={gapGetParam_packet_default,gapGetParam_packet_default,gapGetParam_packet_default,gapGetParam_packet_default};
    gapEstablishLinkRequest_packet pckEstablish=gapEstablishLinkRequest_packet_default;
    attWriteReq_packet pckCharConf=attWriteReq_packet_default;
    gapTerminateLinkRequest_packet pckTerminate=gapTerminateLinkRequest_packet_default;

    unsigned char * readBuffer;
    unsigned long readBufferLen;
    unsigned char ** readHciPackets;
    unsigned char readHciPacketsLen;


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
        tmpMacElem=strtol(pMacAddrStr+3*i,0,16);
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
    readBuffer = readUartHciPacket(serialHandle, &readBuffer, &readBufferLen);
    printf("readBuffer addr=%p\n",readBuffer);
    for (i=0;i!=readBufferLen;i++) printf("%02X ",readBuffer[i]);
    printf("\n");

    bufhcitokenize(readBuffer, readBufferLen, &readHciPackets, &readHciPacketsLen);
    printf("HCI packets read:%d\n",readHciPacketsLen);
    for (i=0;i!=readHciPacketsLen;i++) {
        printf("Packet addr=%p\nBytes:",readHciPackets[i]);
        for (j=0;j!=readHciPackets[i][2];j++) printf("%02X ",readHciPackets[i][j+3]);
        printf("\n");
    }


    free(readHciPackets);
    free(readBuffer);

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

    //TERMINATE CONNECTION
    writeUartHciPacket(serialHandle, &pckTerminate, sizeof(pckTerminate));
    readUartHciPacketStdOut(serialHandle);

    //Close COM
    CloseHandle(serialHandle);

    return (0);
}
*/

int main(int argc, char **argv) {
    return (main_PROD(argc,argv));
}

