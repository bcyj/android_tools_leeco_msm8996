/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "p11_crypto.h"
#include <dlfcn.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))

const char* Pki::userName;
const char* Pki::userPin;
const char* Pki::salt;

bool Pki::pkcsModule  = false;
Pki* Pki::instance    = NULL_PTR;

CK_FUNCTION_LIST_PTR Pki::pFunctionList = NULL_PTR;
CK_SLOT_ID           Pki::currentSlotId;

CK_RV Pki::create_mutex(void **mutex)
{
  CK_RV ret = CKR_OK;

  if (!mutex) {
    ret = CKR_ARGUMENTS_BAD;
  } else {

    pthread_mutex_t *mut;
    mut = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    if (mut == NULL) {
      assert(mut);
      return CKR_HOST_MEMORY;
    }

    if (pthread_mutex_init(mut, NULL) != 0)
      assert(0);
    *mutex = mut;

  }

  return ret;
}

CK_RV Pki::destroy_mutex(void *mutex)
{
  CK_RV ret = CKR_OK;

  if (!mutex) {
    ret = CKR_MUTEX_BAD;
  } else {
    int res = pthread_mutex_destroy((pthread_mutex_t*) mutex);
    if (res != 0) {
      ret = CKR_GENERAL_ERROR;
    } else {
      free(mutex);
    }
  }
  return ret;
}

CK_RV Pki::lock_mutex(void *mutex)
{
  CK_RV ret = CKR_OK;

  if (!mutex) {
    ret = CKR_MUTEX_BAD;
  } else {
    int res = pthread_mutex_lock((pthread_mutex_t*) mutex);
    if (res != 0) {
      ret = CKR_GENERAL_ERROR;
    }
  }
  return ret;
}

CK_RV Pki::unlock_mutex(void *mutex)
{
  CK_RV ret = CKR_OK;

  if (!mutex) {
    ret = CKR_MUTEX_BAD;
  } else {
    int res = pthread_mutex_unlock((pthread_mutex_t*) mutex);
    if (res != 0) {
      ret = CKR_GENERAL_ERROR;
    }
  }
  return ret;
}

#define SALT "Salt"

Pki::Pki(const char* intStr){
  if(initModule(intStr)) {
    pkcsModule = true;
  }
}


bool
Pki::initModule(const char* intStr){
  CK_RV rv;
  CK_C_GetFunctionList pC_GetFunctionList = NULL_PTR;
  CK_C_Initialize      pC_Initialize      = NULL_PTR;

  LOGD("Pki::initModule+");

  void * module = dlopen("libSSEPKCS11.so", RTLD_NOW);
  if (module == NULL_PTR) {
    LOGE("Could not open library libSSEPKCS11.so : %s", dlerror());
    LOGD("Pki::initModule-");
    return false;
  }

  pC_GetFunctionList = (CK_C_GetFunctionList) dlsym(module, "C_GetFunctionList");
  if (pC_GetFunctionList == NULL_PTR) {
    LOGE("Could not C_GetFunctionList symbol in Lib");
    LOGD("Pki::initModule-");
    return false;
  }

  // initializing module functions
  // rv = C_GetFunctionList(&pFunctionList);
  rv = (pC_GetFunctionList)(&Pki::pFunctionList);
  if (CKR_OK != rv){
    LOGE("Error in C_GetFunctionList : 0x%x", rv);
    LOGD("Pki::initModule-");
    return false;
  }

  pC_Initialize = Pki::pFunctionList -> C_Initialize;
  // Call the C_Initialize function in the library
  CK_C_INITIALIZE_ARGS init_args;

  memset(&init_args, 0, sizeof(init_args));
  init_args.flags        = 0;
  init_args.CreateMutex  = Pki::create_mutex;
  init_args.DestroyMutex = Pki::destroy_mutex;
  init_args.LockMutex    = Pki::lock_mutex;
  init_args.UnlockMutex  = Pki::unlock_mutex;
  init_args.pReserved    = (CK_VOID_PTR)intStr;

  rv = (*pC_Initialize)(&init_args);
  if (CKR_OK != rv){
    LOGE("Error in C_Initialize : 0x%x", rv);
    LOGD("Pki::initModule-");
    return false;
  }
  LOGD("Pki::initModule-");
  return true;
}

