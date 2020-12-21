/*******************************************************************************
 \file  ble.c
 \brief sources for Teenage specific BLE commands
 \author SARA
 \date 20200227
*******************************************************************************/
#include "te.h"
#include "uart.h"
#include <stdio.h>
#include <stddef.h>

const gattWriteNoRsp_TE7_packet gattWriteNoRsp_TEPING_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0B, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    TE_GATT_HANDLE, //uint16 handle;
    {0x04,0x51,0x06,0x01,0x02,0x75,0x00} //uint8 value[7];
};

const gattWriteNoRsp_TE7_packet gattWriteNoRsp_TEFACDIS_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0B, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    TE_GATT_HANDLE, //uint16 handle;
    {0x04,0x51,0x0c,0x07,0x02,0x8a,0x00} //uint8 value[7];
};

const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TEVOLMAX_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0C, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    TE_GATT_HANDLE, //uint16 handle;
    {0x07,0x51,0x09,0x44,0x01,0x55,0x29,0x00} //uint8 value[8];
};

const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TELEFTENA_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0C, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    TE_GATT_HANDLE, //uint16 handle;
    {0x07,0x51,0x0b,0x46,0x01,0x10,0x24,0x00} //uint8 value[8];
};

const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TERIGHTENA_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0C, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    TE_GATT_HANDLE, //uint16 handle;
    {0x07,0x51,0x0c,0x46,0x01,0x20,0x34,0x00} //uint8 value[8];
};

const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TEBOTHENA_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0C, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    TE_GATT_HANDLE, //uint16 handle;
    {0x07,0x51,0x0a,0x46,0x01,0x30,0x18,0x00} //uint8 value[8];
};

const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TECHARGENA_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0C, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    TE_GATT_HANDLE, //uint16 handle;
    {0x07,0x51,0x06,0x56,0x01,0xb9,0xcb,0x00} //uint8 value[8];
};

const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TECHARGDIS_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0C, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    TE_GATT_HANDLE, //uint16 handle;
    {0x07,0x51,0x06,0x56,0x01,0xbd,0x33,0x00} //uint8 value[8];
};

const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TESRCADC_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0C, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    TE_GATT_HANDLE, //uint16 handle;
    {0x07,0x51,0x0d,0x4d,0x01,0xab,0xad,0x00} //uint8 value[8];
};

const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TESRCFM_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0C, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    TE_GATT_HANDLE, //uint16 handle;
    {0x07,0x51,0x0c,0x4d,0x01,0xad,0x8b,0x00} //uint8 value[8];
};

const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TEBLEX_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0C, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    TE_GATT_HANDLE, //uint16 handle;
    {0x07,0x51,0x12,0x6a,0x01,0x44,0x9c,0x00} //uint8 value[8];
};

const gattWriteNoRsp_TE11_packet gattWriteNoRsp_TEFACENA_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0F, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    TE_GATT_HANDLE, //uint16 handle;
    {0x0a,0x51,0x08,0x07,0x04,0xca,0xfe,0xba,0xbe,0xb7,0x00} //uint8 value[11];
};

const hciLeExtEvent_TE7_attHndlValNotif_packet hciLeExtEvent_TEPING_attHndlValNotif_packet_default={
    HCI_PACKETTYPE_EVENT, //uint8 type;
    HCI_LE_EVENT_EXTEVENT, //uint8 eventCode;
    0x0F, //uint8 dataLength;
    ATT_HANDLEVALUENOTIFICATION, //uint16 event;
    HCI_SUCCESS, //uint8 status;
    0x0000, //uint16 connHandle;
    0x09, //uint8 pduLen;
    TE_GATT_HANDLE, //uint16 handle;
    {0x04,0x15,0x00,0x02,0x02,0x00,0x00} //uint8 value[0x10];
};

