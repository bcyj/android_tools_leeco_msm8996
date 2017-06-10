/*==============================================================================
 *  @file UIBCInputParser.cpp
 *
 *  @par  DESCRIPTION:
 *        Class Definition of the UIBC Input Parser(Wifi Display Source)
 *        Contains interfaces and members to parse the UIBC Packet and
 *        convert it into an intermediate data structure that can be translated
 *        to input events across platforms
 *
 *  Copyright (c) 2011 - 2013 by Qualcomm Technologies, Inc. All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/uibc/source/src/UIBCInputParser.cpp#1 $
$DateTime: 2011/11/14 18:31:33 $

===============================================================================*/
#include "UIBCInputParser.h"
#include "MMDebugMsg.h"


//Constructor; Default as of now
UIBCInputParser::UIBCInputParser()
{

}

//Destructor; Default as of now
UIBCInputParser::~UIBCInputParser()
{

}

/** @brief     UIBCInputParser ParseGenericInput
  *            Method to translate UIBC packet into intermediate UIBC data
  *            structure so that input translation is platform agnostic.
  *            This function parses Generic Inputs inside UIBC packet
  *
  * @param
  *        [in]  index       Contains the offset in UIBC packet being parsed
  *        [in]  uibcPacket  Pointer to the buffer containing UIBC packet
  *        [out] inputEvent  UIBC event data structure
  *
  * @return    TRUE , if translation is successful
  */

boolean UIBCInputParser::ParseGenericInput(uint32& index,
                                           uint8* uibcPacket,
                                           WFD_uibc_event_t* inputEvent,
                                           WFD_uibc_capability_t *pNegotiatedCapability)
{
  uint8  ucIeType = uibcPacket[index++];
  uint16 usLength = static_cast<uint16>(uibcPacket[index]<<8|uibcPacket[index+1]);
  index += 2;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                 "UIBCInputParser:ParseGenericInput  Generic input type %d",ucIeType);

    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, "UIBCInputParser:ParseGenericInput UIBC negotiated_height %u  negotiated_width %u",
                 pNegotiatedCapability->negotiated_height,
                 pNegotiatedCapability->negotiated_width);
  switch (ucIeType)
  {
  case UIBC_GENERIC_ID_TOUCH_DOWN:
  case UIBC_GENERIC_ID_TOUCH_UP:
  case UIBC_GENERIC_ID_TOUCH_MOVE:
    inputEvent->parms.touch_event.num_pointers = uibcPacket[index++];
    for (int i = 0; i < inputEvent->parms.touch_event.num_pointers;i++)
    {
      inputEvent->parms.touch_event.pointer_id[i] = uibcPacket[index++];
      inputEvent->parms.touch_event.coordinate_x[i] = uibcPacket[index]<<8|uibcPacket[index+1];
      MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,"Coordinates wtih NW*X NH*Y %lf %lf ",
                   inputEvent->parms.touch_event.coordinate_x[i],
                   inputEvent->parms.touch_event.coordinate_y[i]);
      if(pNegotiatedCapability ->negotiated_width != 0)
      inputEvent->parms.touch_event.coordinate_x[i]/= static_cast<double>(pNegotiatedCapability ->negotiated_width);
      index+=2;
      inputEvent->parms.touch_event.coordinate_y[i]= uibcPacket[index]<<8|uibcPacket[index+1];
      if(pNegotiatedCapability ->negotiated_height!= 0)
      inputEvent->parms.touch_event.coordinate_y[i]/= static_cast<double>(pNegotiatedCapability ->negotiated_height);
      index+=2;
      MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,"Coordinates wtih X/NW  Y/ NH %lf %lf ",
                   inputEvent->parms.touch_event.coordinate_x[i],
                   inputEvent->parms.touch_event.coordinate_y[i]);
    }
    //the decrement should happen if the for loop was entered
    //make sure you add relevant check for number of inputs
    index--;
    inputEvent->type = WFD_UIBC_TOUCH;
    inputEvent->parms.touch_event.type = (WFD_uibc_touch_event_type)ucIeType;
    break;

  case UIBC_GENERIC_ID_KEY_DOWN:
  case UIBC_GENERIC_ID_KEY_UP:
    if (ucIeType == UIBC_GENERIC_ID_KEY_DOWN)
    {
      inputEvent->parms.key_event.type = WFD_UIBC_KEY_DOWN;
    }
    else
    {
      inputEvent->parms.key_event.type = WFD_UIBC_KEY_UP;
    }
    inputEvent->type = WFD_UIBC_KEY;
    index++; //1 byte is reserved
    inputEvent->parms.key_event.key_code_1 = static_cast<uint16>(uibcPacket[index]<<8|uibcPacket[index+1]);
    index+=2;
    inputEvent->parms.key_event.key_code_2 = static_cast<uint16>(uibcPacket[index]<<8|uibcPacket[index+1]);
    index+=2;
    break;
  case UIBC_GENERIC_ID_ZOOM:
    inputEvent->type = WFD_UIBC_ZOOM;
    inputEvent->parms.zoom_event.coordinate_x  = uibcPacket[index]<<8|uibcPacket[index+1];
    if(pNegotiatedCapability ->negotiated_height != 0)
    inputEvent->parms.zoom_event.coordinate_x /= static_cast<double>(pNegotiatedCapability ->negotiated_width);
    index+=2;
    inputEvent->parms.zoom_event.coordinate_y  = uibcPacket[index]<<8|uibcPacket[index+1];
    if(pNegotiatedCapability ->negotiated_width != 0)
    inputEvent->parms.zoom_event.coordinate_y /= static_cast<double>(pNegotiatedCapability ->negotiated_height);
    index+=2;
    inputEvent->parms.zoom_event.num_times_zoom_int = uibcPacket[index++];
    inputEvent->parms.zoom_event.num_times_zoom_fraction = uibcPacket[index++];
    break;
  case UIBC_GENERIC_ID_SCROLL_VERTICAL:
  case UIBC_GENERIC_ID_SCROLL_HORIZONTAL:
    inputEvent->type = WFD_UIBC_SCROLL;
    if (ucIeType == UIBC_GENERIC_ID_SCROLL_VERTICAL)
    {
      inputEvent->parms.scroll_event.type = WFD_UIBC_SCROLL_VERTICAL;
    }
    else
    {
      inputEvent->parms.scroll_event.type = WFD_UIBC_SCROLL_HORIZONTAL;
    }
    inputEvent->parms.scroll_event.num_pixels_scrolled  = static_cast<int16>(uibcPacket[index]<<8|uibcPacket[index+1]);
    index+=2;
    break;
  case UIBC_GENERIC_ID_ROTATE:
    inputEvent->type = WFD_UIBC_ROTATE;
    inputEvent->parms.rotate_event.num_rotate_int = uibcPacket[index++];
    inputEvent->parms.rotate_event.num_rotate_fraction = uibcPacket[index++];
    break;
  default: //do nothing
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG,
                 "UIBCInputParser:ParseGenericInput Unsupported Generic input %d",ucIeType);
    return FALSE;
  }
  return TRUE;
}

