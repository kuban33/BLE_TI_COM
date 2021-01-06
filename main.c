#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include "uart.h"
#include "ble.h"
#include "te.h"

#define VERSION "beta 12"

#define STR_LEN_MAX 0xFF
#define DELAY_ESTABLISH 500
#define DELAY_SEPRVISORTIMEOUT 1000
#define MAC_STR_WDELIM (BD_ADDR_LEN*2)+(BD_ADDR_LEN-1)

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
    printf("   %s COM_PORT BLE_MAC_ADRESS COMMAND1 [COMMAND2 [COMMAND3 [COMMAND4]]]\n",arg0);
    printf("   %s --help\n",arg0);
    printf("   %s --version\n",arg0);
    printf("COM port COM_PORT:\n");
    printf("   Win device COM port name (see examples below)\n");
    printf("BLE MAC address BLE_MAC_ADRESS:\n");
    printf("   6-hex byte address separated by whatever character (see examples below)\n");
    printf("Commands COMMANDx:\n");
    printf("   FACTORYENABLE - Puts spider into factory mode\n");
    printf("   FACTORYDISABLE - Puts spider out of factory mode\n");
    printf("   PING - Pings spider and confirms response\n");
    printf("   VOLMAX (*) - Sets volume to 100%%\n");
    printf("   LEFT (*) - Enables only left speakers\n");
    printf("   RIGHT (*) - Enables only right speakers\n");
    printf("   BOTH (*) - Enables both left and right speakers\n");
    printf("   CHARGE (*) - Enables charging mode (caution disables factory mode)\n");
    printf("   CHARGEX (*) - Disables charging mode\n");
    printf("   SOURCEADC (*) - Selects Line-In source\n");
    printf("   SOURCEFM (*) - Selects Radio source\n");
    printf("   BLEX (*^) - Disables audio bluetooth\n");
    printf("   RESET (^) - included (FACTORYENABLE,CHARGEX,FACTORYDISABLE)\n");
    printf("Notes:\n");
    printf("   Commands marked with (*) requires factory mode enabled - FACTORYENABLE\n");
    printf("   Commands marked with (^) are directly sent without TE response check\n");
    printf("Examples:\n");
    printf("   %s COM10 E2:17:4D:B8:E0:EE FACTORYENABLE VOLMAX LEFT\n",arg0);
    printf("   %s COM10 E2:17:4D:B8:E0:EE BOTH\n",arg0);
    printf("   %s COM10 E2:17:4D:B8:E0:EE FACTORYDISABLE\n",arg0);
    printf("   %s COM8 \"F9 98 16 F4 02 24\" FACTORYENABLE VOLMAX\n",arg0);
    printf("   %s COM8 \"F9 98 16 F4 02 24\" RESET\n",arg0);
    printf("\n");
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

unsigned char resetHci(HANDLE serialHandle) {
    unsigned char * buffer;
    unsigned long bufferLen;
    unsigned char overallCmdStatus;
    hciReset_packet pckHciReset=hciReset_packet_default;

    printHciPackets((unsigned char *) &pckHciReset,sizeof(pckHciReset));
    writeUart(serialHandle, &pckHciReset, sizeof(pckHciReset));
    readUart(serialHandle,&buffer,&bufferLen);
    printHciPackets(buffer,bufferLen);
    if (checkHciEventReset(buffer,bufferLen)==0) overallCmdStatus=HCI_SUCCESS;
    else overallCmdStatus=HCI_NA;

    free(buffer);

    return (overallCmdStatus);
}

unsigned char initializeBleTICC2540(HANDLE serialHandle) {
    unsigned char * buffer;
    unsigned long bufferLen;
    unsigned char * eventStatuses;
    unsigned char eventStatusesNum;
    unsigned char overallCmdStatus=0x00;
    gapDeviceInit_packet pckDevInit=gapDeviceInit_packet_default;
    gapSetParam_packet pkcSetParam=gapSetParam_packet_default;

    //TEXAS INIT
    //TI Init packets
    printHciPackets((unsigned char *) &pckDevInit,sizeof(pckDevInit));
    writeUart(serialHandle, &pckDevInit, sizeof(pckDevInit));
    readUart(serialHandle,&buffer,&bufferLen);
    overallCmdStatus|=checkHciEventPacketsStatuses(buffer,bufferLen,&eventStatuses,&eventStatusesNum);
    printHciPackets(buffer,bufferLen);
    //Supervision timeout setup 0x64 ~ 1000 ms
    printHciPackets((unsigned char *) &pkcSetParam,sizeof(pkcSetParam));
    writeUart(serialHandle, &pkcSetParam, sizeof(pkcSetParam));
    readUart(serialHandle,&buffer,&bufferLen);
    overallCmdStatus|=checkHciEventPacketsStatuses(buffer,bufferLen,&eventStatuses,&eventStatusesNum);
    printHciPackets(buffer,bufferLen);

    free(eventStatuses);
    free(buffer);

    return (overallCmdStatus);
}

