/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "p11_crypto_Interface.h"
#include "p11_crypto.h"

extern "C" jboolean
Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoInit (JNIEnv*  env, jclass cls, jstring inJNIStr)
{
  const char *inCStr = env->GetStringUTFChars(inJNIStr, NULL);
  if (NULL == inCStr)
    return false;

  LOGD("Pki_p11_crypto_init+  inJNIStr = %s",inCStr );
  Pki * pki = Pki::initPkiInstance(inCStr);
  if (NULL_PTR == pki) {
    env->ReleaseStringUTFChars(inJNIStr, inCStr);
    LOGE("Pki::initPkiInstance returned NULL_PTR");
    LOGD("Pki_p11_crypto_init-");
    return false;
  }
  env->ReleaseStringUTFChars(inJNIStr, inCStr);
  LOGD("Pki_p11_crypto_init-");
  return true;
}

extern "C" jboolean
Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoObtainToken (JNIEnv*  env, jclass cls, jstring inJNIStr)
{
  const char *inCStr = env->GetStringUTFChars(inJNIStr, NULL);
  if (NULL == inCStr)
    return false;

  LOGD("Pki_p11_crypto_obtainToken+  inJNIStr = %s",inCStr);
  Pki * pki =  Pki::getPkiInstance();
  if (NULL_PTR == pki) {
    env->ReleaseStringUTFChars(inJNIStr, inCStr);
    LOGE("Pki::getPkiInstance() returned NULL_PTR");
    LOGD("Pki_p11_crypto_obtainToken-");
    return false;
  }

  if (!pki->obtainToken(inCStr)){
    env->ReleaseStringUTFChars(inJNIStr, inCStr);
    LOGE("pki->obtainToken(inCStr) returned false ");
    LOGD("Pki_p11_crypto_obtainToken-");
    return false;
  }

  env->ReleaseStringUTFChars(inJNIStr, inCStr);
  LOGD("Pki_p11_crypto_obtainToken-");
  return true;
}

extern "C" jlong
Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoCreateKey (JNIEnv*  env, jclass cls, jstring inJNIStr1, jstring inJNIStr2)
{

  const char *inCStr1 = env->GetStringUTFChars(inJNIStr1, NULL);
  if (NULL == inCStr1)
    return false;

  const char *inCStr2 = env->GetStringUTFChars(inJNIStr2, NULL);
  if (NULL == inCStr2)
    return false;

  LOGD("Pki_p11_crypto_createKey+  inJNIStr1 = %s , inJNIStr2 = %s");
  Pki * pki = Pki::getPkiInstance();
  if (NULL_PTR == pki) {
    env->ReleaseStringUTFChars(inJNIStr1, inCStr1);
    env->ReleaseStringUTFChars(inJNIStr2, inCStr2);
    LOGE("Pki::getPkiInstance() returned NULL_PTR");
    LOGD("Pki_p11_crypto_createKey-");
    return 0;
  }

  long val = pki->createKey(inCStr1,inCStr2);
  if (!val){
    env->ReleaseStringUTFChars(inJNIStr1, inCStr1);
    env->ReleaseStringUTFChars(inJNIStr2, inCStr2);
    LOGE("pki->createKey(inCStr) returned false ");
    LOGD("Pki_p11_crypto_createKey-");
    return 0;
  }

  env->ReleaseStringUTFChars(inJNIStr1, inCStr1);
  env->ReleaseStringUTFChars(inJNIStr2, inCStr2);
  LOGD("Pki_p11_crypto_createKey-");
  return val;

}

extern "C" jlong
Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoRetrieve (JNIEnv*  env, jclass cls, jstring inJNIStr)
{
  const char *inCStr = env->GetStringUTFChars(inJNIStr, NULL);
  if (NULL == inCStr)
    return 0;

  LOGD("Pki_p11_crypto_retrieve+ inJNIStr = %s",inCStr);
  Pki * pki = Pki::getPkiInstance();;
  if (NULL_PTR == pki) {
    env->ReleaseStringUTFChars(inJNIStr, inCStr);
    LOGE("Pki::getPkiInstance() returned NULL_PTR");
    LOGD("Pki_p11_crypto_retrieve-");
    return 0;
  }

  long val  = pki->retrieveKey(inCStr);
  if (0 == val){
    env->ReleaseStringUTFChars(inJNIStr, inCStr);
    LOGE("pki->retrieveKey(inCStr) returned 0 ");
    LOGD("Pki_p11_crypto_retrieve-");
    return 0;
  }

  LOGD("Pki_p11_crypto_retrieve- 0x%x",val);
  env->ReleaseStringUTFChars(inJNIStr, inCStr);
  return val;
}

extern "C" jboolean
Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoDelete(JNIEnv*  env, jclass cls, jstring inJNIStr)
{
  const char *inCStr = env->GetStringUTFChars(inJNIStr, NULL);
  if (NULL == inCStr)
    return false;

  LOGD("Pki_p11_crypto_delete+  inJNIStr = %s",inCStr);
  Pki * pki = Pki::getPkiInstance();
  if (NULL_PTR == pki) {
    env->ReleaseStringUTFChars(inJNIStr, inCStr);
    LOGE("Pki::getPkiInstance() returned NULL_PTR");

    return false;
  }

  if (!pki->deleteKey(inCStr)){
    env->ReleaseStringUTFChars(inJNIStr, inCStr);
    LOGE("pki->deleteKey(inCStr) returned false ");
    LOGD("Pki_p11_crypto_delete-");
    return false;
  }

  env->ReleaseStringUTFChars(inJNIStr, inCStr);
  LOGD("Pki_p11_crypto_delete-");
  return true;
}