Pki*
Pki::getPkiInstance(){
    return Pki::instance;
}

Pki*
Pki::initPkiInstance(const char* initLabel) {
  if ( NULL_PTR == Pki::instance ) {
    Pki::instance = new Pki(initLabel);
  }
  if ( NULL_PTR != Pki::instance ) {
    if ( false == Pki::instance->pkcsModule ) {
      delete Pki::instance;
      Pki::instance = NULL_PTR;
    }
  }
  return Pki::instance;
}

void
Pki::closePki() {
  LOGD("Pki::closePki()+");
  if ((NULL_PTR != Pki::instance)
    &&(Pki::instance->pkcsModule)) {

    CK_C_Finalize pC_Finalize = pFunctionList -> C_Finalize;
    if (NULL_PTR != pC_Finalize) {
      (*pC_Finalize)(NULL_PTR);
    }
    delete Pki::instance;
    Pki::instance = NULL_PTR;
  }
  LOGD("Pki::closePki()-");
}

bool
Pki::obtainToken(const char* tokenLabel){
  CK_ULONG ulCount;
  CK_SLOT_ID_PTR pSlotList;
  CK_SLOT_INFO slotInfo;
  CK_TOKEN_INFO tokenInfo;
  CK_RV rv;

  LOGD("Pki::obtainToken+");
  rv = Pki::pFunctionList -> C_GetSlotList(CK_FALSE,
                                           NULL_PTR,
                                           &ulCount);
  if(rv != CKR_OK) {
    LOGE("C_GetSlotList error rv = 0x%x",rv);
    LOGD("Pki::obtainToken-");
    return false;
  }

  if (0 == ulCount) {
    LOGE("ulCount error : ulCount = %d",ulCount);
    LOGD("Pki::obtainToken-");
    return false;
  }
  else {
    LOGD("ulCount = %d",ulCount);
    pSlotList = (CK_SLOT_ID_PTR) malloc ( ulCount * sizeof(CK_SLOT_ID));
    if (NULL == pSlotList) {
      LOGE("(CK_SLOT_ID_PTR) malloc failed");
      LOGD("Pki::obtainToken-");
      return false;
    }
    rv = Pki::pFunctionList -> C_GetSlotList(CK_FALSE,
                                             pSlotList,
                                             &ulCount);
    if (rv != CKR_OK){
      LOGE("C_GetSlotList error rv = 0x%x",rv);
      LOGD("Pki::obtainToken-");
      return false;
    }

    for (unsigned int i = 0; i < ulCount ; i++) {
      /* Get slot information for the slot */
      rv = Pki::pFunctionList -> C_GetSlotInfo(pSlotList[i],
                                               &slotInfo);
      if (rv != CKR_OK) {
        LOGE("C_GetSlotInfo error rv = 0x%x",rv);
        continue;
      }
      else {
        if (slotInfo.flags & CKF_TOKEN_PRESENT) {
          /* Get token information for the slot */
          rv = Pki::pFunctionList -> C_GetTokenInfo(pSlotList[i],
                                                    &tokenInfo);
          if (rv == CKR_TOKEN_NOT_PRESENT) {
            LOGE("C_GetTokenInfo error rv after slotInfo.flags.CKF_TOKEN_PRESENT");
            /* look in the next slot */
            continue;
          }
          /* need token without Secure Keypad*/
          if( tokenInfo.flags & CKF_PROTECTED_AUTHENTICATION_PATH  ) {
            /* look in the next slot */
            continue;
          }

          /* ckeck if the token is already initialized */
          if( tokenInfo.flags & CKF_TOKEN_INITIALIZED  ) {
            LOGD("Token already initialized");
            LOGD("current slot == %d", pSlotList[i]);
            currentSlotId = pSlotList[i];
            break;
          }else {
            LOGD("Token not initialized");
            int labelLen  = strlen(tokenLabel);
            CK_UTF8CHAR label[32];
            memset(label, ' ', sizeof(label));
            memcpy(label, tokenLabel, labelLen );

            rv = Pki::pFunctionList -> C_InitToken(pSlotList[i],
                                                 (CK_UTF8CHAR_PTR)Pki::userPin,
                                                 strlen(Pki::userPin),
                                                 label);
            if (rv != CKR_OK) {
              LOGE("C_InitToken error rv = 0x%x", rv);
              continue;
            } else {
              LOGD("current slot == %d", pSlotList[i]);
              currentSlotId = pSlotList[i];
              break;
            }
          }
        } else {
           rv = CKR_OPERATION_NOT_INITIALIZED;
        }
      }
    }
    free(pSlotList);
  }
  if (rv != CKR_OK) {
    LOGD("Pki::obtainToken- error 0x%x",rv);
    return false;
  }
  LOGD("Pki::obtainToken-");
  return true;
}

