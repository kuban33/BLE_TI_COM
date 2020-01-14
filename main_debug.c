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
        return (1);
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
        return (1);
    }
    printf("BytesWritten:%lu\n",(unsigned long) bytesWritten);
}

int bufhcitok(unsigned char * buffer, unsigned char bufferLen, unsigned char ** hciTokens, unsigned char * hciTokensLen) {
    unsigned char i;
    unsigned char tokensLen=0;
    unsigned char * pToken;

    for (i=0;i!=bufferLen;i++) {
        if (*buffer==HCI_PACKETTYPE_EVENT)
            // continue
            return (0);
        else return (-1);
    }

    return (0);
}

int main_manual()
{
    //unsigned char Buffer[]={0x01,0x00,0xFE,0x26,0x08,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00};
    DWORD BytesWritten=0;
    HANDLE serialHandle;

    char key='Y';

    gapDeviceInit_packet *pgapDeviceInit_packet;
    gapDeviceDiscoveryRequest_packet pcktDiscover;
    gapGetParam_packet pckt1;
    gapEstablishLinkRequest_packet pckt2;
    gapTerminateLinkRequest_packet pckt3;
    gattDiscAllChars_packet pcktGattDiscovery;
    gattWriteNoRsp_TE8_packet pcktGattTEPING;

    pgapDeviceInit_packet=&gapDeviceInit_packet_default;

    pcktDiscover = gapDeviceDiscoveryRequest_packet_default;

    pckt1 = gapGetParam_packet_default;

    pckt2 = gapEstablishLinkRequest_packet_default;
    pckt2.peerAddr[5]=0xE2;
    pckt2.peerAddr[4]=0x17;
    pckt2.peerAddr[3]=0x4D;
    pckt2.peerAddr[2]=0xB8;
    pckt2.peerAddr[1]=0xE0;
    pckt2.peerAddr[0]=0xEE;

    pckt3 = gapTerminateLinkRequest_packet_default;
    pcktGattDiscovery = gattDiscAllChars_packet_default;
    uint16 handle;
    pcktGattTEPING = gattWriteNoRsp_TEPING_packet_default;

    serialHandle = openSerial("\\\\.\\COM10");

    // --------------- WRITE ---------------
    printf("\nPress key to send DeviceInit:\n");
    printHciCommandPacket((unsigned char *) &gapDeviceInit_packet_default,sizeof(gapDeviceInit_packet_default));
    getchar();
    if (!WriteFile(serialHandle, &gapDeviceInit_packet_default, sizeof(gapDeviceInit_packet_default), &BytesWritten, NULL)) {
    //if (!WriteFile(serialHandle, Buffer, sizeof(Buffer), &BytesWritten, NULL)) {
        printf("Error WriteFile:\n");
        return (1);
    }
    printf("BytesWritten:%lu\n",(unsigned long) BytesWritten);

    // --------------- READ ---------------
    readUartHciPacketStdOut(serialHandle);

    // --------------- WRITE ---------------
    printf("\nPress key to send GetParam packets:\n");
    pckt1.paramId=0x15;
    printHciCommandPacket((unsigned char *) &pckt1,sizeof(pckt1));
    pckt1.paramId=0x16;
    printHciCommandPacket((unsigned char *) &pckt1,sizeof(pckt1));
    pckt1.paramId=0x1A;
    printHciCommandPacket((unsigned char *) &pckt1,sizeof(pckt1));
    getchar();
    pckt1.paramId=0x15;
    if (!WriteFile(serialHandle, &pckt1, sizeof(pckt1), &BytesWritten, NULL)) {
        printf("Error WriteFile:\n");
        return (1);
    }
    printf("BytesWritten:%lu\n",(unsigned long) BytesWritten);
    // --------------- WRITE ---------------
    pckt1.paramId=0x16;
    if (!WriteFile(serialHandle, &pckt1, sizeof(pckt1), &BytesWritten, NULL)) {
        printf("Error WriteFile:\n");
        return (1);
    }
    printf("BytesWritten:%lu\n",(unsigned long) BytesWritten);
    // --------------- WRITE ---------------
    pckt1.paramId=0x1A;
    if (!WriteFile(serialHandle, &pckt1, sizeof(pckt1), &BytesWritten, NULL)) {
        printf("Error WriteFile:\n");
        return (1);
    }
    printf("BytesWritten:%lu\n",(unsigned long) BytesWritten);

    // --------------- READ ---------------
    readUartHciPacketStdOut(serialHandle);
/*
    // --------------- WRITE ---------------
    printf("\nPress key to send DeviceDiscovery:\n");
    printHciCommandPacket((unsigned char *) &pcktDiscover,sizeof(pcktDiscover));
    getchar();
    if (!WriteFile(serialHandle, &pcktDiscover, sizeof(pcktDiscover), &BytesWritten, NULL)) {
    //if (!WriteFile(serialHandle, Buffer, sizeof(Buffer), &BytesWritten, NULL)) {
        printf("Error WriteFile:\n");
        return (1);
    }
    printf("BytesWritten:%lu\n",(unsigned long) BytesWritten);

    // --------------- READ ---------------
    while (key=='Y' || key=='y') {
        readUartHciPacketStdOut(serialHandle);
        printf(">>> read next device [Y]n ? ");
        key=getchar();
        if (key!='N' && key!='n') key='Y';
    }
*/
    // --------------- WRITE ---------------
    printf("\nPress key to send EstablishLink:\n");
    printHciCommandPacket((unsigned char *) &pckt2,sizeof(pckt2));
    getchar();
    if (!WriteFile(serialHandle, &pckt2, sizeof(pckt2), &BytesWritten, NULL)) {
    //if (!WriteFile(serialHandle, Buffer, sizeof(Buffer), &BytesWritten, NULL)) {
        printf("Error WriteFile:\n");
        return (1);
    }
    printf("BytesWritten:%lu\n",(unsigned long) BytesWritten);

    // --------------- READ ---------------
    readUartHciPacketStdOut(serialHandle);

    // --------------- WRITE ---------------
    printf("\nPress key to send DiscoverAllChars:\n");
    printHciCommandPacket((unsigned char *) &gattDiscAllChars_packet_default,sizeof(gattDiscAllChars_packet_default));
    getchar();
    if (!WriteFile(serialHandle, &gattDiscAllChars_packet_default, sizeof(gattDiscAllChars_packet_default), &BytesWritten, NULL)) {
    //if (!WriteFile(serialHandle, Buffer, sizeof(Buffer), &BytesWritten, NULL)) {
        printf("Error WriteFile:\n");
        return (1);
    }
    printf("BytesWritten:%lu\n",(unsigned long) BytesWritten);

    // --------------- READ ---------------
    while (key=='Y' || key=='y') {
        readUartHciPacketStdOut(serialHandle);
        printf(">>> read next [Y]n ? ");
        key=getchar();
        if (key!='N' && key!='n') key='Y';
    }

    // -------------- HANDLE ----------------
    printf("\nEnter Handle 0xXXXX:");
    scanf("%X",&handle);
    pcktGattTEPING.handle=handle;
    getchar();


    // --------------- WRITE ---------------
    printf("\nPress key to send TE_PING GATT_WriteNoRsp:\n");
    printHciCommandPacket((unsigned char *) &pcktGattTEPING,sizeof(pcktGattTEPING));
    getchar();
    if (!WriteFile(serialHandle, &pcktGattTEPING, sizeof(pcktGattTEPING), &BytesWritten, NULL)) {
    //if (!WriteFile(serialHandle, Buffer, sizeof(Buffer), &BytesWritten, NULL)) {
        printf("Error WriteFile:\n");
        return (1);
    }
    printf("BytesWritten:%lu\n",(unsigned long) BytesWritten);

    // --------------- READ ---------------
    readUartHciPacketStdOut(serialHandle);

    // --------------- WRITE ---------------
    printf("\nPress key to send TerminateLink:\n");
    printHciCommandPacket((unsigned char *) &pckt3,sizeof(pckt3));
    getchar();
    if (!WriteFile(serialHandle, &pckt3, sizeof(pckt3), &BytesWritten, NULL)) {
    //if (!WriteFile(serialHandle, Buffer, sizeof(Buffer), &BytesWritten, NULL)) {
        printf("Error WriteFile:\n");
        return (1);
    }
    printf("BytesWritten:%lu\n",(unsigned long) BytesWritten);

    // --------------- READ ---------------
    readUartHciPacketStdOut(serialHandle);

    // --------------- READ ---------------
    readUartHciPacketStdOut(serialHandle);

    CloseHandle(serialHandle);

//*/

    return (0);
}

