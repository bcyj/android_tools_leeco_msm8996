/*==============================================================================
*  @file UIBCInterface.h
*
*  @par  DESCRIPTION:
*        UIBC Interface with Session Manager.
*        This is a base class and Source and Sink UIBC Interfaces will be based
*        on it
*
*
*  Copyright (c) 2012 - 2014 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================

  $Header:
==============================================================================*/

#ifndef _UIBCINTERFACE_H_
#define _UIBCINTERFACE_H_

#include "UIBCDefs.h"

#define UNUSED(x) ((void)x)

class UIBCInterface
{
public:
  int m_bSupportedFlag;
  UIBCInterface()
  {
      m_bSupportedFlag = 0;
  }
  virtual ~UIBCInterface(){}
  virtual void getUibcCapability(WFD_uibc_capability_t* pCapability) {UNUSED(pCapability);}
  static  UIBCInterface* createInstance(uibcDeviceType myDeviceType);

  virtual bool getNegotiatedCapability
  (
   WFD_uibc_capability_config_t *,
   WFD_uibc_capability_config_t *,
   WFD_uibc_capability_config_t *
  ){return false;}
  virtual void setUibcCapability(WFD_uibc_capability_t*, wfd_uibc_capability_change_cb){}
  virtual void registerUibcCallback(wfd_uibc_attach_cb,wfd_uibc_send_event_cb,wfd_uibc_hid_event_cb) {}
  virtual bool createUIBCSession() {return false;}
  virtual bool destroyUIBCSession() {return false;}
  virtual bool Enable(){return false;}
  virtual bool Disable(){return false;}
  virtual bool sendUIBCEvent (WFD_uibc_event_t* event){UNUSED(event); return false;}
  virtual bool readUIBCCfgCapability(WFD_uibc_capability_t* pUibcCapability){UNUSED(pUibcCapability);return false;}
};
#endif