bool
Pki::createKey(const char* pPassword, const char* pKeyLabel){

  CK_SESSION_HANDLE hSession;
  CK_OBJECT_HANDLE  object;
  CK_RV rv;

  LOGD("Pki::createKey+ %s , %s", pPassword, pKeyLabel );

  rv = Pki::pFunctionList -> C_OpenSession( currentSlotId,
                                            CKF_SERIAL_SESSION | CKF_RW_SESSION,
                                            (CK_VOID_PTR) NULL_PTR,
                                            NULL_PTR,
                                            &hSession);
  if (rv != CKR_OK) {
    LOGE("C_OpenSession failed with rv = 0x%x", rv);
    LOGD("Pki::createOTPKey-");
    return false;
  }

  LOGD("Pki::createKey - Pki::salt     - %s",    Pki::salt);
  LOGD("Pki::createKey - Pki::userPin  - %s",    Pki::userPin);
  LOGD("Pki::createKey - Pki::userName - %s",    Pki::userName);

  rv = Pki::pFunctionList -> C_Login( hSession,
                                      CKU_USER,
                                      (CK_UTF8CHAR_PTR)Pki::userPin,
                                      strlen(Pki::userPin));
  if (rv != CKR_OK) {
    LOGE("C_Login failed with rv = 0x%x", rv);
    Pki::pFunctionList -> C_CloseSession(hSession);
    LOGD("Pki::createKey-");
    return false;
  }

  CK_ULONG passwordLength = strlen(pPassword);
  CK_PKCS5_PBKD2_PARAMS pkcs_pbkd2_param;

  pkcs_pbkd2_param.saltSource          = 1;
  pkcs_pbkd2_param.pSaltSourceData     = (CK_VOID_PTR)Pki::salt;
  pkcs_pbkd2_param.ulSaltSourceDataLen = strlen(Pki::salt);
  pkcs_pbkd2_param.iterations          = 100;
  pkcs_pbkd2_param.prf                 = CKP_PKCS5_PBKD2_HMAC_SHA256;
  pkcs_pbkd2_param.pPrfData            = NULL;
  pkcs_pbkd2_param.ulPrfDataLen        = 0;
  pkcs_pbkd2_param.pPassword           = (CK_UTF8CHAR_PTR)pPassword;
  pkcs_pbkd2_param.ulPasswordLen       = &passwordLength;

  CK_MECHANISM mech = { CKM_PKCS5_PBKD2,
                        &pkcs_pbkd2_param,
                        sizeof(CK_PKCS5_PBKD2_PARAMS)};

  CK_OBJECT_CLASS aes_class = CKO_SECRET_KEY;
  CK_KEY_TYPE aes_keyType = CKK_AES;
  CK_BYTE value[32];
  CK_ULONG _length32  = 32;
  CK_BBOOL _true  = CK_TRUE;
  CK_BBOOL _false = CK_FALSE;
  CK_ATTRIBUTE aes_template[] = {
                               {CKA_CLASS,     &aes_class,     sizeof(aes_class)},
                               {CKA_KEY_TYPE,  &aes_keyType,   sizeof(aes_keyType)},
                               {CKA_TOKEN,     &_true,         sizeof(_true)},
                               {CKA_SENSITIVE, &_false,        sizeof(_false)},
                               {CKA_LABEL,(CK_VOID_PTR )pKeyLabel,strlen(pKeyLabel)},
                               {CKA_ENCRYPT,   &_true,         sizeof(_true)},
                               {CKA_DECRYPT,   &_true,         sizeof(_true)},
                               {CKA_VALUE_LEN, &_length32,     sizeof(_length32)},
                               //{CKA_VALUE,     value,          sizeof(value)},
                            };
  rv = Pki::pFunctionList -> C_GenerateKey ( hSession,
                                               &mech,
                                               aes_template,
                                               8,
                                               &object);

  if (rv != CKR_OK) {
    LOGE("C_GenerateKey failed with rv = 0x%x", rv);
    Pki::pFunctionList -> C_Logout(hSession);
    Pki::pFunctionList -> C_CloseSession(hSession);
    LOGD("Pki::createKey-");
    return false;
  }

  Pki::pFunctionList -> C_Logout(hSession);
  Pki::pFunctionList -> C_CloseSession(hSession);
  LOGD("Pki::createKey-");
  return true;
}