const hciLeExtEvent_TE_attFindInfoRsp_packet hciLeExtEvent_TE_attFindInfoRsp_packet_default={
    HCI_PACKETTYPE_EVENT, //uint8 type;
    HCI_LE_EVENT_EXTEVENT, //uint8 eventCode;
    0x19, //uint8 dataLength;
    ATT_FINDINFORSP, //uint16 event;
    HCI_SUCCESS, //uint8 status;
    0x0000, //uint16 connHandle;
    0x13, //uint8 pduLen;
    0x02, //uint8 format;
    0x0000, //uint16 handle; //handle unknown to detect
    {0x3b,0x21,0xa8,0x25,0x85,0x20,0x75,0x95,0x97,0x4b,0x09,0x64,0xeb,0x2a,0x83,0xc1} //uint8 value[0x10];
};

unsigned char compareTEPING(const unsigned char * packet, const unsigned char packetLen) {
    hciLeExtEvent_TE7_attHndlValNotif_packet pckVerify=hciLeExtEvent_TEPING_attHndlValNotif_packet_default;
    hciLeExtEvent_TE7_attHndlValNotif_packet * ptrPacket=(hciLeExtEvent_TE7_attHndlValNotif_packet *) packet;

    if (sizeof(pckVerify)!=packetLen) return (0);

    //seq
    pckVerify.value[2]=ptrPacket->value[2];
    //crc8
    pckVerify.value[5]=ptrPacket->value[5];
    if (memcmp(&pckVerify,ptrPacket,packetLen)==0) return (1);
    else return (0);
}

unsigned char testTEpacket(const unsigned char * packet, const unsigned char packetLen, unsigned char (* funcCompare)(const unsigned char *,const unsigned char)) {
    return (*funcCompare)(packet,packetLen);
}

unsigned char confirmDefaultHandleViaTEPING(HANDLE serialHandle) {
    unsigned char * buffer;
    unsigned long bufferLen;
    unsigned char * eventStatuses;
    unsigned char eventStatusesNum;
    unsigned char overallCmdStatus=HCI_SUCCESS;
    unsigned char ** readHciPackets=NULL;
    unsigned char readHciPacketsLen;
    unsigned char result=0;
    gattWriteNoRsp_TE7_packet pckTEPING=gattWriteNoRsp_TEPING_packet_default;;

    printHciPackets((unsigned char *) &pckTEPING,sizeof(pckTEPING));
    writeUart(serialHandle,(unsigned char *) &pckTEPING,sizeof(pckTEPING));
    Sleep(DELAY_TE_COMMAND);
    readUart(serialHandle,&buffer,&bufferLen);
    overallCmdStatus|=checkHciEventPacketsStatuses(buffer,bufferLen,&eventStatuses,&eventStatusesNum);
    if (overallCmdStatus == HCI_SUCCESS) {
        if (bufhcitokenize(buffer, bufferLen, &readHciPackets, &readHciPacketsLen)) {
            if (readHciPacketsLen==2) {
                if (testTEpacket(readHciPackets[1],buffer+bufferLen-readHciPackets[1],compareTEPING)) {
                    result=1;
                }
            }
        }
    }
    printHciPackets(buffer,bufferLen);

    free(readHciPackets);
    free(eventStatuses);
    free(buffer);

    return (result);
}