int main_TE_PING(char * comPortStr, unsigned char * macAddr) {
    char serialStr[50];
    HANDLE serialHandle;

    char rspStdIn[16];

    gapDeviceInit_packet pckDevInit=gapDeviceInit_packet_default;
    //why multi init
    gapGetParam_packet pckGetParam[4]={gapGetParam_packet_default,gapGetParam_packet_default,gapGetParam_packet_default,gapGetParam_packet_default};
    gapEstablishLinkRequest_packet pckEstablish=gapEstablishLinkRequest_packet_default;
    attWriteReq_packet pckCharConf=attWriteReq_packet_default;
    gattWriteNoRsp_TE8_packet pckTEPING=gattWriteNoRsp_TEPING_packet_default;
    gattWriteNoRsp_TE12_packet pckTEFACENA=gattWriteNoRsp_TEFACENA_packet_default;
    gattWriteNoRsp_TE9_packet pckTEVOLMAX=gattWriteNoRsp_TEVOLMAX_packet_default;
    gattWriteNoRsp_TE9_packet pckTELEFTENA=gattWriteNoRsp_TELEFTENA_packet_default;
    gattWriteNoRsp_TE9_packet pckTERIGHTENA=gattWriteNoRsp_TERIGHTENA_packet_default;
    gattWriteNoRsp_TE9_packet pckTEBOTHENA=gattWriteNoRsp_TEBOTHENA_packet_default;
    gattWriteNoRsp_TE8_packet pckTEFACDIS=gattWriteNoRsp_TEFACDIS_packet_default;
    gapTerminateLinkRequest_packet pckTerminate=gapTerminateLinkRequest_packet_default;

    //TI Init packets
    pckGetParam[0].paramId=0x15;
    pckGetParam[1].paramId=0x16;
    pckGetParam[2].paramId=0x1A;
    pckGetParam[3].paramId=0x19;

    //MAC address to establish
    memcpy(&pckEstablish.peerAddr,macAddr,BD_ADDR_LEN);

    sprintf(serialStr,"\\\\.\\%s",comPortStr);

    serialHandle = openSerial(serialStr);


    printf("\nPress key to send DeviceInit:\n");
    writeUartHciPacketStdOut(serialHandle, &pckDevInit, sizeof(pckDevInit));
    readUartHciPacketStdOut(serialHandle);

    printf("\nPress key to TI initialize params:\n");
    writeUartHciPacketStdOut(serialHandle, &pckGetParam[0], sizeof(pckGetParam[0]));
    printf("\nPress key to TI initialize params:\n");
    writeUartHciPacketStdOut(serialHandle, &pckGetParam[1], sizeof(pckGetParam[1]));
    printf("\nPress key to TI initialize params:\n");
    writeUartHciPacketStdOut(serialHandle, &pckGetParam[2], sizeof(pckGetParam[2]));
    //readUartHciPacketStdOut(serialHandle);
    printf("\nPress key to TI initialize params:\n");
    writeUartHciPacketStdOut(serialHandle, &pckGetParam[3], sizeof(pckGetParam[3]));
    readUartHciPacketStdOut(serialHandle);

    printf("\nPress key to send EstablishLink:\n");
    writeUartHciPacketStdOut(serialHandle, &pckEstablish, sizeof(pckEstablish));
    readUartHciPacketStdOut(serialHandle);
    printf("\nConfirm to receive EstablishLink response:");
    fgets(rspStdIn,2,stdin);
    readUartHciPacketStdOut(serialHandle);

    printf("\nPress key to set Client Characteristics Configuration:\n");
    writeUartHciPacketStdOut(serialHandle, &pckCharConf, sizeof(pckCharConf));
    readUartHciPacketStdOut(serialHandle);

    printf("\nPress key to send TE PING command:\n");
    writeUartHciPacketStdOut(serialHandle, &pckTEPING, sizeof(pckTEPING));
    printf("\nConfirm to receive TE PONG response:");
    fgets(rspStdIn,2,stdin);
    readUartHciPacketStdOut(serialHandle);

    printf("\nPress key to send TE FACTORY MODE ENABLE command:\n");
    writeUartHciPacketStdOut(serialHandle, &pckTEFACENA, sizeof(pckTEFACENA));
    printf("\nConfirm to receive TE FACTORY MODE ENABLE response:");
    fgets(rspStdIn,2,stdin);
    readUartHciPacketStdOut(serialHandle);

    printf("\nPress key to send TE VOLUME MAX command:\n");
    writeUartHciPacketStdOut(serialHandle, &pckTEVOLMAX, sizeof(pckTEVOLMAX));
    printf("\nConfirm to receive TE VOLUME MAX response:");
    fgets(rspStdIn,2,stdin);
    readUartHciPacketStdOut(serialHandle);

    printf("\nPress key to send TE LEFT ENABLE command:\n");
    writeUartHciPacketStdOut(serialHandle, &pckTELEFTENA, sizeof(pckTELEFTENA));
    printf("\nConfirm to receive TE LEFT ENABLE response:");
    fgets(rspStdIn,2,stdin);
    readUartHciPacketStdOut(serialHandle);

    printf("\nPress key to send TE RIGHT ENABLE command:\n");
    writeUartHciPacketStdOut(serialHandle, &pckTERIGHTENA, sizeof(pckTERIGHTENA));
    printf("\nConfirm to receive TE RIGHT ENABLE response:");
    fgets(rspStdIn,2,stdin);
    readUartHciPacketStdOut(serialHandle);

    printf("\nPress key to send TE BOTH ENABLE command:\n");
    writeUartHciPacketStdOut(serialHandle, &pckTEBOTHENA, sizeof(pckTEBOTHENA));
    printf("\nConfirm to receive TE BOTH ENABLE response:");
    fgets(rspStdIn,2,stdin);
    readUartHciPacketStdOut(serialHandle);

    printf("\nPress key to send TE FACTORY MODE DISABLE command:\n");
    writeUartHciPacketStdOut(serialHandle, &pckTEFACDIS, sizeof(pckTEFACDIS));
    printf("\nConfirm to receive TE FACTORY MODE DISABLE response:");
    fgets(rspStdIn,2,stdin);
    readUartHciPacketStdOut(serialHandle);

    printf("\nPress key to TerminateLink:\n");
    writeUartHciPacketStdOut(serialHandle, &pckTerminate, sizeof(pckTerminate));
    readUartHciPacketStdOut(serialHandle);


    //Close COM
    CloseHandle(serialHandle);

    return (0);
}

