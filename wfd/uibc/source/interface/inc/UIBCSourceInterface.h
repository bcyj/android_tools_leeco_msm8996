/*==============================================================================
*  @file UIBCSourceInterface.h
*
*  @par  DESCRIPTION:
*        UIBC Source Interface with Session Manager.
*        This is source interface for UIBC with Session Manager
*
*
*  Copyright (c) 2012 - 2013 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================

  $Header:
==============================================================================*/


#ifndef _UIBCSOURCEINTERFACE_H_
#define _UIBCSOURCEINTERFACE_H_

#include "UIBCDefs.h"
#include "UIBCInterface.h"

class UIBCInputReceiver;

class UIBCSourceInterface : public UIBCInterface
{
public:
  UIBCSourceInterface();
  ~UIBCSourceInterface();
  virtual void getUibcCapability(WFD_uibc_capability_t*);
  virtual bool getNegotiatedCapability(WFD_uibc_capability_config_t *,
                                       WFD_uibc_capability_config_t *,
                                       WFD_uibc_capability_config_t *
                                      );
  virtual void setUibcCapability(WFD_uibc_capability_t*,
                                 wfd_uibc_capability_change_cb capChangeCB);
  virtual void registerUibcCallback(wfd_uibc_attach_cb,wfd_uibc_send_event_cb,wfd_uibc_hid_event_cb);
  virtual bool readUIBCCfgCapability(WFD_uibc_capability_t* pUibcCapability);
  virtual bool createUIBCSession();
  virtual bool destroyUIBCSession();
  virtual bool Enable();
  virtual bool Disable();
  //virtual bool sendUIBCEvent (WFD_uibc_event_t* event){}
  //Not Implemented in Source
  private:
  UIBCInputReceiver* m_pUIBCHandle;
};

#endif