CK_OBJECT_HANDLE
Pki::retrieveKey(const char* keyLabel){

  CK_SESSION_HANDLE hSession;
  CK_RV rv = CKR_OK;

  LOGD("Pki::retrieveKey+");

  rv = Pki::pFunctionList -> C_OpenSession( currentSlotId,
                                            CKF_SERIAL_SESSION | CKF_RW_SESSION,
                                            (CK_VOID_PTR) NULL_PTR,
                                            NULL_PTR,
                                            &hSession);
  if (rv != CKR_OK) {
    LOGE("C_OpenSession failed with rv = 0x%x", rv);
    LOGD("Pki::retrieveKey-");
    return NULL_PTR;
  }

  rv = Pki::pFunctionList -> C_Login( hSession,
                                      CKU_USER,
                                      (CK_UTF8CHAR_PTR)Pki::userPin,
                                      strlen(Pki::userPin));
  if (rv != CKR_OK) {
    LOGE("C_Login failed with rv = 0x%x", rv);
    Pki::pFunctionList -> C_CloseSession(hSession);
    LOGD("Pki::retrieveKey-");
    return NULL_PTR;
  }

  CK_OBJECT_HANDLE hObject;
  CK_ULONG ulObjectCount;
  CK_ATTRIBUTE _Key[] = {
    {CKA_LABEL,        (CK_VOID_PTR)keyLabel,     strlen(keyLabel)},
  };

  rv = Pki::pFunctionList -> C_FindObjectsInit(hSession, _Key, 1);
  if (rv != CKR_OK) {
    LOGE("C_FindObjectsInit failed with rv = 0x%x", rv);
    Pki::pFunctionList -> C_Logout(hSession);
    Pki::pFunctionList -> C_CloseSession(hSession);
    LOGD("Pki::retrieveKey-");
    return NULL_PTR;
  }

  while (1) {
    rv = Pki::pFunctionList -> C_FindObjects(hSession,
                                             &hObject,
                                             1,
                                             &ulObjectCount);
    if (rv != CKR_OK || ulObjectCount == 0){
      LOGE("C_FindObjects failed with rv = 0x%x and ulObjectCount = %d", rv, ulObjectCount);
      break;
    }
    else {

      Pki::pFunctionList -> C_FindObjectsFinal(hSession);
      Pki::pFunctionList -> C_Logout(hSession);
      Pki::pFunctionList -> C_CloseSession(hSession);
      LOGD("Found Key");
      LOGD("Pki::retrieveKey-");
      return hObject;
    }
  }
  rv = Pki::pFunctionList -> C_FindObjectsFinal(hSession);
  if (rv != CKR_OK) {
    LOGE("C_FindObjectsFinal failed with rv = 0x%x", rv);
    Pki::pFunctionList -> C_Logout(hSession);
    Pki::pFunctionList -> C_CloseSession(hSession);
    LOGD("Pki::retrieveKey-");
    return NULL_PTR;
  }
  //not found
  Pki::pFunctionList -> C_Logout(hSession);
  Pki::pFunctionList -> C_CloseSession(hSession);
  LOGD("Pki::retrieveKey-");
  return NULL_PTR;
}

