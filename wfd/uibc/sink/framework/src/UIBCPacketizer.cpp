/*==============================================================================
  *        UIBCPacketizer.cpp
  *
  *  DESCRIPTION: 
  *       Provides utility APIs to prepare UIBC packet
  *
  *
  *  Copyright (c) 2011 - 2014 by Qualcomm Technologies, Inc. All Rights Reserved.
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
    
#include "UIBCPacketizer.h"
#include "MMDebugMsg.h"


/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */


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
**                            Function Definitions
** ======================================================================= */

UIBCPacketizer::UIBCPacketizer()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCPacketizer:constructor");
}

UIBCPacketizer::~UIBCPacketizer()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCPacketizer:destructor");
}

/*==========================================================================
   FUNCTION     : constructUIBCPacket

   DESCRIPTION: constrcuts the UIBC raw packet from the event structure. The packet is padded up to integer number of 16 bits
         
   PARAMETERS :     event[in]  - UIBC  event 
                           buffer[out]  - UIBC raw packet buffer
                                size[in]  - size of the buffer
   
   Return Value  : returns the number of bytes of the constructed packet.
  ===========================================================================*/

int32 UIBCPacketizer::constructUIBCPacket(WFD_uibc_event_t* event, uint8* buffer, uint16 size)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCPacketizer:constructUIBCPacket");

  int32 rawPacketSize = 0;

  if( event != NULL && buffer != NULL && size > UIBC_PACKET_HEADER_LENGTH )
  {
     /**
          *Wi-Fi_Display_Specification_1.22,Figure 4-9 : Encapsulations of User Inputs over TCP/IP
               ---------------------------------------------------------------------------------------
               Bit offset  |  0      1      2  |   3   |   4      5      6      7      8      9      10      11   |  12      13      14      15 |
               ---------------------------------------------------------------------------------------
                       0     |   Version       |   T   |             Reserved                                        |   Input category           |  
               ---------------------------------------------------------------------------------------
                        16  |        Length                                                                                                                      |
               ---------------------------------------------------------------------------------------
                        32  |   Timestamp(optional)                                                                                                       |
               ---------------------------------------------------------------------------------------
                              |          Input Body 
               --------------------------------------------------------------------------------------- 
          */

    //initialize with 0s
    std_memset(buffer, 0, size);

    int32 timestampLength = (event->timestamp != UINT16_MAX) ? UIBC_PACKET_TIMESTAMP_LENGTH : 0;

    int32 inputBodyLength = feedInputBody(event, buffer + UIBC_PACKET_HEADER_LENGTH + timestampLength, 
                                                                   static_cast<uint16>(size - UIBC_PACKET_HEADER_LENGTH - timestampLength));

    if( inputBodyLength > 0 )
    {
      //0-2 bits represents version
      //note:most significant bit of a byte is numbered as zero
      buffer[0] |= (uint8)( UIBC_WFD_VERSION << 5);

      if( event->timestamp != UINT16_MAX )
      {
         //3rd bit represents timestamp flag
         buffer[0] |= (uint8)(1 << 4);
      } 

      /*4-11 bits are reserved*/

      //12-15 bits offsets represent the input category
      if( event->type  == WFD_UIBC_TOUCH  || 
           event->type == WFD_UIBC_KEY    || 
           event->type == WFD_UIBC_ZOOM   ||
           event->type == WFD_UIBC_SCROLL ||
           event->type == WFD_UIBC_ROTATE)
     {
        buffer[1] |= (uint8)( 0x0F & UIBC_GENERIC_INPUT_CATEGORY_ID );
     }
     else
     {
        buffer[1] |= (uint8)( 0x0F & UIBC_HIDC_INPUT_CATEGORY_ID );
     }

     rawPacketSize = UIBC_PACKET_HEADER_LENGTH + timestampLength + inputBodyLength;

     //16-31 bits offsets represent length of total packet
     //filling in network/Big endian byte order
     buffer[2] = (uint8)( rawPacketSize >> 8 );
     buffer[3] = (uint8)( rawPacketSize );

     if( event->timestamp != UINT16_MAX )
     {
        uint16 timestamp = event->timestamp;
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketizer:constructUIBCPacket:UIBC event Timestamp is %hu",timestamp);
        //fourth and fifth byte offsets represents timestamp if present
        buffer[4] = (uint8)( timestamp >> 8 );
        buffer[5] = (uint8)( timestamp );
     }

     /*input body is already filled in*/

    }   
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketizer:constructUIBCPacket:Bad parm");
  }

  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "UIBCPacketizer:constructUIBCPacket:constructed packet length %d", rawPacketSize);
  return rawPacketSize;
}

