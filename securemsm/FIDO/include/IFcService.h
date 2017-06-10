/*
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*/

#pragma once

#include <binder/IInterface.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <utility>

namespace android {

  class IFcService : public IInterface {
    public:
      static const char* getServiceName();

      enum {
        INITIALIZE = IBinder::FIRST_CALL_TRANSACTION,
        GET_INFO,
        REGISTER,
        SIGN,
        SIGN_FILTER,
        DEREGISTER,
        PROVISION,
        FINALIZE
      };

      virtual status_t initialize(uint32_t &session, uint32_t &version, uint8_t const (&cookie)[16]) = 0;
      virtual status_t finalize(uint32_t session) = 0;
      virtual status_t getInfo(uint32_t session) = 0;
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
          uint8_t * const response) = 0;
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
          uint8_t * const response) = 0;
      virtual status_t signFilter(
          uint32_t session,
          size_t khAccessTokenLen,
          uint8_t const * const khAccessToken,
          size_t tokenLen,
          uint8_t const * const token,
          std::vector<std::pair<size_t,uint8_t*> > const &handles,
          uint32_t &rv,
          std::vector<std::pair<size_t,uint8_t*> > &filteredHandles,
          std::vector<std::string> &usernames) = 0;
      virtual status_t deregister(uint32_t session) = 0;
      virtual status_t provision(
          uint32_t session,
          size_t dataLen,
          uint8_t const * const data) = 0;

      DECLARE_META_INTERFACE(FcService);
  };
}; // namespace android
