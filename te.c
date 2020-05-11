/*******************************************************************************
 \file  ble.c
 \brief sources for Teenage specific BLE commands
 \author SARA
 \date 20200227
*******************************************************************************/
#include "te.h"
#include "uart.h"

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

const gattWriteNoRsp_TE9_packet gattWriteNoRsp_TECHARGENA_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0D, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    0x0017, //uint16 handle;
    {0x02,0x07,0x51,0x06,0x56,0x01,0xb9,0xcb,0x00} //uint8 value[9];
};

const gattWriteNoRsp_TE9_packet gattWriteNoRsp_TECHARGDIS_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x0D, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    0x0017, //uint16 handle;
    {0x02,0x07,0x51,0x06,0x56,0x01,0xbd,0x33,0x00} //uint8 value[9];
};

const gattWriteNoRsp_TE12_packet gattWriteNoRsp_TEFACENA_packet_default={
    HCI_PACKETTYPE_COMMAND, //uint8 type;
    GATT_WRITENORESPONSE, //uint16 opCode;
    0x10, //uint8 dataLength;
    0x0000, //uint16 connHandle;
    0x0017, //uint16 handle;
    {0x02,0x0a,0x51,0x08,0x07,0x04,0xca,0xfe,0xba,0xbe,0xb7,0x00} //uint8 value[8];
};

unsigned char configureTECharacteristics(HANDLE serialHandle) {
    unsigned char * buffer;
    unsigned long bufferLen;
    unsigned char * eventStatuses;
    unsigned char eventStatusesNum;
    unsigned char overallCmdStatus=0x00;
    attWriteReq_packet pckCharConf=attWriteReq_packet_default;

    printHciPackets((unsigned char *) &pckCharConf,sizeof(pckCharConf));
    writeUart(serialHandle, &pckCharConf, sizeof(pckCharConf));
    readUart(serialHandle,&buffer,&bufferLen);
    overallCmdStatus|=checkHciEventPacketsStatuses(buffer,bufferLen,&eventStatuses,&eventStatusesNum);
    printHciPackets(buffer,bufferLen);

    free(eventStatuses);
    free(buffer);

    return (overallCmdStatus);
}

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

