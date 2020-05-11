/*******************************************************************************
 \file  te.h
 \brief definitions for Teenage specific BLE commands
 \author SARA
 \date 20200227
*******************************************************************************/
#ifndef __TE_H__
#define __TE_H__

#include "ble.h"
#include "uart.h"

//no struct padding
#pragma pack(1)

#define DELAY_TE_COMMAND 500

typedef struct {
    uint8 type;
    uint16 opCode;
    uint8 dataLength;
    uint16 connHandle;
    uint16 handle;
    uint8 value[8];
} gattWriteNoRsp_TE8_packet;
extern const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TEPING_packet_default;
extern const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TEFACDIS_packet_default;

typedef struct {
    uint8 type;
    uint16 opCode;
    uint8 dataLength;
    uint16 connHandle;
    uint16 handle;
    uint8 value[9];
} gattWriteNoRsp_TE9_packet;
extern const gattWriteNoRsp_TE9_packet gattWriteNoRsp_TEVOLMAX_packet_default;
extern const gattWriteNoRsp_TE9_packet gattWriteNoRsp_TELEFTENA_packet_default;
extern const gattWriteNoRsp_TE9_packet gattWriteNoRsp_TERIGHTENA_packet_default;
extern const gattWriteNoRsp_TE9_packet gattWriteNoRsp_TEBOTHENA_packet_default;
extern const gattWriteNoRsp_TE9_packet gattWriteNoRsp_TECHARGENA_packet_default;
extern const gattWriteNoRsp_TE9_packet gattWriteNoRsp_TECHARGDIS_packet_default;

typedef struct {
    uint8 type;
    uint16 opCode;
    uint8 dataLength;
    uint16 connHandle;
    uint16 handle;
    uint8 value[12];
} gattWriteNoRsp_TE12_packet;
extern const gattWriteNoRsp_TE12_packet gattWriteNoRsp_TEFACENA_packet_default;

extern unsigned char configureTECharacteristics(HANDLE serialHandle);
extern unsigned char processTECommand(HANDLE serialHandle, unsigned char * packet, unsigned long packetLength);

#endif // __TE_H__
