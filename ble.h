/*******************************************************************************
 \file  ble.h
 \brief definitions for BLE
 \author SARA
 \date 20191009
*******************************************************************************/
#ifndef __BLE_H__
#define __BLE_H__

//no struct padding
#pragma pack(1)

#define HCI_RESET 0x0C03
#define HCI_CONNECTIONHANDLE_INIT 0xFFFE
#define HCI_PACKETS_MAX 0xFF
#define HCI_PACKETTYPE_COMMAND 0x01
#define HCI_PACKETTYPE_EVENT 0x04
#define HCI_PACKET_MINSIZE 0x03
#define HCI_PACKET_MAXSIZE 0xFF
#define HCI_PACKET_COMMAND_MINSIZE 0x04
#define HCI_PACKET_EVENT_MINSIZE 0x03
#define HCI_EVENT_COMMANDCOMPLETE 0x0E
#define HCI_SUCCESS 0x00
#define HCI_NA 0xFF

#define BD_ADDR_LEN 0x06
#define BD_RESOLVEKEY_LEN 0x10

#define ATT_WRITEREQUEST 0xFD12
//This is Teenage hardcoded handle
#define ATT_DEFHANDLE 0x001A

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
} hci_packetHeader;

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
    uint8 packetType;
    hciOpCode_t opCode;
    uint8 dataLength;
} hciCommand_packetHeader;

typedef struct {
    uint8 packetType;
    uint8 eventCode;
    uint8 dataLength;
} hciEvent_packetHeader;

typedef struct {
    hciEvent_packetHeader header;
    uint16 event;
    uint8 status;
} hciEvent_packetWStatus;

typedef struct {
    uint8 type;
    uint16 opCode;
    uint8 dataLength;
} hciReset_packet;
extern const hciReset_packet hciReset_packet_default;

typedef struct {
    uint8 type;
    //double check uint8 really?
    uint8 eventCode;
    uint8 dataLength;
    uint8 packets;
    uint16 opCode;
    uint8 status;
} hciReset_event;
extern const hciReset_event hciReset_event_success;

extern int checkHciEventReset(unsigned char * buffer, unsigned long bufferLen);

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
extern const gapTerminateLinkRequest_packet gapTerminateLinkRequest_packet_init;

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

/*
** DESCRIPTION: tokenizes (splits) buffer of bytes into hci packets, support both EVENT and COMMAND HCI packets
** INPUTS:
**   buffer - pointer to input byte array (e.g. received from uart)
**   bufferLen - length of buffer
** OUTPUTS:
**   hciPackets - pointer to allocated array of split HCI packets
**   hciPacketsLen - number of found HCI packets
** RETURNS: number of found HCI packets
*/
extern unsigned char bufhcitokenize(unsigned char * buffer, unsigned long bufferLen, unsigned char *** hciPackets, unsigned char * hciPacketsLen);

/*
** DESCRIPTION: prints HCI packets from stream of bytes in a friendly way
** DEPENDANCY: bufhcitokenize
** INPUTS:
**   ptrStream - pointer to input array (e.g. received from uart)
**   sizeofStream - length of stream
*/
extern void printHciPackets(unsigned char * ptrStream, unsigned long sizeofStream);

/*
** DESCRIPTION: check statuses of all HCI Event packets in a byte stream
** INPUTS:
**   ptrStream - pointer to input array (e.g. received from uart)
**   sizeofStream - length of stream
** OUTPUTS:
**   statuses - pointer to allocated array of HCI Events statuses
**   statusesNum - pointer where number of statuses is saved
** RETURNS: first FAIL status or PASS status
*/
extern unsigned char checkHciEventPacketsStatuses(unsigned char * ptrStream, unsigned long sizeofStream, unsigned char ** statuses, unsigned char * statusesNum );

#endif // BLE_H
