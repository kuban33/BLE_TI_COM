/*******************************************************************************
 \file  ble.c
 \brief sources for BLE
 \author SARA
 \date 20191009
*******************************************************************************/
#include "ble_gap_events.h"

const const gapDeviceInitDone_t gapDeviceInitDone_default={
    GAP_EVENT_DEVICEINITDONE, //uint16 event;
    HCI_SUCCESS, //uint8 status;
    {0x00}, //bdAddr_t devAddr;
    0x001B, //uint16 dataPktLen;
    0x04, //uint8 numDataPkts;
    {0x00}, //rslvKey_t irk;
    {0x00} //rslvKey_t csrk;
};

const const gapHciExtentionCommandStatus_t gapHciExtentionCommandStatus_default={
    GAP_EVENT_HCIEXTENTIONCOMMANDSTATUS, //uint16 event;
    HCI_SUCCESS, //uint8 status;
    0x00, //hciOpCode_t opCode;
    0x00, //uint8 dataLength;
    0 //uint8 * paramValue;
};

uint16 getEvent(hciEvent_packet * evntpckt) {
    return evntpckt->event;
}
uint8 getEventStatus(hciEvent_packet * evntpckt) {
    return evntpckt->status;
}

