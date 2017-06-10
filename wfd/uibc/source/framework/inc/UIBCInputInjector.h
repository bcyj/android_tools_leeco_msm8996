/*==============================================================================
 *  @file UIBCInputInjector.h
 *
 *  @par  DESCRIPTION: 
 *        Class declaration of the UIBC Input Injector(Wifi Display Source)
 *        Contains interfaces to inject input to the Input subsystem
 *        It is a platform dependent module, 
 *
 *
 *  Copyright (c) 2011 by Qualcomm Technologies, Inc. All Rights Reserved. 
 *  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE 

$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/uibc/source/inc/UIBCInputInjector.h#1 $
$DateTime: 2011/11/14 18:31:33 $

===============================================================================*/
#ifndef UIBCInputInjector_H
#define UIBCInputInjector_H

#include "UIBCDefs.h"

class UIBCInputInjector
{
void InjectTouchEvent(int down, int x, int y);
public:
  UIBCInputInjector();
  ~UIBCInputInjector();
  int32 InjectInput(WFD_uibc_event_t*);
  private:
    int touchfd;
    int xmin, xmax, ymin, ymax;

};

#endif
