/**
  @file pkcs11f.h
  @brief Include file for PKCS #11; Revision: 1.4.

  This header file contains the Cryptoki function prototypes. Because this
  information is used for more than just declaring function prototypes, the
  order of the functions appearing herein is important and should not be
  altered.
*/


/* ============================================================================
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  Notifications and licenses are retained for attribution purposes only

  License to copy and use this software is granted provided that it is
  identified as "RSA Security Inc. PKCS #11 Cryptographic Token Interface
  (Cryptoki)" in all material mentioning or referencing this software.

  License is also granted to make and use derivative works provided that
  such works are identified as "derived from the RSA Security Inc. PKCS #11
  Cryptographic Token Interface (Cryptoki)" in all material mentioning or
  referencing the derived work.

  RSA Security Inc. makes no representations concerning either the
  merchantability of this software or the suitability of this software for
  any particular purpose. It is provided "as is" without express or implied
  warranty of any kind.

============================================================================ */


/* General-purpose */

/** @name Cryptoki Library
  These functions manage the Cryptoki library.
  @{
*/
/** @anchor C_Initialize

  Initializes the Cryptoki library.

  This implementation of the PKCS #11 standard requires additional
  configuration compared to the original standard. The additional configuration
  is passed to the library by means of the pReserved pointer in the
  CK_C_INITIALIZE_ARGS structure that is passed during the library
  initialization. This library expects a NULL-terminated initialization string
  (char *) to be passed as the pReserved pointer of the structure.

  The initialization string is a multiline string blob with the following
  format:

@verbatim
GLOBAL_PARAMETER=VALUE
# comment
< token0 configuration
TOKEN0_PARAMETER0=VALUE00
TOKEN0_PARAMETER1=VALUE01
...
>
@endverbatim
@keep{5}
  The string blob is a list of parameters and associated values with one
  parameter per line. Lines beginning with the # character are ignored and
  are considered to be comments. The string blob supports two scopes:
  - Global -- All parameters common to all tokens are declared.
  - Single token parameter list -- Parameters specific to one token are declared.
    No nested token scopes are allowed.

  Scopes are initiated by a less-than character (<) with the remaining part
  of the line being treated as a comment. Scopes are terminated by a
  greater-than character (>) on a separate line.

  The following global variables are recognized:

  - DB_IMPLEMENTATION -- Library used to support token and object storage. The
    library is loaded using dlopen(); therefore, the library must be specified
    with its full name. The only supported library in this release is SQLite.
    The value of this is libsqlite.so by default when the parameter
	is omitted.

  - DB_PATH -- Location in the host filesystem where the database files are
    stored. The calling application must have read/write access to this
    location. No default is provided. This field must be specified by the
    calling application.

  For each token the following parameters can be specified:
  - PIN_RETRY_BEHAVIOR -- Reserved for future use.
  - SOFT_TOKEN_NAME -- Filename to be used to store the database associated
    with this token. No default is provided. This field must be specified by
    the calling application.
  - TOKEN_LIBRARY -- Library to be loaded to support an external hardware
    token. Only a maximum of one hardware token is supported by the library. If
    this parameter is specified for a token, all other token-specific
    parameters for that token are ignored. No default value is provided. If
    omitted, no external library is loaded.
  - TOKEN_IMAGE_PATH -- Full name, including path, of the image that will be
    associated to the token during C_InitToken and afterwards displayed whenever
    the user is requested to enter his PIN. This image MUST be present when the
    token is initialized, and should deleted afterwards. A copy is maintained
    in the secure file system.

  @datatypes
  #CK_VOID_PTR

  @param[in] pInitArgs Pointer to #CK_C_INITIALIZE_ARGS_PTR (optional
                       arguments) and dereferenced, if not set to #NULL_PTR.

  @keep{2}
  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_ALREADY_INITIALIZED -- Cryptoki library is already
            initialized. \n
  CKR_ARGUMENTS_BAD -- One of the following:\n
            - pInitArgs is NULL.
            - Configuration cannot be read.
            - Configuration blob is larger than 4k.
  @par
  CKR_FUNCTION_FAILED -- Library could not be opened. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  None.

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_Initialize)
#ifdef CK_NEED_ARG_LIST
(
  CK_VOID_PTR   pInitArgs
);
#endif

/** Indicates that the use of the Cryptoki library by the application is
  finished.

  @datatypes
  #CK_VOID_PTR

  @param[in] pReserved Set to #NULL_PTR.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_ARGUMENTS_BAD -- Function is called with a non-NULL argument. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  The library must have been initialized with C_Initialize().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_Finalize)
#ifdef CK_NEED_ARG_LIST
(
  CK_VOID_PTR   pReserved
);
#endif

/** Obtains the general information about the Cryptoki library.

  @datatypes
  #CK_INFO_PTR

  @param[out] pInfo Pointer to the location where the information is to be
                    received.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_ARGUMENTS_BAD -- Function is called with a NULL argument. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  The library must have been initialized with C_Initialize().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_GetInfo)
#ifdef CK_NEED_ARG_LIST
(
  CK_INFO_PTR   pInfo
);
#endif

/** Initializes and returns the function list.

  @datatypes
  #CK_FUNCTION_LIST_PTR_PTR

  @param[out] ppFunctionList Pointer to the function list.

  @return
  CKR_OK -- Success. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  The library must have been initialized with C_Initialize().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_GetFunctionList)
#ifdef CK_NEED_ARG_LIST
(
  CK_FUNCTION_LIST_PTR_PTR ppFunctionList
);
#endif
/** @} */ /* end_name Cryptoki Library */

