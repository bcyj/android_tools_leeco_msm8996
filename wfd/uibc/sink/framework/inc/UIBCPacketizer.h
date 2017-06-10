#ifndef __UIBC_PACKETIZER_H__
#define __UIBC_PACKETIZER_H__
/*==============================================================================
 *        UIBCPacketizer.h
 *
 *  DESCRIPTION: 
 *       Provides utility APIs to prepare UIBC packet
 *
 *
 *  Copyright (c) 2011 - 2012,2014 by Qualcomm Technologies, Inc. All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *==============================================================================*/

/* =======================================================================
                             Edit History
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "AEEstd.h"
#include "wdsm_mm_interface.h" 
#include "UIBCDefs.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define UIBC_PACKET_HEADER_LENGTH 4

#define UIBC_PACKET_TIMESTAMP_LENGTH 2

#define UIBC_GENERIC_EVENT_HEADER_LENGTH 3

/*length of description fields*/
#define UIBC_GENERIC_TOUCH_EVENT_MAX_IE_LENGTH (1 + (5*UIBC_MAX_TOUCH_INPUTS))
#define UIBC_GENERIC_KEY_EVENT_IE_LENGTH 5
#define UIBC_GENERIC_ZOOM_EVENT_IE_LENGTH 6
#define UIBC_GENERIC_SCROLL_EVENT_IE_LENGTH 2
#define UIBC_GENERIC_ROTATE_EVENT_IE_LENGTH 2

#define UIBC_GENERIC_MAX_IE_LENGTH UIBC_GENERIC_TOUCH_EVENT_MAX_IE_LENGTH

/*note:need to change if HIDC support is added*/
#define UIBC_MAX_IE_LENGTH UIBC_GENERIC_MAX_IE_LENGTH

#define UIBC_PADDING_BYTE 1
#define UIBC_PACKET_MAX_LENGTH (UIBC_PACKET_HEADER_LENGTH                      \
                              + UIBC_PACKET_TIMESTAMP_LENGTH                   \
                              + UIBC_GENERIC_EVENT_HEADER_LENGTH               \
                              + UIBC_MAX_IE_LENGTH                             \
                              + UIBC_PADDING_BYTE)

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef enum
{
  UIBC_SINK_SUCCESS = 0,
  UIBC_SINK_ERROR_INVALID = -1,
  UIBC_SINK_ERROR_UNSUPPORTED = -2,
  UIBC_SINK_ERROR_UNKNOWN = -3
}UIBC_sink_status_t;


/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */


/* =======================================================================
**                        Class Declarations
** ======================================================================= */

/*
  @brief    UIBCPacketizer Module.

  @details  Provides the API's to construct UIBC packets      

*/

class UIBCPacketizer
{
public:

  UIBCPacketizer();

  virtual ~UIBCPacketizer();

  int32 constructUIBCPacket(WFD_uibc_event_t* event, uint8* buffer, uint16 length);

private:

  int32 feedInputBody(WFD_uibc_event_t* event, uint8* buffer,uint16 size);

  int32 feedGenericTouchEvent(WFD_uibc_event_t* event, uint8* buffer,uint16 size);
  int32 feedGenericKeyEvent(WFD_uibc_event_t* event, uint8* buffer,uint16 size);
  int32 feedGenericZoomEvent(WFD_uibc_event_t* event, uint8* buffer,uint16 size);
  int32 feedGenericScrollEvent(WFD_uibc_event_t* event, uint8* buffer,uint16 size);
  int32 feedGenericRotateEvent(WFD_uibc_event_t* event, uint8* buffer,uint16 size);

};

#endif /*__UIBC_PACKETIZER_H__ */

