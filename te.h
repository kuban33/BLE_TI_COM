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

#define DELAY_TE_COMMAND 250
#define DELAY_TE_CHARDESCDISC 3000
//This is Teenage default/hardcoded handle
#define TE_GATT_HANDLE 0x001D

typedef struct {
    uint8 type;
    uint16 opCode;
    uint8 dataLength;
    uint16 connHandle;
    uint16 handle;
    uint8 value[7];
} gattWriteNoRsp_TE7_packet;
extern const gattWriteNoRsp_TE7_packet gattWriteNoRsp_TEPING_packet_default;
extern const gattWriteNoRsp_TE7_packet gattWriteNoRsp_TEFACDIS_packet_default;

typedef struct {
    uint8 type;
    uint16 opCode;
    uint8 dataLength;
    uint16 connHandle;
    uint16 handle;
    uint8 value[8];
} gattWriteNoRsp_TE8_packet;
extern const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TEVOLMAX_packet_default;
extern const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TELEFTENA_packet_default;
extern const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TERIGHTENA_packet_default;
extern const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TEBOTHENA_packet_default;
extern const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TECHARGENA_packet_default;
extern const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TECHARGDIS_packet_default;
extern const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TESRCADC_packet_default;
extern const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TESRCFM_packet_default;
extern const gattWriteNoRsp_TE8_packet gattWriteNoRsp_TEBLEX_packet_default;

typedef struct {
    uint8 type;
    uint16 opCode;
    uint8 dataLength;
    uint16 connHandle;
    uint16 handle;
    uint8 value[11];
} gattWriteNoRsp_TE11_packet;
extern const gattWriteNoRsp_TE11_packet gattWriteNoRsp_TEFACENA_packet_default;

typedef struct {
    uint8 type;
    uint8 eventCode;
    uint8 dataLength;
    uint16 event;
    uint8 status;
    uint16 connHandle;
    uint8 pduLen;
    uint16 handle;
    uint8 value[7];
} hciLeExtEvent_TE7_attHndlValNotif_packet;
extern const hciLeExtEvent_TE7_attHndlValNotif_packet hciLeExtEvent_TEPING_attHndlValNotif_packet_default;

typedef struct {
    uint8 type;
    uint8 eventCode;
    uint8 dataLength;
    uint16 event;
    uint8 status;
    uint16 connHandle;
    uint8 pduLen;
    uint16 handle;
    uint8 value[8];
} hciLeExtEvent_TE8_attHndlValNotif_packet;
extern const hciLeExtEvent_TE8_attHndlValNotif_packet hciLeExtEvent_TEFACENA_attHndlValNotif_packet_default;
extern const hciLeExtEvent_TE8_attHndlValNotif_packet hciLeExtEvent_TEFACDIS_attHndlValNotif_packet_default;
extern const hciLeExtEvent_TE8_attHndlValNotif_packet hciLeExtEvent_TEVOLMAX_attHndlValNotif_packet_default;
extern const hciLeExtEvent_TE8_attHndlValNotif_packet hciLeExtEvent_TELEFTENA_attHndlValNotif_packet_default;
extern const hciLeExtEvent_TE8_attHndlValNotif_packet hciLeExtEvent_TERIGHTENA_attHndlValNotif_packet_default;
extern const hciLeExtEvent_TE8_attHndlValNotif_packet hciLeExtEvent_TEBOTHENA_attHndlValNotif_packet_default;
extern const hciLeExtEvent_TE8_attHndlValNotif_packet hciLeExtEvent_TECHARGENA_attHndlValNotif_packet_default;
extern const hciLeExtEvent_TE8_attHndlValNotif_packet hciLeExtEvent_TECHARGDIS_attHndlValNotif_packet_default;
extern const hciLeExtEvent_TE8_attHndlValNotif_packet hciLeExtEvent_TESRCADC_attHndlValNotif_packet_default;
extern const hciLeExtEvent_TE8_attHndlValNotif_packet hciLeExtEvent_TESRCFM_attHndlValNotif_packet_default;

//packet with handle for Teenage 128-bit UUID
typedef struct {
    uint8 type;
    uint8 eventCode;
    uint8 dataLength;
    uint16 event;
    uint8 status;
    uint16 connHandle;
    uint8 pduLen;
    uint8 format;
    uint16 handle;
    uint8 uuid[0x10];
} hciLeExtEvent_TE_attFindInfoRsp_packet;
extern const hciLeExtEvent_TE_attFindInfoRsp_packet hciLeExtEvent_TE_attFindInfoRsp_packet_default;

extern unsigned char compareTEPING(const unsigned char * packet, const unsigned char packetLen);
extern unsigned char compareTEFACENA(const unsigned char * packet, const unsigned char packetLen);
extern unsigned char compareTEFACDIS(const unsigned char * packet, const unsigned char packetLen);
extern unsigned char compareTEVOLMAX(const unsigned char * packet, const unsigned char packetLen);
extern unsigned char compareTELEFTENA(const unsigned char * packet, const unsigned char packetLen);
extern unsigned char compareTERIGHTENA(const unsigned char * packet, const unsigned char packetLen);
extern unsigned char compareTEBOTHENA(const unsigned char * packet, const unsigned char packetLen);
extern unsigned char compareTECHARGENA(const unsigned char * packet, const unsigned char packetLen);
extern unsigned char compareTECHARGDIS(const unsigned char * packet, const unsigned char packetLen);
extern unsigned char compareTESRCADC(const unsigned char * packet, const unsigned char packetLen);
extern unsigned char compareTESRCFM(const unsigned char * packet, const unsigned char packetLen);
extern unsigned char testTEpacket(const unsigned char * packet, const unsigned char packetLen, unsigned char (* funcCompare)(const unsigned char *,const unsigned char));
extern unsigned char confirmDefaultHandleViaTEPING(HANDLE serialHandle);
extern unsigned char getTEService(HANDLE serialHandle, unsigned short * handle);
extern unsigned char getTECharDesc(HANDLE serialHandle, unsigned short * handle);
extern unsigned char configureTECharacteristics(HANDLE serialHandle, unsigned short serviceHandle);
extern unsigned char processTECommand(HANDLE serialHandle, unsigned char * packet, unsigned long packetLength, unsigned short teHandle);
extern unsigned char processntestTECommand(HANDLE serialHandle, unsigned char * packet, unsigned long packetLength, unsigned short teHandle, unsigned char (* funcTest)(const unsigned char *,const unsigned char));

#endif // __TE_H__