/*==========================================================================
   FUNCTION     : feedInputBody

   DESCRIPTION: Feeds the inputbody corresponds to the event.
         
   PARAMETERS :     event[in]  - UIBC  event 
                           buffer[out]  - buffer pointer to store event inputbody
                                size[in]  - size of the buffer
   
   Return Value  : returns the number of bytes written. Should be padded up to integer number of 16 bits
  ===========================================================================*/

int32 UIBCPacketizer::feedInputBody(WFD_uibc_event_t* event, uint8* buffer,uint16 size)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCPacketizer:feedInputBody");

  int inputBodyLength = 0;
  switch( event->type )
  {
    case WFD_UIBC_TOUCH:
    {
      inputBodyLength = feedGenericTouchEvent(event, buffer, size);
      break;
    }
    case WFD_UIBC_KEY:
    {
      inputBodyLength = feedGenericKeyEvent(event, buffer, size);
      break;
    }
    case WFD_UIBC_ZOOM:
    {
      inputBodyLength = feedGenericZoomEvent(event, buffer, size);
      break;
    }
    case WFD_UIBC_SCROLL:
    {
      inputBodyLength = feedGenericScrollEvent(event, buffer, size);
      break;
    }
    case WFD_UIBC_ROTATE:
    {
      inputBodyLength = feedGenericRotateEvent(event, buffer, size);
      break;
    }
    default:
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketizer:feedInputBody:unsupported packet type %d", event->type);
    }
  }

  boolean isPaddingRequired = ((inputBodyLength % 2) != 0) ? TRUE : FALSE;

  if(isPaddingRequired)
  {
    if( size > inputBodyLength )
    {
      buffer[inputBodyLength] = 0x00;
      inputBodyLength++;
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketizer:feedInputBody:Insufficient memory for padding");
      inputBodyLength = 0;
    }
  }
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG, "UIBCPacketizer:feedInputBody: written %d bytes", inputBodyLength);
  return inputBodyLength;
}

/*==========================================================================
   FUNCTION     : feedGenericTouchEvent

   DESCRIPTION: Feeds the inputbody for touch event

   PARAMETERS :     event[in]  - UIBC  event
                           buffer[out]  - buffer pointer to store event inputbody
                                size[in]  - size of the buffer

   Return Value  : returns the number of bytes written.
  ===========================================================================*/

int32 UIBCPacketizer::feedGenericTouchEvent(WFD_uibc_event_t* event, uint8* buffer,uint16 size)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCPacketizer:feedGenericTouchEvent");

  int32 numBytes = 0;

  if( event!=  NULL && buffer != NULL )
  {
    WFD_uibc_touch_event_parms touchEvent = event->parms.touch_event;

    int32 touchEventPayloadLength = 1 + (5 * touchEvent.num_pointers);
    int32 touchEventLength = UIBC_GENERIC_EVENT_HEADER_LENGTH + touchEventPayloadLength;

    if( size >= touchEventLength )
    {
      switch( touchEvent.type )
      {
        //first octet represents Generic event type 
        case WFD_UIBC_TOUCH_DOWN:
        {
          buffer[0] = UIBC_GENERIC_ID_TOUCH_DOWN;
          break;
        }
        case WFD_UIBC_TOUCH_UP:
        {
          buffer[0] = UIBC_GENERIC_ID_TOUCH_UP;
          break;
        }
        case WFD_UIBC_TOUCH_MOVE:
        {
          buffer[0] = UIBC_GENERIC_ID_TOUCH_MOVE;
          break;
        }
      }

      //Padding will be present if total length is not 16 bit aligned 
      int32 touchEventIELength = touchEventPayloadLength + (touchEventLength % 2);

      //second,third octets  represents length of  description field. 
      buffer[1] = (uint8)( touchEventIELength >> 8 );
      buffer[2] = (uint8)( touchEventIELength );
  
      /*Description field:WFD spec 1.25 Table 4-6, 4-7, 4-8*/

      buffer[3] = touchEvent.num_pointers; //number of pointers

      int32 pointerIndex = 0;

      while(pointerIndex < touchEvent.num_pointers)
      {
        buffer[4 + (pointerIndex*5)] = touchEvent.pointer_id[pointerIndex];   //current pointer id

        //X-coordinate in network byte order
        buffer[5 + (pointerIndex*5)] = (uint8)( (uint16)touchEvent.coordinate_x[pointerIndex] >> 8 );
        buffer[6 + (pointerIndex*5)] = (uint8)( (uint16)touchEvent.coordinate_x[pointerIndex] );

        //Y-coordinate in network byte order
        buffer[7 + (pointerIndex*5)] = (uint8)( (uint16)touchEvent.coordinate_y[pointerIndex] >> 8 );
        buffer[8 + (pointerIndex*5)] = (uint8)( (uint16)touchEvent.coordinate_y[pointerIndex] );

        pointerIndex++;
      }

      numBytes = touchEventLength; 
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketizer:feedGenericTouchEvent:Insufficient memory");
    }
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketizer:feedGenericTouchEvent:Bad parm");
  }

  return numBytes;
}

