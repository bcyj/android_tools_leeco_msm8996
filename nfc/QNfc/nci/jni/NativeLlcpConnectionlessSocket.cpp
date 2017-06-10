/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <semaphore.h>
#include <errno.h>
#include "OverrideLog.h"
#ifdef DTA // <DTA>
#include "LlcpConnectionlessSocket.h"
#endif // </DTA>

#include "NfcJniUtil.h"
#include "JavaClassConstants.h"
#include <ScopedLocalRef.h>
#include <ScopedPrimitiveArray.h>
extern "C"
{
    #include "nfa_api.h"
    #include "nfa_p2p_api.h"
    #ifdef DTA // <DTA>
    #include "dta_mode.h"
    #endif // </DTA>
}


namespace android
{


/*****************************************************************************
**
** private variables and functions
**
*****************************************************************************/
static sem_t        sConnlessRecvSem;
static jboolean     sConnlessRecvWaitingForData = JNI_FALSE;
static uint8_t*     sConnlessRecvBuf = NULL;
static uint32_t     sConnlessRecvLen = 0;
static uint32_t     sConnlessRecvRemoteSap = 0;


/*******************************************************************************
**
** Function:        nativeLlcpConnectionlessSocket_doSendTo
**
** Description:     Send data to peer.
**                  e: JVM environment.
**                  o: Java object.
**                  nsap: service access point.
**                  data: buffer for data.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nativeLlcpConnectionlessSocket_doSendTo (JNIEnv *e, jobject o, jint nsap, jbyteArray data)
{
    ALOGD ("%s: nsap = %d", __FUNCTION__, nsap);

    ScopedLocalRef<jclass> c(e, e->GetObjectClass(o));
    jfieldID f = e->GetFieldID(c.get(), "mHandle", "I");
    jint handle = e->GetIntField(o, f);

    ScopedByteArrayRO bytes(e, data);
    if (bytes.get() == NULL)
    {
        return JNI_FALSE;
    }
    size_t byte_count = bytes.size();

    ALOGD("NFA_P2pSendUI: len = %d", byte_count);
    UINT8* raw_ptr = const_cast<UINT8*>(reinterpret_cast<const UINT8*>(&bytes[0])); // TODO: API bug; NFA_P2pSendUI should take const*!
    tNFA_STATUS status = NFA_P2pSendUI((tNFA_HANDLE) handle, nsap, byte_count, raw_ptr);

    ALOGD("%s: NFA_P2pSendUI done, status = %d", __FUNCTION__, status);
    if (status != NFA_STATUS_OK)
    {
        ALOGE("%s: NFA_P2pSendUI failed, status = %d", __FUNCTION__, status);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}


/*******************************************************************************
**
** Function:        nativeLlcpConnectionlessSocket_receiveData
**
** Description:     Receive data from the stack.
**                  data: buffer contains data.
**                  len: length of data.
**                  remoteSap: remote service access point.
**
** Returns:         None
**
*******************************************************************************/
void nativeLlcpConnectionlessSocket_receiveData (uint8_t* data, uint32_t len, uint32_t remoteSap)
{
    ALOGD ("%s: waiting for data = %d, len = %d", __FUNCTION__, sConnlessRecvWaitingForData, len);

    // Sanity...
    if (sConnlessRecvLen < len)
    {
        len = sConnlessRecvLen;
    }

    if (sConnlessRecvWaitingForData)
    {
        sConnlessRecvWaitingForData = JNI_FALSE;
        sConnlessRecvLen = len;
        memcpy (sConnlessRecvBuf, data, len);
        sConnlessRecvRemoteSap = remoteSap;

        sem_post (&sConnlessRecvSem);
    }
}


/*******************************************************************************
**
** Function:        connectionlessCleanup
**
** Description:     Free resources.
**
** Returns:         None
**
*******************************************************************************/
static jobject connectionlessCleanup ()
{
    sConnlessRecvWaitingForData = JNI_FALSE;
    sConnlessRecvLen = 0;
    if (sConnlessRecvBuf != NULL)
    {
        free (sConnlessRecvBuf);
        sConnlessRecvBuf = NULL;
    }
    return NULL;
}


/*******************************************************************************
**
** Function:        nativeLlcpConnectionlessSocket_abortWait
**
** Description:     Abort current operation and unblock threads.
**
** Returns:         None
**
*******************************************************************************/
void nativeLlcpConnectionlessSocket_abortWait ()
{
#ifdef DTA // <DTA>
    if(!in_dta_mode())
    {
        sem_post (&sConnlessRecvSem);
    }
#endif // </DTA>
}


/*******************************************************************************
**
** Function:        nativeLlcpConnectionlessSocket_doReceiveFrom
**
** Description:     Receive data from a peer.
**                  e: JVM environment.
**                  o: Java object.
**                  linkMiu: max info unit
**
** Returns:         LlcpPacket Java object.
**
*******************************************************************************/
static jobject nativeLlcpConnectionlessSocket_doReceiveFrom (JNIEnv* e, jobject, jint linkMiu)
{
    ALOGD ("%s: linkMiu = %d", __FUNCTION__, linkMiu);
    jobject llcpPacket = NULL;
    ScopedLocalRef<jclass> clsLlcpPacket(e, NULL);

    if (sConnlessRecvWaitingForData != JNI_FALSE)
    {
        ALOGD ("%s: Already waiting for incoming data", __FUNCTION__);
        return NULL;
    }

    sConnlessRecvBuf = (uint8_t*) malloc (linkMiu);
    if (sConnlessRecvBuf == NULL)
    {
        ALOGD ("%s: Failed to allocate %d bytes memory buffer", __FUNCTION__, linkMiu);
        return NULL;
    }
    sConnlessRecvLen = linkMiu;

    // Create the write semaphore
    if (sem_init (&sConnlessRecvSem, 0, 0) == -1)
    {
        ALOGE ("%s: semaphore creation failed (errno=0x%08x)", __FUNCTION__, errno);
        return connectionlessCleanup ();
    }

    sConnlessRecvWaitingForData = JNI_TRUE;

    // Wait for sConnlessRecvSem completion status
    if (sem_wait (&sConnlessRecvSem))
    {
        ALOGE ("%s: Failed to wait for write semaphore (errno=0x%08x)", __FUNCTION__, errno);
        goto TheEnd;
    }

    // Create new LlcpPacket object
    if (nfc_jni_cache_object_local (e, "com/android/nfc/LlcpPacket", &(llcpPacket)) == -1)
    {
        ALOGE ("%s: Find LlcpPacket class error", __FUNCTION__);
        return connectionlessCleanup ();
    }

    // Get NativeConnectionless class object
    clsLlcpPacket.reset(e->GetObjectClass(llcpPacket));
    if (e->ExceptionCheck())
    {
        e->ExceptionClear();
        ALOGE ("%s: Get Object class error", __FUNCTION__);
        return connectionlessCleanup ();
    }

    // Set Llcp Packet remote SAP
    jfieldID f;
    f = e->GetFieldID(clsLlcpPacket.get(), "mRemoteSap", "I");
    e->SetIntField(llcpPacket, f, (jbyte) sConnlessRecvRemoteSap);

    // Set Llcp Packet Buffer
    ALOGD ("%s: Received Llcp packet buffer size = %d\n", __FUNCTION__, sConnlessRecvLen);
    f = e->GetFieldID(clsLlcpPacket.get(), "mDataBuffer", "[B");

    {
        ScopedLocalRef<jbyteArray> receivedData(e, e->NewByteArray(sConnlessRecvLen));
        e->SetByteArrayRegion(receivedData.get(), 0, sConnlessRecvLen, (jbyte*) sConnlessRecvBuf);
        e->SetObjectField(llcpPacket, f, receivedData.get());
    }

TheEnd: // TODO: should all the "return connectionlessCleanup()"s in this function jump here instead?
    connectionlessCleanup ();
    if (sem_destroy (&sConnlessRecvSem))
    {
        ALOGE ("%s: Failed to destroy sConnlessRecvSem semaphore (errno=0x%08x)", __FUNCTION__, errno);
    }
    return llcpPacket;
}


/*******************************************************************************
**
** Function:        nativeLlcpConnectionlessSocket_doClose
**
** Description:     Close socket.
**                  e: JVM environment.
**                  o: Java object.
**
** Returns:         True if ok.
**
*******************************************************************************/
static jboolean nativeLlcpConnectionlessSocket_doClose (JNIEnv *e, jobject o)
{
    ALOGD ("%s", __FUNCTION__);

    ScopedLocalRef<jclass> c(e, e->GetObjectClass(o));
    jfieldID f = e->GetFieldID(c.get(), "mHandle", "I");
    jint handle = e->GetIntField(o, f);

    tNFA_STATUS status = NFA_P2pDisconnect((tNFA_HANDLE) handle, FALSE);
    if (status != NFA_STATUS_OK)
    {
        ALOGE ("%s: disconnect failed, status = %d", __FUNCTION__, status);
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

#ifdef DTA // <DTA>
/**
 * Opens a LLCP connectionless socket.
 *
 * @param e JVM environment.
 * @param o Calling Java object.
 * @param localServiceName The local service name of the socket.
 * @param localSap The local SAP of the socket.
 *
 * @return The local SAP of the socket or -1 if opening the socket failed.
 */
static jint nativeLlcpConnectionlessSocket_doOpen(JNIEnv *e, jobject o, jstring localServiceName, jint localSap) {

  char* sn = (char*)e->GetStringUTFChars(localServiceName, JNI_FALSE);

  LlcpConnectionlessSocket* socket = LlcpConnectionlessSocket::open(sn, localSap);

  e->ReleaseStringUTFChars(localServiceName, sn);

  if (socket == 0) {
    ALOGD("%s [CL]: socket=0 !!", __FUNCTION__);
    return -1;
  }

  ALOGD("%s [CL]: local SAP=0x%x", __FUNCTION__, socket->getLocalSap());

  return socket->getLocalSap();
}

/**
 * Sends a PDU data packet to the specified remote SAP.
 *
 * @param e JVM environment.
 * @param o Calling Java object.
 * @param remoteSap The remote SAP where the PDU is sent to.
 * @param data The vector containing the PDU.
 * @param localSap The local SAP of the socket.
 *
 * @return JNI_TRUE if sending was successfull, JNI_FALSE if sending failed.
 */
static jboolean nativeLlcpConnectionlessSocket_doSendToCL(JNIEnv *e, jobject o, jint remoteSap, jbyteArray data, jint localSap) {

  LlcpConnectionlessSocket* socket = LlcpConnectionlessSocket::getSocket(localSap);
  if (socket == 0) {
    return 0;
  }
  uint8_t* begin = reinterpret_cast<uint8_t*>(e->GetByteArrayElements (data, NULL));
  uint8_t* end = begin + e->GetArrayLength(data);

  std::vector<uint8_t> writeBuffer(begin, end);
  ALOGD("%s [CL]: vector length = %d", __FUNCTION__, writeBuffer.size());

  return socket->send(remoteSap, writeBuffer);
}

/**
 * Sends a PDU data packet to a remote peer the provides the specified service name.
 *
 * @param e JVM environment.
 * @param o Calling Java object.
 * @param remoteServiceName The name of the service provided by the remote peer.
 * @param data The vector containing the PDU.
 * @param localSap The local SAP of the socket.
 *
 * @return JNI_TRUE if sending was successfull, JNI_FALSE if sending failed.
 */
static jboolean nativeLlcpConnectionlessSocket_doSendSDP(JNIEnv *e, jobject o, jstring remoteServiceName, jbyteArray data, jint localSap) {

  LlcpConnectionlessSocket* socket = LlcpConnectionlessSocket::getSocket(localSap);
  if (socket == 0) {
    return JNI_FALSE;
  }
  uint8_t* begin = reinterpret_cast<uint8_t*>(e->GetByteArrayElements (data, NULL));
  uint8_t* end = begin + e->GetArrayLength(data);

  std::vector<uint8_t> writeBuffer(begin, end);
  ALOGD("%s [CL]: vector length = %d", __FUNCTION__, writeBuffer.size());
  char* sn = (char*)e->GetStringUTFChars(remoteServiceName, JNI_FALSE);

  jboolean status = socket->send(sn, writeBuffer);

  e->ReleaseStringUTFChars(remoteServiceName, sn);

  return status;
}

/**
 * Receives a PDU from a remote peer. This method will block indefinitely until it receives the PDU or
 * the socket gets closed.
 *
 * @param e JVM environment.
 * @param o Calling Java object.
 * @param llcpPacket The LlcpPacket instance where the received PDU is stored.
 * @param linkMiu TODO
 * @param localSap The local SAP of the socket.
 *
 * @return JNI_TRUE if sending was successfull, JNI_FALSE if sending failed.
 */
static jint nativeLlcpConnectionlessSocket_doReceiveFromCL (JNIEnv *e, jobject o, jobject llcpPacket, jint linkMiu, jint localSap) {

  LlcpConnectionlessSocket* socket = LlcpConnectionlessSocket::getSocket(localSap);
  if (socket == 0) {
    return -1;
  }

  std::vector<unsigned char> readBuffer;

  int remoteSap = socket->receive(readBuffer);
  if (remoteSap == -1) {
    return -1;
  }

  // Assign remote SAP and received data array into LlcpPacket Java instance:
  jclass cls = e->GetObjectClass(llcpPacket);

  jfieldID f = e->GetFieldID(cls, "mRemoteSap", "I");
  e->SetIntField(llcpPacket, f, remoteSap);

  f = e->GetFieldID(cls, "mDataBuffer", "[B");
  jbyteArray receivedData = e->NewByteArray(readBuffer.size());
  e->SetByteArrayRegion (receivedData, 0, readBuffer.size(), (jbyte*) &(readBuffer[0]));
  e->SetObjectField(llcpPacket, f, receivedData);

  return remoteSap;
}

/**
 * Closes the socket.
 *
 * @param e JVM environment.
 * @param o Calling Java object.
 * @param localSap The local SAP of the socket.
 *
 * @return JNI_TRUE TODO: change to void
 */
static jboolean nativeLlcpConnectionlessSocket_doCloseCL(JNIEnv *e, jobject o, jint localSap) {

  LlcpConnectionlessSocket* socket = LlcpConnectionlessSocket::getSocket(localSap);
  if (socket != 0) {
    socket->close();
  }

  return JNI_TRUE;
}
#endif // </DTA>

/*****************************************************************************
**
** Description:     JNI functions
**
*****************************************************************************/
static JNINativeMethod gMethods[] =
{
    {"doSendTo", "(I[B)Z", (void*) nativeLlcpConnectionlessSocket_doSendTo},
    {"doReceiveFrom", "(I)Lcom/android/nfc/LlcpPacket;", (void*) nativeLlcpConnectionlessSocket_doReceiveFrom},
    {"doClose", "()Z", (void*) nativeLlcpConnectionlessSocket_doClose},
#ifdef DTA // <DTA>
    {"doOpen", "(Ljava/lang/String;I)I", (void*) nativeLlcpConnectionlessSocket_doOpen},
    {"doSendTo", "(I[BI)Z", (void*) nativeLlcpConnectionlessSocket_doSendToCL},
    {"doSendTo", "(Ljava/lang/String;[BI)Z", (void*) nativeLlcpConnectionlessSocket_doSendSDP},
    {"doReceiveFrom", "(Lcom/android/nfc/LlcpPacket;II)I", (void*) nativeLlcpConnectionlessSocket_doReceiveFromCL},
    {"doClose", "(I)Z", (void*) nativeLlcpConnectionlessSocket_doCloseCL},
#endif // </DTA>
};


/*******************************************************************************
**
** Function:        register_com_android_nfc_NativeLlcpConnectionlessSocket
**
** Description:     Regisgter JNI functions with Java Virtual Machine.
**                  e: Environment of JVM.
**
** Returns:         Status of registration.
**
*******************************************************************************/
int register_com_android_nfc_NativeLlcpConnectionlessSocket (JNIEnv *e)
{
    return jniRegisterNativeMethods (e, gNativeLlcpConnectionlessSocketClassName, gMethods, NELEM(gMethods));
}


} // android namespace
