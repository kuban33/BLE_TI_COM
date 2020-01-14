/*******************************************************************************
 \file  ble.h
 \brief definitions for BLE GAP events
 \author SARA
 \date 20191015
*******************************************************************************/
#ifndef __BLE_GAP_EVENTS_H__
#define __BLE_GAP_EVENTS_H__

#ifdef __cplusplus
extern "C"
{
#endif

//no struct padding
#pragma pack(1)

#include "ble.h"

//GAP Vendor Specific Events
#define GAP_EVENT_DEVICEINITDONE 0x0600
#define GAP_EVENT_HCIEXTENTIONCOMMANDSTATUS 0x067F

typedef struct {
    uint16 event;
    uint8 status;
    bdAddr_t devAddr;
    uint16 dataPktLen;
    uint8 numDataPkts;
    rslvKey_t irk;
    rslvKey_t csrk;
} gapDeviceInitDone_t;
extern const gapDeviceInitDone_t gapDeviceInitDone_default;

typedef struct {
    uint16 event;
    uint8 status;
    hciOpCode_t opCode;
    uint8 dataLength;
    uint8 * paramValue;
} gapHciExtentionCommandStatus_t;
extern const gapHciExtentionCommandStatus_t gapHciExtentionCommandStatus_default;

#ifdef __cplusplus
}
#endif

#endif // BLE_GAP_EVENTS_H
