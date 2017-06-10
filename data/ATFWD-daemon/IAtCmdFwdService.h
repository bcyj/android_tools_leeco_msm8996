/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef ANDROID_HARDWARE_IATCMDFWDSERVICE_H
#define ANDROID_HARDWARE_IATCMDFWDSERVICE_H

#include <binder/IInterface.h>
#include <utils/String16.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include "AtCmdFwd.h"


namespace android {

class IAtCmdFwdService : public IInterface
{
public:
    DECLARE_META_INTERFACE(AtCmdFwdService);

    virtual AtCmdResponse *processCommand(const AtCmd &cmd) = 0;
};

class DeathNotifier: public IBinder::DeathRecipient
{
public:
    DeathNotifier() {
    }

    virtual void binderDied(const wp<IBinder>& who);
};

}; // namespace android
#endif // ANDROID_HARDWARE_IATCMDFWDSERVICE_H

