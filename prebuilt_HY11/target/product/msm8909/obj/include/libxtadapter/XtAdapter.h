/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef XT_ADAPTER_H
#define XT_ADAPTER_H

#include <IzatAdapterBase.h>

using namespace loc_core;
using namespace izat_core;

class XtAdapter : public IzatAdapterBase {
    bool mUserPref;
public:
    XtAdapter(MsgTask::tCreate tCreator);
    XtAdapter(MsgTask::tAssociate tAsso);
    inline virtual ~XtAdapter() {}
    void setUserPreferenceSync(bool user_pref);
    virtual bool setUserPreference(bool user_pref);
    virtual void handleEngineUpEvent();
};

#endif /* XT_ADAPTER_H */