/* Slot and token management */
/** @name Slot and Token Management

  These functions interface with and manage the software tokens provided by the
  PKCS #11 module.

  In the case of the QTI PKCS #11 interface, the number and characteristics of the tokens are
  configured via a configuration string passed to the module during
  initialization (C_Initialize()).
  @{
*/
/** Obtains a list of slots in the system.

  If pSlotList is #NULL_PTR, C_GetSlotList returns in pulCount the number
  of slots without actually returning a list of slots. The contents of the
  buffer pointed to by pulCount on entry to C_GetSlotList has no meaning in
  this case, and the call returns the CKR_OK value.

  If pSlotList is not #NULL_PTR, pulCount must contain the size (in terms of
  #CK_SLOT_ID elements) of the buffer pointed to by pSlotList. If that buffer
  is large enough to hold the list of slots, the list is returned in the
  buffer and CKR_OK is returned. If the buffer is not large enough, the call
  to C_GetSlotList returns the CKR_BUFFER_TOO_SMALL value. In either case,
  the value pulCount is set to hold the number of slots.

  @datatypes
  #CK_BBOOL \n
  #CK_SLOT_ID_PTR \n
  #CK_ULONG_PTR

  @param[in] tokenPresent  Indicates whether the list includes only those slots
                           with a token present. \n
                           - TRUE -- Only slots with a token present.
                           - FALSE -- All slots.
                           @tablebulletend
  @param[out] pSlotList    Pointer to the array of slot IDs.
  @param[in,out] pulCount  Pointer to the location that receives the number of
                           slots.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_ARGUMENTS_BAD -- pulCount is NULL. \n
  CKR_BUFFER_TOO_SMALL -- Size of the return buffer is not enough. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  The library must have been initialized with C_Initialize().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_GetSlotList)
#ifdef CK_NEED_ARG_LIST
(
  CK_BBOOL       tokenPresent,
  CK_SLOT_ID_PTR pSlotList,
  CK_ULONG_PTR   pulCount
);
#endif

/** Obtains information about a specific slot in the system.

  @datatypes
  #CK_SLOT_ID \n
  #CK_SLOT_INFO_PTR

  @param[in]  slotID  Slot ID.
  @param[out] pInfo   Pointer to the slot information.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_ARGUMENTS_BAD -- pInfo is NULL. \n
  CKR_SLOT_ID_INVALID -- Function could not find the slot ID. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  The library must have been initialized with C_Initialize(). \n
  The list of slots must have been obtained with C_GetSlotList().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_GetSlotInfo)
#ifdef CK_NEED_ARG_LIST
(
  CK_SLOT_ID       slotID,
  CK_SLOT_INFO_PTR pInfo
);
#endif

/** Obtains information about a specific token in the system.

  @datatypes
  #CK_SLOT_ID \n
  #CK_TOKEN_INFO_PTR

  @param[in]  slotID  Slot ID.
  @param[out] pInfo   Pointer to the token information.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_ARGUMENTS_BAD -- pInfo is NULL. \n
  CKR_SLOT_ID_INVALID -- Function could not find the slot ID. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  The library must have been initialized with C_Initialize(). \n
  The list of slots must have been obtained with C_GetSlotList().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_GetTokenInfo)
#ifdef CK_NEED_ARG_LIST
(
  CK_SLOT_ID        slotID,
  CK_TOKEN_INFO_PTR pInfo
);
#endif

/** Obtains a list of mechanism types supported by a token.

  If pMechanismList is #NULL_PTR, this function returns in pulCount the
  number of mechanisms without actually returning a list of mechanisms. The
  contents of pulCount on entry to this function has no meaning in this
  case, and the call returns the CKR_OK value.

  If pMechanismList is not #NULL_PTR, pulCount must contain the size (in terms
  of #CK_MECHANISM_TYPE elements) of the buffer pointed to by pMechanismList.
  If that buffer is large enough to hold the list of mechanisms, the list is
  returned in the buffer and CKR_OK is returned. If the buffer is not large
  enough, the call to this function returns the CKR_BUFFER_TOO_SMALL
  value. In either case, the pulCount value is set to hold the number of
  mechanisms.

  @datatypes
  #CK_SLOT_ID \n
  #CK_MECHANISM_TYPE_PTR \n
  #CK_ULONG_PTR

  @param[in] slotID           Slot ID.
  @param[out] pMechanismList  Pointer to the location that receives the
                              mechanism list.
  @param[in,out] pulCount     Pointer to the location that receives the number
                              of mechanisms.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_ARGUMENTS_BAD -- pulCount is NULL. \n
  CKR_BUFFER_TOO_SMALL -- Number of mechanisms is larger than the number
            supported. \n
  CKR_SLOT_ID_INVALID -- Function could not find the slot ID. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  The library must have been initialized with C_Initialize(). \n
  The list of slots must have been obtained with C_GetSlotList(). \n
  The presence of a token must have been verified with C_GetTokenInfo().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_GetMechanismList)
#ifdef CK_NEED_ARG_LIST
(
  CK_SLOT_ID            slotID,
  CK_MECHANISM_TYPE_PTR pMechanismList,
  CK_ULONG_PTR          pulCount
);
#endif

/** Obtains information about a specific mechanism that can be supported by a
  token.

  @datatypes
  #CK_SLOT_ID \n
  #CK_MECHANISM_TYPE \n
  #CK_MECHANISM_INFO_PTR

  @param[in]  slotID  Slot ID.
  @param[in]  type    Type of the mechanism.
  @param[out] pInfo   Pointer to the mechanism information.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_ARGUMENTS_BAD -- pInfo is NULL. \n
  CKR_MECHANISM_INVALID -- Mechanism is not supported. \n
  CKR_SLOT_ID_INVALID -- Function could not find the slot ID. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  The library must have been initialized with C_Initialize(). \n
  The list of slots must have been obtained with C_GetSlotList(). \n
  The presence of a token must have been verified with C_GetTokenInfo(). \n
  The list of supported mechanisms must have been obtained with
  C_GetMechanismList().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_GetMechanismInfo)
#ifdef CK_NEED_ARG_LIST
(
  CK_SLOT_ID            slotID,
  CK_MECHANISM_TYPE     type,
  CK_MECHANISM_INFO_PTR pInfo
);
#endif

/** @anchor C_InitToken

  Initializes a token.

  @datatypes
  #CK_SLOT_ID \n
  #CK_UTF8CHAR_PTR \n
  #CK_ULONG \n
  #CK_UTF8CHAR_PTR

  @param[in] slotID   Slot ID.
  @param[in] pPin     Initial PIN, which does not need to be NULL-terminated.
  @param[in] ulPinLen Length of the PIN in bytes.
  @param[in] pLabel   Pointer to the label of the token that is 32 bytes in
                      length and blank padded.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_EXISTS -- Token has open sessions. \n
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pLabel is empty.
            - pLabel is too short.
            - pLabel consists of invalid characters.
            - PIN consists of invalid characters.
  @par
  CKR_FUNCTION_FAILED -- One of the following: \n
            - PIN attempt was too close to the last one.
            - Token failed to be deleted from the database.
            - Token database failed to be created.
  @par
  CKR_TOKEN_WRITE_PROTECTED -- Token is write-protected. \n
  CKR_SK_CANCELLED -- PIN entry aborted by the user. \n
  CKR_SLOT_ID_INVALID -- Function could not find the slot ID. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  The library must have been initialized with C_Initialize(). \n
  The list of slots must have been obtained with C_GetSlotList(). \n
  The presence of a token must have been verified with C_GetTokenInfo().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_InitToken)
#ifdef CK_NEED_ARG_LIST
(
  CK_SLOT_ID      slotID,
  CK_UTF8CHAR_PTR pPin,
  CK_ULONG        ulPinLen,
  CK_UTF8CHAR_PTR pLabel
);
#endif

/** @anchor C_InitPIN

  Initializes the PIN for a user who is not a security officer.

  @note1hang This function is only available to #CKU_SO.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_UTF8CHAR_PTR \n
  #CK_ULONG

  @param[in] hSession  Handle of the session.
  @param[in] pPin      Pointer to the user's PIN.
  @param[in] ulPinLen  Length of the PIN in bytes.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Handle of the session is invalid. \n
  CKR_SESSION_READ_ONLY -- Session is read-only. \n
  CKR_USER_NOT_LOGGED_IN -- User is not logged in. \n
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pPin and ulPinLen are NULL.
            - PIN contains invalid characters.
  @par
  CKR_FUNCTION_FAILED -- PIN attempt was too close to the last failed
            attempt. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  The library must have been initialized with C_Initialize(). \n
  The list of slots must have been obtained with C_GetSlotList(). \n
  The presence of a token must have been verified with C_GetTokenInfo(). \n
  The token must have been initialized with C_InitTokenSO().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_InitPIN)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_UTF8CHAR_PTR   pPin,
  CK_ULONG          ulPinLen
);
#endif

/** @anchor C_SetPIN
  Modifies the PIN of a user who is logged in or the #CKU_USER PIN if the
  session is not logged in.

  @note1hang This function is not available to #CKU_SO.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_UTF8CHAR_PTR \n
  #CK_ULONG

  @param[in] hSession  Handle of the session.
  @param[in] pOldPin   Pointer to the old PIN.
  @param[in] ulOldLen  Length of the old PIN in bytes.
  @param[in] pNewPin   Pointer to the new PIN.
  @param[in] ulNewLen  Length of the new PIN in bytes.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Handle of the session is invalid. \n
  CKR_SESSION_READ_ONLY -- Session is read-only. \n
  CKR_USER_NOT_LOGGED_IN -- User is not logged in. \n
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pOldPin, pNewPin, ulOldLen and ulNewLen are NULL or 0.
            - Old/new PIN contains invalid characters.
  @par
  CKR_FUNCTION_FAILED -- PIN attempt was too close to the last failed
            attempt. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  The library must have been initialized with C_Initialize(). \n
  The list of slots must have been obtained with C_GetSlotList(). \n
  The presence of a token must have been verified with C_GetTokenInfo(). \n
  The token must have been initialized with C_InitToken() or
  C_InitTokenSO().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_SetPIN)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_UTF8CHAR_PTR   pOldPin,
  CK_ULONG          ulOldLen,
  CK_UTF8CHAR_PTR   pNewPin,
  CK_ULONG          ulNewLen
);
#endif
/** @} */ /* end_name Slot and Token Management */

/* Session management */
/** @name Session Management
  These functions operate on a software token. They provide an interface to
  manage sessions used to issue commands to the token itself and to change its
  authentication status.
  @{
*/
/** @anchor C_OpenSession

  Opens a session between an application and a token in a specific slot.

  @datatypes
  #CK_SLOT_ID \n
  #CK_FLAGS \n
  #CK_VOID_PTR \n
  #CK_NOTIFY \n
  #CK_SESSION_HANDLE_PTR

  @param[in] slotID        Slot ID.
  @param[in] flags         Indicates the type of session.
  @param[in] pApplication  Application-defined pointer to be passed to the
                           notification callback.
  @param[in] Notify        Address of the notification callback function.
  @param[out] phSession    Pointer to the handle of the new session.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_ARGUMENTS_BAD -- Function is called with a NULL session argument. \n
  CKR_SLOT_ID_INVALID -- Function could not find the slot ID. \n
  CKR_FUNCTION_FAILED -- One of the following: \n
            - Function failed to register the session on the token.
            - Token is not initialized.
  @par
  CKR_SESSION_PARALLEL_NOT_SUPPORTED -- Parallel sessions are not supported and
            a parallel session has been requested.
  @par
  CKR_SESSION_COUNT -- Maximum supported number of sessions has been exceeded.
  CKR_TOKEN_WRITE_PROTECTED -- Token is write-protected. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  The library must have been initialized with C_Initialize(). \n
  The list of slots must have been obtained with C_GetSlotList(). \n
  The presence of a token must have been verified with C_GetTokenInfo(). \n
  The token must have been initialized with C_InitToken() or
  C_InitTokenSO().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_OpenSession)
#ifdef CK_NEED_ARG_LIST
(
  CK_SLOT_ID            slotID,
  CK_FLAGS              flags,
  CK_VOID_PTR           pApplication,
  CK_NOTIFY             Notify,
  CK_SESSION_HANDLE_PTR phSession
);
#endif

/** Closes a session between an application and a token.

  When a session is closed, all session objects created by the session are
  destroyed automatically, even if the application has other sessions using
  those objects.

  @datatypes
  #CK_SESSION_HANDLE

  @param[in] hSession  Handle of the session.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_FAILED -- Error occurred while updating the token status. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_CloseSession)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession
);
#endif

/** @ingroup func_C_CloseAllSessions

  Closes all sessions between an application and a token.

  When a session is closed, all session objects created by the session are
  destroyed automatically.

  The logged user state is reset after all sessions are closed.

  @datatypes
  #CK_SLOT_ID

  @param[in] slotID  Slot ID of the token.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_SLOT_ID_INVALID -- Function could not find the slot ID. \n
  CKR_FUNCTION_FAILED -- Objects failed to be removed from the database. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  The library must have been initialized with C_Initialize(). \n
  The list of slots must have been obtained with C_GetSlotList(). \n
  The presence of a token must have been verified with C_GetTokenInfo(). \n
  The token must have been initialized with C_InitToken() or
  C_InitTokenSO().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_CloseAllSessions)
#ifdef CK_NEED_ARG_LIST
(
  CK_SLOT_ID     slotID
);
#endif

/** Obtains information about a session.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_SESSION_INFO_PTR

  @param[in] hSession  Handle of the session.
  @param[out] pInfo    Pointer to the session information.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_ARGUMENTS_BAD -- Function is called with a NULL pInfo argument. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_GetSessionInfo)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE   hSession,
  CK_SESSION_INFO_PTR pInfo
);
#endif

/** Obtains the state of the cryptographic operation in a session.

  @note1hang This function is not supported in the current version of
             the QTI PKCS #11.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG_PTR

  @param[in] hSession                  Handle of the session.
  @param[out] pOperationState          Pointer to the state of operation.
  @param[in,out] pulOperationStateLen  Pointer to the length of the state of the
                                       operation.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_GetOperationState)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pOperationState,
  CK_ULONG_PTR      pulOperationStateLen
);
#endif

/** Restores the state of a cryptographic operation in a session.

  @note1hang This function is not supported in the current version of
             the QTI PKCS #11 interface.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG \n
  #CK_OBJECT_HANDLE

  @param[in] hSession            Handle of the session.
  @param[in] pOperationState     Pointer to the state of the operation.
  @param[in] ulOperationStateLen Length of the state of the operation.
  @param[in] hEncryptionKey      Handle of the encryption/decryption key.
  @param[in] hAuthenticationKey  Handle of the sign/verify key.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR - General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_SetOperationState)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR      pOperationState,
  CK_ULONG         ulOperationStateLen,
  CK_OBJECT_HANDLE hEncryptionKey,
  CK_OBJECT_HANDLE hAuthenticationKey
);
#endif

/** @anchor C_Login

  Logs a user into a token.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_USER_TYPE \n
  #CK_UTF8CHAR_PTR \n
  #CK_ULONG

  @param[in] hSession  Handle of the session.
  @param[in] userType  Type of user.
  @param[in] pPin      Pointer to the PIN of the user.
  @param[in] ulPinLen  Length of the PIN in bytes.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_USER_TYPE_INVALID -- Type of the user is invalid. \n
  CKR_USER_ANOTHER_ALREADY_LOGGED_IN -- User is already logged in. Only one
            user can login at one time.
  @par
  CKR_USER_ALREADY_LOGGED_IN -- User is already logged in. \n
  CKR_SESSION_READ_ONLY_EXISTS -- Existing read-only session is open. \n
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pPin is NULL or ulPinLen is 0.
            - PIN contains invalid characters.
  @par
  CKR_FUNCTION_FAILED -- Login attempt was too close to the last failed
            attempt. \n
  CKR_PIN_INCORRECT -- PIN entry details do not match. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_Login)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_USER_TYPE      userType,
  CK_UTF8CHAR_PTR   pPin,
  CK_ULONG          ulPinLen
);
#endif

/** Logs a user out of a token.

  @datatypes
  #CK_SESSION_HANDLE

  @param[in] hSession  Handle of the session.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_USER_NOT_LOGGED_IN -- User is not logged in. \n
  CKR_FUNCTION_FAILED -- Token could not be updated. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_Logout)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession
);
#endif
/** @} */  /* end_name Session Management */

