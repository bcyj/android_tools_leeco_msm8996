/*==============================================================================
*  @file UIBCDefs.h
*
*  @par  DESCRIPTION:
*        UIBC Data structure and capability definitions.
*
*
*  Copyright (c) 2012 -2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================

  $Header:
==============================================================================*/

#ifndef _UIBCDEFS_H_
#define _UIBCDEFS_H_

#include "AEEStdDef.h"
#include "AEEstd.h"

typedef enum uibcDeviceType {
    UIBC_SOURCE,
    UIBC_PRIMARY_SINK,
    UIBC_SECONDARY_SINK,
    UIBC_UNKNOWN
  } uibcDeviceType;

//this enum should be in sync with defn. in WFDEnums.java
typedef enum HIDDataType {
    HID_INVALID_DATA,
    HID_REPORT,
    HID_REPORT_DESCRIPTOR
}HIDDataType;

#define BIT0       0x00000001
#define BIT1       0x00000002
#define BIT2       0x00000004
#define BIT3       0x00000008
#define BIT4       0x00000010
#define BIT5       0x00000020
#define BIT6       0x00000040
#define BIT7       0x00000080

#ifndef UINT16_MAX
#define UINT16_MAX 65535
#endif

#define UIBC_PORT_ID    3456
/* Bitmask for INPUT CATEGORY */

#define   GENERIC       BIT0
#define   HIDC          BIT1

/* Bitmask for INPUT TYPES */

#define   KEYBOARD      BIT0
#define   MOUSE         BIT1
#define   SINGLETOUCH   BIT2
#define   MULTITOUCH    BIT3
#define   JOYSTICK      BIT4
#define   CAMERA        BIT5
#define   GESTURE       BIT6
#define   REMOTECONTROL BIT7

/* Bitmask for INPUT PATHS */

#define   INFRARED      0
#define   USB           1
#define   BT            2
#define   ZIGBEE        3
#define   WIFI          4
#define   NOSP          5

/* Number of Input Paths defined currently */
#define  UIBC_NUM_INPUT_PATHS  6

#define  UIBC_MAX_TOUCH_INPUTS 10

#define UNUSED(x) ((void)x)

/* ============================================
 * UIBC Capability Configuration data structure
 * Contains bitmaps for each member
 * ============================================
 */

typedef struct
{
  uint16    category;              /* 16 possible values types defined,
                                    * 2-15 reserved */
  uint8     generic_input_type;    /* Currently 8 types defined */
  uint8     hid_input_type_path[UIBC_NUM_INPUT_PATHS];
                                   /* Currently 6 input types are defined
                                      each of 8 bits represents a device
                                      in the order defined above.
                                      For example hid_input_type_path[1] shall
                                      correspond to all devices connected
                                      through USB */
} WFD_uibc_capability_config_t;

/* =====================================
 * UIBC Capability data structure
 * Contains bitmaps for each member
 * =====================================
 */

typedef struct
{
  WFD_uibc_capability_config_t  config;     /* UIBC Capability config */
  uint16                        port_id;    /* Port No. to listen to */
  uint32                        ipv4_addr;  /* Peer IP Address */
  uint32                        negotiated_height;
  uint32                        negotiated_width;
} WFD_uibc_capability_t;



//WFD spec 1.22,line#1253
#define UIBC_WFD_VERSION               0

//WFD spec 1.22, Table 4-6
#define UIBC_GENERIC_INPUT_CATEGORY_ID 0
#define UIBC_HIDC_INPUT_CATEGORY_ID    1

//WFD Spec 1.22, Table 4-8
#define UIBC_GENERIC_ID_TOUCH_DOWN          0
#define UIBC_GENERIC_ID_TOUCH_UP            1
#define UIBC_GENERIC_ID_TOUCH_MOVE          2
#define UIBC_GENERIC_ID_KEY_DOWN            3
#define UIBC_GENERIC_ID_KEY_UP              4
#define UIBC_GENERIC_ID_ZOOM                5
#define UIBC_GENERIC_ID_SCROLL_VERTICAL     6
#define UIBC_GENERIC_ID_SCROLL_HORIZONTAL   7
#define UIBC_GENERIC_ID_ROTATE              8

/* -----------------------------------------------------------------------
 * Type definitions for Touch event Parms
 * -----------------------------------------------------------------------
 */
typedef enum
{
  WFD_UIBC_TOUCH_DOWN,
  WFD_UIBC_TOUCH_UP,
  WFD_UIBC_TOUCH_MOVE
}WFD_uibc_touch_event_type;

