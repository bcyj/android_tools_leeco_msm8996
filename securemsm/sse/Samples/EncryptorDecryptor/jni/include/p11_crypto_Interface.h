/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <jni.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jboolean JNICALL Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoInit
  (JNIEnv *, jclass, jstring );

JNIEXPORT jboolean JNICALL Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoObtainToken
  (JNIEnv *, jclass, jstring );

JNIEXPORT jlong JNICALL Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoCreateKey
  (JNIEnv *, jclass, jstring, jstring );

JNIEXPORT jlong JNICALL Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoRetrieve
  (JNIEnv *, jclass, jstring );

JNIEXPORT jboolean JNICALL Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoDelete
  (JNIEnv *, jclass, jstring);

JNIEXPORT jbyteArray JNICALL Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoEncrypt
  (JNIEnv *, jclass,jbyteArray , jlong);

JNIEXPORT jbyteArray JNICALL Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoDecrypt
  (JNIEnv *, jclass, jbyteArray ,jlong );

JNIEXPORT void JNICALL Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoClose
  (JNIEnv *, jclass );

#ifdef __cplusplus
}
#endif