/* Object management */
/** @anchor h_p11fom
  @name Object Management
  These functions control objects during their entire lifetime: creation,
  deletion, modification, and lookup.
  @{
*/
/** Creates a new object.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_ATTRIBUTE_PTR \n
  #CK_ULONG \n
  #CK_OBJECT_HANDLE_PTR

  @param[in] hSession  Handle of the session.
  @param[in] pTemplate Pointer to the template for the object.
  @param[in] ulCount   Number of attributes in the template.
  @param[out] phObject Pointer to the handle of the object.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pTemplate, ulCount or phObject is NULL.
            - Object type is not supported.
            - Object matches the token challenge object.
  @par
  CKR_FUNCTION_FAILED -- Creation of the object failed. \n
  CKR_TEMPLATE_INCONSISTENT -- One of the following: \n
            - Template contains two or more of the same attribute with
              different values.
            - One or more attributes are not consistent with the expected
              attributes for the object type.
  @par
  CKR_ATTRIBUTE_TYPE_INVALID -- Template contains an attribute that is
            invalid. \n
  CKR_ATTRIBUTE_VALUE_INVALID -- One of the following: \n
            - Template contains an attribute with a length >0 and a NULL
              pointer to the value.
            - Template contains an attribute with a pointer to a value but a
              length == 0.
            - One or more attributes are not valid for the object type.
  @par
  CKR_TEMPLATE_INCOMPLETE -- One or more required attributes are missing from
            the template. \n
  CKR_ATTRIBUTE_READ_ONLY -- One or more attributes in the template are
            not modifiable, cannot be set at creation time, and already have a
            value.
  @par
  CKR_USER_NOT_LOGGED_IN -- User attempted to create a token object or a
            private object without being logged in.
  @par
  @keep{3}
  CKR_SESSION_READ_ONLY -- One of the following: \n
            - Attempted to create a public token object without a read/write
              session.
            - Attempted to write a private token object from a read-only
              session.
  @par
  CKR_TOKEN_WRITE_PROTECTED -- Write operation is requested but the token is
            read-only. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_CreateObject)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_ATTRIBUTE_PTR  pTemplate,
  CK_ULONG          ulCount,
  CK_OBJECT_HANDLE_PTR phObject
);
#endif

/** Copies the content of an object after creating a new object.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_OBJECT_HANDLE \n
  #CK_ATTRIBUTE_PTR \n
  #CK_ULONG \n
  #CK_OBJECT_HANDLE_PTR

  @param[in] hSession     Handle of the session.
  @param[in] hObject      Handle of the object.
  @param[in] pTemplate    Pointer to the template of the object.
  @param[in] ulCount      Number of attributes in the template.
  @param[out] phNewObject Pointer to the handle for the copy of the object.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pTemplate is NULL and ulCount >0.
            - phNewObject is NULL.
  @par
  CKR_OBJECT_HANDLE_INVALID -- One of the following: \n
            - Object referenced by the handle could not be found.
            - Object is malformed; object has no #CKA_TOKEN, #CKA_PRIVATE or
              #CKA_MODIFIABLE.
            - Object's type does not match one of the supported types.
  @par
  CKR_TEMPLATE_INCONSISTENT -- Template contains two or more of the same
            attribute with different values.
  @par
  @keep{3}
  CKR_ATTRIBUTE_TYPE_INVALID -- One of the following: \n
            - Template contains an attribute that is invalid.
            - Source object is missing one or more attributes.
  @par
  @keep{4}
  CKR_ATTRIBUTE_VALUE_INVALID -- One of the following: \n
            - Template contains an attribute with a length >0 and a NULL
              pointer to the value.
            - Template contains an attribute with a pointer to a value but a
              length == 0.
            - One or more attributes are not valid for the object type.
  @par
  CKR_ATTRIBUTE_READ_ONLY -- One or more attributes in the template are not
            modifiable. \n
  CKR_TOKEN_WRITE_PROTECTED -- Write operation is requested but the token is
            read-only. \n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to create a public token object without being
              logged in.
            - User attempted to access a private session object without being
              logged in.
  @par
  CKR_SESSION_READ_ONLY -- One of the following: \n
            - User attempted to create a public token object without a read/write
              session.
            - User attempted to write a private token object from a read-only
              session.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_CopyObject)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE    hSession,
  CK_OBJECT_HANDLE     hObject,
  CK_ATTRIBUTE_PTR     pTemplate,
  CK_ULONG             ulCount,
  CK_OBJECT_HANDLE_PTR phNewObject
);
#endif

/** Destroys an object.

  Only session objects can be destroyed during a read-only session. Only public
  objects can be destroyed unless the user is logged in.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_OBJECT_HANDLE

  @param[in] hSession  Handle of the session.
  @param[in] hObject   Handle of the object.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_TOKEN_WRITE_PROTECTED -- Write operation was requested but the token is
            read-only. \n
  CKR_SESSION_READ_ONLY -- Destruction of a public token object was attempted
            in a read/write session.
  @par
  CKR_FUNCTION_FAILED -- Object failed to be deleted. \n
  CKR_OBJECT_HANDLE_INVALID -- Object associated with the handle could not be
            found. \n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to destroy a public token object without being
              logged in.
            - User attempted to destroy a private session object without being
              logged in.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_DestroyObject)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_OBJECT_HANDLE  hObject
);
#endif

/** Obtains the size of an object in bytes.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_OBJECT_HANDLE \n
  #CK_ULONG_PTR

  @param[in] hSession  Handle of the session.
  @param[in] hObject   Handle of the object.
  @param[out] pulSize  Pointer to the size of the object in bytes.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_OBJECT_HANDLE_INVALID -- Object could not be found. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_GetObjectSize)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_OBJECT_HANDLE  hObject,
  CK_ULONG_PTR      pulSize
);
#endif

/** @anchor C_GetAttributeValue

  Obtains the value of one or more object attributes.

  If an error is encountered when processing the template, the rest of the
  template is processed but an error value is returned.

  The specification does not indicate a priority of error values, so
  CKR_ATTRIBUTE_TYPE_INVALID can be overridden by CKR_ATTRIBUTE_SENSITIVE if a
  sensitive attribute follows an invalid one.

  This function also handles the QTI PKCS #11 specific challenge object.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_OBJECT_HANDLE \n
  #CK_ATTRIBUTE_PTR \n
  #CK_ULONG

  @param[in] hSession       Handle of the session.
  @param[in] hObject        Handle of the object.
  @param[in,out] pTemplate  Pointer to the template of the object.
  @param[in] ulCount        Number of attributes in the template.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- One of the following: \n
            - Token information could not be retrieved.
            - Attribute is not in the object.
            - Value of a requested attribute could not be retrieved.
  @par
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_OBJECT_HANDLE_INVALID -- Object associated with the handle could not be
            found. \n
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pTemplate is NULL.
            - ulCount is 0.
            - hObject is #CK_INVALID_HANDLE.
            - Template is shorter than the advertised ulCount.
  @par
  CKR_ATTRIBUTE_VALUE_INVALID -- One or more attributes are not valid for the
            object type. \n
  CKR_ATTRIBUTE_TYPE_INVALID -- Attribute of the token is invalid. \n
  CKR_BUFFER_TOO_SMALL -- Buffer supplied for the attribute in the template is
            too small to hold the attribute value.
  @par
  CKR_ATTRIBUTE_SENSITIVE -- Attempted to read a value of a sensitive
            attribute. \n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to access a public token object without being
              logged in.
            - User attempted to access a private session object without being
              logged in.
  @par
  CKR_SESSION_READ_ONLY -- Creation of a public token object was attempted
            without a read/write session.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_GetAttributeValue)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_OBJECT_HANDLE  hObject,
  CK_ATTRIBUTE_PTR  pTemplate,
  CK_ULONG          ulCount
);
#endif

/** Modifies the value of one or more object attributes.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_OBJECT_HANDLE \n
  #CK_ATTRIBUTE_PTR \n
  #CK_ULONG

  @param[in] hSession   Handle of the session.
  @param[in] hObject    Handle of the object.
  @param[in] pTemplate  Pointer to the template of the object that specifies
                        which attribute values are to be modified and their new
                        values.
  @param[in] ulCount    Number of attributes in the template.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized.
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_OBJECT_HANDLE_INVALID -- Object associated with the handle could not be
            found. \n
  CKR_TOKEN_WRITE_PROTECTED -- Write operation is requested but the token is
            read-only. \n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to create a public token object without being
              logged in.
            - User attempted to access a private session object without being
              logged in.
  @par
  CKR_SESSION_READ_ONLY -- Creation of a public token object was attempted
            without a read/write session.
  @par
  CKR_ATTRIBUTE_VALUE_INVALID -- One or more attributes are not valid for the
            object type. \n
  CKR_ATTRIBUTE_TYPE_INVALID -- Attribute of the token is invalid. \n
  CKR_ATTRIBUTE_READ_ONLY -- One or more attributes in the template are
            not modifiable. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_SetAttributeValue)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_OBJECT_HANDLE  hObject,
  CK_ATTRIBUTE_PTR  pTemplate,
  CK_ULONG          ulCount
);
#endif

/** Initializes a search for the token and session objects that match a template.
  The matching criterion is an exact byte-for-byte match with all attributes in
  the template. To find all objects, set ulCount to 0.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_ATTRIBUTE_PTR \n
  #CK_ULONG

  @param[in] hSession   Handle to the session.
  @param[in] pTemplate  Pointer to a search template that specifies the
                        attribute values to match.
  @param[in] ulCount    Number of attributes in the search template.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized.
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_ARGUMENTS_BAD -- pTemplate is NULL and the ulCount is not equal to 0. \n
  CKR_OPERATION_ACTIVE -- Search is already in progress for this session. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of currently
            active sessions. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_FindObjectsInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_ATTRIBUTE_PTR  pTemplate,
  CK_ULONG          ulCount
);
#endif

/** Continues a search for the token and session objects that match a template,
  and obtains additional object handles.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_OBJECT_HANDLE_PTR \n
  #CK_ULONG \n
  #CK_ULONG_PTR

  @param[in] hSession          Handle of the session.
  @param[in] phObject          Pointer to the object handle.
  @param[in] ulMaxObjectCount  Maximum number of object handles to be returned.
  @param[out] pulObjectCount   Pointer to the actual number of object handles
                               returned.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - phObject or pulObjectCount is NULL.
            - ulMaxObjectCount is equal to 0.
  @par
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_OPERATION_NOT_INITIALIZED -- C_FindObjectsInit() has not been called
            for this session. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_FindObjects)
#ifdef CK_NEED_ARG_LIST
(
 CK_SESSION_HANDLE    hSession,
 CK_OBJECT_HANDLE_PTR phObject,
 CK_ULONG             ulMaxObjectCount,
 CK_ULONG_PTR         pulObjectCount
);
#endif

/** Finishes a search for token and session objects.

  @datatypes
  #CK_SESSION_HANDLE

  @param[in] hSession  Handle of the session.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized.
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_OPERATION_NOT_INITIALIZED -- C_FindObjectsInit() has not been called
            for this session. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_FindObjectsFinal)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession
);
#endif
/** @} */  /* end_name Object Management */