unsigned char establishBLEConnection(HANDLE serialHandle, const unsigned char * macAddr, unsigned char macAddrLen) {
    unsigned char * buffer;
    unsigned long bufferLen;
    unsigned char * eventStatuses;
    unsigned char eventStatusesNum;
    unsigned char overallCmdStatus=0x00;
    gapEstablishLinkRequest_packet pckEstablish=gapEstablishLinkRequest_packet_default;

    //MAC address to establish
    memcpy(&pckEstablish.peerAddr,macAddr,BD_ADDR_LEN);
    //ESTABLISH CONNECTION
    printHciPackets((unsigned char *) &pckEstablish,sizeof(pckEstablish));
    writeUart(serialHandle, &pckEstablish, sizeof(pckEstablish));
    readUart(serialHandle,&buffer,&bufferLen);
    overallCmdStatus|=checkHciEventPacketsStatuses(buffer,bufferLen,&eventStatuses,&eventStatusesNum);
    printHciPackets(buffer,bufferLen);
    Sleep(DELAY_ESTABLISH);
    if (!readUart(serialHandle,&buffer,&bufferLen)) {
        overallCmdStatus=HCI_NA;
    }
    printHciPackets(buffer,bufferLen);

    free(eventStatuses);
    free(buffer);

    return (overallCmdStatus);
}

unsigned char terminateBLEConnection(HANDLE serialHandle, unsigned char force) {
    unsigned char * buffer;
    unsigned long bufferLen;
    unsigned char * eventStatuses;
    unsigned char eventStatusesNum;
    unsigned char overallCmdStatus=0x00;
    gapTerminateLinkRequest_packet pckTerminate=gapTerminateLinkRequest_packet_default;

    //force init Terminate
    if (force) pckTerminate=gapTerminateLinkRequest_packet_init;

    //TERMINATE CONNECTION
    printHciPackets((unsigned char *) &pckTerminate,sizeof(pckTerminate));
    writeUart(serialHandle, &pckTerminate, sizeof(pckTerminate));
    readUart(serialHandle,&buffer,&bufferLen);
    overallCmdStatus|=checkHciEventPacketsStatuses(buffer,bufferLen,&eventStatuses,&eventStatusesNum);
    printHciPackets(buffer,bufferLen);
    readUart(serialHandle,&buffer,&bufferLen);
    printHciPackets(buffer,bufferLen);

    free(eventStatuses);
    free(buffer);

    return (overallCmdStatus);
}

