/*==============================================================================
*  @file UIBCInterface.cpp
*
*  @par  DESCRIPTION:
*        UIBC Interface with Session Manager.
*        This is a base class and Source and Sink UIBC Interfaces will be based
*        on it
*
*
*  Copyright (c) 2012 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================

  $Header:
==============================================================================*/


#include "UIBCSourceInterface.h"
#ifdef WFD_SINK_ENABLED
#include "UIBCSinkInterface.h"
#endif

UIBCInterface* UIBCInterface::createInstance(uibcDeviceType myDeviceType)
{
  if (myDeviceType == UIBC_SOURCE)
  {
    return new UIBCSourceInterface();
  }
  else if (myDeviceType == UIBC_PRIMARY_SINK)
  {
#ifdef WFD_SINK_ENABLED
    return new UIBCSinkInterface();
#else
    return new UIBCInterface();
#endif
  }
  else
    return NULL;
}