/* Encryption and decryption */
/** @name Encryption and Decryption
  These functions perform encryption and decryption using a key object already
  present in the token on a buffer provided by the caller.

  Not all key objects can be used for encryption/decryption. Refer to
  @ref S3 "[S3]". \n
  - The object itself might not allow it.
  - The object might not support the selected encryption mechanism.
  - The object type might not be compatible with the selected encryption
    mechanism.
  @{
*/
/** Initializes an encryption operation.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_MECHANISM_PTR \n
  #CK_OBJECT_HANDLE

  @param[in] hSession    Handle of the session.
  @param[in] pMechanism  Pointer to the encryption mechanism.
  @param[in] hKey        Handle of the encryption key.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_ARGUMENTS_BAD -- pMechanism is NULL. \n
  CKR_KEY_HANDLE_INVALID -- One of the following: \n
            - Key handle is invalid.
            - Key type cannot be used with the specified mechanism.
  @par
  CKR_OPERATION_ACTIVE -- Session already has an active operation. \n
  CKR_MECHANISM_INVALID -- One of the following: \n
            - Mechanism is not supported.
            - Mechanism does not support the requested function.
  @par
  CKR_OBJECT_HANDLE_INVALID -- Object is invalid. \n
  CKR_KEY_FUNCTION_NOT_PERMITTED -- Key does not support the requested
            operation. \n
  CKR_KEY_TYPE_INCONSISTENT -- Object type is not compatible with the specified
            mechanism. \n
  CKR_TOKEN_WRITE_PROTECTED -- Write operation is requested but the token is
            read-only. \n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to access a public token object without being
              logged in.
            - User attempted to access a private session object without being
              logged in.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_EncryptInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_MECHANISM_PTR  pMechanism,
  CK_OBJECT_HANDLE  hKey
);
#endif

/** Encrypts single-part data.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG \n
  #CK_ULONG_PTR

  @param[in] hSession                 Handle of the session.
  @param[in] pData                    Pointer to the plaintext data.
  @param[in] ulDataLen                Length of the plaintext data in bytes.
  @param[out] pEncryptedData          Pointer to the ciphertext data.
  @param[in,out] pulEncryptedDataLen  Pointer to the length of the ciphertext
                                      data in bytes.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized.
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a multiple-part mechanism or
            session. \n
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pData is NULL or the ulDataLen is 0.
            - Output parameters are NULL.
  @par
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_Encrypt)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pData,
  CK_ULONG          ulDataLen,
  CK_BYTE_PTR       pEncryptedData,
  CK_ULONG_PTR      pulEncryptedDataLen
);
#endif

/** Continues a multiple-part encryption operation that processes another data
  part.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG \n
  #CK_ULONG_PTR

  @param[in] hSession                Handle of the session.
  @param[in] pPart                   Pointer to a part of plaintext data.
  @param[in] ulPartLen               Length of the part of plaintext data in bytes.
  @param[out] pEncryptedPart         Pointer to a part of ciphertext data.
  @param[in,out] pulEncryptedPartLen Pointer to the size of the part of ciphertext
                                     data in bytes.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a single-part mechanism.
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pData is NULL or the ulDataLen is 0.
            - Output parameters are NULL.
  @par
  CKR_SIGNATURE_INVALID -- Signatures do not match. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_EncryptUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pPart,
  CK_ULONG          ulPartLen,
  CK_BYTE_PTR       pEncryptedPart,
  CK_ULONG_PTR      pulEncryptedPartLen
);
#endif

/** Finishes a multiple-part encryption operation.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG_PTR

  @param[in] hSession                     Handle of the session.
  @param[out] pLastEncryptedPart          Pointer to the last part of
                                          ciphertext data, if any.
  @param[in,out] pulLastEncryptedPartLen  Pointer to the size of the last part
                                          of ciphertext data in bytes.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a single-part mechanism. \n
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pData is NULL or the ulDataLen is 0.
            - Output parameters are NULL.
  @par
  CKR_SIGNATURE_INVALID -- Signatures do not match. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_EncryptFinal)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pLastEncryptedPart,
  CK_ULONG_PTR      pulLastEncryptedPartLen
);
#endif

/** Initializes a decryption operation.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_MECHANISM_PTR \n
  #CK_OBJECT_HANDLE

  @param[in] hSession    Handle of the session.
  @param[in] pMechanism  Pointer to the decryption mechanism.
  @param[in] hKey        Handle of the decryption key.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_ARGUMENTS_BAD -- pMechanism is NULL. \n
  CKR_KEY_HANDLE_INVALID -- One of the following: \n
            - Key handle is invalid.
            - Key type cannot be used with the specified mechanism.
  @par
  CKR_OPERATION_ACTIVE -- Session already has an active operation. \n
  CKR_MECHANISM_INVALID -- One of the following: \n
            - Mechanism is not supported.
            - Failed to get mechanism information.
            - Mechanism does not support the requested function.
  @par
  CKR_OBJECT_HANDLE_INVALID -- Object is invalid. \n
  CKR_KEY_FUNCTION_NOT_PERMITTED -- Key does not support the requested
            operation. \n
  CKR_KEY_TYPE_INCONSISTENT -- Object type is not compatible with the specified
            mechanism. \n
  CKR_TOKEN_WRITE_PROTECTED -- Write operation is requested but the token is
            read-only. \n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to create a public token object without being
              logged in.
            - User attempted to access a private session object without being
              logged in.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_DecryptInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_MECHANISM_PTR  pMechanism,
  CK_OBJECT_HANDLE  hKey
);
#endif

/** Decrypts single-part data.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG \n
  #CK_ULONG_PTR

  @param[in] hSession             Handle of the session.
  @param[in] pEncryptedData       Pointer to the ciphertext.
  @param[in] ulEncryptedDataLen   Length of the ciphertext data in bytes.
  @param[out] pData               Pointer to the plaintext data.
  @param[in,out] pulDataLen       Pointer to the length of the plaintext data.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a multiple-part mechanism or
            session. \n
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pEncryptedData is NULL or ulEncryptedDataLen is 0.
            - Output parameters are NULL.
            - Signature is not valid in the verification operation.
  @par
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_Decrypt)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pEncryptedData,
  CK_ULONG          ulEncryptedDataLen,
  CK_BYTE_PTR       pData,
  CK_ULONG_PTR      pulDataLen
);
#endif

/** Continues a multiple-part decryption that processes another data part.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG \n
  #CK_ULONG_PTR

  @param[in] hSession            Handle of the session.
  @param[in] pEncryptedPart      Pointer to the ciphertext data.
  @param[in] ulEncryptedPartLen  Length of the part of ciphertext data in
                                 bytes.
  @param[out] pPart              Pointer to the part of plaintext data.
  @param[in,out] pulPartLen      Pointer to the size of the part of
                                 plaintext data.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized.
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a single-part mechanism. \n
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pEncryptedPart is NULL or ulEncryptedPartLen is 0.
            - Output parameters are NULL.
  @par
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_DecryptUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pEncryptedPart,
  CK_ULONG          ulEncryptedPartLen,
  CK_BYTE_PTR       pPart,
  CK_ULONG_PTR      pulPartLen
);
#endif

/** Finishes a multiple-part decryption.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG_PTR

  @param[in] hSession            Handle of the session.
  @param[in,out] pLastPart       Pointer to the last part of plaintext data,
                                 if any.
  @param[in,out] pulLastPartLen  Pointer to the size of the last part of
                                 plaintext data in bytes.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a single-part mechanism. \n
  CKR_ARGUMENTS_BAD -- Output parameters are NULL. \n
  CKR_SIGNATURE_INVALID -- Signatures do not match. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_DecryptFinal)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pLastPart,
  CK_ULONG_PTR      pulLastPartLen
);
#endif
/** @} */  /* end_name Encryption and Decryption  */