extern "C" jbyteArray
Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoEncrypt(JNIEnv*  env, jclass cls, jbyteArray inJNIArray, jlong handle)
{

  jbyte *inCArray = env->GetByteArrayElements(inJNIArray, NULL);
  if (NULL == inCArray)
    return NULL;
  jsize length = env->GetArrayLength(inJNIArray);

  LOGD("Pki_p11_crypto_encrypt+  inJNIStr.length = %d,handle = %d", length, handle);

  Pki * pki = Pki::getPkiInstance();
  if (NULL_PTR == pki) {
    env->ReleaseByteArrayElements(inJNIArray, inCArray, 0);
    LOGE("Pki::getPkiInstance() returned NULL_PTR");
    LOGD("Pki_p11_crypto_encrypt-");
    return NULL;
  }

  // AES out length computation
  int D = (int )(length / 16);
  jsize outLen = length + 16;
  if (0 != ( length - D * 16)) {
      outLen -= (length % 16);
  }
  LOGD("AES out length computation - insize %d , outSize %d ", length, outLen );


  jbyte *outCArray= (jbyte *) malloc( sizeof(jbyte)* outLen );
  if (NULL == outCArray){
    env->ReleaseByteArrayElements(inJNIArray, inCArray, 0);
    LOGE("malloc error in JNI  ");
    LOGD("Pki_p11_crypto_encrypt-");
    return NULL;
  }

  if (!pki->encrypt((CK_BYTE_PTR)inCArray, length, (CK_BYTE_PTR)outCArray, outLen, handle)){
    env->ReleaseByteArrayElements(inJNIArray, inCArray, 0);
    LOGE("pki->encrypt() returned NULL ");
    LOGD("Pki_p11_crypto_encrypt-");
    return NULL;
  }

  env->ReleaseByteArrayElements(inJNIArray, inCArray, 0);

  jbyteArray outJNIArray = env->NewByteArray(outLen);
  if (NULL == outJNIArray) {
      LOGE("JNI NewByteArray returned NULL ");
      LOGD("Pki_p11_crypto_encrypt-");
      return NULL;
  }
  env->SetByteArrayRegion(outJNIArray, 0 , outLen, outCArray);
  LOGD("Pki_p11_crypto_encrypt-");
  return outJNIArray;
}

extern "C" jbyteArray
Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoDecrypt(JNIEnv*  env, jclass cls, jbyteArray inJNIArray, jlong handle)
{
  jbyte *inCArray = env->GetByteArrayElements(inJNIArray, NULL);
  if (NULL == inCArray)
    return NULL;
  jsize length = env->GetArrayLength(inJNIArray);

  LOGD("Pki_p11_crypto_encrypt+  inJNIStr.length = %d,handle = %d", length, handle);

  Pki * pki = Pki::getPkiInstance();
  if (NULL_PTR == pki) {
    env->ReleaseByteArrayElements(inJNIArray, inCArray, 0);
    LOGE("Pki::getPkiInstance() returned NULL_PTR");
    LOGD("Pki_p11_crypto_encrypt-");
    return NULL;
  }

  jbyte *outCArray= (jbyte *) malloc( sizeof(jbyte)* length );
  if (NULL == outCArray){
    env->ReleaseByteArrayElements(inJNIArray, inCArray, 0);
    LOGE("malloc error in JNI  ");
    LOGD("Pki_p11_crypto_encrypt-");
    return NULL;
  }

  CK_ULONG outLen;
  if (!pki->decrypt((CK_BYTE_PTR)inCArray, length, (CK_BYTE_PTR) outCArray,&outLen , handle)){
    env->ReleaseByteArrayElements(inJNIArray, inCArray, 0);
    LOGE("pki->encrypt() returned NULL ");
    LOGD("Pki_p11_crypto_encrypt-");
    return NULL;
  }

  env->ReleaseByteArrayElements(inJNIArray, inCArray, 0);

  jbyteArray outJNIArray = env->NewByteArray(outLen);
  if (NULL == outJNIArray) {
      LOGE("JNI NewByteArray returned NULL ");
      LOGD("Pki_p11_crypto_encrypt-");
      return NULL;
  }
  env->SetByteArrayRegion(outJNIArray, 0 , outLen, outCArray);
  LOGD("Pki_p11_crypto_encrypt-");
  return outJNIArray;
}

extern "C" void
Java_com_qualcomm_secureservices_encryptordecryptor_Pki_p11CryptoClose (JNIEnv*  env, jclass cls)
{
  LOGD("Pki_p11_crypto_close+");
  Pki * pki = Pki::getPkiInstance();
  if (NULL_PTR != pki) {
    pki->closePki();
  }
  LOGD("Pki_p11_crypto_close-");
}

