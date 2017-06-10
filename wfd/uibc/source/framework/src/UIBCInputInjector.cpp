/*==============================================================================
 *  @file UIBCInputInjector.cpp
 *
 *  @par  DESCRIPTION: 
 *        Class Definition of the UIBC Input Injector(Wifi Display Source)
 *        Contains interfaces and members to inject inputs through device 
 *        drivers. This implementation is Android/Linux specific
 *
 *  Copyright (c) 2011 by Qualcomm Technologies, Inc. All Rights Reserved. 
 *  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE 

$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/uibc/source/src/UIBCInputInjector.cpp#1 $
$DateTime: 2011/11/14 18:31:33 $

===============================================================================*/

#include "UIBCInputInjector.h"
#include "MMDebugMsg.h"

UIBCInputInjector::UIBCInputInjector()
{

}


UIBCInputInjector::~UIBCInputInjector()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG,"~UIBCInputInjector");
}

int32 UIBCInputInjector::InjectInput(WFD_uibc_event_t *InputEvent)
{
  UNUSED(InputEvent);
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG,"UIBCInputInjector::InjectInput \n");
  return 1;
}

void UIBCInputInjector::InjectTouchEvent(int down, int x, int y)
{
  UNUSED(down);
  UNUSED(x);
  UNUSED(y);
}