/* Message digesting */
/** @name Message Digesting
  These functions perform digesting on a buffer provided by the caller.

  @note1hang These functions do not operate with a key object, except
             C_DigestKey(). In the C_DigestKey() function, the content of the
             key is the buffer being digested. For information on digesting
             mechanisms that perform signature generations or verifications
             with key objects, see @ref h_p11fsgn "Signing and MACing".
  @{
*/
/** Initializes a message-digesting operation.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_MECHANISM_PTR

  @param[in] hSession    Handle of the session.
  @param[in] pMechanism  Pointer to the digesting mechanism.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_ARGUMENTS_BAD -- pMechanism is NULL. \n
  CKR_KEY_HANDLE_INVALID -- One of the following: \n
            - Key handle is invalid.
            - Key type cannot be used with the specified mechanism.
  @par
  CKR_OPERATION_ACTIVE -- Session already has an active operation. \n
  CKR_MECHANISM_INVALID -- One of the following: \n
            - Mechanism is not supported.
            - Mechanism does not support the requested function.
  @par
  CKR_OBJECT_HANDLE_INVALID -- Object is invalid. \n
  CKR_KEY_FUNCTION_NOT_PERMITTED -- Key does not support the requested
            operation. \n
  CKR_KEY_TYPE_INCONSISTENT -- Object type is not compatible with the
            specified mechanism. \n
  CKR_TOKEN_WRITE_PROTECTED -- Write operation is requested but the token is
            read-only. \n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to access a public token object without being
              logged in.
            - User attempted to access a private session object without being
              logged in.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_DigestInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_MECHANISM_PTR  pMechanism
);
#endif

/** Digests data in a single part.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG \n
  #CK_ULONG_PTR

  @param[in] hSession         Handle of the session.
  @param[in] pData            Pointer to the data to be digested.
  @param[in] ulDataLen        Length of data in bytes to digest.
  @param[in,out] pDigest      Pointer to the message digest.
  @param[in,out] pulDigestLen Pointer to the length of the message digest in
                              bytes.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized.
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a multiple-part mechanism.
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pData is NULL or the ulDataLen is 0.
            - Output parameters are NULL.
  @par
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_Digest)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pData,
  CK_ULONG          ulDataLen,
  CK_BYTE_PTR       pDigest,
  CK_ULONG_PTR      pulDigestLen
);
#endif

/** Continues a multiple-part message digesting operation.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG

  @param[in] hSession   Handle of the session.
  @param[in] pPart      Pointer to the part of data to be digested.
  @param[in] ulPartLen  Length of the part of data in bytes to digest.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a single-part mechanism. \n
  CKR_ARGUMENTS_BAD -- pPart is NULL or ulPartLen is 0. \n
  CKR_SIGNATURE_INVALID -- Signatures do not match. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_DigestUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pPart,
  CK_ULONG          ulPartLen
);
#endif

/** Continues a multiple-part message digesting operation by digesting the
  value of a secret key as part of the data already digested.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_OBJECT_HANDLE

  @param[in] hSession  Handle of the session.
  @param[in] hKey      Handle of the secret key to digest.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_KEY_HANDLE_INVALID -- Key handle is invalid. \n
  CKR_KEY_INDIGESTIBLE -- Key type does not support digesting. \n
  CKR_OBJECT_HANDLE_INVALID -- Object handle is invalid. \n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to access a public token object without being
              logged in.
            - User attempted to access a private session object without being
              logged in.
  @par
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_DigestKey)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_OBJECT_HANDLE  hKey
);
#endif

/** Finishes a multiple-part message digesting operation.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG_PTR

  @param[in] hSession          Handle of the session.
  @param[out] pDigest          Pointer to the message digest.
  @param[in,out] pulDigestLen  Pointer to the length of the digest in bytes.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a single-part mechanism. \n
  CKR_ARGUMENTS_BAD -- pDigest is NULL or pulDigestLen is 0. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_DigestFinal)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pDigest,
  CK_ULONG_PTR      pulDigestLen
);
#endif
/** @} */  /* end_name Message Digesting */

/* Signing and MACing */
/** @anchor h_p11fsgn
  @name Signing and MACing
  These functions perform signature generation using a key object already
  present in the token on buffers provided by the caller.

  Not all key objects can be used for signature generation. Refer to
  @ref S3 "[S3]".\n
  - The object itself might not allow it.
  - The object might not support the selected mechanism.
  - The object type might not be compatible with the object.
  @{
*/
/** Initializes a signature (private key encryption) operation, where the
  signature is an appendix to the data and the plaintext data cannot be
  recovered from the signature.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_MECHANISM_PTR \n
  #CK_OBJECT_HANDLE

  @param[in] hSession    Handle of the session.
  @param[in] pMechanism  Pointer to the signature mechanism.
  @param[in] hKey        Handle of the signature key.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_ARGUMENTS_BAD -- pMechanism is NULL. \n
  CKR_KEY_HANDLE_INVALID -- One of the following: \n
            - Key handle is invalid.
            - Key type cannot be used with the specified mechanism.
  @par
  CKR_OPERATION_ACTIVE -- Session already has an active operation. \n
  CKR_MECHANISM_INVALID -- One of the following: \n
            - Mechanism is not supported.
            - Mechanism does not support the requested function.
  @par
  CKR_OBJECT_HANDLE_INVALID -- Object is invalid. \n
  CKR_KEY_FUNCTION_NOT_PERMITTED -- Key does not support the requested
            operation. \n
  CKR_KEY_TYPE_INCONSISTENT -- Object type is not compatible with the specified
            mechanism. \n
  CKR_TOKEN_WRITE_PROTECTED -- Write operation is requested but the token is
            read-only. \n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to access a public token object without being
              logged in.
            - User attempted to access a private session object without being
              logged in.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_SignInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_MECHANISM_PTR  pMechanism,
  CK_OBJECT_HANDLE  hKey
);
#endif

/** @anchor C_Sign

  Signs (encrypts with a private key) data in a single part, where the
  signature is an appendix to the data and the plaintext data cannot be
  recovered from the signature.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG \n
  #CK_ULONG_PTR

  @param[in] hSession             Handle of the session.
  @param[in] pData                Pointer to the data to sign.
  @param[in] ulDataLen            Length of the data in bytes to sign.
  @param[in,out] pSignature       Pointer to the signature.
  @param[in,out] pulSignatureLen  Pointer to the length of the signature.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a multiple-part mechanism.
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pData is NULL or the ulDataLen is 0.
            - Output parameters are NULL.
  @par
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_Sign)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pData,
  CK_ULONG          ulDataLen,
  CK_BYTE_PTR       pSignature,
  CK_ULONG_PTR      pulSignatureLen
);
#endif

/** Continues a multiple-part signature operation, where the signature is an
  appendix to the data and the plaintext data cannot be recovered from the
  signature.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG

  @param[in] hSession   Handle of the session.
  @param[in] pPart      Pointer to the part of data to sign.
  @param[in] ulPartLen  Length of the part of data in bytes to sign.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a single-part mechanism. \n
  CKR_ARGUMENTS_BAD -- pPart is NULL or ulPartLen is 0. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_SignUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pPart,
  CK_ULONG          ulPartLen
);
#endif

/** @anchor C_SignFinal

  Finishes a multiple-part signature operation that returns the signature.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG_PTR

  @param[in] hSession             Handle of the session.
  @param[out] pSignature          Pointer to the signature.
  @param[in,out] pulSignatureLen  Pointer to the length of the signature.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a single-part mechanism. \n
  CKR_ARGUMENTS_BAD -- Output parameters are NULL. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_SignFinal)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pSignature,
  CK_ULONG_PTR      pulSignatureLen
);
#endif

/** Initializes a signature operation, where the data can be recovered from the
  signature.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_MECHANISM_PTR \n
  #CK_OBJECT_HANDLE

  @param[in] hSession    Handle of the session.
  @param[in] pMechanism  Pointer to the signature mechanism.
  @param[in] hKey        Handle of the signature key.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_ARGUMENTS_BAD -- pMechanism is NULL. \n
  CKR_KEY_HANDLE_INVALID -- One of the following: \n
            - Key handle is invalid.
            - Key type cannot be used with the specified mechanism.
  @par
  CKR_OPERATION_ACTIVE -- Session already has an active operation. \n
  CKR_MECHANISM_INVALID -- One of the following: \n
            - Mechanism is not supported.
            - Mechanism does not support the requested function.
  @par
  CKR_OBJECT_HANDLE_INVALID -- Object is invalid. \n
  CKR_KEY_FUNCTION_NOT_PERMITTED -- Key does not support the requested
            operation. \n
  CKR_KEY_TYPE_INCONSISTENT -- Object type is not compatible with the specified
            mechanism.  \n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to access a public token object without being
              logged in.
            - User attempted to access a private session object without being
              logged in.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_SignRecoverInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_MECHANISM_PTR  pMechanism,
  CK_OBJECT_HANDLE  hKey
);
#endif

/** Signs data in a single operation, where the data can be recovered from the
  signature.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG \n
  #CK_ULONG_PTR

  @param[in] hSession             Handle of the session.
  @param[in] pData                Pointer to the data to sign.
  @param[in] ulDataLen            Length of the data in bytes to sign.
  @param[out] pSignature          Pointer to the signature.
  @param[in,out] pulSignatureLen  Pointer to the length of the signature.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a non multiple-part
            mechanism. \n
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pData is NULL or ulDataLen is 0.
            - Output parameters are NULL.
  @par
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_SignRecover)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pData,
  CK_ULONG          ulDataLen,
  CK_BYTE_PTR       pSignature,
  CK_ULONG_PTR      pulSignatureLen
);
#endif
/** @} */  /* end_name Signing and MACing */