/** @brief     UIBCInputParser ParseHIDInput
  *            Method to translate UIBC packet into intermediate UIBC data
  *            structure so that input translation is platform agnostic.
  *            This function parses HID Inputs inside UIBC packet
  *
  * @param
  *        [in]  index       Contains the offset in UIBC packet being parsed
  *        [in]  uibcPacket  Pointer to the buffer containing UIBC packet
  *        [out] hidDataType Indicates whether received payload was an HID
                             report or report descriptor
           [out] HIDType     inidcates the HID input type(Mouse/Keyboard/etc.)
  *
  * @return    length of the HID payload
  */
ssize_t UIBCInputParser::ParseHIDInput(uint32& index,
                                       uint8* uibcPacket,
                                       HIDDataType& hidDataType,
                                       uint8&  HIDType)
{
  uint8  ucInputPath = uibcPacket[index++];
  HIDType            = uibcPacket[index++];
  uint8  ucUsage         = uibcPacket[index++];
  uint16 usLength = -1;
  switch(ucUsage)
  {
    //As per Wi-Fi Display spec 1.47 Table 4-15
    case 0x0:
        hidDataType = HID_REPORT;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"UIBCInputParser:ParseHIDInput Received HID_REPORT");
        break;
    case BIT0:
        hidDataType = HID_REPORT_DESCRIPTOR;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"UIBCInputParser:ParseHIDInput Received HID_REPORT_DESCRIPTOR");
        break;
    default:
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"UIBCInputParser:ParseHIDInput Unsupported HID type %d",ucUsage);
  }
  //usLength is the actual HID payload
  usLength        = static_cast<uint16>(uibcPacket[index]<<8|uibcPacket[index+1]);
  index += 2;
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG,"UIBCInputParser:ParseHIDInput HID payload of lenth %d",usLength);
  return usLength;
}