bool
Pki::deleteKey(const char* keyLabel){

  CK_SESSION_HANDLE hSession;
  CK_RV rv;

  LOGD("Pki::deleteKey+");

  CK_OBJECT_HANDLE hObject = retrieveKey(keyLabel);
  if (NULL_PTR == hObject) {
    LOGE("retrieveKey returned NULL ");
    LOGD("Pki::deleteKey-");
    return false;
  }

  rv = Pki::pFunctionList -> C_OpenSession( currentSlotId,
                                            CKF_SERIAL_SESSION | CKF_RW_SESSION,
                                            (CK_VOID_PTR) NULL_PTR,
                                            NULL_PTR,
                                            &hSession);
  if (rv != CKR_OK) {
    LOGE("C_OpenSession failed with rv = 0x%x", rv);
    LOGD("Pki::deleteKey-");
    return false;
  }

  rv = Pki::pFunctionList -> C_Login( hSession,
                                      CKU_USER,
                                      (CK_UTF8CHAR_PTR)Pki::userPin,
                                      strlen(Pki::userPin));
  if (rv != CKR_OK) {
    LOGE("C_Login failed with rv = 0x%x", rv);
    Pki::pFunctionList -> C_CloseSession(hSession);
    LOGD("Pki::deleteKey-");
    return false;
  }

  rv = Pki::pFunctionList -> C_DestroyObject(hSession,
                                             hObject);
  if (rv != CKR_OK) {
    LOGE("C_DestroyObject failed with rv = 0x%x", rv);
    Pki::pFunctionList -> C_Logout(hSession);
    Pki::pFunctionList -> C_CloseSession(hSession);
    LOGD("Pki::deleteKey-");
    return false;
  }

  Pki::pFunctionList -> C_Logout(hSession);
  Pki::pFunctionList -> C_CloseSession(hSession);
  LOGD("Pki::deleteKey-");
  return true;
}

bool
Pki::encrypt(CK_BYTE_PTR inBuff, CK_ULONG inSize, CK_BYTE_PTR outBuff,CK_ULONG outSize, CK_OBJECT_HANDLE key_handle){

  CK_SESSION_HANDLE hSession;
  CK_RV rv;

  LOGD("Pki::encrypt+");

  if (NULL_PTR == key_handle) {
    LOGE("Key is NULL");
    LOGD("Pki::encrypt-");
    return false;
  }

  rv = Pki::pFunctionList -> C_OpenSession( currentSlotId,
                                            CKF_SERIAL_SESSION | CKF_RW_SESSION,
                                            (CK_VOID_PTR) NULL_PTR,
                                            NULL_PTR,
                                            &hSession);
  if (rv != CKR_OK) {
    LOGE("C_OpenSession failed with rv = 0x%x", rv);
    LOGD("Pki::encrypt-");
    return false;
  }

  rv = Pki::pFunctionList -> C_Login( hSession,
                                      CKU_USER,
                                      (CK_UTF8CHAR_PTR)Pki::userPin,
                                      strlen(Pki::userPin));
  if (rv != CKR_OK) {
    LOGE("C_Login failed with rv = 0x%x", rv);
    Pki::pFunctionList -> C_CloseSession(hSession);
    LOGD("Pki::encrypt-");
    return false;
  }

  CK_MECHANISM mech;
  CK_BYTE iv[16] = { 0x76, 0x49, 0x20, 0x22, 0x00, 0x19, 0x12, 0x46,
                     0x25, 0x29, 0x41, 0x46, 0x12, 0x19, 0x19, 0x74 };

  mech.mechanism      = CKM_AES_CBC_PAD;
  mech.pParameter     = iv;
  mech.ulParameterLen = sizeof(iv);

  rv = Pki::pFunctionList -> C_EncryptInit(hSession,
                                           &mech,
                                           key_handle);
  if (rv != CKR_OK) {
    LOGE("C_EncryptInit failed with rv = 0x%x", rv);
    Pki::pFunctionList -> C_Logout(hSession);
    Pki::pFunctionList -> C_CloseSession(hSession);
    LOGD("Pki::encrypt-");
    return false;
  }

  CK_ULONG    out_len = outSize;
  rv = Pki::pFunctionList -> C_Encrypt(hSession,
                                         inBuff,
                                         inSize,
                                         outBuff,
                                         &out_len);
  if (rv != CKR_OK) {
    LOGE("C_Encrypt failed with rv = 0x%x", rv);
    Pki::pFunctionList -> C_Logout(hSession);
    Pki::pFunctionList -> C_CloseSession(hSession);
    LOGD("Pki::encrypt-");
    return false;
  }

  if (outSize != out_len ) {
    LOGE("Length Error during C_Encrypt -> Input size = %d , Output size = %d", inSize,out_len );
    Pki::pFunctionList -> C_Logout(hSession);
    Pki::pFunctionList -> C_CloseSession(hSession);
    LOGD("Pki::encrypt-");
    return false;
  }

  Pki::pFunctionList -> C_Logout(hSession);
  Pki::pFunctionList -> C_CloseSession(hSession);
  LOGD("Pki::encrypt-");
  return true;
}

