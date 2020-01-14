/*******************************************************************************
 \file  ble.c
 \brief sources for BLE
 \author SARA
 \date 20191009
*******************************************************************************/
#include "ble.h"
#include <stdlib.h>
#include <string.h>

const attWriteReq_packet attWriteReq_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type
    ATT_WRITEREQUEST, //uint16 opCode
    0x08, //uint8 dataLength
    0x0000, //uint16 connHandle;
    0x00, //uint8 signature;
    0x00, //uint8 command;
    0x0018, //uint16 handle;
    {0x01, 0x00} //uint8 value[0x02];
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

const gattDiscAllChars_packet gattDiscAllChars_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_DISCOVERALLCHARS, //uint16 opCode;
    0x06, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    0x0001, //uint16 startHandle;
    0xFFFF //uint16 endHandle;
};

const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TEPING_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0C, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    0x0017, //uint16 handle;
    {0x02,0x04,0x51,0x06,0x01,0x02,0x75,0x00} //uint8 value[8];
};

const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TEFACDIS_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0C, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    0x0017, //uint16 handle;
    {0x02,0x04,0x51,0x0c,0x07,0x02,0x8a,0x00} //uint8 value[8];
};

const gattWriteNoRsp_TE9_packet gattWriteNoRsp_TEVOLMAX_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0D, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    0x0017, //uint16 handle;
    {0x02,0x07,0x51,0x09,0x44,0x01,0x55,0x29,0x00} //uint8 value[9];
};

const gattWriteNoRsp_TE9_packet gattWriteNoRsp_TELEFTENA_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0D, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    0x0017, //uint16 handle;
    {0x02,0x07,0x51,0x0b,0x46,0x01,0x10,0x24,0x00} //uint8 value[9];
};

const gattWriteNoRsp_TE9_packet gattWriteNoRsp_TERIGHTENA_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0D, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    0x0017, //uint16 handle;
    {0x02,0x07,0x51,0x0c,0x46,0x01,0x20,0x34,0x00} //uint8 value[9];
};

const gattWriteNoRsp_TE9_packet gattWriteNoRsp_TEBOTHENA_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0D, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    0x0017, //uint16 handle;
    {0x02,0x07,0x51,0x0a,0x46,0x01,0x30,0x18,0x00} //uint8 value[9];
};

const gattWriteNoRsp_TE12_packet gattWriteNoRsp_TEFACENA_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x10, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    0x0017, //uint16 handle;
    {0x02,0x0a,0x51,0x08,0x07,0x04,0xca,0xfe,0xba,0xbe,0xb7,0x00} //uint8 value[8];
};

//TODO unsigned long does not make sense when 0xFF*0xFF ~ unsigned short
unsigned char bufhcitokenize(unsigned char * buffer, unsigned long bufferLen, unsigned char *** hciPackets, unsigned char * hciPacketsLen) {
    unsigned long i;
    unsigned char * hciPcktsDetected[HCI_PACKETS_MAX];
    unsigned char hciPcktsLen=0;
    hciEvent_packetHeader * tempHciEventHeader;

    if (bufferLen < HCI_PACKET_MINSIZE) {
        *hciPackets=0;
        *hciPacketsLen=0;
        return (0);
    }
    for (i=0;i<bufferLen;i++) {
        switch (buffer[i]) {
            case HCI_PACKETTYPE_EVENT:
                if (i+sizeof(hciEvent_packetHeader) <= bufferLen) {
                    //printf("-->&buffer[i]=%p<--",&buffer[i]);
                    hciPcktsDetected[hciPcktsLen]=&buffer[i];
                    hciPcktsLen++;
                    tempHciEventHeader=(hciEvent_packetHeader *) &buffer[i];
                    i+=sizeof(hciEvent_packetHeader)-1+tempHciEventHeader->dataLength;
                    //i+=sizeof(hciEvent_packetHeader)-1+buffer[i+2];
                }
                else continue;
                break;

            default:
                //UNKNOWN packet type
                *hciPackets=0;
                *hciPacketsLen=0;
                return (0);
        }
    }
    //for (i=0;i!=hciPcktsLen;i++) printf("-->hciPcktsDetected[i]=%p<--",hciPcktsDetected[i]);
    *hciPackets=memcpy(malloc(sizeof(unsigned char *)*hciPcktsLen),hciPcktsDetected,hciPcktsLen);
    *hciPacketsLen=hciPcktsLen;
    return (hciPcktsLen);
}