unsigned char terminationBLEFlush(HANDLE serialHandle) {
    unsigned char * buffer;
    unsigned long bufferLen;

    Sleep(DELAY_SEPRVISORTIMEOUT);
    readUart(serialHandle,&buffer,&bufferLen);
    printHciPackets(buffer,bufferLen);

    free(buffer);

    return (HCI_SUCCESS);
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
    unsigned char teCmdProcessed=0;
    unsigned char teCmdStatus;
    unsigned short serviceATTHandle;
    //default TE handle set first trial to PING, sedond will detect
    unsigned short charTEHandle=TE_GATT_HANDLE;
    unsigned char teDisconnectInitiated=0;

    gattWriteNoRsp_TE7_packet pckTEPING=gattWriteNoRsp_TEPING_packet_default;
    gattWriteNoRsp_TE11_packet pckTEFACENA=gattWriteNoRsp_TEFACENA_packet_default;
    gattWriteNoRsp_TE8_packet pckTEVOLMAX=gattWriteNoRsp_TEVOLMAX_packet_default;
    gattWriteNoRsp_TE8_packet pckTELEFTENA=gattWriteNoRsp_TELEFTENA_packet_default;
    gattWriteNoRsp_TE8_packet pckTERIGHTENA=gattWriteNoRsp_TERIGHTENA_packet_default;
    gattWriteNoRsp_TE8_packet pckTEBOTHENA=gattWriteNoRsp_TEBOTHENA_packet_default;
    gattWriteNoRsp_TE7_packet pckTEFACDIS=gattWriteNoRsp_TEFACDIS_packet_default;
    gattWriteNoRsp_TE8_packet pckTECHARGENA=gattWriteNoRsp_TECHARGENA_packet_default;
    gattWriteNoRsp_TE8_packet pckTECHARGDIS=gattWriteNoRsp_TECHARGDIS_packet_default;
    gattWriteNoRsp_TE8_packet pckTESRCADC=gattWriteNoRsp_TESRCADC_packet_default;
    gattWriteNoRsp_TE8_packet pckTESRCFM=gattWriteNoRsp_TESRCFM_packet_default;
    gattWriteNoRsp_TE8_packet pckTEBLEX=gattWriteNoRsp_TEBLEX_packet_default;

    if (argc==2 && strcmp(argv[1],"--help")==0) {
        printHelp(argv[0]);
        return (0);
    }
    if (argc==2 && strcmp(argv[1],"--version")==0) {
        printVersion(argv[0]);
        return (0);
    }
    if (argc<4 || argc>7) {
        printTryHelp(argv[0]);
        return (1);
    }
    pComPortStr=argv[1];
    pMacAddrStr=argv[2];

    //reference length = mac add length * 2chars + delimiters in between
    if (strlen(pMacAddrStr) != MAC_STR_WDELIM) {
        fprintf(stderr,"ERROR: wrong length=%I64u of macAddrStr=%s\n",strlen(pMacAddrStr),pMacAddrStr);
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

    //RESET HCI (controller, link manager, baseband, link layer)
    printf(" ... reseting HCI controller, link\n");
    if (resetHci(serialHandle)!=HCI_SUCCESS) {
        fprintf(stderr,"ERROR: Reseting HCI controller, link failed.\n");
        printf("HINT: Try to reconnect TI CC2540 Bluetooth out and back in to USB port.\n");
        printf(" ... closing serial connection\n");
        CloseHandle(serialHandle);
        return (1);
    }
    printf(" HCI reset successful.\n");

    //INITIALIZE Texas CC2540EMK-USB
    printf(" ... initializing Bluetooth Texas Instruments CC2540EMK-USB Dongle\n");
    if (initializeBleTICC2540(serialHandle)!=HCI_SUCCESS) {
        fprintf(stderr,"ERROR: Not able to BLE initialize Texas Instruments CC2540 USB Bluetooth Dongle on port=%s\n",serialStr);
        printf("HINT: Try to reconnect TI CC2540 Bluetooth out and back in to USB port.\n");
        printf(" ... closing serial connection\n");
        CloseHandle(serialHandle);
        return (1);
    }
    printf(" TI CC2540 initialization completed.\n");

    //ESTABLISH CONNECTION
    printf(" ... establishing connection to MAC address=%s \n",macAddrStr);
    if (establishBLEConnection(serialHandle,macAddr,BD_ADDR_LEN)!=HCI_SUCCESS) {
        fprintf(stderr,"ERROR: not able to establish link with MAC address=%s\n",macAddrStr);
        printf("HINT: Double check MAC address. Is spider discoverable by other BLE device?\n");
        printf(" ... terminating BLE connection\n");
        terminateBLEConnection(serialHandle,1);
        printf(" ... closing serial connection\n");
        CloseHandle(serialHandle);
        return (1);
    }
    printf(" Connection established.\n");

    //TE SERVICE DISCOVERY
    printf(" ... discovering TE BLE service\n");
    if (getTEService(serialHandle, &serviceATTHandle)!=HCI_SUCCESS) {
        fprintf(stderr,"ERROR: not able to discover TE BLE service with UUID=0x00,0x00,0x00,0xAD,0xBB,0xDA,0xBA,0xAB,0xE3,0x4D,0x00,0x6E,0x0A,0xE1,0xA6,0x10\n");
        printf("HINT: Possible wrong piece.\n");
        printf(" ... terminating BLE connection\n");
        terminateBLEConnection(serialHandle,0);
        printf(" ... closing serial connection\n");
        CloseHandle(serialHandle);
        return (1);
    }
    printf(" TE BLE service found at handle=0x%04X\n",serviceATTHandle);

    //TE SERVICE CONFIGURATION
    printf(" ... configuring TE BLE service\n");
    if (configureTECharacteristics(serialHandle,serviceATTHandle)!=HCI_SUCCESS) {
        fprintf(stderr,"ERROR: not able to configure TE service in Dev with MAC=%s\n",macAddrStr);
        printf("HINT: Possible wrong piece.\n");
        printf(" ... terminating BLE connection\n");
        terminateBLEConnection(serialHandle,0);
        printf(" ... closing serial connection\n");
        CloseHandle(serialHandle);
        return (1);
    }

    //TE CONFIRM DEFAULT HANDLE
    printf(" ... confirming TE BLE default characteristics handle\n");
    if (confirmDefaultHandleViaTEPING(serialHandle)) {
        printf(" TE BLE default characteristics handle=0x%04X CONFIRMED\n",charTEHandle);
    }
    else {
        printf(" TE BLE default characteristics handle=0x%04X NOT CONFIRMED\n",charTEHandle);
        //TE CHARACTERISTICS DISCOVERY
        printf(" ... discovering TE BLE characteristics\n");
        if (getTECharDesc(serialHandle, &charTEHandle)!=HCI_SUCCESS) {
            fprintf(stderr,"ERROR: not able to discover TE BLE characteristics with UUID=0x3B,0x21,0xA8,0x25,0x85,0x20,0x75,0x95,0x97,0x4B,0x09,0x64,0xEB,0x2A,0x83,0xC1\n");
            printf("HINT: Possible wrong piece.\n");
            printf(" ... terminating BLE connection\n");
            terminateBLEConnection(serialHandle,0);
            printf(" ... closing serial connection\n");
            CloseHandle(serialHandle);
            return (1);
        }
        printf(" TE BLE characteristics found at handle=0x%04X\n",charTEHandle);
    }

    //TE COMMANDS section
    for (i=3;i!=argc;i++) {
        printf("TE command=%s\n",argv[i]);
        if (strcmp(argv[i],"FACTORYENABLE")==0) {
            teCmdStatus=processntestTECommand(serialHandle,(unsigned char *) &pckTEFACENA,sizeof(pckTEFACENA),charTEHandle,compareTEFACENA);
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"FACTORYDISABLE")==0) {
            teCmdStatus=processntestTECommand(serialHandle,(unsigned char *) &pckTEFACDIS,sizeof(pckTEFACDIS),charTEHandle,compareTEFACDIS);
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"PING")==0) {
            teCmdStatus=processntestTECommand(serialHandle,(unsigned char *) &pckTEPING,sizeof(pckTEPING),charTEHandle,compareTEPING);
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"VOLMAX")==0) {
            teCmdStatus=processntestTECommand(serialHandle,(unsigned char *) &pckTEVOLMAX,sizeof(pckTEVOLMAX),charTEHandle,compareTEVOLMAX);
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"LEFT")==0) {
            teCmdStatus=processntestTECommand(serialHandle,(unsigned char *) &pckTELEFTENA,sizeof(pckTELEFTENA),charTEHandle,compareTELEFTENA);
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"RIGHT")==0) {
            teCmdStatus=processntestTECommand(serialHandle,(unsigned char *) &pckTERIGHTENA,sizeof(pckTERIGHTENA),charTEHandle,compareTERIGHTENA);
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"BOTH")==0){
            teCmdStatus=processntestTECommand(serialHandle,(unsigned char *) &pckTEBOTHENA,sizeof(pckTEBOTHENA),charTEHandle,compareTEBOTHENA);
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"CHARGE")==0) {
            teCmdStatus=processntestTECommand(serialHandle,(unsigned char *) &pckTECHARGENA,sizeof(pckTECHARGENA),charTEHandle,compareTECHARGENA);
            teCmdProcessed=1;
            teDisconnectInitiated=1;
        }
        else if (strcmp(argv[i],"CHARGEX")==0) {
            teCmdStatus=processntestTECommand(serialHandle,(unsigned char *) &pckTECHARGDIS,sizeof(pckTECHARGDIS),charTEHandle,compareTECHARGDIS);
            teCmdProcessed=1;
            teDisconnectInitiated=1;
        }
        else if (strcmp(argv[i],"SOURCEADC")==0) {
            teCmdStatus=processntestTECommand(serialHandle,(unsigned char *) &pckTESRCADC,sizeof(pckTESRCADC),charTEHandle,compareTESRCADC);
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"SOURCEFM")==0) {
            teCmdStatus=processntestTECommand(serialHandle,(unsigned char *) &pckTESRCFM,sizeof(pckTESRCFM),charTEHandle,compareTESRCFM);
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"BLEX")==0) {
            teCmdStatus=processTECommand(serialHandle,(unsigned char *) &pckTEBLEX,sizeof(pckTEBLEX),charTEHandle);
            teCmdProcessed=1;
        }
        else if (strcmp(argv[i],"RESET")==0) {
            //bitwise OR in order to group cmd results
            teCmdStatus=processTECommand(serialHandle,(unsigned char *) &pckTEFACENA,sizeof(pckTEFACENA),charTEHandle);
            teCmdStatus|=processTECommand(serialHandle,(unsigned char *) &pckTECHARGDIS,sizeof(pckTECHARGDIS),charTEHandle);
            teCmdStatus|=processTECommand(serialHandle,(unsigned char *) &pckTEFACDIS,sizeof(pckTEFACDIS),charTEHandle);
            teCmdProcessed=1;
            //teDisconnectInitiated=1;
            //not fully correct but what to do when there is TE bug on CHARGEX cmd
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

    if (teDisconnectInitiated && teCmdStatus==HCI_SUCCESS) {
        //TE TEMINATION INITIATED
        printf(" ... BLE connection was terminated by TE\n");
        terminationBLEFlush(serialHandle);
    }
    else {
        //TERMINATE CONNECTION
        printf(" ... terminating BLE connection\n");
        terminateBLEConnection(serialHandle,0);
    }
    //Close COM
    printf(" ... closing serial connection\n");
    CloseHandle(serialHandle);

    return (teCmdStatus);
}

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

