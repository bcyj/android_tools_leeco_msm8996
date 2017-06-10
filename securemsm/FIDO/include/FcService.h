/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#pragma once

#include <map>

#include <binder/Parcel.h>
#include <binder/BinderService.h>
#include "BnFcService.h"

class IFCConnector;

namespace android {
  class FcService: public BinderService<FcService>,  public BnFcService
  {
    friend class BinderService<FcService>;

    public:
      FcService();
      virtual ~FcService();
      virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0);

      virtual status_t initialize(uint32_t &session, uint32_t &version, uint8_t const (&cookie)[16]);
      virtual status_t finalize(uint32_t session);
      virtual status_t getInfo(uint32_t session);
      virtual status_t reg(
          uint32_t session,
          size_t finalChallengeLen,
          uint8_t const * const finalChallenge,
          size_t khAccessTokenLen,
          uint8_t const * const khAccessToken,
          const std::string &username,
          size_t tokenLen,
          uint8_t const * const token,
          size_t &responseLen,
          uint8_t * const response);
      virtual status_t sign(
          uint32_t session,
          size_t finalChallengeLen,
          uint8_t const * const finalChallenge,
          size_t khAccessTokenLen,
          uint8_t const * const khAccessToken,
          size_t keyHandleLen,
          uint8_t const * const keyHandle,
          size_t transactionLen,
          uint8_t const * const transaction,
          size_t tokenLen,
          uint8_t const * const token,
          size_t &responseLen,
          uint8_t * const response);
      virtual status_t signFilter(
          uint32_t session,
          size_t khAccessTokenLen,
          uint8_t const * const khAccessToken,
          size_t tokenLen,
          uint8_t const * const token,
          std::vector<std::pair<size_t,uint8_t*> > const &handles,
          uint32_t &rv,
          std::vector<std::pair<size_t,uint8_t*> > &filteredHandles,
          std::vector<std::string> &usernames);
      virtual status_t deregister(uint32_t session);
      virtual status_t provision(
          uint32_t session,
          size_t dataLen,
          uint8_t const * const data);

    private:
      std::map<uint32_t, void *> sessions;
      IFCConnector *fc;
  };
}; // namespace android

