/*==============================================================================
*  @file UIBCAdaptor.h
*
*  @par DESCRIPTION:
*        Interface between SM-B and UIBC Module.
*
*
*  Copyright (c) 2012 - 2014 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

#include "UIBCInterface.h"
#include "UIBCDefs.h"
#include "Device.h"

class UIBCAdaptor
{
public:
  UIBCAdaptor();
  ~UIBCAdaptor();
  static void setDeviceType(DeviceType devType);
  static void registerUIBCCallbacks(wfd_uibc_attach_cb ,
                                    wfd_uibc_send_event_cb,
                                    wfd_uibc_hid_event_cb);
  static void createUibcInterface();
  static void destroyUibcInterface();
  static bool updateLocalUIBCCapability(WFD_uibc_capability_t*);
  static bool getNegotiatedCapability(WFD_uibc_capability_t*,
                                      WFD_uibc_capability_t*,
                                      WFD_uibc_capability_t*);


  static bool createSession(WFD_uibc_capability_t* );
  static bool destroySession();
  static bool startUIBC();
  static bool stopUIBC();

  static bool sendUIBCEvent (WFD_uibc_event_t* event);
  static void handleUIBCEvent(bool bUIBCEvent);

private:
  static DeviceType                myDeviceType;
  static UIBCInterface*            pUibcInterface;
  static wfd_uibc_attach_cb        AttachVM;
  static wfd_uibc_send_event_cb    SendEvent;
  static wfd_uibc_hid_event_cb     sendHIDEvent;
  static bool                      bUIBCSessionValid;
  static bool                      bUIBCenableSendEvent;
  static wfd_uibc_capability_change_cb capChangeCB;
};

