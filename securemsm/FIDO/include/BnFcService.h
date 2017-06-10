/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#pragma once

#include <binder/Parcel.h>
#include "IFcService.h"

namespace android {
  class BnFcService: public BnInterface<IFcService>
  {
    public:
      virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0);

  };
}; // namespace android