/* Verifying signatures and MACs */
/** @name Verifying Signatures and MACs
  These functions perform signature verification using a key object already
  present in the token on buffers provided by the caller.

  Not all key objects can be used for signature verification. Refer to
  @ref S3 "[S3]". \n
  - The object itself might not allow it.
  - The object might not support the selected mechanism.
  - The object type might not be compatible with the object.
  @{
*/
/** Initializes a verification operation, where the signature is an appendix to
  the data and the plaintext data cannot be recovered from the signature.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_MECHANISM_PTR \n
  #CK_OBJECT_HANDLE

  @param[in] hSession    Handle of the session.
  @param[in] pMechanism  Pointer to the verification mechanism.
  @param[in] hKey        Handle of the verification key.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_ARGUMENTS_BAD -- pMechanism is NULL. \n
  CKR_KEY_HANDLE_INVALID -- One of the following: \n
            - Key handle is invalid.
            - Key type cannot be used with the specified mechanism.
  @par
  CKR_OPERATION_ACTIVE -- Session already has an active operation. \n
  CKR_MECHANISM_INVALID -- One of the following: \n
            - Mechanism is not supported.
            - Mechanism does not support the requested function.
  @par
  CKR_OBJECT_HANDLE_INVALID -- Object is invalid. \n
  CKR_KEY_FUNCTION_NOT_PERMITTED -- Key does not support the requested
            operation. \n
  CKR_KEY_TYPE_INCONSISTENT -- Object type is not compatible with the specified
            mechanism. \n
  CKR_TOKEN_WRITE_PROTECTED -- Write operation is requested but the token is
            read-only. \n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to access a public token object without being
              logged in.
            - User attempted to access a private session object without being
              logged in.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_VerifyInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_MECHANISM_PTR  pMechanism,
  CK_OBJECT_HANDLE  hKey
);
#endif

/** Verifies a signature in a single-part operation, where the signature is an
  appendix to the data and the plaintext data cannot be recovered from the
  signature.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR  \n
  #CK_ULONG

  @param[in] hSession        Handle of the session.
  @param[in] pData           Pointer to the signed data.
  @param[in] ulDataLen       Length of the signed data.
  @param[in] pSignature      Pointer to the signature.
  @param[in] ulSignatureLen  Length of the signature.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a multiple-part mechanism. \n
  CKR_ARGUMENTS_BAD  -- One of the following: \n
            - pData is NULL or ulDataLen is 0.
            - Output parameters are NULL.
  @par
  CKR_SIGNATURE_INVALID -- Signatures do not match. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_Verify)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pData,
  CK_ULONG          ulDataLen,
  CK_BYTE_PTR       pSignature,
  CK_ULONG          ulSignatureLen
);
#endif

/** Continues a multiple-part verification operation, where the signature is an
  appendix to the data and the plaintext data cannot be recovered from the signature.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG

  @param[in] hSession   Handle of the session.
  @param[in] pPart      Pointer to the part of signed data.
  @param[in] ulPartLen  Length of the signed data.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR - General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a single-part mechanism. \n
  CKR_ARGUMENTS_BAD -- pPart is NULL or ulPartLen is 0. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_VerifyUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pPart,
  CK_ULONG          ulPartLen
);
#endif

/** Finishes a multiple-part verification operation and checks the signature.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG

  @param[in] hSession        Handle of the session.
  @param[in] pSignature      Pointer to the signature to verify.
  @param[in] ulSignatureLen  Length of the signature.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a single-part mechanism. \n
  CKR_ARGUMENTS_BAD -- Output parameters are NULL. \n
  CKR_SIGNATURE_INVALID -- Signatures do not match. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_VerifyFinal)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pSignature,
  CK_ULONG          ulSignatureLen
);
#endif

/** Initializes a signature verification operation, where the data is recovered
  from the signature.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_MECHANISM_PTR \n
  #CK_OBJECT_HANDLE

  @param[in] hSession    Handle of the session.
  @param[in] pMechanism  Pointer to the verification mechanism.
  @param[in] hKey        Handle of the verification key.

  @return
  CKR_OK -- Success.\n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized.\n
  CKR_GENERAL_ERROR -- General failure.\n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_ARGUMENTS_BAD -- pMechanism is NULL.\n
  CKR_KEY_HANDLE_INVALID -- One of the following: \n
            - Key handle is invalid.
            - Key type cannot be used with the specified mechanism.
  @par
  CKR_OPERATION_ACTIVE -- Session already has an active operation.\n
  CKR_MECHANISM_INVALID -- One of the following: \n
            - Mechanism is not supported.
            - Mechanism does not support the requested function.
  @par
  CKR_OBJECT_HANDLE_INVALID -- Object is invalid.\n
  CKR_KEY_FUNCTION_NOT_PERMITTED -- Key does not support the requested
            operation.\n
  CKR_KEY_TYPE_INCONSISTENT -- Object type is not compatible with the specified
            mechanism.\n
  CKR_TOKEN_WRITE_PROTECTED -- Write operation is requested but the token is
            read-only.\n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to access a public token object without being
              logged in.
            - User attempted to access a private session object without being
              logged in.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_VerifyRecoverInit)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_MECHANISM_PTR  pMechanism,
  CK_OBJECT_HANDLE  hKey
);
#endif

/** Verifies a signature in a single-part operation, where the data is
  recovered from the signature.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG \n
  #CK_ULONG_PTR

  @param[in] hSession        Handle of the session.
  @param[in] pSignature      Pointer to the signature to verify.
  @param[in] ulSignatureLen  Length of the signature.
  @param[in] pData           Pointer to the signed data.
  @param[in] pulDataLen      Pointer to the length of the signed data.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_OPERATION_NOT_INITIALIZED -- Operation is not valid for the current
            session. \n
  CKR_FUNCTION_FAILED -- Function is called on a multiple-part mechanism. \n
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pSignature is NULL or ulSignatureLen is 0.
            - Output parameters are NULL.
  @par
  CKR_SIGNATURE_INVALID -- Signatures do not match. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_VerifyRecover)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pSignature,
  CK_ULONG          ulSignatureLen,
  CK_BYTE_PTR       pData,
  CK_ULONG_PTR      pulDataLen
);
#endif
/** @} */  /* end_name Verifying Signatures and MACs */

/* Dual-function cryptographic operations */
/** @name Dual-function Cryptographic Operations
  These functions perform multiple operations on the same data buffer in the
  same session to avoid unnecessary data movement to and from the token.\n
  @note1hang These functions are not supported on the QTI PKCS #11 interface.
  @{
*/
/** Continues a multiple-part digesting and encryption operation.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG \n
  #CK_ULONG_PTR

  @param[in] hSession                Handle of the session.
  @param[in] pPart                   Pointer to the part of plaintext data.
  @param[in] ulPartLen               Length of the part of plaintext data.
  @param[out] pEncryptedPart         Pointer to the part of ciphertext data.
  @param[in,out] pulEncryptedPartLen Pointer to the length of the part of
                                     ciphertext data in bytes.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_DigestEncryptUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pPart,
  CK_ULONG          ulPartLen,
  CK_BYTE_PTR       pEncryptedPart,
  CK_ULONG_PTR      pulEncryptedPartLen
);
#endif

/** Continues a multiple-part decryption and digesting operation.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG \n
  #CK_ULONG_PTR

  @param[in] hSession            Handle of the session.
  @param[in] pEncryptedPart      Pointer to the part of ciphertext data.
  @param[in] ulEncryptedPartLen  Length of the part of ciphertext data in
                                 bytes.
  @param[out] pPart              Pointer to the part of plaintext data.
  @param[in,out] pulPartLen      Pointer to the length of the part of plaintext
                                 data.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_DecryptDigestUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pEncryptedPart,
  CK_ULONG          ulEncryptedPartLen,
  CK_BYTE_PTR       pPart,
  CK_ULONG_PTR      pulPartLen
);
#endif

/** Continues a multiple-part signing and encryption operation.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG \n
  #CK_ULONG_PTR

  @param[in] hSession                Handle of the session.
  @param[in] pPart                   Pointer to the part of plaintext data.
  @param[in] ulPartLen               Length of the part of plaintext data.
  @param[out] pEncryptedPart         Pointer to the part of ciphertext data.
  @param[in,out] pulEncryptedPartLen Pointer to the length of the part of
                                     ciphertext data in bytes.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_SignEncryptUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pPart,
  CK_ULONG          ulPartLen,
  CK_BYTE_PTR       pEncryptedPart,
  CK_ULONG_PTR      pulEncryptedPartLen
);
#endif

/** Continues a multiple-part decryption and verification operation.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG \n
  #CK_ULONG_PTR

  @param[in] hSession            Handle of the session.
  @param[in] pEncryptedPart      Pointer to the part of ciphertext data.
  @param[in] ulEncryptedPartLen  Pointer to the length of the part of
                                 ciphertext data in bytes.
  @param[out] pPart              Pointer to the part of plaintext data.
  @param[in,out] pulPartLen      Pointer to the length of the part of plaintext
                                 data.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_DecryptVerifyUpdate)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pEncryptedPart,
  CK_ULONG          ulEncryptedPartLen,
  CK_BYTE_PTR       pPart,
  CK_ULONG_PTR      pulPartLen
);
#endif
/** @} */  /* end_name Dual-function Cryptographic Operations */

