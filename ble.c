/*******************************************************************************
 \file  ble.c
 \brief sources for BLE
 \author SARA
 \date 20191009
*******************************************************************************/
#include "ble.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

const hciReset_packet hciReset_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type
    HCI_RESET, //uint16 opCode
    0x00 //uint8 dataLength
};

const hciReset_event hciReset_event_success={
    HCI_PACKETTYPE_EVENT, //uint8 type
    HCI_EVENT_COMMANDCOMPLETE, //uint8 eventCode
    0x04, //uint8 dataLength
    0x01, //uint8 packets
    HCI_RESET, //uint16 opCode
    HCI_SUCCESS //uint8 status
};

const gapDeviceInit_packet gapDeviceInit_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type
    GAP_DEVICEINIT, //uint16 opCode
    0x26, //uint8 dataLength
    0x08, //uint8 profileRole
    0x05, //uint8 maxScanRsps
    {0x00}, //unit8 irk[0x10]
    {0x00}, //uint8 csrk[0x10]
    0x01 //unit32 signCounter
};

const gapDeviceDiscoveryRequest_packet gapDeviceDiscoveryRequest_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type
    GAP_DEVICEDISCOVERYREQUEST, //uint16 opCode
    0x03, //uint8 dataLength
    0x03, //uint8 mode;
    0x01, //uint8 activeScan;
    0x00//uint8 whiteList;
};

const gapEstablishLinkRequest_packet gapEstablishLinkRequest_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type
    GAP_ESTABLISHLINKREQUEST, //uint16 opCode
    0x09, //uint8 dataLength
    0x00, //uint8 highDutyCycle
    0x00, //uint8 whiteList
    0x01, //uint8 addrTypePeer 0x00=public, 0x01=static
    {0x00} //uint8 peerAddr[BLE_ADDR_LEN]
};

const gapTerminateLinkRequest_packet gapTerminateLinkRequest_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type
    GAP_TERMINATELINKREQUEST, //uint16 opCode
    0x03, //uint8 dataLength
    0x0000, //uint16 connHandle
    0x13 //uint8 discReason
};

const gapTerminateLinkRequest_packet gapTerminateLinkRequest_packet_init={
    HCI_PACKETTYPE_COMMAND, //uint8 type
    GAP_TERMINATELINKREQUEST, //uint16 opCode
    0x03, //uint8 dataLength
    HCI_CONNECTIONHANDLE_INIT, //uint16 connHandle
    0x13 //uint8 discReason
};

const gapGetParam_packet gapGetParam_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type
    GAP_GETPARAM, //uint16 opCode
    0x01, //uint8 dataLength
    0x00 //uint8 paramId
};

const attWriteReq_packet attWriteReq_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type
    ATT_WRITEREQUEST, //uint16 opCode
    0x08, //uint8 dataLength
    0x0000, //uint16 connHandle;
    0x00, //uint8 signature;
    0x00, //uint8 command;
    ATT_DEFHANDLE, //uint16 handle;
    {0x01, 0x00} //uint8 value[0x02];
};

const gattDiscAllChars_packet gattDiscAllChars_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_DISCOVERALLCHARS, //uint16 opCode;
    0x06, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    0x0001, //uint16 startHandle;
    0xFFFF //uint16 endHandle;
};

const gattDiscPrimaryServiceByUUID_packet gattDiscPrimaryServiceByUUID_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_DISCOVERPRIMARYSERVICEBYUUID, //uint16 opCode;
    0x12, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    {0x00,0x00,0x00,0xAD,0xBB,0xDA,0xBA,0xAB,0xE3,0x4D,0x00,0x6E,0x0A,0xE1,0xA6,0x10} //uint8 value[0x10];
};

int checkHciEventReset(unsigned char * buffer, unsigned long bufferLen) {
    hciReset_event evntHciResetSuccess=hciReset_event_success;

    //printf(">>>>>> sizeof(evntHciResetSuccess)=%d\n",sizeof(evntHciResetSuccess));
    //printf(">>>>>> (bufferLen*sizeof(*buffer))=%d\n",(bufferLen*sizeof(*buffer)));
    if (sizeof(evntHciResetSuccess)!=(bufferLen*sizeof(*buffer))) return (1);
    return (memcmp(&evntHciResetSuccess,buffer,sizeof(evntHciResetSuccess)));
};

