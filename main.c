#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <windows.h>
#include "ble.h"

#define VERSION "beta6"

#define RX_BUFFER_MAX 0xFF
#define STR_LEN_MAX 0xFF
#define DELAY_ESTABLISH 500
//AGRESSIVE
#define DELAY_COMMAND 250
#define DELAY_TE_COMMAND 500
//#define DELAY_COMMAND 500
//#define DELAY_TE_COMMAND 1000
#define MAC_STR_WDELIM (BD_ADDR_LEN*2)+(BD_ADDR_LEN-1)

HANDLE openSerial(const char *port) {
    // Open serial port
    HANDLE serialHandle;

    serialHandle = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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
unsigned char * readUartHciPacket(HANDLE serialHandle, unsigned char ** bytes, unsigned long * bytesRead) {
    unsigned char buffer[RX_BUFFER_MAX];

    if (!ReadFile(serialHandle, buffer, RX_BUFFER_MAX, bytesRead, NULL)) {
        fprintf(stderr,"Error ReadFile BLE:\n");
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

unsigned long writeUartHciPacket(HANDLE serialHandle, void * packet, unsigned long packetLength) {
    DWORD bytesWritten=0;
    if (!WriteFile(serialHandle, packet, packetLength, &bytesWritten, NULL)) {
        printf("Error WriteFile:\n");
        return (0);
    }
    return bytesWritten;
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

unsigned char processTECommand(HANDLE serialHandle, unsigned char * packet, unsigned long packetLength) {
    unsigned char * buffer;
    unsigned long bufferLen;
    unsigned char * statuses;
    unsigned char statusesNum;
    unsigned char overallStatus;

    printHciPackets(packet,packetLength);
    writeUartHciPacket(serialHandle,packet,packetLength);
    Sleep(DELAY_TE_COMMAND);
    readUartHciPacket(serialHandle,&buffer,&bufferLen);
    printHciPackets(buffer,bufferLen);
    overallStatus=checkHciEventPacketsStatuses(buffer,bufferLen,&statuses,&statusesNum);

    free(buffer);
    free(statuses);

    return (overallStatus);
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
    printf("   FACTORYENABLE - puts spider into factory mode\n");
    printf("   FACTORYDISABLE - puts spider out of factory mode\n");
    printf("   PING - pings spider and confirms response\n");
    printf("   VOLMAX (*) - sets volume to 100%%\n");
    printf("   LEFT (*) - Enables only left speakers\n");
    printf("   RIGHT (*) - Enables only right speakers\n");
    printf("   BOTH (*) - Enables both left and right speakers\n");
    printf("   CHARGE (*) - Enables charging mode\n");
    printf("   CHARGEX - Disables charging mode\n");
    printf("   RESET - included (FACTORYENABLE,VOLMAX,BOTH,CHARGEX,FACTORYDISABLE)\n");
    printf("Notes:\n");
    printf("   Commands marked with (*) requires factory mode enabled - FACTORYENABLE\n");
    printf("Examples:\n");
    printf("   %s COM10 E2:17:4D:B8:E0:EE FACTORYENABLE VOLMAX LEFT\n",arg0);
    printf("   %s COM10 E2:17:4D:B8:E0:EE BOTH\n",arg0);
    printf("   %s COM10 E2:17:4D:B8:E0:EE FACTORYDISABLE\n",arg0);
    printf("   %s COM8 \"F9 98 16 F4 02 24\" FACTORYENABLE VOLMAX\n",arg0);
    printf("   %s COM8 \"F9 98 16 F4 02 24\" RESET\n",arg0);
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
    unsigned char teCmdProcessed=0;
    unsigned char teCmdStatus;

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
    gattWriteNoRsp_TE9_packet pckTECHARGENA=gattWriteNoRsp_TECHARGENA_packet_default;
    gattWriteNoRsp_TE9_packet pckTECHARGDIS=gattWriteNoRsp_TECHARGDIS_packet_default;

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
    printHciPackets((unsigned char *) &pckDevInit,sizeof(pckDevInit));
    writeUartHciPacket(serialHandle, &pckDevInit, sizeof(pckDevInit));
    readUartHciPacket(serialHandle,&buffer,&bufferLen);
    printHciPackets(buffer,bufferLen);
    printHciPackets((unsigned char *) &pckGetParam[0],sizeof(pckGetParam[0]));
    writeUartHciPacket(serialHandle, &pckGetParam[0], sizeof(pckGetParam[0]));
    printHciPackets((unsigned char *) &pckGetParam[1],sizeof(pckGetParam[1]));
    writeUartHciPacket(serialHandle, &pckGetParam[1], sizeof(pckGetParam[1]));
    printHciPackets((unsigned char *) &pckGetParam[2],sizeof(pckGetParam[2]));
    writeUartHciPacket(serialHandle, &pckGetParam[2], sizeof(pckGetParam[2]));
    printHciPackets((unsigned char *) &pckGetParam[3],sizeof(pckGetParam[3]));
    writeUartHciPacket(serialHandle, &pckGetParam[3], sizeof(pckGetParam[3]));
    readUartHciPacket(serialHandle,&buffer,&bufferLen);
    printHciPackets(buffer,bufferLen);
    // HANDLE rx packets?
    //printf(" Initialize completed.\n");

    //ESTABLISH CONNECTION
    //MAC address to establish
    memcpy(&pckEstablish.peerAddr,macAddr,BD_ADDR_LEN);
    printf(" ... establishing connection to MAC address=%s \n",macAddrStr);
    printHciPackets((unsigned char *) &pckEstablish,sizeof(pckEstablish));
    writeUartHciPacket(serialHandle, &pckEstablish, sizeof(pckEstablish));
    readUartHciPacket(serialHandle,&buffer,&bufferLen);
    printHciPackets(buffer,bufferLen);
    Sleep(DELAY_ESTABLISH);
    if (!readUartHciPacket(serialHandle,&buffer,&bufferLen)) {
        printHciPackets(buffer,bufferLen);
        fprintf(stderr,"ERROR: not able to establish link with MAC address=%s\n",macAddrStr);
        printf("HINT: Double check MAC address. Is spider discoverable by other bluetooth device?\n");
        //TI Init termination
        printHciPackets((unsigned char *) &pckTerminateInit,sizeof(pckTerminateInit));
        writeUartHciPacket(serialHandle, &pckTerminateInit, sizeof(pckTerminateInit));
        readUartHciPacket(serialHandle,&buffer,&bufferLen);
        printHciPackets(buffer,bufferLen);
        CloseHandle(serialHandle);
        return (1);
    }
    printf(" Connection established.\n");

    //TE STUFF
    //CHARACTERISTIC CONFIGURATION
    printf(" ... configuring bluetooth characteristics, handle=0x%04X\n",pckCharConf.handle);
    printHciPackets((unsigned char *) &pckCharConf,sizeof(pckCharConf));
    writeUartHciPacket(serialHandle, &pckCharConf, sizeof(pckCharConf));
    readUartHciPacket(serialHandle,&buffer,&bufferLen);
    printHciPackets(buffer,bufferLen);

    //FACTORY MODE ENABLE
    /*
    printf("TE command=FACTORYENABLE\n");
    printHciPackets((unsigned char *) &pckTEFACENA,sizeof(pckTEFACENA));
    writeUartHciPacket(serialHandle, &pckTEFACENA, sizeof(pckTEFACENA));
    Sleep(DELAY_TE_COMMAND);
    readUartHciPacket(serialHandle,&buffer,&bufferLen);
    printHciPackets(buffer,bufferLen);
    */

    /*
    printf("   VOLMAX - sets volume to 100%\n");
    printf("   LEFT - Enables only left speakers\n");
    printf("   RIGHT - Enables only right speakers\n");
    printf("   BOTH - Enables both left and right speakers\n");
    printf("   RESET - Out of factory mode, included (VOLMAX + BOTH)\n");
    */
    for (i=3;i!=argc;i++) {
        printf("TE command=%s\n",argv[i]);
        if (strcmp(argv[i],"FACTORYENABLE")==0) {
            teCmdStatus=processTECommand(serialHandle,(unsigned char *) &pckTEFACENA,sizeof(pckTEFACENA));
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"FACTORYDISABLE")==0) {
            teCmdStatus=processTECommand(serialHandle,(unsigned char *) &pckTEFACDIS,sizeof(pckTEFACDIS));
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"PING")==0) {
            teCmdStatus=processTECommand(serialHandle,(unsigned char *) &pckTEPING,sizeof(pckTEPING));
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"VOLMAX")==0) {
            teCmdStatus=processTECommand(serialHandle,(unsigned char *) &pckTEVOLMAX,sizeof(pckTEVOLMAX));
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"LEFT")==0) {
            teCmdStatus=processTECommand(serialHandle,(unsigned char *) &pckTELEFTENA,sizeof(pckTELEFTENA));
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"RIGHT")==0) {
            teCmdStatus=processTECommand(serialHandle,(unsigned char *) &pckTERIGHTENA,sizeof(pckTERIGHTENA));
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"BOTH")==0){
            teCmdStatus=processTECommand(serialHandle,(unsigned char *) &pckTEBOTHENA,sizeof(pckTEBOTHENA));
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"CHARGE")==0) {
            teCmdStatus=processTECommand(serialHandle,(unsigned char *) &pckTECHARGENA,sizeof(pckTECHARGENA));
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"CHARGEX")==0) {
            teCmdStatus=processTECommand(serialHandle,(unsigned char *) &pckTECHARGDIS,sizeof(pckTECHARGDIS));
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"RESET")==0) {
            teCmdStatus+=processTECommand(serialHandle,(unsigned char *) &pckTEFACENA,sizeof(pckTEFACENA));
            teCmdStatus+=processTECommand(serialHandle,(unsigned char *) &pckTEVOLMAX,sizeof(pckTEVOLMAX));
            teCmdStatus+=processTECommand(serialHandle,(unsigned char *) &pckTEBOTHENA,sizeof(pckTEBOTHENA));
            teCmdStatus+=processTECommand(serialHandle,(unsigned char *) &pckTECHARGDIS,sizeof(pckTECHARGDIS));
            teCmdStatus+=processTECommand(serialHandle,(unsigned char *) &pckTEFACDIS,sizeof(pckTEFACDIS));
            teCmdProcessed=1;
        }
        else {
            fprintf(stderr,"ERROR: UNKNOWN TE command=%s\n",argv[i]);
            teCmdStatus=HCI_NA;
            break;
        }
        if (teCmdProcessed) {
            teCmdProcessed=0;
            printf("TE command=%s --> OVERALLSTATUS=0x%02X",argv[i],teCmdStatus);
            if (teCmdStatus==HCI_SUCCESS) {
                printf("(PASS)\n");
                continue;
            }
            else {
                printf("(FAIL)\n");
                fprintf(stderr,"ERROR: TEENAGE command=%s NOT PROCESSED --> OVERALLSTATUS=0x%02X\n",argv[i],teCmdStatus);
                break;
            }
        }
    }

    //TERMINATE CONNECTION
    printf(" ... terminating bluetooth connection\n");
    printHciPackets((unsigned char *) &pckTerminate,sizeof(pckTerminate));
    writeUartHciPacket(serialHandle, &pckTerminate, sizeof(pckTerminate));
    readUartHciPacket(serialHandle,&buffer,&bufferLen);
    printHciPackets(buffer,bufferLen);

    free(buffer);
    //Close COM
    printf(" ... closing serial connection\n");
    CloseHandle(serialHandle);

    return (teCmdStatus);
}
/*
int main_DEVLhcitokenize(int argc, char **argv) {
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
    Sleep(DELAY_COMMAND);
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
    printf("-->NEW printHciPackets\n");
    printHciPackets(readBuffer,readBufferLen);

    readBufferLen=sizeof(gapGetParam_packet)*4;
    readBuffer=(unsigned char *) realloc(readBuffer,readBufferLen);
    memcpy(readBuffer,&pckGetParam,readBufferLen);

    printf("readBuffer addr=%p\n",readBuffer);
    for (i=0;i!=readBufferLen;i++) printf("%02X ",readBuffer[i]);
    printf("\n");

    bufhcitokenize(readBuffer, readBufferLen, &readHciPackets, &readHciPacketsLen);
    printf("HCI packets GetParam:%d\n",readHciPacketsLen);
    for (i=0;i!=readHciPacketsLen;i++) {
        printf("Packet addr=%p\nBytes:",readHciPackets[i]);
        for (j=0;j!=readHciPackets[i][3];j++) printf("%02X ",readHciPackets[i][j+4]);
        printf("\n");
    }
    printf("-->NEW printHciPackets\n");
    printHciPackets(readBuffer,readBufferLen);

    //TERMINATE CONNECTION
    printHciPackets((unsigned char *) &pckTerminate,sizeof(pckTerminate));
    writeUartHciPacket(serialHandle, &pckTerminate, sizeof(pckTerminate));
    Sleep(DELAY_COMMAND);
    readBuffer = readUartHciPacket(serialHandle, &readBuffer, &readBufferLen);
    printHciPackets(readBuffer,readBufferLen);


    free(readHciPackets);
    free(readBuffer);

    //Close COM
    CloseHandle(serialHandle);

    return (0);
}
*/