/* Key management */
/** @name Key Management
  These functions interact with key objects. They treat the key objects as
  cryptographic keys rather than generic objects. For more information on
  generic objects, see @ref h_p11fom "Object Management".
  @{
*/
/** @anchor C_GenerateKey

  Generates a secret key, which creates a new key object.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_MECHANISM_PTR \n
  #CK_ATTRIBUTE_PTR \n
  #CK_ULONG  \n
  #CK_OBJECT_HANDLE_PTR

  @param[in] hSession    Handle of the session.
  @param[in] pMechanism  Pointer to the key generation mechanism.
  @param[in] pTemplate   Pointer to the template for a new key or a set of
                         domain parameters.
  @param[in] ulCount     Number of attributes in the template.
  @param[out] phKey      Pointer to the handle of a new key.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_ARGUMENTS_BAD -- One of the following: \n
            - pMechanism is NULL.
            - phKey is NULL.
            - pTemplate is NULL and ulCount is greater than 0.
  @par
  CKR_MECHANISM_INVALID -- Mechanism is not supported. \n
  CKR_FUNCTION_FAILED -- Secret key failed to be created. \n
  CKR_TEMPLATE_INCONSISTENT -- One of the following: \n
            - Template contains two or more of the same attribute with
              different values.
            - One or more attributes are not consistent with the expected
              attributes for the object type.
  @par
  CKR_TEMPLATE_INCOMPLETE -- One or more required attributes are missing from
            the template. \n
  @keep{5}
  CKR_ATTRIBUTE_VALUE_INVALID -- One of the following: \n
            - Template contains an attribute with a length >0 and a NULL
              pointer to the value.
            - Template contains an attribute with a pointer to a value but a
              length == 0.
            - One or more attributes are not valid for the object type.
            - Certificate type or key type is unrecognised/unsupported.
            - Number of array attributes in the template exceeded the maximum
              value.
  @par
  CKR_ATTRIBUTE_TYPE_INVALID -- Template contains an attribute that is
            invalid. \n
  CKR_ATTRIBUTE_READ_ONLY -- One or more attributes in the template are not
            modifiable. \n
  CKR_OBJECT_HANDLE_INVALID -- Object is invalid. \n
  CKR_TOKEN_WRITE_PROTECTED -- Token is write-protected. \n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to access/create a public token object without being
              logged in.
            - User attempted to access/create a private session object without
              being logged in.
  @par
  CKR_SESSION_READ_ONLY -- Creation of a public token object was attempted
            without a read/write session.
  @par
  CKR_SLOT_ID_INVALID -- Function could not find the slot ID. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_GenerateKey)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE    hSession,
  CK_MECHANISM_PTR     pMechanism,
  CK_ATTRIBUTE_PTR     pTemplate,
  CK_ULONG             ulCount,
  CK_OBJECT_HANDLE_PTR phKey
);
#endif

/** Generates a public-key/private-key pair, which creates new key objects.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_MECHANISM_PTR \n
  #CK_ATTRIBUTE_PTR \n
  #CK_ULONG \n
  #CK_OBJECT_HANDLE_PTR

  @param[in] hSession                   Handle of the session.
  @param[in] pMechanism                 Pointer to the key generation
                                        mechanism.
  @param[in] pPublicKeyTemplate         Pointer to the template for public
                                        keys.
  @param[in] ulPublicKeyAttributeCount  Number of attributes in the public-key
                                        template.
  @param[in] pPrivateKeyTemplate        Pointer to the template for private
                                        keys.
  @param[in] ulPrivateKeyAttributeCount Number of attributes in the private-key
                                        template.
  @param[out] phPublicKey               Pointer to the handle to a public key.
  @param[out] phPrivateKey              Pointer to the handle to a private key.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_ARGUMENTS_BAD -- Arguments are either NULL or 0. \n
  CKR_MECHANISM_INVALID -- One of the following: \n
            - Mechanism is not supported.
            - Mechanism does not support the requested function.
  @par
  CKR_TEMPLATE_INCONSISTENT -- One of the following: \n
            - Mechanism is not supported.
            - Key types are inconsistent.
            - Attribute class for the public/private key is inconsistent.
            - Attribute appears more than once in the template.
  @par
  CKR_TEMPLATE_INCOMPLETE -- One or more required attributes are missing from
            the template. \n
  @keep{4}
  CKR_ATTRIBUTE_VALUE_INVALID-- One of the following: \n
            - Key type is unrecognised/unsupported.
            - Attribute length or data pointer are invalid.
            - Length exceeds the maximum value.
  @par
  CKR_ATTRIBUTE_READ_ONLY -- One or more attributes in the template are not
            modifiable. \n
  CKR_ATTRIBUTE_TYPE_INVALID -- Template contains an attribute that is
            invalid. \n
  CKR_TOKEN_WRITE_PROTECTED -- Write operation is requested but the token is
            read-only. \n
  CKR_FUNCTION_FAILED -- Function failed. \n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to set #CKA_TRUSTED without being logged in as
              a security officer (#CKU_SO).
            - User attempted to create a public token object without being
              logged in.
            - User attempted to create a private session object without being
              logged in.
  @par
  CKR_SESSION_READ_ONLY -- Creation of an object was attempted without a
            read/write session. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_GenerateKeyPair)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE    hSession,
  CK_MECHANISM_PTR     pMechanism,
  CK_ATTRIBUTE_PTR     pPublicKeyTemplate,
  CK_ULONG             ulPublicKeyAttributeCount,
  CK_ATTRIBUTE_PTR     pPrivateKeyTemplate,
  CK_ULONG             ulPrivateKeyAttributeCount,
  CK_OBJECT_HANDLE_PTR phPublicKey,
  CK_OBJECT_HANDLE_PTR phPrivateKey
);
#endif

/** Wraps (encrypts) a key.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_MECHANISM_PTR \n
  #CK_OBJECT_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG_PTR

  @param[in] hSession             Handle of the session.
  @param[in] pMechanism           Pointer to the wrapping mechanism.
  @param[in] hWrappingKey         Handle of the wrapping key.
  @param[in] hKey                 Handle of the key to be wrapped.
  @param[out] pWrappedKey         Pointer to the wrapped key.
  @param[in,out] pulWrappedKeyLen Pointer to the length of the wrapped key.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_ARGUMENTS_BAD -- pMechanism or pulWrappedKeyLen are NULL. \n
  CKR_MECHANISM_INVALID -- One of the following: \n
            - Mechanism is not supported.
            - Mechanism does not support the requested function.
  @par
  CKR_WRAPPING_KEY_HANDLE_INVALID -- Key handle is invalid. \n
  CKR_OBJECT_HANDLE_INVALID -- Object is invalid. \n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to access a public token object without being
              logged in.
            - User attempted to access a private session object without being
              logged in.
  @par
  CKR_WRAPPING_KEY_TYPE_INCONSISTENT -- One of the following: \n
            - Key does not support wrapping.
            - Object type cannot be used with the mechanism.
            - CKA_WRAP_TEMPLATE was not found in the wrapping key.
  @par
  CKR_WRAPPED_KEY_INVALID -- Key handle is invalid. \n
  CKR_KEY_NOT_WRAPPABLE -- Wrapped key cannot be wrapped. \n
  CKR_KEY_UNEXTRACTABLE -- Wrapped key is not extractable. \n
  @keep{2}
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_WrapKey)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_MECHANISM_PTR  pMechanism,
  CK_OBJECT_HANDLE  hWrappingKey,
  CK_OBJECT_HANDLE  hKey,
  CK_BYTE_PTR       pWrappedKey,
  CK_ULONG_PTR      pulWrappedKeyLen
);
#endif

/** @anchor C_UnwrapKey

  Unwraps (decrypts) a wrapped key, which creates a new key object.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_MECHANISM_PTR \n
  #CK_OBJECT_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG \n
  #CK_ATTRIBUTE_PTR \n
  #CK_OBJECT_HANDLE_PTR

  @param[in] hSession          Handle of the session.
  @param[in] pMechanism        Pointer to the unwrapping mechanism.
  @param[in] hUnwrappingKey    Handle of the unwrapping key.
  @param[in] pWrappedKey       Pointer to the wrapped key.
  @param[in] ulWrappedKeyLen   Length of the wrapped key.
  @param[in] pTemplate         Pointer to the template for new keys.
  @param[in] ulAttributeCount  Number of attributes in the template.
  @param[out] phKey            Pointer to the handle of the new key.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_ARGUMENTS_BAD -- pMechanism, phKey or pTemplate is NULL. \n
  CKR_MECHANISM_INVALID -- One of the following: \n
            - Mechanism is not supported.
            - Mechanism does not support the requested function.
  @par
  CKR_OBJECT_HANDLE_INVALID -- Object is invalid. \n
  CKR_TOKEN_WRITE_PROTECTED -- Write operation is requested but the token is
            read-only. \n
  @keep{3}
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to create/access a public token object without
              being logged in.
            - User attempted to create/access a private session object without
              being logged in.
  @par
  CKR_SESSION_READ_ONLY -- One of the following: \n
            - Attempted to create a public token object without a read/write
              session.
            - Attempted to create a private token object from a read-only
              session.
  @par
  CKR_TEMPLATE_INCONSISTENT -- One of the following: \n
            - Template contains two or more of the same attribute with
              different values.
            - Key types are inconsistent.
            - Additional attributes could not be supplied from the mechanism.
            - Attribute appears more than once in the template.
  @par
  CKR_TEMPLATE_INCOMPLETE -- One or more required attributes are missing from
            the template. \n
  CKR_ATTRIBUTE_VALUE_INVALID -- One of the following: \n
            - Key type is unrecognised/unsupported.
            - Attribute length or data pointer are invalid.
            - Length exceeds the maximum value.
            - Number of attributes in the (un)wrap key exceeded the maximum
              value.
            - Number of attributes in the template exceeded the maximum value.
  @par
  CKR_ATTRIBUTE_READ_ONLY -- One or more attributes in the template are not
            modifiable. \n
  CKR_ATTRIBUTE_TYPE_INVALID -- Template contains an attribute that is
            invalid. \n
  CKR_WRAPPED_KEY_LEN_RANGE -- Wrapped key length is invalid. \n
  CKR_WRAPPED_KEY_INVALID -- Wrapped key is invalid. \n
  CKR_UNWRAPPING_KEY_HANDLE_INVALID -- Key handle is invalid. \n
  CKR_UNWRAPPING_KEY_TYPE_INCONSISTENT -- Key type is not consistent with the
            operation or mechanism requested.
  @par
  CKR_FUNCTION_FAILED -- Function failed. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_UnwrapKey)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE    hSession,
  CK_MECHANISM_PTR     pMechanism,
  CK_OBJECT_HANDLE     hUnwrappingKey,
  CK_BYTE_PTR          pWrappedKey,
  CK_ULONG             ulWrappedKeyLen,
  CK_ATTRIBUTE_PTR     pTemplate,
  CK_ULONG             ulAttributeCount,
  CK_OBJECT_HANDLE_PTR phKey
);
#endif

/** Derives a key from a base key, which creates a new key object.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_MECHANISM_PTR \n
  #CK_OBJECT_HANDLE \n
  #CK_ATTRIBUTE_PTR \n
  #CK_ULONG \n
  #CK_OBJECT_HANDLE_PTR

  @param[in] hSession          Handle of the session.
  @param[in] pMechanism        Pointer to the key derivation mechanism.
  @param[in] hBaseKey          Handle of the base key.
  @param[in] pTemplate         Pointer to the template for the new key.
  @param[in] ulAttributeCount  Number of attributes in the template.
  @param[out] phKey            Pointer to the handle of the new key.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_MECHANISM_INVALID -- One of the following: \n
            - Mechanism is not supported.
            - Mechanism does not support the requested function.
  @par
  CKR_OBJECT_HANDLE_INVALID -- Object is invalid. \n
  CKR_TOKEN_WRITE_PROTECTED -- Write operation is requested but the token is
            read-only. \n
  CKR_USER_NOT_LOGGED_IN -- One of the following: \n
            - User attempted to create/access a public token object without
              being logged in.
            - User attempted to create/access a private session object without
              being logged in.
  @par
  CKR_KEY_HANDLE_INVALID -- Key handle is invalid. \n
  CKR_KEY_FUNCTION_NOT_PERMITTED -- Requested function is not supported by the
            key. \n
  @keep{3}
  CKR_TEMPLATE_INCONSISTENT -- One of the following: \n
            - Template contains two or more of the same attribute with
              different values.
            - Key types are inconsistent.
            - Additional attributes could not be supplied from the mechanism.
            - Attribute appears more than once in the template.
  @par
  CKR_TEMPLATE_INCOMPLETE -- One or more required attributes are missing from
            the template. \n
  CKR_ATTRIBUTE_VALUE_INVALID-- One of the following: \n
            - Key type is unrecognised/unsupported.
            - Attribute length or data pointer is invalid.
            - Length exceeds the maximum value.
            - Number of attributes in the key exceeded the maximum value.
            - Number of attributes in the template exceeded the supported
              value.
  @par
  CKR_ATTRIBUTE_READ_ONLY -- One or more attributes in the template are not
            modifiable. \n
  CKR_ATTRIBUTE_TYPE_INVALID -- Template contains an attribute that is
            invalid. \n
  CKR_ARGUMENTS_BAD -- Arguments are either NULL or 0. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_DeriveKey)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE    hSession,
  CK_MECHANISM_PTR     pMechanism,
  CK_OBJECT_HANDLE     hBaseKey,
  CK_ATTRIBUTE_PTR     pTemplate,
  CK_ULONG             ulAttributeCount,
  CK_OBJECT_HANDLE_PTR phKey
);
#endif
/** @} */ /* end_name Key Management */