typedef struct
{
  WFD_uibc_touch_event_type type;         //Type of touch event
  uint8                     num_pointers; //Number of active touch points
                                          //on the screen
  uint8                     pointer_id[UIBC_MAX_TOUCH_INPUTS];
                            //Id of the pointer
  double                    coordinate_x[UIBC_MAX_TOUCH_INPUTS];
                            //X-Coordinate with respect to the
                            //negotiated display resolution
  double                    coordinate_y[UIBC_MAX_TOUCH_INPUTS];
                            //Y-Coordinate with respect to the negotiated
                            //display resolution
}WFD_uibc_touch_event_parms;


/* -----------------------------------------------------------------------
 * Type definitions for key board event Parms
 * -----------------------------------------------------------------------
 */
typedef enum
{
  WFD_UIBC_KEY_DOWN,
  WFD_UIBC_KEY_UP
}WFD_uibc_key_event_type;

typedef struct
{
  WFD_uibc_key_event_type type;       //Type of key event
  uint16                  key_code_1; //The key code of the first key event
                                      //in the format specified in WFD spec
  uint16                  key_code_2; //The key code of the second key event
                                      //in the format specified in WFD spec
}WFD_uibc_key_event_parms;


/* -----------------------------------------------------------------------
 * Type definitions for Zoom event parms
 * -----------------------------------------------------------------------
 */

typedef struct
{
    double  coordinate_x;           //Reference X-Coordinate for zoom with respect
                                  //to the negotiated display resolution
    double  coordinate_y;           //Reference Y-Coordinate for zoom with respect
                                  //to the negotiated display resolution

  uint8   num_times_zoom_int;     //Unsigned integer portion of the number of times
                                  //to zoom
  uint8   num_times_zoom_fraction;//Fraction portion of the number of times to zoom
}WFD_uibc_zoom_event_parms;

/* -----------------------------------------------------------------------
 * Type definitions for scroll event Parms
 * -----------------------------------------------------------------------
 */

typedef enum
{
  WFD_UIBC_SCROLL_VERTICAL,
  WFD_UIBC_SCROLL_HORIZONTAL
}WFD_uibc_scroll_event_type;

typedef struct
{
  WFD_uibc_scroll_event_type  type; //Type of scroll event
  int16       num_pixels_scrolled;  //Number of pixels scrolled with respect to
                                    //the negotiated display resolution
                                    //For vertical scroll, a negative number
                                    //indicates to scroll up; a positive number
                                    //indicates to scroll down
                                    //For horizontal scroll, a negative number
                                    //indicates to scroll right;a positive number
                                    //indicates to scroll left
}WFD_uibc_scroll_event_parms;

/* -----------------------------------------------------------------------
 * Type definitions for Rotate event Parms
 * -----------------------------------------------------------------------
 */

typedef struct
{
  int8  num_rotate_int;       //The signed integer portion of the amount units
                              //in radians to rotate.
                              //A negative number indicates to rotate clockwise;
                              //a positive number indicates to rotate counter-clockwise
  uint8 num_rotate_fraction;  //The fraction portion of the amount in units of
                              //radians to rotate.
}WFD_uibc_rotate_event_parms;


/* -----------------------------------------------------------------------
 * Type definitions for uibc event
 * -----------------------------------------------------------------------
 */

/* This enumerated type lists the different types of uibc events*/
typedef enum
{
  WFD_UIBC_TOUCH,
  WFD_UIBC_KEY,
  WFD_UIBC_ZOOM,
  WFD_UIBC_SCROLL,
  WFD_UIBC_ROTATE
}WFD_uibc_event_type;

typedef union
{
  WFD_uibc_touch_event_parms  touch_event;
  WFD_uibc_key_event_parms    key_event;
  WFD_uibc_zoom_event_parms   zoom_event;
  WFD_uibc_scroll_event_parms scroll_event;
  WFD_uibc_rotate_event_parms rotate_event;
}WFD_uibc_event_parms;

typedef struct
{
  WFD_uibc_event_type  type;       //type of uibc event
  WFD_uibc_event_parms parms;      //parameters of the event
  uint16               timestamp;  //The last 16 bits of the WFD source marked
                       //RTP timestamp of the frames that are being displayed
                       //when user inputs are applied(from spec 1.22,line1266)
                       //otherwise "UINT16_MAX"
}WFD_uibc_event_t;

typedef void (*wfd_uibc_capability_change_cb)(void *);
/* Val parameter = 1 indicates attach and 0 detach */
typedef boolean (*wfd_uibc_attach_cb)(boolean val, void * pClientData);
typedef boolean (*wfd_uibc_send_event_cb)(WFD_uibc_event_t*, void *pClientData);
typedef boolean (*wfd_uibc_hid_event_cb)(uint8* HIDPacket, uint8 packetLen, HIDDataType dataType);

#endif