unsigned char getTEService(HANDLE serialHandle, unsigned short * handle) {
    unsigned char * buffer;
    unsigned long bufferLen;
    unsigned char * eventStatuses;
    unsigned char eventStatusesNum;
    unsigned char overallCmdStatus=HCI_SUCCESS;
    unsigned char attValueCmdStatus;
    gattDiscPrimaryServiceByUUID_packet pckDiscServUUID=gattDiscPrimaryServiceByUUID_packet_default;
    unsigned char ** readHciPackets=NULL;
    unsigned char readHciPacketsLen;
    hciEvent_packetHeader * hciEventHeader;
    attFindByTypeValueRsp_eventHeader * attFindByTypeValueRspHeader;

    printHciPackets((unsigned char *) &pckDiscServUUID,sizeof(pckDiscServUUID));
    writeUart(serialHandle, &pckDiscServUUID, sizeof(pckDiscServUUID));
    //GAP HCI Command status
    readUart(serialHandle,&buffer,&bufferLen);
    overallCmdStatus|=checkHciEventPacketsStatuses(buffer,bufferLen,&eventStatuses,&eventStatusesNum);
    printHciPackets(buffer,bufferLen);
    //ATT value
    readUart(serialHandle,&buffer,&bufferLen);
    overallCmdStatus|=checkHciEventPacketsStatuses(buffer,bufferLen,&eventStatuses,&eventStatusesNum);
    if (overallCmdStatus == HCI_SUCCESS) {
        if (bufhcitokenize(buffer, bufferLen, &readHciPackets, &readHciPacketsLen)) {
            hciEventHeader=(hciEvent_packetHeader *) readHciPackets[readHciPacketsLen-1];
            if (hciEventHeader->dataLength == 0x0A) {
                attFindByTypeValueRspHeader=(attFindByTypeValueRsp_eventHeader *) (hciEventHeader+1);
                if (attFindByTypeValueRspHeader->status == HCI_SUCCESS) {
                    *handle=(unsigned short) *((unsigned short *) ((unsigned char *)attFindByTypeValueRspHeader+hciEventHeader->dataLength-sizeof(unsigned short)));
                }
                else overallCmdStatus|=HCI_NA;
            }
            else overallCmdStatus|=HCI_NA;
        }
        else overallCmdStatus|=HCI_NA;
    }
    printHciPackets(buffer,bufferLen);
    //ATT value status
    readUart(serialHandle,&buffer,&bufferLen);
    attValueCmdStatus=checkHciEventPacketsStatuses(buffer,bufferLen,&eventStatuses,&eventStatusesNum);
    if (attValueCmdStatus == HCI_EVENT_PROCEDURECOMPLETED) overallCmdStatus|=HCI_SUCCESS;
    else overallCmdStatus|=attValueCmdStatus;
    printHciPackets(buffer,bufferLen);

    free(readHciPackets);
    free(eventStatuses);
    free(buffer);

    return (overallCmdStatus);
}

unsigned char getTECharDesc(HANDLE serialHandle, unsigned short * handle) {
    unsigned char * buffer;
    unsigned long bufferLen;
    unsigned char * eventStatuses;
    unsigned char eventStatusesNum;
    unsigned char overallCmdStatus=HCI_SUCCESS;
    gattDiscAllChars_packet pckAllDesc=gattDiscAllCharDescs_packet_default;
    unsigned char ** readHciPackets=NULL;
    unsigned char readHciPacketsLen;
    hciEvent_packetHeader * hciEventHeader;
    hciLeExtEvent_TE_attFindInfoRsp_packet pckTECharDesc=hciLeExtEvent_TE_attFindInfoRsp_packet_default;
    hciLeExtEvent_TE_attFindInfoRsp_packet * targetTECharDesc;
    unsigned char i;

    printHciPackets((unsigned char *) &pckAllDesc,sizeof(pckAllDesc));
    writeUart(serialHandle, &pckAllDesc, sizeof(pckAllDesc));
    //TODO analyze
    Sleep(DELAY_TE_CHARDESCDISC);
    readUart(serialHandle,&buffer,&bufferLen);
    overallCmdStatus|=checkHciEventPacketsStatuses(buffer,bufferLen,&eventStatuses,&eventStatusesNum);
    printHciPackets(buffer,bufferLen);
    //printf("@@@@@@ DEBUG: overallCmdStatus = 0x%02X\n",overallCmdStatus);
    if (overallCmdStatus == HCI_PROCEDURECOMPLETED) {
        readHciPacketsLen=bufhcitokenize(buffer,bufferLen,&readHciPackets,&readHciPacketsLen);
        //printf("@@@@@@ DEBUG: detected CharDesc packets = %d\n",readHciPacketsLen);
        for (i=0;i!=readHciPacketsLen;i++) {
            hciEventHeader=(hciEvent_packetHeader *) readHciPackets[i];
            //compare size of packet
            if (hciEventHeader->dataLength == pckTECharDesc.dataLength) {
                //printf("@@@@@@ DEBUG: detected packet of same size = %d\n",i);
                //compare packet if header of desired HCI_LE_ExtEvent
                if (memcmp(readHciPackets[i],&pckTECharDesc,offsetof(hciLeExtEvent_TE_attFindInfoRsp_packet, handle))==0) {
                    //printf("@@@@@@ DEBUG: detected packet of same hciLeExtEvent structure = %d\n",i);
                    targetTECharDesc=(hciLeExtEvent_TE_attFindInfoRsp_packet *) readHciPackets[i];
                    //compare uuid of characteristic
                    if (memcmp(targetTECharDesc->uuid,pckTECharDesc.uuid,sizeof(pckTECharDesc.uuid))==0) {
                        //printf("@@@@@@ DEBUG: target UUID found handle = 0x%02X\n",targetTECharDesc->handle);
                        overallCmdStatus=HCI_SUCCESS;
                        *handle=targetTECharDesc->handle;
                    }
                }
            }
        }
    }
    else overallCmdStatus|=HCI_NA;

    free(readHciPackets);
    free(eventStatuses);
    free(buffer);

    return (overallCmdStatus);
}