/*
int mainDEVLevent(int argc, char **argv) {
    char * pComPortStr;
    char serialStr[STR_LEN_MAX];
    HANDLE serialHandle;
    OVERLAPPED o;
    DWORD dwEvtMask;
    DWORD dwMask;
    unsigned char eventHandeled;
    unsigned short eventTrials;
    unsigned char * buffer;
    unsigned long bufferLen;

    gapDeviceInit_packet pckDevInit=gapDeviceInit_packet_default;
    //why multi init
    gapGetParam_packet pckGetParam[4]={gapGetParam_packet_default,gapGetParam_packet_default,gapGetParam_packet_default,gapGetParam_packet_default};
    gapTerminateLinkRequest_packet pckTerminate=gapTerminateLinkRequest_packet_default;

    if (argc != 2) {
        printf("Daj tam enem COM!\n");
        return (1);
    }
    pComPortStr=argv[1];

    //SERIAL OPEN
    //TODO string length check
    sprintf(serialStr,"\\\\.\\%s",pComPortStr);
    serialHandle=openSerialEventMode(serialStr);
    if (!serialHandle) exit (1);

    o.hEvent=CreateEvent(NULL,1,0,NULL);
    //GetCommMask();

    //TEXAS INIT
    //TI Init packets
    pckGetParam[0].paramId=0x15;
    pckGetParam[1].paramId=0x16;
    pckGetParam[2].paramId=0x1A;
    pckGetParam[3].paramId=0x19;
    printf(" ... writing TI init packets to UART:\n");
    printHciPackets((unsigned char *) &pckDevInit,sizeof(pckDevInit));
    writeUartHciPacket(serialHandle, &pckDevInit, sizeof(pckDevInit));
    eventHandeled=0;
    eventTrials=0;
    while (eventHandeled==0 && eventTrials<1024) {
        printf("Trial num %d\n",++eventTrials);
        if (WaitCommEvent(serialHandle, &dwEvtMask, &o)) {
            if (dwEvtMask & EV_TXEMPTY) {
                printf(" >>> TX buffer emptied\n");
                eventHandeled=1;
            }
        }
        else {
            DWORD dwRet=GetLastError();
            if(dwRet==ERROR_IO_PENDING) {
                printf("I/O is pending...\n");
            }
            else printf("Wait failed with error %lu.\n", GetLastError());
        }
        printf("dwEvtMask=%08X\n",dwEvtMask);
        Sleep(100);
        ResetEvent(o.hEvent);
    }
    WaitCommEvent(serialHandle, &dwEvtMask, &o);
    printf("dwEvtMask=%08X\n",dwEvtMask);
    ResetEvent(o.hEvent);
    readUartHciPacket(serialHandle,&buffer,&bufferLen);
    //WaitCommEvent(serialHandle, &dwEvtMask, &o);
    //printf("dwEvtMask=%08X\n",dwEvtMask);
    //ResetEvent(o.hEvent);

    //TERMINATE CONNECTION
    printHciPackets((unsigned char *) &pckTerminate,sizeof(pckTerminate));
    writeUartHciPacket(serialHandle, &pckTerminate, sizeof(pckTerminate));
    Sleep(DELAY_COMMAND);
    readUartHciPacket(serialHandle,&buffer,&bufferLen);
    printHciPackets(&buffer,&bufferLen);

    //Close COM
    CloseHandle(serialHandle);

    return (0);
}
*/

