/*******************************************************************************
 \file  ble.h
 \brief definitions for BLE
 \author SARA
 \date 20191009
*******************************************************************************/
#ifndef __BLE_H__
#define __BLE_H__

#ifdef __cplusplus
extern "C"
{
#endif

//no struct padding
#pragma pack(1)

#define HCI_SUCCESS 0x00

#define HCI_PACKETTYPE_COMMAND 0x01
#define HCI_PACKETTYPE_EVENT 0x04

#define HCI_PACKET_COMMAND_MAXSIZE 0xFF

#define BD_ADDR_LEN 0x06
#define BD_RESOLVEKEY_LEN 0x10

#define ATT_WRITEREQUEST 0xFD12

#define GAP_DEVICEINIT 0xFE00
#define GAP_DEVICEDISCOVERYREQUEST 0xFE04
#define GAP_ESTABLISHLINKREQUEST 0xFE09
#define GAP_TERMINATELINKREQUEST 0xFE0A
#define GAP_GETPARAM 0xFE31

#define GATT_DISCOVERALLCHARS 0xFDB2
#define GATT_WRITENORESPONSE 0xFDB6

typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int16;
typedef unsigned short uint16;
typedef signed long int32;
typedef unsigned long uint32;

//Bluetooth device ~ MAC address
typedef uint8 bdAddr_t[BD_ADDR_LEN];
typedef uint8 rslvKey_t[BD_RESOLVEKEY_LEN];

typedef struct {
    uint8 packetType;
    void * packet;
} hci_packet;

typedef union {
    uint16 opCode;
    struct {
        uint16 ocf : 10;
        uint16 ogf : 6;
    } opCode_std;
    struct {
        uint16 command : 7;
        uint16 csg : 3;
        uint16 ogf : 6;
    } opCode_vendor;
} hciOpCode_t;

typedef struct {
    hciOpCode_t opCode;
    uint8 parameterLength;
    void * parameters;
} hciCommand_packet;

/*
typedef struct {
    uint8 eventCode;
    uint8 parameterLength;
    void * parameters;
} hciEvent_packet;
*/

typedef struct {
    uint8 eventCode;
    uint8 dataLength;
    uint16 event;
    uint8 status;
} hciEvent_packet;

typedef struct {
    uint8 type;
    uint16 opCode;
    uint8 dataLength;
    uint16 connHandle;
    uint8 signature;
    uint8 command;
    uint16 handle;
    uint8 value[0x02];
} attWriteReq_packet;
extern const attWriteReq_packet attWriteReq_packet_default;

typedef struct {
    uint8 type;
    uint16 opCode;
    uint8 dataLength;
    uint8 profileRole;
    uint8 maxScanRsps;
    uint8 irk[0x10];
    uint8 csrk[0x10];
    uint32 signCounter;
} gapDeviceInit_packet;
extern const gapDeviceInit_packet gapDeviceInit_packet_default;

typedef struct {
    uint8 type;
    uint16 opCode;
    uint8 dataLength;
    uint8 mode;
    uint8 activeScan;
    uint8 whiteList;
} gapDeviceDiscoveryRequest_packet;
extern const gapDeviceDiscoveryRequest_packet gapDeviceDiscoveryRequest_packet_default;

typedef struct {
    uint8 type;
    uint16 opCode;
    uint8 dataLength;
    uint8 highDutyCycle;
    uint8 whiteList;
    uint8 addrTypePeer;
    uint8 peerAddr[BD_ADDR_LEN];
} gapEstablishLinkRequest_packet;
extern const gapEstablishLinkRequest_packet gapEstablishLinkRequest_packet_default;

typedef struct {
    uint8 type;
    uint16 opCode;
    uint8 dataLength;
    uint16 connHandle;
    uint8 discReason;
} gapTerminateLinkRequest_packet;
extern const gapTerminateLinkRequest_packet gapTerminateLinkRequest_packet_default;

typedef struct {
    uint8 type;
    uint16 opCode;
    uint8 dataLength;
    uint8 paramId;
} gapGetParam_packet;
extern const gapGetParam_packet gapGetParam_packet_default;

typedef struct {
    uint8 type;
    uint16 opCode;
    uint8 dataLength;
    uint16 connHandle;
    uint16 startHandle;
    uint16 endHandle;
} gattDiscAllChars_packet;
extern const gattDiscAllChars_packet gattDiscAllChars_packet_default;

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

typedef struct {
    uint8 type;
    uint16 opCode;
    uint8 dataLength;
    uint16 connHandle;
    uint16 handle;
    uint8 value[12];
} gattWriteNoRsp_TE12_packet;
extern const gattWriteNoRsp_TE12_packet gattWriteNoRsp_TEFACENA_packet_default;

#ifdef __cplusplus
}
#endif

#endif // BLE_H
