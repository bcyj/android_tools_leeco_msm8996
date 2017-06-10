//====================================================================
//
// Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
// Qualcomm Technologies Proprietary and Confidential.
//
//====================================================================

#ifndef STA_PROXY_API_CLIENT_CONTROL_API_H
#define STA_PROXY_API_CLIENT_CONTROL_API_H

//#define LOG_NDEBUG 0
#define LOG_TAG "STA_API"

#include <android/log.h>
#include <inttypes.h>
#include <utils/Log.h>
#include <utils/Trace.h>

// The exported API declarations
extern "C" {
   int isSTAProxySupported();
   int getSTAProxyAlwaysAccelerateServicePort();
   int getSTAProxySelectivelyAccelerateServicePort();
   bool isProxyHealthy();
}

namespace sta_proxy
{

enum StaProxyMessageId {
   STA_PROXY_IS_FEATURE_SUPPORTED_REQUEST = 0,
   STA_PROXY_IS_FEATURE_SUPPORTED_RESPONSE,
   STA_PROXY_GET_ALWAYS_ACCELERATE_SERVICE_PORT_REQUEST,
   STA_PROXY_GET_ALWAYS_ACCELERATE_SERVICE_PORT_RESPONSE,
   STA_PROXY_GET_SELECTIVELY_ACCELERATE_SERVICE_PORT_REQUEST,
   STA_PROXY_GET_SELECTIVELY_ACCELERATE_SERVICE_PORT_RESPONSE,
   STA_PROXY_IS_PROXY_HEALTHY_REQUEST,
   STA_PROXY_IS_PROXY_HEALTHY_RESPONSE
};

#define PROXY_CLIENT_VERSION 1

//
// @brief this class implements the STA control API
// It is implemented as a singleton.
//
class ControlAPI
{
public:
   ~ControlAPI() {
      if (sock_) {
         // close(sock_);
      }
   }
   // @brief retrieves the singleton instance
   static ControlAPI& getInstance();

   // @brief If positive the feature is supported and the return value represents
    // the version number of messaging protocol, otherwise the feature is not supported
   int isFeatureSupported();

   // @brief returns the port used for accelerating all HTTP GET requests, -1 if none exists
   int getAlwaysAccelerateServicePort();

   // @brief returns the port used for accelerating selectively, -1 if none exists
   // The details of the HTTP tags used to determine which get accelerated
   // are described in the STA control API document
   int getSelectivelyAccelerateServicePort();

   // @brief returns true if proxy is healthy, false otherwise
   bool isProxyHealthy();

private:
   void createClientSocket();
   bool SendReceiveMessage(int msg_id, int *result);

   // Private constructor for singleton pattern
   ControlAPI() : version_(0){
      createClientSocket();
   }
   // Dis-allowed due to singleton
   ControlAPI(const ControlAPI&);
   // Dis-allowed due to singleton
   ControlAPI& operator=(const ControlAPI&);

   // File descriptor of the client's unix domain socket
   int sock_;
   unsigned short version_;
}; //class ControlAPI

} // namespace sta_proxy

#endif //STA_PROXY_API_CLIENT_CONTROL_API_H