/*==========================================================================
   FUNCTION     : feedGenericKeyEvent

   DESCRIPTION: Feeds the inputbody for key event
         
   PARAMETERS :     event[in]  - UIBC  event 
                           buffer[out]  - buffer pointer to store event inputbody
                                size[in]  - size of the buffer
   
   Return Value  : returns the number of bytes written.
  ===========================================================================*/

int32 UIBCPacketizer::feedGenericKeyEvent(WFD_uibc_event_t * event,uint8 * buffer,uint16 size)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCPacketizer:feedGenericKeyEvent");

  int32 keyEventLength = UIBC_GENERIC_EVENT_HEADER_LENGTH + UIBC_GENERIC_KEY_EVENT_IE_LENGTH;
  int32 numBytes = 0;

  if( event!=  NULL && buffer != NULL && size >= keyEventLength )
  {
    WFD_uibc_key_event_parms keyEvent = event->parms.key_event;

    switch( keyEvent.type )
    {
      //first octet represents Generic event type 
      case WFD_UIBC_KEY_DOWN:
      {
        buffer[0] = UIBC_GENERIC_ID_KEY_DOWN;
        break;
      }
      case WFD_UIBC_KEY_UP:
      {
        buffer[0] = UIBC_GENERIC_ID_KEY_UP;
        break;
      }
    }

    //Padding will be present if total length is not 16 bit aligned 
    int32 keyEventIELength = UIBC_GENERIC_KEY_EVENT_IE_LENGTH + (keyEventLength % 2);
    
    //second,third octets  represents length of  description field. 
    buffer[1] = (uint8)( keyEventIELength >> 8 );
    buffer[2] = (uint8)( keyEventIELength );

    /* Description field: WFD spec 1.22 Table 4-12,4-13 */

    /*description field first byte is reserved*/
    buffer[3] = 0x00;

    //keycode_1 in network byte order
    buffer[4] = (uint8)( keyEvent.key_code_1 >> 8 );
    buffer[5] = (uint8)( keyEvent.key_code_1 );

    //keycode_2 in network byte order
    buffer[6] = (uint8)( keyEvent.key_code_2 >> 8 );
    buffer[7] = (uint8)( keyEvent.key_code_2 );   

    numBytes = keyEventLength;
  }  
  else
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketizer:feedGenericKeyEvent:Bad parm");
  }
  return numBytes;
}


/*==========================================================================
   FUNCTION     : feedGenericZoomEvent

   DESCRIPTION: Feeds the inputbody for zoom event
         
   PARAMETERS :     event[in]  - UIBC  event 
                           buffer[out]  - buffer pointer to store event inputbody
                                size[in]  - size of the buffer
   
   Return Value  : returns the number of bytes written.
  ===========================================================================*/

int32 UIBCPacketizer::feedGenericZoomEvent(WFD_uibc_event_t * event,uint8 * buffer,uint16 size)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCPacketizer:feedGenericZoomEvent");

  int32 zoomEventLength = UIBC_GENERIC_EVENT_HEADER_LENGTH + UIBC_GENERIC_ZOOM_EVENT_IE_LENGTH;
  int32 numBytes = 0;

  if( event!=  NULL && buffer != NULL && size >= zoomEventLength )
  {
    WFD_uibc_zoom_event_parms zoomEvent = event->parms.zoom_event;

    //first octet represents Generic event type 
    buffer[0] = UIBC_GENERIC_ID_ZOOM;

    //Padding will be present if total length is not 16 bit aligned 
    int32 zoomEventIELength = UIBC_GENERIC_ZOOM_EVENT_IE_LENGTH + (zoomEventLength % 2);

    //second,third octets  represents length of  description field. 
    buffer[1] = (uint8)( zoomEventIELength >> 8 );
    buffer[2] = (uint8)( zoomEventIELength );

    /* Description field: WFD spec 1.22 Table 4-14 */

    //reference X-Coordinate
    buffer[3] = (uint8)((uint16) zoomEvent.coordinate_x >> 8 );
    buffer[4] = (uint8)( (uint16)zoomEvent.coordinate_x );

    //reference Y-Coordinate
    buffer[5] = (uint8)( (uint16)zoomEvent.coordinate_y >> 8 );
    buffer[6] = (uint8)( (uint16)zoomEvent.coordinate_y );

    //integer portion of number of times to zoom
    buffer[7] = zoomEvent.num_times_zoom_int;

    //fractional portion of number of times to zoom
    buffer[8] = zoomEvent.num_times_zoom_fraction;

    numBytes = zoomEventLength;
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketizer:feedGenericZoomEvent:Bad parm");
  }

  return numBytes;
}