unsigned char testGetTEService () {
    //DVT2
    //unsigned char macAddr[]={0x4A,0xE8,0x6F,0x6F,0x80,0xF4};
    //char macAddrStr[]="F4:80:6F:6F:E8:4A";
    //DVT3c
    unsigned char macAddr[]={0x05,0xD5,0x81,0xB4,0x7D,0xC9};
    char macAddrStr[]="C9:7D:B4:81:D5:05";
    char serialStr[]="\\\\.\\COM10";
    HANDLE serialHandle;
    unsigned short serviceATTHandle;
    unsigned short teATTHandle;

    //SERIAL OPEN
    //TODO string length check
    printf(" ... opening serial connection to TI BLE on port=%s\n",serialStr);
    serialHandle=openSerial(serialStr);
    if (!serialHandle) {
        fprintf(stderr,"ERROR: BLE not able to open serial on port=%s\n",serialStr);
        printf("HINT: Check on which COM port of computer is TI CC2540 USB Bluetooth Dongle connected? Or isn't COM port occupied by other process?\n");
        return (1);
    }
    printf(" Serial connection opened.\n");

    //RESET HCI (controller, link manager, baseband, link layer)
    printf(" ... reseting HCI controller, link\n");
    if (resetHci(serialHandle)!=HCI_SUCCESS) {
        fprintf(stderr,"ERROR: Reseting HCI controller, link failed.\n");
        printf("HINT: Try to reconnect TI CC2540 Bluetooth out and back in to USB port.\n");
        printf(" ... closing serial connection\n");
        CloseHandle(serialHandle);
        return (1);
    }
    printf(" HCI reset successful.\n");

    //INITIALIZE Texas CC2540EMK-USB
    printf(" ... initializing Bluetooth Texas Instruments CC2540EMK-USB Dongle\n");
    if (initializeBleTICC2540(serialHandle)!=HCI_SUCCESS) {
        fprintf(stderr,"ERROR: Not able to BLE initialize Texas Instruments CC2540 USB Bluetooth Dongle on port=%s\n",serialStr);
        printf("HINT: Try to reconnect TI CC2540 Bluetooth out and back in to USB port.\n");
        printf(" ... closing serial connection\n");
        CloseHandle(serialHandle);
        return (1);
    }
    printf(" TI CC2540 initialization completed.\n");

    //ESTABLISH CONNECTION
    printf(" ... establishing connection to MAC address=%s \n",macAddrStr);
    if (establishBLEConnection(serialHandle,macAddr,BD_ADDR_LEN)!=HCI_SUCCESS) {
        fprintf(stderr,"ERROR: not able to establish link with MAC address=%s\n",macAddrStr);
        printf("HINT: Double check MAC address. Is spider discoverable by other BLE device?\n");
        printf(" ... terminating BLE connection\n");
        terminateBLEConnection(serialHandle,1);
        printf(" ... closing serial connection\n");
        CloseHandle(serialHandle);
        return (1);
    }
    printf(" Connection established.\n");

    //TE SERVICE DISCOVERY
    printf(" ... discovering TE BLE service\n");
    if (getTEService(serialHandle, &serviceATTHandle)!=HCI_SUCCESS) {
        fprintf(stderr,"ERROR: not able to discovering TE BLE service with UUID=0x00,0x00,0x00,0xAD,0xBB,0xDA,0xBA,0xAB,0xE3,0x4D,0x00,0x6E,0x0A,0xE1,0xA6,0x10\n");
        printf("HINT: Reset TE device. Repeat. Reconnect TI CC2540 USB Dongle.\n");
        printf(" ... terminating BLE connection\n");
        terminateBLEConnection(serialHandle,0);
        printf(" ... closing serial connection\n");
        CloseHandle(serialHandle);
        return (1);
    }
    printf(" TE BLE service found at handle=0x%04X\n",serviceATTHandle);
    //CAUTION DVT2 does not support dynamic handle notification that's why this:
    teATTHandle=serviceATTHandle-1;
    printf(" TE BLE characteristic derived at handle=0x%04X\n",teATTHandle);

    //TERMINATE CONNECTION
    printf(" ... terminating BLE connection\n");
    terminateBLEConnection(serialHandle,0);
    //Close COM
    printf(" ... closing serial connection\n");
    CloseHandle(serialHandle);

    return (0);
}