/* Random number generation */
/** @name Random Number Generation
  These functions are used for random number generation.
  @{
*/
/** Mixes additional seed material into the token's random number generator. \n
  @note1hang This function is not used.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG

  @param[in] hSession   Handle of the session.
  @param[in] pSeed      Pointer to the seed material.
  @param[in] ulSeedLen  Length of the seed material.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_SeedRandom)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       pSeed,
  CK_ULONG          ulSeedLen
);
#endif

/** Generates random data.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_BYTE_PTR \n
  #CK_ULONG

  @param[in] hSession      Handle of the session.
  @param[out] RandomData   Pointer to the random data.
  @param[in] ulRandomLen   Number of bytes to generate.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_ARGUMENTS_BAD -- RandomData is NULL or ulRandomLen is equal to 0. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_GenerateRandom)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession,
  CK_BYTE_PTR       RandomData,
  CK_ULONG          ulRandomLen
);
#endif
/** @} */  /* end_name Random Number Generation */

/** @cond
*/
/* Parallel function management */
/** @name Parallel Function Management
  These functions are for parallel function management. \n
  @note1hang These functions are deprecated in recent versions of the PKCS #11
             standard and are not supported by the QTI PKCS #11 interface.
  @{
*/
/** @deprecated This function is deprecated in recent versions of the
                PKCS #11 standard and is not supported by the QTI PKCS #11 interface.

  Obtains an updated status of a function running in parallel with an
  application. This is a legacy function.

  @datatypes
  #CK_SESSION_HANDLE

  @param[in] hSession  Handle of the session.

  @return
  CKR_OK -- Success. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_PARALLEL -- No function in parallel.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_GetFunctionStatus)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession
);
#endif

/** @deprecated This function is deprecated in recent versions of the
                PKCS #11 standard and is not supported by the QTI PKCS #11 interface.

  Cancels a function running in parallel. This is a legacy function.

  @datatypes
  #CK_SESSION_HANDLE

  @param[in] hSession  Handle of the session.

  @return
  CKR_OK -- Success. \n
  CKR_SESSION_HANDLE_INVALID -- Session was not found in the list of
            currently active sessions.
  @par
  CKR_FUNCTION_NOT_PARALLEL -- No function in parallel.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_CancelFunction)
#ifdef CK_NEED_ARG_LIST
(
  CK_SESSION_HANDLE hSession
);
#endif
/** @} */  /* end_name Parallel Function Management */
/** @endcond */

/** @name Function to be moved
*/
/** @anchor C_WaitForSlotEvent

  Waits for a slot event (e.g., token insertion, token removal) to occur.

  The QTI PKCS #11 interface provides access to software tokens, which are configured at runtime
  using the configuration passed in the C_Initialize() function. For this
  reason tokens cannot appear at runtime in a slot.

  @note1hang Specification variation: This function is not supported in this system.

  @datatypes
  #CK_FLAGS \n
  #CK_SLOT_ID_PTR \n
  #CK_VOID_PTR

  @param[in] flags      Blocks or unblocks the message.
  @param[in] pSlot      Pointer to the location that receives the slot ID.
  @param[in] pRserved   Pointer set to #NULL_PTR.

  @return
  CKR_CRYPTOKI_NOT_INITIALIZED  -- Cryptoki library is not initialized.
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  A session must have been opened with C_OpenSession().

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_WaitForSlotEvent)
#ifdef CK_NEED_ARG_LIST
(
  CK_FLAGS flags,
  CK_SLOT_ID_PTR pSlot,
  CK_VOID_PTR pRserved
);
#endif


/* Functions added for QTI PKCS #11 interface */
/** @name New Functions for the QTI PKCS #11 Interface
  These functions extend the PKCS #11 standard to take advantage of the QTI
  PKCS #11 interface architecture.
  @{
*/

/** @anchor C_Reset
  Removes and deletes a token.

  @datatypes
  #CK_SLOT_ID

  @param[in] slotID   Slot ID.

  @return
  CKR_OK -- Success. \n
  CKR_CRYPTOKI_NOT_INITIALIZED -- Cryptoki library is not initialized. \n
  CKR_SLOT_ID_INVALID -- Function could not find the slot ID. \n
  CKR_FUNCTION_NOT_SUPPORTED -- Function is not supported.

  @dependencies
  None.

  @newpage
*/
CK_PKCS11_FUNCTION_INFO(C_Reset)
#ifdef CK_NEED_ARG_LIST
(
  CK_SLOT_ID                slotID
);
#endif
/** @} */  /* end_name New Functions for the QTI PKCS #11 Interface */