/*==========================================================================
   FUNCTION     : feedGenericScrollEvent

   DESCRIPTION: Feeds the inputbody for scroll event
         
   PARAMETERS :     event[in]  - UIBC  event 
                           buffer[out]  - buffer pointer to store event inputbody
                                size[in]  - size of the buffer
   
   Return Value  : returns the number of bytes written.
  ===========================================================================*/

int32 UIBCPacketizer::feedGenericScrollEvent(WFD_uibc_event_t * event,uint8 * buffer,uint16 size)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCPacketizer:feedGenericScrollEvent");

  int32 scrollEventLength = UIBC_GENERIC_EVENT_HEADER_LENGTH + UIBC_GENERIC_SCROLL_EVENT_IE_LENGTH;
  int32 numBytes = 0;

  if( event!=  NULL && buffer != NULL && size >= scrollEventLength )
  {
    WFD_uibc_scroll_event_parms scrollEvent = event->parms.scroll_event;

    switch( scrollEvent.type )
    {
      //first octet represents Generic event type 
      case WFD_UIBC_SCROLL_VERTICAL:
      {
        buffer[0] = UIBC_GENERIC_ID_SCROLL_VERTICAL;
        break;
      }
      case WFD_UIBC_SCROLL_HORIZONTAL:
      {
        buffer[0] = UIBC_GENERIC_ID_SCROLL_HORIZONTAL;
        break;
      }
    }

    //Padding will be present if total length is not 16 bit aligned 
    int32 scrollEventIELength = UIBC_GENERIC_SCROLL_EVENT_IE_LENGTH + (scrollEventLength % 2);

    //second,third octets  represents length of  description field. 
    buffer[1] = (uint8)( scrollEventIELength >> 8 );
    buffer[2] = (uint8)( scrollEventIELength );

    /* Description field: WFD spec 1.22 Table 4-15,4-16 */

    //number of pixels scrolled
    buffer[3] = (uint8)( scrollEvent.num_pixels_scrolled >> 8 );
    buffer[4] = (uint8)( scrollEvent.num_pixels_scrolled );


    numBytes = scrollEventLength;
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketizer:feedGenericScrollEvent:Bad parm");
  }
  return numBytes;
}

/*==========================================================================
   FUNCTION     : feedGenericRotateEvent

   DESCRIPTION: Feeds the inputbody for rotate event
         
   PARAMETERS :     event[in]  - UIBC  event 
                           buffer[out]  - buffer pointer to store event inputbody
                                size[in]  - size of the buffer
   
   Return Value  : returns the number of bytes written.
  ===========================================================================*/

int32 UIBCPacketizer::feedGenericRotateEvent(WFD_uibc_event_t * event,uint8 * buffer,uint16 size)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "UIBCPacketizer:feedGenericRotateEvent");

  int32 rotateEventLength = UIBC_GENERIC_EVENT_HEADER_LENGTH + UIBC_GENERIC_ROTATE_EVENT_IE_LENGTH;
  int32 numBytes = 0;

  if( event!=  NULL && buffer != NULL && size >= rotateEventLength )
  {
    WFD_uibc_rotate_event_parms rotateEvent = event->parms.rotate_event;

    //first octet represents Generic event type 
    buffer[0] = UIBC_GENERIC_ID_ROTATE;

    //Padding will be present if total length is not 16 bit aligned 
    int32 rotateEventIELength = UIBC_GENERIC_ROTATE_EVENT_IE_LENGTH + (rotateEventLength % 2);

    //second,third octets  represents length of  description field. 
    buffer[1] = (uint8)( rotateEventIELength >> 8 );
    buffer[2] = (uint8)( rotateEventIELength );

    /* Description field: WFD spec 1.22 Table 4-17 */

    //integer portion of rotate
    buffer[3] = rotateEvent.num_rotate_int;

    //fractional portion of rotate
    buffer[4] = rotateEvent.num_rotate_fraction;

    numBytes = rotateEventLength;
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketizer:feedGenericRotateEvent:Bad parm");
  }

  return numBytes;
}




