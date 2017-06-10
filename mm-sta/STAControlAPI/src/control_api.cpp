//====================================================================
//
// Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
// Qualcomm Technologies Proprietary and Confidential.
//
//====================================================================
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stddef.h>

using namespace std;

#include "control_api.h"

// The external API definiitons
extern "C" {
   int isSTAProxySupported()
   {
      return sta_proxy::ControlAPI::getInstance().isFeatureSupported();
   }

   int getSTAProxyAlwaysAccelerateServicePort()
   {
      return sta_proxy::ControlAPI::getInstance().getAlwaysAccelerateServicePort();
   }

   int getSTAProxySelectivelyAccelerateServicePort()
   {
      return sta_proxy::ControlAPI::getInstance().getSelectivelyAccelerateServicePort();
   }

   bool isProxyHealthy()
   {
      return sta_proxy::ControlAPI::getInstance().isProxyHealthy();
   }
}

namespace sta_proxy
{

const char sta_socket_name[] = "sta_proxy_control_socket";

ControlAPI& ControlAPI::getInstance()
{
   static ControlAPI instance;
   return instance;
}

int ControlAPI::isFeatureSupported()
{
   int isSupported;
   if (SendReceiveMessage(STA_PROXY_IS_FEATURE_SUPPORTED_REQUEST, &isSupported) && isSupported) {
      return version_;
   } else {
      if (sock_ < 0) {
        ALOGE("client socket in bad state, recreate and use it");
        createClientSocket();
        if (SendReceiveMessage(STA_PROXY_IS_FEATURE_SUPPORTED_REQUEST, &isSupported) && isSupported) {
            return version_;
        }
      }
      return 0;
   }
}

int ControlAPI::getAlwaysAccelerateServicePort()
{
   int port;
   if (SendReceiveMessage(STA_PROXY_GET_ALWAYS_ACCELERATE_SERVICE_PORT_REQUEST, &port)) {
      return port;
   } else {
      if (sock_ < 0) {
         ALOGE("client socket in bad state, recreate and use it");
         createClientSocket();
         if (SendReceiveMessage(STA_PROXY_GET_ALWAYS_ACCELERATE_SERVICE_PORT_REQUEST, &port)) {
            return port;
         }
      }
      return -1;
   }
}

int ControlAPI::getSelectivelyAccelerateServicePort()
{
   //Not supported
   return -1;
}

bool ControlAPI::isProxyHealthy()
{
   int isHealthy;
   if (SendReceiveMessage(STA_PROXY_IS_PROXY_HEALTHY_REQUEST, &isHealthy)) {
      return (bool)isHealthy;
   } else {
      if (sock_ < 0) {
         ALOGE("client socket in bad state, recreate and use it");
         createClientSocket();
         if (SendReceiveMessage(STA_PROXY_IS_PROXY_HEALTHY_REQUEST, &isHealthy)) {
             return (bool)isHealthy;
         }
      }
      return false;
   }
}

void ControlAPI::createClientSocket()
{
   sock_ = socket(PF_UNIX, SOCK_STREAM, 0);
   if (sock_ < 0) {
      ALOGE("Failed to create a client socket");
      return;
   }

   sockaddr_un addr;
   memset(&addr, 0, sizeof(addr));
   addr.sun_family = AF_UNIX;
   socklen_t addr_len;

    // Convert the path given into abstract socket name. It must start with
    // the '\0' character, so we are adding it. |addr_len| must specify the
    // length of the structure exactly, as potentially the socket name may
    // have '\0' characters embedded (although we don't support this).
    // Note that addr.sun_path is already zero initialized.
   ALOGV("CreateClientSocket abstract connecting to %s %zu", sta_socket_name, sizeof(sta_socket_name)-1);
    memcpy(addr.sun_path + 1, sta_socket_name, sizeof(sta_socket_name)-1);
    addr_len = sizeof(sta_socket_name) + offsetof(struct sockaddr_un, sun_path);

   if (connect(sock_, reinterpret_cast<sockaddr*>(&addr), addr_len) != 0) {
      ALOGE("CreateClientSocket connect error %s", strerror(errno));
      close(sock_);
      sock_ = -1;
      return;
   }

   ALOGV("CreateClientSocket connect success");
   return;
}

bool ControlAPI::SendReceiveMessage(int msg_id, int *result)
{
   if (sock_ < 0) {
      ALOGE("SendReceiveMessage:: control socket is invalid");
      return false;
   }

   char buf[1024];
   char *msg_ptr = buf;

   // Request message length is 6 bytes
   int len = sizeof(uint16_t) + sizeof(uint32_t);
   len = htonl(len);
   uint16_t version = PROXY_CLIENT_VERSION;
   version = htons(version);
   int nmsg_id = htonl(msg_id);

   memcpy(msg_ptr, &len, sizeof(uint32_t));
   msg_ptr += sizeof(uint32_t);
   memcpy(msg_ptr, &version, sizeof(uint16_t));
   msg_ptr += sizeof(uint16_t);
   memcpy(msg_ptr, &nmsg_id, sizeof(uint32_t));

   int ret = send(sock_, buf, sizeof(uint16_t) + 2*sizeof(uint32_t), 0);
   ALOGV("SendReceiveMessage::send ret %d", ret);

   if (ret == -1) {
      ALOGE("SendReceiveMessage::send error %s", strerror(errno));
      close(sock_);
      sock_ = -1;
      return false;
   }

   ret = read(sock_, buf, sizeof(uint16_t) + 3*sizeof(uint32_t));
   ALOGV("SendReceiveMessage::read ret %d", ret);

   if (ret == -1) {
      ALOGE("SendReceiveMessage::read error %s", strerror(errno));
      close(sock_);
      sock_ = -1;
      return false;
   }

   // Reset message pointer to the head of the buffer
   msg_ptr = buf;

   if (ret != sizeof(uint16_t) + 3*sizeof(uint32_t)) {
      ALOGE("SendReceiveMessage received invalid message");
      return false;
   }
   else {
      memcpy(&len, msg_ptr, sizeof(uint32_t));
      len = ntohl(len);
      if (len != sizeof(uint16_t) + 2*sizeof(uint32_t)) {
         ALOGE("SendReceiveMessage received wrong message length %d", len);
         return false;
      }
   }

   msg_ptr += sizeof(uint32_t);
   memcpy(&version, msg_ptr, sizeof(uint16_t));
   version_ = ntohs(version);
   if (version_ == 0) {
      ALOGE("SendReceiveMessage recvd invalid version");
      return false;
   }

   msg_ptr += sizeof(uint16_t);
   memcpy(&msg_id, msg_ptr, sizeof(uint32_t));
   msg_id = ntohl(msg_id);

   msg_ptr += sizeof(uint32_t);
   memcpy(result, msg_ptr, sizeof(uint32_t));
   *result = ntohl(*result);

   ALOGV("SendReceiveMessage recvd version %d msg_id %d, rt %d",
                      version_, msg_id, *result);
   ALOGV("Client version: %d", PROXY_CLIENT_VERSION);

   return true;
}

} // namespace sta_proxy
