/*==============================================================================
 *  @file UIBCInputParser.h
 *
 *  @par  DESCRIPTION:
 *        Class Declaration of the UIBC Input Parser(Wifi Display Source)
 *        Contains interfaces and members to parse the UIBC Packet and
 *        convert it into an intermediate data structure that can be translated
 *        to input events across platforms
 *
 *  Copyright (c) 2011 - 2013 by Qualcomm Technologies, Inc. All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/uibc/source/inc/UIBCInputParser.h#1 $
$DateTime: 2011/11/14 18:31:33 $

===============================================================================*/

#ifndef UIBC_INPUT_PARSER_H
#define UIBC_INPUT_PARSER_H

#include "UIBCDefs.h"

class UIBCInputParser
{
public:
  UIBCInputParser();
  ~UIBCInputParser();
  boolean       ParseGenericInput(uint32&, uint8*, WFD_uibc_event_t*, WFD_uibc_capability_t *);
  ssize_t       ParseHIDInput(uint32&, uint8*, HIDDataType&, uint8&);
};
#endif //UIBC_INPUT_PARSER_H