unsigned char testDiscAllCharDescs() {
    //DVT2
    //unsigned char macAddr[]={0x4A,0xE8,0x6F,0x6F,0x80,0xF4};
    //char macAddrStr[]="F4:80:6F:6F:E8:4A";
    //DVT3c
    unsigned char macAddr[]={0x26,0xFD,0x85,0x76,0x8A,0xE2};
    char macAddrStr[]="E2:8A:76:85:FD:26";
    char serialStr[]="\\\\.\\COM9";
    HANDLE serialHandle;
    unsigned short serviceATTHandle;
    unsigned short charTEHandle;

    //SERIAL OPEN
    //TODO string length check
    printf(" ... opening serial connection to TI BLE on port=%s\n",serialStr);
    serialHandle=openSerial(serialStr);
    if (!serialHandle) {
        fprintf(stderr,"ERROR: BLE not able to open serial on port=%s\n",serialStr);
        printf("HINT: Check on which COM port of computer is TI CC2540 USB Bluetooth Dongle connected? Or isn't COM port occupied by other process?\n");
        return (1);
    }
    printf(" Serial connection opened.\n");

    //RESET HCI (controller, link manager, baseband, link layer)
    printf(" ... reseting HCI controller, link\n");
    if (resetHci(serialHandle)!=HCI_SUCCESS) {
        fprintf(stderr,"ERROR: Reseting HCI controller, link failed.\n");
        printf("HINT: Try to reconnect TI CC2540 Bluetooth out and back in to USB port.\n");
        printf(" ... closing serial connection\n");
        CloseHandle(serialHandle);
        return (1);
    }
    printf(" HCI reset successful.\n");

    //INITIALIZE Texas CC2540EMK-USB
    printf(" ... initializing Bluetooth Texas Instruments CC2540EMK-USB Dongle\n");
    if (initializeBleTICC2540(serialHandle)!=HCI_SUCCESS) {
        fprintf(stderr,"ERROR: Not able to BLE initialize Texas Instruments CC2540 USB Bluetooth Dongle on port=%s\n",serialStr);
        printf("HINT: Try to reconnect TI CC2540 Bluetooth out and back in to USB port.\n");
        printf(" ... closing serial connection\n");
        CloseHandle(serialHandle);
        return (1);
    }
    printf(" TI CC2540 initialization completed.\n");

    //ESTABLISH CONNECTION
    printf(" ... establishing connection to MAC address=%s \n",macAddrStr);
    if (establishBLEConnection(serialHandle,macAddr,BD_ADDR_LEN)!=HCI_SUCCESS) {
        fprintf(stderr,"ERROR: not able to establish link with MAC address=%s\n",macAddrStr);
        printf("HINT: Double check MAC address. Is spider discoverable by other BLE device?\n");
        printf(" ... terminating BLE connection\n");
        terminateBLEConnection(serialHandle,1);
        printf(" ... closing serial connection\n");
        CloseHandle(serialHandle);
        return (1);
    }
    printf(" Connection established.\n");

    //TE SERVICE DISCOVERY
    printf(" ... discovering TE BLE service\n");
    if (getTEService(serialHandle, &serviceATTHandle)!=HCI_SUCCESS) {
        fprintf(stderr,"ERROR: not able to discovering TE BLE service with UUID=0x00,0x00,0x00,0xAD,0xBB,0xDA,0xBA,0xAB,0xE3,0x4D,0x00,0x6E,0x0A,0xE1,0xA6,0x10\n");
        printf("HINT: Reset TE device. Repeat. Reconnect TI CC2540 USB Dongle.\n");
        printf(" ... terminating BLE connection\n");
        terminateBLEConnection(serialHandle,0);
        printf(" ... closing serial connection\n");
        CloseHandle(serialHandle);
        return (1);
    }
    printf(" TE BLE service found at handle=0x%04X\n",serviceATTHandle);

    //TE CHARACTERISTICS DISCOVERY
    printf(" ... discovering TE BLE characteristics\n");
    if (getTECharDesc(serialHandle, &charTEHandle)!=HCI_SUCCESS) {
        fprintf(stderr,"ERROR: while discovering all characteristic descriptions \n");
        printf("HINT: Reset TE device. Repeat. Reconnect TI CC2540 USB Dongle.\n");
        printf(" ... terminating BLE connection\n");
        terminateBLEConnection(serialHandle,0);
        printf(" ... closing serial connection\n");
        CloseHandle(serialHandle);
        return (1);
    }
    printf(" TE BLE characteristics found at handle=0x%04X\n",charTEHandle);

    //TERMINATE CONNECTION
    printf(" ... terminating BLE connection\n");
    terminateBLEConnection(serialHandle,0);
    //Close COM
    printf(" ... closing serial connection\n");
    CloseHandle(serialHandle);

    return (0);
}

