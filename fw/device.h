#pragma once


#ifdef __cplusplus
extern "C" {
#endif


enum CDCItf
{
    DATA  = 0,
    EVENT
};


#define CDC_DATA_EP_NOTIF    0x81
#define CDC_DATA_EP_OUT      0x02
#define CDC_DATA_EP_IN       0x82

#define CDC_EVENT_EP_NOTIF   0x83
#define CDC_EVENT_EP_OUT     0x04
#define CDC_EVENT_EP_IN      0x84



#define IOIG_MANUFACTURER_STR "INGECOM"
#define IOIG_PRODUCT_STR      "IoIg Multi Protocol Dongle"
#define IOIG_VID              0xcafe
#define IOIG_PID              0x4002



#ifdef __cplusplus
}
#endif