int main_TE_auto(char * comPortStr, unsigned char * macAddr) {
    char serialStr[50];
    HANDLE serialHandle;

    char rspStdIn[16];

    gapDeviceInit_packet pckDevInit=gapDeviceInit_packet_default;
    //why multi init
    gapGetParam_packet pckGetParam[4]={gapGetParam_packet_default,gapGetParam_packet_default,gapGetParam_packet_default,gapGetParam_packet_default};
    gapEstablishLinkRequest_packet pckEstablish=gapEstablishLinkRequest_packet_default;
    attWriteReq_packet pckCharConf=attWriteReq_packet_default;
    gattWriteNoRsp_TE8_packet pckTEPING=gattWriteNoRsp_TEPING_packet_default;
    gattWriteNoRsp_TE12_packet pckTEFACENA=gattWriteNoRsp_TEFACENA_packet_default;
    gattWriteNoRsp_TE9_packet pckTEVOLMAX=gattWriteNoRsp_TEVOLMAX_packet_default;
    gattWriteNoRsp_TE9_packet pckTELEFTENA=gattWriteNoRsp_TELEFTENA_packet_default;
    gattWriteNoRsp_TE9_packet pckTERIGHTENA=gattWriteNoRsp_TERIGHTENA_packet_default;
    gattWriteNoRsp_TE9_packet pckTEBOTHENA=gattWriteNoRsp_TEBOTHENA_packet_default;
    gattWriteNoRsp_TE8_packet pckTEFACDIS=gattWriteNoRsp_TEFACDIS_packet_default;
    gapTerminateLinkRequest_packet pckTerminate=gapTerminateLinkRequest_packet_default;
    //TI Init packets
    pckGetParam[0].paramId=0x15;
    pckGetParam[1].paramId=0x16;
    pckGetParam[2].paramId=0x1A;
    pckGetParam[3].paramId=0x19;

    //MAC address to establish
    memcpy(&pckEstablish.peerAddr,macAddr,BD_ADDR_LEN);

    sprintf(serialStr,"\\\\.\\%s",comPortStr);

    serialHandle = openSerial(serialStr);

    writeUartHciPacket(serialHandle, &pckDevInit, sizeof(pckDevInit));
    readUartHciPacketStdOut(serialHandle);

    writeUartHciPacket(serialHandle, &pckGetParam[0], sizeof(pckGetParam[0]));
    writeUartHciPacket(serialHandle, &pckGetParam[1], sizeof(pckGetParam[1]));
    writeUartHciPacket(serialHandle, &pckGetParam[2], sizeof(pckGetParam[2]));
    writeUartHciPacket(serialHandle, &pckGetParam[3], sizeof(pckGetParam[3]));
    readUartHciPacketStdOut(serialHandle);

    writeUartHciPacket(serialHandle, &pckEstablish, sizeof(pckEstablish));
    readUartHciPacketStdOut(serialHandle);
    Sleep(500);
    readUartHciPacketStdOut(serialHandle);

    writeUartHciPacket(serialHandle, &pckCharConf, sizeof(pckCharConf));
    readUartHciPacketStdOut(serialHandle);

    writeUartHciPacket(serialHandle, &pckTEFACENA, sizeof(pckTEFACENA));
    //delay
    Sleep(100);
    readUartHciPacketStdOut(serialHandle);

    writeUartHciPacket(serialHandle, &pckTEVOLMAX, sizeof(pckTEVOLMAX));
    //delay
    Sleep(100);
    readUartHciPacketStdOut(serialHandle);

    writeUartHciPacket(serialHandle, &pckTEBOTHENA, sizeof(pckTEBOTHENA));
    //delay
    Sleep(100);
    readUartHciPacketStdOut(serialHandle);

    writeUartHciPacket(serialHandle, &pckTEFACDIS, sizeof(pckTEFACDIS));
    //delay
    Sleep(100);
    readUartHciPacketStdOut(serialHandle);

    writeUartHciPacket(serialHandle, &pckTerminate, sizeof(pckTerminate));
    readUartHciPacketStdOut(serialHandle);

    //Close COM
    CloseHandle(serialHandle);

    return (0);
}