unsigned char testTEcompare() {
    unsigned char testPacket[18]={0x04,0xFF,0x0F,0x1B,0x05,0x00,0x00,0x00,0x09,0x1D,0x00,0x04,0x15,0x5C,0x02,0x02,0x5A,0x00};
    unsigned char testPacket2[19]={0x04,0xFF,0x0F,0x1B,0x05,0x00,0x00,0x00,0x09,0x1D,0x00,0x07,0x15,0x1f,0x08,0x01,0x03,0x3f,0x00};
    unsigned char testPacket3[18]={0x04,0xFF,0x0F,0x1B,0x05,0x00,0x00,0x00,0x09,0x1D,0x00,0x07,0x51,0x0b,0x46,0x00,0x24,0x00};

    if (testTEpacket(testPacket,18,compareTEPING)) printf("TEST OK\n");
    else printf("TEST KO\n");
    if (testTEpacket(testPacket2,19,compareTEPING)) printf("TEST KO\n");
    else printf("TEST OK\n");
    if (testTEpacket(testPacket3,18,compareTEPING)) printf("TEST KO\n");
    else printf("TEST OK\n");

    return (0);
}

int main(int argc, char **argv) {
    return (main_PROD(argc,argv));

    //return ((int) testCheckHciEventPacketsStatuses());
    //return ((int) testGetTEService());
    //return ((int) testDiscAllCharDescs());
    //return ((int) testTEcompare());
}