bool
Pki::decrypt(CK_BYTE_PTR inBuff, CK_ULONG inSize, CK_BYTE_PTR outBuff, CK_ULONG_PTR pOutSize, CK_OBJECT_HANDLE key_handle){

  CK_SESSION_HANDLE hSession;
  CK_RV rv;

  LOGD("Pki::decrypt+");

  if (NULL_PTR == key_handle) {
    LOGE("Key is NULL");
    LOGD("Pki::decrypt-");
    return false;
  }

  rv = Pki::pFunctionList -> C_OpenSession( currentSlotId,
                                            CKF_SERIAL_SESSION | CKF_RW_SESSION,
                                            (CK_VOID_PTR) NULL_PTR,
                                            NULL_PTR,
                                            &hSession);
  if (rv != CKR_OK) {
    LOGE("C_OpenSession failed with rv = 0x%x", rv);
    LOGD("Pki::decrypt-");
    return false;
  }

  rv = Pki::pFunctionList -> C_Login( hSession,
                                      CKU_USER,
                                      (CK_UTF8CHAR_PTR)Pki::userPin,
                                      strlen(Pki::userPin));
  if (rv != CKR_OK) {
    LOGE("C_Login failed with rv = 0x%x", rv);
    Pki::pFunctionList -> C_CloseSession(hSession);
    LOGD("Pki::decrypt-");
    return false;
  }

  CK_MECHANISM mech;
  CK_BYTE iv[16] = { 0x76, 0x49, 0x20, 0x22, 0x00, 0x19, 0x12, 0x46,
                     0x25, 0x29, 0x41, 0x46, 0x12, 0x19, 0x19, 0x74 };

  mech.mechanism      = CKM_AES_CBC_PAD;
  mech.pParameter     = iv;
  mech.ulParameterLen = sizeof(iv);

  rv = Pki::pFunctionList -> C_DecryptInit(hSession,
                                           &mech,
                                           key_handle);
  if (rv != CKR_OK) {
    LOGE("C_DecryptInit failed with rv = 0x%x", rv);
    Pki::pFunctionList -> C_Logout(hSession);
    Pki::pFunctionList -> C_CloseSession(hSession);
    LOGD("Pki::decrypt-");
    return false;
  }

  CK_ULONG    out_len = inSize;
  rv = Pki::pFunctionList -> C_Decrypt(hSession,
                                       inBuff,
                                       inSize,
                                       outBuff,
                                       &out_len);
  if (rv != CKR_OK) {
    LOGE("C_Decrypt failed with rv = 0x%x", rv);
    Pki::pFunctionList -> C_Logout(hSession);
    Pki::pFunctionList -> C_CloseSession(hSession);
    LOGD("Pki::decrypt-");
    return false;
  }

  *pOutSize = out_len;
  LOGD("AES out length after decryption  - insize %d , outSize %d ", inSize, out_len);

  Pki::pFunctionList -> C_Logout(hSession);
  Pki::pFunctionList -> C_CloseSession(hSession);
  LOGD("Pki::decrypt-");
  return true;
}