//TODO unsigned long does not make sense when 0xFF*0xFF ~ unsigned short
unsigned char bufhcitokenize(unsigned char * buffer, unsigned long bufferLen, unsigned char *** hciPackets, unsigned char * hciPacketsNum) {
    unsigned long i;
    unsigned char * hciPcktsDetected[HCI_PACKETS_MAX];
    unsigned char hciPcktsNum=0;
    hciEvent_packetHeader * hciEventHeader;
    hciCommand_packetHeader * hciCommandHeader;

    if (bufferLen < HCI_PACKET_MINSIZE) {
        *hciPackets=NULL;
        *hciPacketsNum=0;
        return (0);
    }
    for (i=0;i<bufferLen;i++) {
        switch (buffer[i]) {
            case HCI_PACKETTYPE_EVENT:
                if (i+sizeof(hciEvent_packetHeader) <= bufferLen) {
                    //printf("-->&buffer[i]=%p<--",&buffer[i]);
                    hciPcktsDetected[hciPcktsNum]=&buffer[i];
                    hciPcktsNum++;
                    hciEventHeader=(hciEvent_packetHeader *) &buffer[i];
                    i+=sizeof(hciEvent_packetHeader)-1+hciEventHeader->dataLength;
                }
                else continue;
                break;

            case HCI_PACKETTYPE_COMMAND:
                if (i+sizeof(hciCommand_packetHeader) <= bufferLen) {
                    //printf("-->&buffer[i]=%p<--",&buffer[i]);
                    hciPcktsDetected[hciPcktsNum]=&buffer[i];
                    hciPcktsNum++;
                    hciCommandHeader=(hciCommand_packetHeader *) &buffer[i];
                    i+=sizeof(hciCommand_packetHeader)-1+hciCommandHeader->dataLength;
                }
                else continue;
                break;

            default:
                //UNKNOWN packet type
                *hciPackets=NULL;
                *hciPacketsNum=0;
                return (0);
        }
    }
    //realloc behaves undefined for uninitialized pointer
    //if (*hciPackets!=NULL) { free(*hciPackets); *hciPackets=NULL; }
    *hciPackets=memcpy(malloc(sizeof(**hciPackets)*hciPcktsNum),hciPcktsDetected,sizeof(*hciPcktsDetected)*hciPcktsNum);
    *hciPacketsNum=hciPcktsNum;
    return (hciPcktsNum);
}

void printHciPackets(unsigned char * ptrStream, unsigned long sizeofStream) {
    unsigned char i;
    unsigned char j;
    unsigned char ** readHciPackets=NULL;
    unsigned char readHciPacketsLen;
    hci_packetHeader * currentPacketHeader;
    unsigned char currentPacketLen;

    bufhcitokenize(ptrStream, sizeofStream, &readHciPackets, &readHciPacketsLen);
    for (j=0;j!=readHciPacketsLen;j++) {
        if (j+1!=readHciPacketsLen) currentPacketLen=readHciPackets[j+1]-readHciPackets[j];
        else currentPacketLen=readHciPackets[0]+sizeofStream-readHciPackets[j];
        currentPacketHeader=(hci_packetHeader *) readHciPackets[j];
        switch (currentPacketHeader->packetType) {
        case HCI_PACKETTYPE_EVENT :
            printf("--> HCI EVENT packet size: %hu\n",currentPacketLen);
            for (i=0;i!=currentPacketLen;i++) {
                printf("%02X",readHciPackets[j][i]);
                if (i==0 || i==1 || i==2 || i==4 || i==5) printf("|");
                else printf(" ");
                if ((i+1)%0x10==0) printf("\n");
            }
            printf("\n");
            for (i=0;i!=currentPacketLen;i++) {
                if (isprint(readHciPackets[j][i])) printf("%c",readHciPackets[j][i]);
                else if (isspace(readHciPackets[j][i])) printf(" ");
                else printf(".");
            }
            printf("\n");
            break;
        case HCI_PACKETTYPE_COMMAND :
            printf("--> HCI COMMAND packet size: %hu\n",currentPacketLen);
            for (i=0;i!=currentPacketLen;i++) {
                printf("%02X",readHciPackets[j][i]);
                if (i==0 || i==2 || i==3) printf("|");
                else printf(" ");
                if ((i+1)%0x10==0) printf("\n");
            }
            printf("\n");
            for (i=0;i!=currentPacketLen;i++) {
                if (isprint(readHciPackets[j][i])) printf("%c",readHciPackets[j][i]);
                else if (isspace(readHciPackets[j][i])) printf(" ");
                else printf(".");
            }
            printf("\n");
            break;
        default :
            //UNKNOWN packet type
            break;
        }
    }
    free(readHciPackets);
}

unsigned char checkHciEventPacketsStatuses(unsigned char * ptrStream, unsigned long sizeofStream, unsigned char ** statuses, unsigned char * statusesNum ) {
    unsigned char i;
    unsigned char j;
    unsigned char ** readHciPackets=NULL;
    unsigned char readHciPacketsLen;
    hciEvent_packetWStatus * currentPacketEventStatus;
    unsigned char currentPacketLen;
    unsigned char * eventStatuses;
    unsigned char eventStatusesNum=0;

    bufhcitokenize(ptrStream, sizeofStream, &readHciPackets, &readHciPacketsLen);
    eventStatuses=(unsigned char *) malloc(sizeof(*eventStatuses) * readHciPacketsLen);
    for (j=0;j!=readHciPacketsLen;j++) {
        if (j+1!=readHciPacketsLen) currentPacketLen=readHciPackets[j+1]-readHciPackets[j];
        else currentPacketLen=readHciPackets[0]+sizeofStream-readHciPackets[j];
        if (currentPacketLen >= sizeof(hciEvent_packetWStatus)) {
            currentPacketEventStatus=(hciEvent_packetWStatus *) readHciPackets[j];
            if (currentPacketEventStatus->header.packetType==HCI_PACKETTYPE_EVENT) eventStatuses[eventStatusesNum++]=currentPacketEventStatus->status;
        }
    }
    free(readHciPackets);

    eventStatuses=(unsigned char *) realloc(eventStatuses,sizeof(*eventStatuses) * eventStatusesNum);
    *statuses=eventStatuses;
    *statusesNum=eventStatusesNum;

    for (i=0;i!=eventStatusesNum;i++) if (eventStatuses[i]!=HCI_SUCCESS) return (eventStatuses[i]);
    if (i>0) return (HCI_SUCCESS);
    return (HCI_NA);
}