unsigned char testCheckHciEventPacketsStatuses() {
    //teststream CMD,EVNT(PASS),CMD,EVNT(FAIL),CMD,EVNT(PASS)
    unsigned char testStream[63]={
        0x01,0xB6,0xFD,0x0C,0x00,0x00,0x17,0x00,0x02,0x04,0x51,0x0C,0x07,0x02,0x8A,0x00,
        0x04,0xFF,0x06,0x7F,0x06,0x00,0xB6,0xFD,0x00,
        0x01,0x09,0xFE,0x09,0x00,0x00,0x01,0x4A,0xE8,0x6F,0x6F,0x80,0xF4,
        0x04,0xFF,0x06,0x7F,0x06,0x11,0x09,0xFE,0x00,
        0x01,0x0A,0xFE,0x03,0x00,0x00,0x13,
        0x04,0xFF,0x06,0x7F,0x06,0x00,0x0A,0xFE,0x00
        };
    unsigned long testStreamLen=63;
    unsigned char testZeroStream[1]={0x00};
    unsigned long testZeroStreamLen=1;
    unsigned char * statuses;
    unsigned char statusesNum;
    unsigned char overallStatus;

    unsigned char i;

    //TEST 1 OK KO OK Events
    printHciPackets(testStream,testStreamLen);
    overallStatus=checkHciEventPacketsStatuses(testStream,testStreamLen,&statuses,&statusesNum);

    printf("overallStatus=0x%02X\n",overallStatus);
    for (i=0;i!=statusesNum;i++) printf("statuses[%u]=0x%02X\n",i,statuses[i]);

    if (statuses!=NULL && statusesNum==3 && statuses[0]==HCI_SUCCESS && statuses[1]==0x11 && statuses[2]==HCI_SUCCESS && overallStatus==0x11)
        printf("TEST PASS\n");
    else
        printf("TEST FAIL\n");

    free(statuses);

    //TEST 2 No Event
    overallStatus=checkHciEventPacketsStatuses(testZeroStream,testZeroStreamLen,&statuses,&statusesNum);

    printf("overallStatus=0x%02X\n",overallStatus);
    for (i=0;i!=statusesNum;i++) printf("statuses[%u]=0x%02X\n",i,statuses[i]);

    if (statuses==NULL && statusesNum==0 && overallStatus==HCI_NA)
        printf("TEST PASS\n");
    else
        printf("TEST FAIL\n");

    free(statuses);

    return (overallStatus);
}

int main(int argc, char **argv) {
    return (main_PROD(argc,argv));
    //return (main_DEVLhcitokenize(argc,argv));
    //return (mainDEVLevent(argc,argv));
    //return ((int) testCheckHciEventPacketsStatuses());
}