unsigned char configureTECharacteristics(HANDLE serialHandle, unsigned short serviceHandle) {
    unsigned char * buffer;
    unsigned long bufferLen;
    unsigned char * eventStatuses;
    unsigned char eventStatusesNum;
    unsigned char overallCmdStatus=HCI_SUCCESS;
    attWriteReq_packet pckCharConf=attWriteReq_packet_default;

    pckCharConf.handle=serviceHandle;
    printHciPackets((unsigned char *) &pckCharConf,sizeof(pckCharConf));
    writeUart(serialHandle, &pckCharConf, sizeof(pckCharConf));
    Sleep(DELAY_TE_COMMAND);
    readUart(serialHandle,&buffer,&bufferLen);
    overallCmdStatus|=checkHciEventPacketsStatuses(buffer,bufferLen,&eventStatuses,&eventStatusesNum);
    printHciPackets(buffer,bufferLen);

    free(eventStatuses);
    free(buffer);

    return (overallCmdStatus);
}

/*
unsigned char processTECommand(HANDLE serialHandle, unsigned char * packet, unsigned long packetLength) {
    unsigned char * buffer;
    unsigned long bufferLen;
    unsigned char * statuses;
    unsigned char statusesNum;
    unsigned char overallStatus;

    printHciPackets(packet,packetLength);
    writeUart(serialHandle,packet,packetLength);
    Sleep(DELAY_TE_COMMAND);
    readUart(serialHandle,&buffer,&bufferLen);
    printHciPackets(buffer,bufferLen);
    overallStatus=checkHciEventPacketsStatuses(buffer,bufferLen,&statuses,&statusesNum);

    free(buffer);
    free(statuses);

    return (overallStatus);
}
*/

unsigned char processTECommand(HANDLE serialHandle, unsigned char * packet, unsigned long packetLength, unsigned short teHandle) {
    unsigned char * buffer;
    unsigned long bufferLen;
    unsigned char * statuses;
    unsigned char statusesNum;
    unsigned char overallStatus;
    gattWriteNoRsp_packetHeader * tePacketHeader;

    tePacketHeader=(gattWriteNoRsp_packetHeader *) packet;
    tePacketHeader->handle=teHandle;

    printHciPackets(packet,packetLength);
    writeUart(serialHandle,packet,packetLength);
    Sleep(DELAY_TE_COMMAND);
    readUart(serialHandle,&buffer,&bufferLen);
    printHciPackets(buffer,bufferLen);
    overallStatus=checkHciEventPacketsStatuses(buffer,bufferLen,&statuses,&statusesNum);

    free(buffer);
    free(statuses);

    return (overallStatus);
}