int test_opcode() {
    hciOpCode_t opcode_test;
    hciOpCode_t * popcode;
    uint16 * u16popcode;

    popcode=&opcode_test;
    u16popcode=(uint16 *)popcode;

    //0x0C2D
    opcode_test.opCode_std.ocf=45;
    opcode_test.opCode_std.ogf=3;
    printf("opcode=%04X\n",*u16popcode);
    opcode_test.opCode=0x0C2D;
    printf("opcode=%04X\n",*u16popcode);
    //0x2021
    opcode_test.opCode_std.ocf=33;
    opcode_test.opCode_std.ogf=8;
    printf("opcode=%04X\n",*u16popcode);
    opcode_test.opCode=0x2021;
    printf("opcode=%04X\n",*u16popcode);
    //0xFD96
    opcode_test.opCode_vendor.ogf=63;
    opcode_test.opCode_vendor.csg=3;
    opcode_test.opCode_vendor.command=22;
    printf("opcode=%04X\n",*u16popcode);
    opcode_test.opCode=0xFD96;
    printf("opcode=%04X\n",*u16popcode);
    return (0);
}

void printHelp(char *arg0) {
    printf("Tool for TE Spider control using Texas Instruments CC2540EMK-USB Dongle\n");
    printf("------------------\n");
    printf("What's up Buddy?\nUsage:\n");
    printf("   %s COM_PORT BLE_MAC_ADRESS COMMANDS...\n",arg0);
    printf("Commands:\n");
    printf("   VOLMAX - sets volume to 100%\n");
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

    char rspStdIn[16];

    gapDeviceInit_packet pckDevInit=gapDeviceInit_packet_default;
    //why multi init
    gapGetParam_packet pckGetParam[4]={gapGetParam_packet_default,gapGetParam_packet_default,gapGetParam_packet_default,gapGetParam_packet_default};
    gapEstablishLinkRequest_packet pckEstablish=gapEstablishLinkRequest_packet_default;
    attWriteReq_packet pckCharConf=attWriteReq_packet_default;
    gattWriteNoRsp_TE8_packet pckTEPING=gattWriteNoRsp_TEPING_packet_default;
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
        if (strcmp("VOLMAX")==0) writeUartHciPacket(serialHandle, &pckTEVOLMAX, sizeof(pckTEVOLMAX));
        if (strcmp("LEFT")==0) writeUartHciPacket(serialHandle, &pckTELEFTENA, sizeof(pckTELEFTENA));
        if (strcmp("RIGHT")==0) writeUartHciPacket(serialHandle, &pckTERIGHTENA, sizeof(pckTERIGHTENA));
        if (strcmp("BOTH")==0) writeUartHciPacket(serialHandle, &pckTEBOTHENA, sizeof(pckTEBOTHENA));
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
