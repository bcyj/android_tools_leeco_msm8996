#ifndef _PKCS11T_H_
#define _PKCS11T_H_ 1

/**
  @file pkcs11t.h
  @brief Include file for PKCS #11; Revision: 1.4.
  See top of pkcs11.h for information about the macros that
  must be defined and the structure-packing conventions that
  must be set before including this file.

*/

/*=============================================================================
NOTE: The @brief description above does not appear in the PDF.

      The pkcs11_mainpage.dox file contains the group/module descriptions that
      are displayed in the output PDF generated using Doxygen and LaTeX. To
      edit or update any of the group/module text in the PDF, edit the
      pkcs11_mainpage.dox file or contact Tech Pubs.
=============================================================================*/

/*=============================================================================
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

=============================================================================*/

#include <stdint.h>

/** @addtogroup pkcs11_constants
  @{ */
/** Cryptoki major version 2. */
#define CRYPTOKI_VERSION_MAJOR 2
/** Cryptoki minor version 20. */
#define CRYPTOKI_VERSION_MINOR 20
/** Cryptoki Amendment 3. */
#define CRYPTOKI_VERSION_AMENDMENT 3

/** TRUE. */
#define CK_TRUE 1
/** FALSE. */
#define CK_FALSE 0

#ifndef CK_DISABLE_TRUE_FALSE
#ifndef FALSE
/** FALSE. */
#define FALSE CK_FALSE
#endif

#ifndef TRUE
/** TRUE. */
#define TRUE CK_TRUE
#endif
#endif
/** @} */ /* end_addtogroup pkcs11_constants */

/* Types used by the interface functions */
/** @addtogroup pkcs11_datatypes
 @{ */

/** Unsigned 8-bit value. */
typedef unsigned char     CK_BYTE;

/** Unsigned 8-bit character. */
typedef CK_BYTE           CK_CHAR;

/** Eight-bit UTF-8 character. */
typedef CK_BYTE           CK_UTF8CHAR;

/** Byte-sized Boolean flag. */
typedef CK_BYTE           CK_BBOOL;

/** Unsigned value, 32 bits in length. */
typedef uint32_t          CK_ULONG;

/** Signed value, the same size as CK_ULONG. */
typedef int32_t           CK_LONG;

/** Minimum of 32 bits, where each bit is a Boolean flag. */
typedef CK_ULONG          CK_FLAGS;
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/* some special values for certain CK_ULONG variables */
/** Information is unavailable. */
#define CK_UNAVAILABLE_INFORMATION (CK_ULONG)(~0)
/** Value is effectively infinite. */
#define CK_EFFECTIVELY_INFINITE    0
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */
/** Pointer to an unsigned 8-bit value. */
typedef CK_BYTE     CK_PTR   CK_BYTE_PTR;
/** Pointer to an unsigned 8-bit character. */
typedef CK_CHAR     CK_PTR   CK_CHAR_PTR;
/** Pointer to an 8-bit UTF-8 character. */
typedef CK_UTF8CHAR CK_PTR   CK_UTF8CHAR_PTR;
/** Pointer to an unsigned value, at least 32 bits in length. */
typedef CK_ULONG    CK_PTR   CK_ULONG_PTR;
/** Pointer to a void. */
typedef void        CK_PTR   CK_VOID_PTR;

/** Pointer to #CK_VOID_PTR (i.e., a pointer to a pointer to a void). */
typedef CK_VOID_PTR CK_PTR CK_VOID_PTR_PTR;

/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** Value is always invalid when used as a session handle or object handle. */
#define CK_INVALID_HANDLE 0
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** Version number of a Cryptoki interface or library, or an SSL
  implementation; the hardware or firmware version of a slot or token. */
typedef struct CK_VERSION {
  CK_BYTE       major;  /**< Major version number. The integer portion of a
                             version number. */
  CK_BYTE       minor;  /**< Minor version number. The portion of a version
                             number incremented in 1/100ths. */
} CK_VERSION;

/** Pointer to the version number of a Cryptoki interface or library specified
  in #CK_VERSION.
*/
typedef CK_VERSION CK_PTR CK_VERSION_PTR;

/** Provides information about the Cryptoki.
*/
typedef struct CK_INFO {
  CK_VERSION    cryptokiVersion;
  /**< Cryptoki interface version. */
  CK_UTF8CHAR   manufacturerID[32];
  /**< Manufacturer ID. This value is blank padded. */
  CK_FLAGS      flags;
  /**< Must be zero. */

  CK_UTF8CHAR   libraryDescription[32];
  /**< Library description. This value is blank padded. */
  CK_VERSION    libraryVersion;
  /**< Version of the library.
       @newpagetable */
} CK_INFO;

/** Pointer to Cryptoki information specified in #CK_INFO. */
typedef CK_INFO CK_PTR    CK_INFO_PTR;

/** Notification types provided by the Cryptoki to an application. */
typedef CK_ULONG CK_NOTIFICATION;

/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** @cond
*/
/** Surrenders the execution of a function to enable the application to perform
  other operations. */
#define CKN_SURRENDER       0

/** The OTP for a key on a connected token just changed. */
#define CKN_OTP_CHANGED     1
/** @endcond */


/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/* Types used by interface functions */

/** Cryptoki-assigned value that identifies a slot. */
typedef CK_ULONG          CK_SLOT_ID;

/** Pointer to the slot ID specified in #CK_SLOT_ID. */
typedef CK_SLOT_ID CK_PTR CK_SLOT_ID_PTR;


/** Slot information. */
typedef struct CK_SLOT_INFO {
  CK_UTF8CHAR   slotDescription[64];
  /**< Description of the slot. The value is blank padded. */
  CK_UTF8CHAR   manufacturerID[32];
  /**< Manufacturer ID. The value is blank padded. */
  CK_FLAGS      flags;
  /**< Bit flags that provide the capabilities of the slot. Values:
  - 0x00000001 -- CKF_TOKEN_PRESENT -- Token is present in the slot.
  - 0x00000002 -- CKF_REMOVABLE_DEVICE -- Reader supports removable devices.
  - 0x00000004 -- CKF_HW_SLOT -- Slot is a hardware slot, as opposed to a
                                 software slot implementing a soft token.
                                 @tablebulletend
  */

  CK_VERSION    hardwareVersion;  /**< Version of the hardware. */
  CK_VERSION    firmwareVersion;  /**< Version of the firmware. */
} CK_SLOT_INFO;

/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/* flags: bit flags that provide capabilities of the slot
 */
/** Token is present. */
#define CKF_TOKEN_PRESENT     0x00000001
/** Removable devices. */
#define CKF_REMOVABLE_DEVICE  0x00000002
/** Hardware slot.*/
#define CKF_HW_SLOT           0x00000004
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** Pointer to the slot information specified in #CK_SLOT_INFO. */
typedef CK_SLOT_INFO CK_PTR CK_SLOT_INFO_PTR;


/** Token information. */
typedef struct CK_TOKEN_INFO {
  CK_UTF8CHAR   label[32];           /**< Label. This value is blank padded. */
  CK_UTF8CHAR   manufacturerID[32];  /**< Manufacturer ID. This value is blank padded. */
  CK_UTF8CHAR   model[16];           /**< Model. This value is blank padded. */
  CK_CHAR       serialNumber[16];    /**< Serial number. This value is blank padded. */
  CK_FLAGS      flags;               /**< Attributes specified as flags. */

  CK_ULONG      ulMaxSessionCount;   /**< Maximum number of open sessions. */
  CK_ULONG      ulSessionCount;      /**< Number of open sessions. */
  CK_ULONG      ulMaxRwSessionCount; /**< Maximum number of sessions with
                                          read/write permission. */
  CK_ULONG      ulRwSessionCount;    /**< Number of open sessions with
                                          read/write permission. */
  CK_ULONG      ulMaxPinLen;         /**< Maximum length for a PIN in bytes. */
  CK_ULONG      ulMinPinLen;         /**< Minimum length for a PIN in bytes. */
  CK_ULONG      ulTotalPublicMemory; /**< Total amount of public memory in bytes. */
  CK_ULONG      ulFreePublicMemory;  /**< Amount of free public memory in bytes. */
  CK_ULONG      ulTotalPrivateMemory;/**< Total amount of private memory in bytes. */
  CK_ULONG      ulFreePrivateMemory; /**< Amount of free private memory in bytes. */

  CK_VERSION    hardwareVersion;     /**< Version of the hardware. */
  CK_VERSION    firmwareVersion;     /**< Version of the firmware. */
  CK_CHAR       utcTime[16];         /**< Time in Universal Coordinated Time.*/
} CK_TOKEN_INFO;


/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** Token has a random number generator. This flag is used for QTI PKCS #11
  notifications for secure keypads. */
#define CKF_RNG                     0x00000001

/** Token is write-protected. */
#define CKF_WRITE_PROTECTED         0x00000002

/** User must login.*/
#define CKF_LOGIN_REQUIRED          0x00000004

/** User's PIN is set. */
#define CKF_USER_PIN_INITIALIZED    0x00000008

/** When the state of cryptographic operations of a session is successfully
  saved, all keys needed to continue those operations are stored in the
  state.
*/
#define CKF_RESTORE_KEY_NOT_NEEDED  0x00000020

/** Token has a clock. The time on that clock is returned in #CK_TOKEN_INFO.
*/
#define CKF_CLOCK_ON_TOKEN          0x00000040

/** Enables the user to login without sending a PIN through the Cryptoki
  library.
  */
#define CKF_PROTECTED_AUTHENTICATION_PATH 0x00000100

/** Enables a single session with the token to perform dual simultaneous
  cryptographic operations (e.g., digest and encrypt; decrypt and digest; sign
  and encrypt; decrypt and sign).
*/
#define CKF_DUAL_CRYPTO_OPERATIONS  0x00000200

/** Token has been initialized using @ref C_InitToken "C_InitToken()" or an equivalent
  mechanism outside the scope of PKCS #11. Calling @ref C_InitToken "C_InitToken()" when this
  flag is set causes the token to reinitialize.
*/
#define CKF_TOKEN_INITIALIZED       0x00000400

/** @cond
*/
/** @deprecated This define is deprecated in v2.11.

  Token supports secondary authentication for private key objects.
*/
#define CKF_SECONDARY_AUTHENTICATION  0x00000800
/** @endcond */

/** Incorrect user login PIN has been entered at least once since the last
  successful authentication.
*/
#define CKF_USER_PIN_COUNT_LOW       0x00010000

/** Supplying an incorrect user PIN causes the login to become locked.
*/
#define CKF_USER_PIN_FINAL_TRY       0x00020000

/** User PIN has been locked. User login to the token is not possible.
*/
#define CKF_USER_PIN_LOCKED          0x00040000

/** User PIN value is the default value set by token initialization or
  manufacturing, or the PIN has been expired by the card.
*/
#define CKF_USER_PIN_TO_BE_CHANGED   0x00080000

/** Incorrect security officer login PIN has been entered at least once since
  the last successful authentication.
*/
#define CKF_SO_PIN_COUNT_LOW         0x00100000

/** Supplying an incorrect security officer PIN causes the login to become
  locked.
*/
#define CKF_SO_PIN_FINAL_TRY         0x00200000

/** Security officer PIN has been locked. The SO login to the token is not
  possible.
*/
#define CKF_SO_PIN_LOCKED            0x00400000

/** Security officer  PIN value is the default value set by token
  initialization or manufacturing, or the PIN has been expired by the card.
*/
#define CKF_SO_PIN_TO_BE_CHANGED     0x00800000

/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** Pointer to the token information specified in CK_TOKEN_INFO. */
typedef CK_TOKEN_INFO CK_PTR CK_TOKEN_INFO_PTR;


/** Cryptoki-assigned value that identifies a session. */
typedef CK_ULONG          CK_SESSION_HANDLE;

/** Pointer to the session handle. */
typedef CK_SESSION_HANDLE CK_PTR CK_SESSION_HANDLE_PTR;


/** Types of Cryptoki users.
*/
typedef CK_ULONG          CK_USER_TYPE;
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** Security officer. */
#define CKU_SO    0

/** User in the non-secure world. */
#define CKU_USER  1

/** @cond
*/

/** Context-specific user type. When the user type is CKU_CONTEXT_SPECIFIC, the
  behavior of @ref C_Login "C_Login()" depends on the context in which it is
  called.
*/
#define CKU_CONTEXT_SPECIFIC   2

/** @endcond */

/** Token has no user logged in. This define is a QTI PKCS #11 extension. */
#define CKU_NONE  8

/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** Session states.
*/
typedef CK_ULONG          CK_STATE;

/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** Read-only permission for public sessions. */
#define CKS_RO_PUBLIC_SESSION  0
/** Read-only permission for user functions. */
#define CKS_RO_USER_FUNCTIONS  1
/** Read/write permission for public sessions. */
#define CKS_RW_PUBLIC_SESSION  2
/** Read/write permission for user functions. */
#define CKS_RW_USER_FUNCTIONS  3
/** Read/write permission for security officer functions. */
#define CKS_RW_SO_FUNCTIONS    4
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** Session information.
*/
typedef struct CK_SESSION_INFO {
  CK_SLOT_ID    slotID;  /**< Slot ID for the session.*/
  CK_STATE      state;   /**< State of the session. */
  CK_FLAGS      flags;   /**< Attributes specified as flags. */ /* see below */

  CK_ULONG      ulDeviceError;  /**< Device-dependent error code. */
} CK_SESSION_INFO;
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** Session has read/write permission. */
#define CKF_RW_SESSION          0x00000002

/** Session is serial, not parallel. */
#define CKF_SERIAL_SESSION      0x00000004

/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** Pointer to the session information. */
typedef CK_SESSION_INFO CK_PTR CK_SESSION_INFO_PTR;

/** Token-specific identifier for an object.  */
typedef CK_ULONG          CK_OBJECT_HANDLE;

/** Pointer to the object handle. */
typedef CK_OBJECT_HANDLE CK_PTR CK_OBJECT_HANDLE_PTR;


/** Identifies the classes (or types) of objects that Cryptoki recognizes.
*/
typedef CK_ULONG          CK_OBJECT_CLASS;
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/* The following classes of objects are defined: */
/** @name Object Classes
  @{ */

/** Contains information defined by the application. */
#define CKO_DATA              0x00000000
/** Contains public-key or attribute certificates. */
#define CKO_CERTIFICATE       0x00000001
/** Contains public encryption or authentication keys. */
#define CKO_PUBLIC_KEY        0x00000002
/** Contains private encryption or authentication keys. */
#define CKO_PRIVATE_KEY       0x00000003
/** Contains secret encryption or authentication keys. */
#define CKO_SECRET_KEY        0x00000004

/** @cond
*/
/** Identifies device features.
*/
#define CKO_HW_FEATURE        0x00000005
/** Contains public domain parameters.
*/
#define CKO_DOMAIN_PARAMETERS 0x00000006
/** Contains information about mechanisms supported by a device.
  @sa #CK_MECHANISM_INFO
*/
#define CKO_MECHANISM         0x00000007
/** @endcond */

/** Contains secret keys used by OTP tokens. */
#define CKO_OTP_KEY           0x00000008

/** Reserved for token vendors. For interoperability, vendors are to register
  their object classes through the PKCS process. */
#define CKO_VENDOR_DEFINED    0x80000000

/** @} */ /* end_name Object Classes */
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** Pointer to an object class. */
typedef CK_OBJECT_CLASS CK_PTR CK_OBJECT_CLASS_PTR;

/** Identifies the hardware feature type of an object with #CK_OBJECT_CLASS.
*/
typedef CK_ULONG          CK_HW_FEATURE_TYPE;

/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** @name Hardware Feature Types
  @{ */

/** Identifies the hardware counters on the device. */
#define CKH_MONOTONIC_COUNTER  0x00000001
/** Identifies the real-time clocks that exist on the device. */
#define CKH_CLOCK           0x00000002
/** Identifies the presentation capabilities of the device.
*/
#define CKH_USER_INTERFACE  0x00000003
/** Reserved for token vendors. For interoperability, vendors are to register
  their feature types through the PKCS process. */
#define CKH_VENDOR_DEFINED  0x80000000

/** @} */ /* end_name_Hardware Feature Types */
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** Identifies a key type.
*/
typedef CK_ULONG          CK_KEY_TYPE;
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/* the following key types are defined: */
/** @name Key Types
  @{ */

/** RSA key type. */
#define CKK_RSA             0x00000000

/** @cond
*/
/** Digital Signature Algorithm key type. */
#define CKK_DSA             0x00000001
/** Diffie-Hellman key type. */
#define CKK_DH              0x00000002

/** @deprecated This define is deprecated in v2.11. See #CKK_EC.

  Elliptic curve key type.
*/
#define CKK_ECDSA           0x00000003
/** Elliptic curve key type. */
#define CKK_EC              0x00000003
/** X9.42 Diffie-Hellman key type. */
#define CKK_X9_42_DH        0x00000004
/** Key Exchange Algorithm type key. */
#define CKK_KEA             0x00000005
/** @endcond */

/** Generic secret key type. */
#define CKK_GENERIC_SECRET  0x00000010

/** @cond
*/
/** RC2 key type. */
#define CKK_RC2             0x00000011
/** RC4 key type. */
#define CKK_RC4             0x00000012
/** @endcond */

/** Single-length Data Encryption Standard key type. */
#define CKK_DES             0x00000013

/** @cond
*/
/** Double-length DES key type. */
#define CKK_DES2            0x00000014
/** @endcond */

/** Triple-length DES key type. */
#define CKK_DES3            0x00000015

/** @cond
*/
/* all these key types are new for v2.0 */
/** Symmetric-key block cipher key type.
*/
#define CKK_CAST            0x00000016
/** CAST3 key type.
*/
#define CKK_CAST3           0x00000017
/** @deprecated This define is deprecated in v2.11. See #CKK_CAST128.

  CAST5 key type.
*/
#define CKK_CAST5           0x00000018
/** CAST128 key type. */
#define CKK_CAST128         0x00000018
/** RC5 key type. */
#define CKK_RC5             0x00000019
/** IDEA key type. */
#define CKK_IDEA            0x0000001A
/** SKIPJACK key type. Holds a single-length MEK or a TEK. */
#define CKK_SKIPJACK        0x0000001B
/** BATON key type. */
#define CKK_BATON           0x0000001C
/** JUNIPER key type. */
#define CKK_JUNIPER         0x0000001D
/** Commercial Data Masking Facility key type.*/
#define CKK_CDMF            0x0000001E
/** @endcond */

/** Advanced Encryption Standard key type.
*/
#define CKK_AES             0x0000001F

/** @cond
*/
/** Blowfish key type. */
#define CKK_BLOWFISH        0x00000020
/** Twofish key type. */
#define CKK_TWOFISH         0x00000021

/** RSA SecurID key type. */
#define CKK_SECURID         0x00000022
/** @endcond */

/** HMAC-based one-time password key type. */
#define CKK_HOTP            0x00000023

/** @cond
*/
/** Comment needed here. */
#define CKK_ACTI            0x00000024

/** Camellia key type. */
#define CKK_CAMELLIA                   0x00000025

/** Aria key type. */
#define CKK_ARIA                       0x00000026
/** @endcond */

/** Reserved for token vendors. For interoperability, vendors are to register
  their key types through the PKCS process. */
#define CKK_VENDOR_DEFINED  0x80000000
/** @} */ /* end_name Key Types */
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** Identifies a certificate type. */
typedef CK_ULONG          CK_CERTIFICATE_TYPE;

/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/* The following certificate types are defined: */
/** @name Certificate Types
  @{ */
/** X.509 certificate type. */
#define CKC_X_509           0x00000000

/** @cond
*/
/** X.509 attribute certificate type. */
#define CKC_X_509_ATTR_CERT 0x00000001
/** Wireless transport layer security certificate type. */
#define CKC_WTLS            0x00000002
/** @endcond */

/** Reserved for token vendors. For interoperability, vendors are to register
  their certificate types through the PKCS process. */
#define CKC_VENDOR_DEFINED  0x80000000
/** @} */ /* end_name Certificate Types */
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** Identifies an attribute type.  */
typedef CK_ULONG          CK_ATTRIBUTE_TYPE;
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** Identifies an attribute that consists of an array of values. */
#define CKF_ARRAY_ATTRIBUTE    0x40000000

/** Produces OTP values in decimal, UTF8-encoded format. (Default)
*/
#define CK_OTP_FORMAT_DECIMAL      0
/** Produces OTP values in hexadecimal, UTF8-encoded format. \n
  @note1hang This define is related to the #CKA_OTP_FORMAT attribute.
*/
#define CK_OTP_FORMAT_HEXADECIMAL  1
/** Produces OTP values in alphanumeric, UTF8-encoded format. \n
  @note1hang This define is related to the #CKA_OTP_FORMAT attribute.
*/
#define CK_OTP_FORMAT_ALPHANUMERIC 2
/** Produces OTP values in binary format only. \n
  @note1hang This define is related to the #CKA_OTP_FORMAT attribute.
*/
#define CK_OTP_FORMAT_BINARY       3

/** Ignores any provided parameter. \n
  @note1hang This define is related to the CKA_OTP_..._REQUIREMENT attributes.
*/
#define CK_OTP_PARAM_IGNORED       0
/** Parameter may be supplied, but is not required. \n
  @note1hang This define is related to the CKA_OTP_..._REQUIREMENT attributes.
*/
#define CK_OTP_PARAM_OPTIONAL      1
/** Parameter is required. \n
  @note1hang This define is related to the CKA_OTP_..._REQUIREMENT attributes.
*/
#define CK_OTP_PARAM_MANDATORY     2
/** Parameter is not supported by the key. \n
  @note1hang This define is related to the CKA_OTP_..._REQUIREMENT attributes.
             This define is a QTI PKCS #11 extension.
*/
#define CK_OTP_PARAM_NOT_SUPPORTED  8


/* The following attribute types are defined: */

/** Object class (type). */
#define CKA_CLASS              0x00000000
/** Indicates whether the object is a token object or a session object. */
#define CKA_TOKEN              0x00000001
/** Indicates whether the object is a private object or a public object. */
#define CKA_PRIVATE            0x00000002
/** Provides a description of the object.  */
#define CKA_LABEL              0x00000003
/** Provides a description of the application that manages the object. */
#define CKA_APPLICATION        0x00000010
/** @anchor CKA_VALUE
  Value of the object. */
#define CKA_VALUE              0x00000011

/** Indicates the type of the data object value. */
#define CKA_OBJECT_ID          0x00000012

/** Type of certificate. */
#define CKA_CERTIFICATE_TYPE   0x00000080
/** Certificate issuer name. */
#define CKA_ISSUER             0x00000081
/** Certificate serial number. */
#define CKA_SERIAL_NUMBER      0x00000082

/** Certificate issuer name. This differs from the CKA_ISSUER attribute in that
  it uses ASN.1 syntax and encoding.
*/
#define CKA_AC_ISSUER          0x00000083
/** Certificate subject. This differs from the CKA_SUBJECT attribute in that it
  uses ASN.1 syntax and encoding.
*/
#define CKA_OWNER              0x00000084
/** Certificate attribute types. The object identifier values enable
  applications to search for a particular attribute certificate without
  fetching and parsing the certificate itself.
*/
#define CKA_ATTR_TYPES         0x00000085


/** Indicates the key can be trusted for the application for which it was
  created.
*/
#define CKA_TRUSTED            0x00000086


/** Indicates whether a stored certificate is: \n
  - A user certificate for which the corresponding private key is available on
    the token (token user)
  - A CA certificate (authority)
  - An other end-entity certificate (other entity)

  @note1hang This attribute may not be modified after an object is created.
*/
#define CKA_CERTIFICATE_CATEGORY        0x00000087
/** Java mobile information device profile security domain certificate. */
#define CKA_JAVA_MIDP_SECURITY_DOMAIN   0x00000088
/** Certificate location. */
#define CKA_URL                         0x00000089
/** SHA-1 hash of the subject public key. */
#define CKA_HASH_OF_SUBJECT_PUBLIC_KEY  0x0000008A
/** SHA-1 hash of the issuer public key. */
#define CKA_HASH_OF_ISSUER_PUBLIC_KEY   0x0000008B
/** Key checksum. */
#define CKA_CHECK_VALUE                 0x00000090

/** Key type the domain parameters can generate. */
#define CKA_KEY_TYPE           0x00000100
/** Certificate subject name. */
#define CKA_SUBJECT            0x00000101
/** Key identifier for a public/private key pair. */
#define CKA_ID                 0x00000102
/** Prevents certain attributes from being revealed outside the token. */
#define CKA_SENSITIVE          0x00000103
/** Supports encryption. */
#define CKA_ENCRYPT            0x00000104
/** Supports decryption. */
#define CKA_DECRYPT            0x00000105
/** Supports wrapping of other keys. */
#define CKA_WRAP               0x00000106
/** Supports unwrapping of other keys. */
#define CKA_UNWRAP             0x00000107
/** Supports authentication codes. */
#define CKA_SIGN               0x00000108
/** Supports signatures where the data can be recovered. */
#define CKA_SIGN_RECOVER       0x00000109
/** Supports verification where the signature is an appendix to the data. */
#define CKA_VERIFY             0x0000010A
/** Supports verification where the data is recovered from the signature. */
#define CKA_VERIFY_RECOVER     0x0000010B
/** Enables other keys to be derived from this key. */
#define CKA_DERIVE             0x0000010C
/** Key start date. */
#define CKA_START_DATE         0x00000110
/** Key end date. */
#define CKA_END_DATE           0x00000111
/** Modulus of the key. */
#define CKA_MODULUS            0x00000120
/** Length of the modulus in bits. */
#define CKA_MODULUS_BITS       0x00000121
/** Public exponent @em e. */
#define CKA_PUBLIC_EXPONENT    0x00000122
/** Private exponent @em d. */
#define CKA_PRIVATE_EXPONENT   0x00000123
/** Prime @em p. */
#define CKA_PRIME_1            0x00000124
/** Prime @em q. */
#define CKA_PRIME_2            0x00000125
/** Private exponent @em d modulo @em p-1. */
#define CKA_EXPONENT_1         0x00000126
/** Private exponent @em d modulo @em q-1. */
#define CKA_EXPONENT_2         0x00000127
/** Chinese Remainder Theorem coefficient @em 1/q modulo @em p. */
#define CKA_COEFFICIENT        0x00000128
/** Prime @em p (512 to 1024 bits, in steps of 64 bits). */
#define CKA_PRIME              0x00000130
/** Subprime @em q (160 bits). */
#define CKA_SUBPRIME           0x00000131
/** Base @em g. */
#define CKA_BASE               0x00000132

/* CKA_PRIME_BITS and CKA_SUB_PRIME_BITS are new for v2.11 */
/** Length of the prime value. */
#define CKA_PRIME_BITS         0x00000133
/** Length of the subprime value. */
#define CKA_SUBPRIME_BITS      0x00000134
/** Length of the subprime value. */
#define CKA_SUB_PRIME_BITS     CKA_SUBPRIME_BITS
/* (To retain backwards-compatibility) */

/** Length of the private value in bits. */
#define CKA_VALUE_BITS         0x00000160
/** Length of the key value in bytes. */
#define CKA_VALUE_LEN          0x00000161

/* CKA_EXTRACTABLE, CKA_LOCAL, CKA_NEVER_EXTRACTABLE,
 * CKA_ALWAYS_SENSITIVE, CKA_MODIFIABLE, CKA_ECDSA_PARAMS,
 * and CKA_EC_POINT are new for v2.0 */
/** Certain attributes of the private key can be revealed outside the token.
*/
#define CKA_EXTRACTABLE        0x00000162
/** Local key generation. */
#define CKA_LOCAL              0x00000163
/** Certain attributes of the private key can never be revealed outside the
  token.
*/
#define CKA_NEVER_EXTRACTABLE  0x00000164
/** Always prevents certain attributes from being revealed outside the
  token.
*/
#define CKA_ALWAYS_SENSITIVE   0x00000165

/** Mechanism used to generate the key material. */
#define CKA_KEY_GEN_MECHANISM  0x00000166

/** Attributes can be modified.
*/
#define CKA_MODIFIABLE         0x00000170

/** @cond
*/
/** @deprecated This define is deprecated in v2.11. See #CKA_EC_PARAMS.

  ANSI X9.62 parameters values.
*/
#define CKA_ECDSA_PARAMS       0x00000180
/** ANSI X9.62 parameters values. */
#define CKA_EC_PARAMS          0x00000180

/** ANSI X9.62 ECPoint value @em Q. */
#define CKA_EC_POINT           0x00000181

/** @deprecated This define is deprecated in v2.11.

  Comment needed here.
*/
#define CKA_SECONDARY_AUTH     0x00000200
/** @deprecated This define is deprecated in v2.11.

  Comment needed here.
*/
#define CKA_AUTH_PIN_FLAGS     0x00000201

/** PIN is required for each use of the key.
*/
#define CKA_ALWAYS_AUTHENTICATE  0x00000202
/** @endcond */

/** Key can only be wrapped with a trusted wrapping key. */
#define CKA_WRAP_WITH_TRUSTED    0x00000210

/** @cond
*/
/** Attribute template to compare with keys wrapped using the wrapping key.
  Keys that do not match cannot be wrapped. */
#define CKA_WRAP_TEMPLATE        (CKF_ARRAY_ATTRIBUTE|0x00000211)
/** Attribute template for keys unwrapped using the wrapping key.
*/
#define CKA_UNWRAP_TEMPLATE      (CKF_ARRAY_ATTRIBUTE|0x00000212)
/** @endcond */

/** Format of the generated OTP. It can be decimal, hexadecimal,
  alphanumeric or binary.
*/
#define CKA_OTP_FORMAT                0x00000220
/** Length of OTP values.
*/
#define CKA_OTP_LENGTH                0x00000221
/** Interval between one-time password values measured in seconds. */
#define CKA_OTP_TIME_INTERVAL         0x00000222
/** Returns one-time passwords in a UTF8-encoded printable string. */
#define CKA_OTP_USER_FRIENDLY_MODE    0x00000223
/** Challenge requirements when generating or verifying OTP values. */
#define CKA_OTP_CHALLENGE_REQUIREMENT 0x00000224
/** Time requirement when generating or verifying OTP values. */
#define CKA_OTP_TIME_REQUIREMENT      0x00000225
/** Counter requirement when generating or verifying OTP values. */
#define CKA_OTP_COUNTER_REQUIREMENT   0x00000226
/** PIN requirement when generating or verifying OTP values. */
#define CKA_OTP_PIN_REQUIREMENT       0x00000227
/** Value of the associated internal counter. */
#define CKA_OTP_COUNTER               0x0000022E
/** Value of the associated internal UTC time. Format: YYYYMMDDhhmmss. */
#define CKA_OTP_TIME                  0x0000022F
/** User associated with the OTP key. */
#define CKA_OTP_USER_IDENTIFIER       0x0000022A
/** Service that can validate OTPs. */
#define CKA_OTP_SERVICE_IDENTIFIER    0x0000022B
/** Logotype that identifies a service. */
#define CKA_OTP_SERVICE_LOGO          0x0000022C
/** MIME type of CKA_OTP_SERVICE_LOGO. */
#define CKA_OTP_SERVICE_LOGO_TYPE     0x0000022D

/** @cond
*/
/** Hardware feature of the device. */
#define CKA_HW_FEATURE_TYPE    0x00000300
/** Resets to a previously returned value if the token is initialized using
  @ref C_InitToken "C_InitToken()".
*/
#define CKA_RESET_ON_INIT      0x00000301
/** Counter has been reset. */
#define CKA_HAS_RESET          0x00000302

/** Screen resolution in pixels in the X axis. */
#define CKA_PIXEL_X                     0x00000400
/** Screen resolution in pixels in the Y axis. */
#define CKA_PIXEL_Y                     0x00000401
/** Screen resoultion measured in DPI, pixels (dots) per inch. */
#define CKA_RESOLUTION                  0x00000402
/** Number of character rows in a character-oriented display. */
#define CKA_CHAR_ROWS                   0x00000403
/** Number of character columns in a character-oriented display. */
#define CKA_CHAR_COLUMNS                0x00000404
/** Color support. */
#define CKA_COLOR                       0x00000405
/** Number of bits of color or grayscale information per pixel. */
#define CKA_BITS_PER_PIXEL              0x00000406
/** Supported character set. */
#define CKA_CHAR_SETS                   0x00000480
/** Supported content transfer encoding methods. */
#define CKA_ENCODING_METHODS            0x00000481
/** Supported MIME types. */
#define CKA_MIME_TYPES                  0x00000482
/** Type of mechanism object. */
#define CKA_MECHANISM_TYPE              0x00000500
/** Required cryptographic message syntax attributes. */
#define CKA_REQUIRED_CMS_ATTRIBUTES     0x00000501
/** Default cryptographic message syntax attributes.*/
#define CKA_DEFAULT_CMS_ATTRIBUTES      0x00000502
/** Supported cryptographic message syntax attributes. */
#define CKA_SUPPORTED_CMS_ATTRIBUTES    0x00000503
/** @endcond */

/** Mechanisms allowed for use. */
#define CKA_ALLOWED_MECHANISMS          (CKF_ARRAY_ATTRIBUTE|0x00000600)

/** Reserved for token vendors. For interoperability, vendors are to register
  their attribute types through the PKCS process.
*/
#define CKA_VENDOR_DEFINED     0x80000000

/** @cond
*/
/** Attribute used to identify the encrypted data associated with an object. */
#define CKA_ENCRYPTED_OBJECT            (CKA_VENDOR_DEFINED|0x00000001)
/** @endcond */
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** Provides the type, length, and value of an attribute.
*/
typedef struct CK_ATTRIBUTE {
  CK_ATTRIBUTE_TYPE type;       /**< Type of the attribute. */
  CK_VOID_PTR       pValue;     /**< Pointer to the attribute value. */
  CK_ULONG          ulValueLen; /**< Length of the attribute value in bytes. */
} CK_ATTRIBUTE;

/** Pointer to #CK_ATTRIBUTE. */
typedef CK_ATTRIBUTE CK_PTR CK_ATTRIBUTE_PTR;

/** Defines a date. */
typedef struct CK_DATE{
  CK_CHAR       year[4];   /**< Year. Value: 1900 to 9999. */
  CK_CHAR       month[2];  /**< Month. Value: 01 to 12. */
  CK_CHAR       day[2];    /**< Day. Value: 01 to 31. */
} CK_DATE;

/** Identifies a mechanism type. */
typedef CK_ULONG          CK_MECHANISM_TYPE;
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** @name Mechanism Types
  @{ */
/** PKCS #1 RSA key pair generation mechanism based on the RSA public-key
  cryptosystem. */
#define CKM_RSA_PKCS_KEY_PAIR_GEN      0x00000000
/** PKCS #1 v1.5 RSA mechanism. Supports single-part encryption and
  decryption. */
#define CKM_RSA_PKCS                   0x00000001

/** @cond
*/
/** ISO/IEC 9796 RSA mechanism. Supports single-part signatures and
  verification with and without message recovery based on the RSA public-key
  cryptosystem and the block formats. This mechanism processes only byte
  strings. */
#define CKM_RSA_9796                   0x00000002
/** X.509 (raw) RSA mechanism. Supports:
  - Single-part encryption and decryption
  - Single-part signatures and verification with and without message recovery
  - Key wrapping
  - Key unwrapping */
#define CKM_RSA_X_509                  0x00000003
/** @endcond */
/** @} */ /* end_name  Mechanism Types */

/** @name Hash and Sign Mechanisms
  @{ */

/** @cond
*/
/** PKCS #1 v1.5 RSA signature with MD2 mechanism. Supports single- and
  multiple-part digital signatures and verification operations without message
  recovery using the MD2 hash function.
*/
#define CKM_MD2_RSA_PKCS               0x00000004
/** PKCS #1 v1.5 RSA signature with MD5 mechanism. Supports single- and
  multiple-part digital signatures and verification operations without message
  recovery using the MD5 hash function.
*/
#define CKM_MD5_RSA_PKCS               0x00000005
/** @endcond */

/** PKCS #1 v1.5 RSA signature with SHA-1 mechanism. Supports single- and
  multiple-part digital signatures and verification operations without message
  recovery using the SHA-1 hash function.
*/
#define CKM_SHA1_RSA_PKCS              0x00000006
/** @} */ /* end_name Hash and Sign Mechanisms */

/** @cond
*/
/** PKCS #1 v1.5 RSA signature with RIPEMD-128 hash function. */
#define CKM_RIPEMD128_RSA_PKCS         0x00000007
/** PKCS #1 v1.5 RSA signature with RIPEMD-160 hash function. */
#define CKM_RIPEMD160_RSA_PKCS         0x00000008
/** PKCS #1 RSA optimal asymmetric encryption padding for RSA mechanism.
  Supports:
  - Single-part encryption and decryption
  - Key wrapping
  - Key unwrapping
*/
#define CKM_RSA_PKCS_OAEP              0x00000009

/** X9.31 RSA key pair generation mechanism.
*/
#define CKM_RSA_X9_31_KEY_PAIR_GEN     0x0000000A
/** ANSI X9.31 RSA mechanism. Supports single-part signatures and verification
  without message recovery based on the RSA public-key cryptosystem and the
  block formats defined in ANSI X9.31.
*/
#define CKM_RSA_X9_31                  0x0000000B
/** ANSI X9.31 RSA signature with SHA-1 mechanism. Supportss single- and
  multiple-part digital signatures and verification operations without message
  recovery.
*/
#define CKM_SHA1_RSA_X9_31             0x0000000C
/** @endcond */

/** PKCS #1 RSA Probabilistic Signature Scheme mechanism. Supports single-part
  signature generation and verification without message recovery and
  corresponds only to block formatting and RSA encryption, given a hash value. It does
  not compute a hash value for the message to be signed.
*/
#define CKM_RSA_PKCS_PSS               0x0000000D
/** PKCS #1 RSA PSS signature with SHA-1 mechanism. Supports single- and
  multiple-part digital signatures and verification operations without message
  recovery.
*/
#define CKM_SHA1_RSA_PKCS_PSS          0x0000000E

/** @cond
*/
/** Digital Signature Algorithm key pair generation mechanism. */
#define CKM_DSA_KEY_PAIR_GEN           0x00000010
/** DSA without hashing mechanism. Supports single-part signatures and
  verification based on the Digital Signature Algorithm and corresponds only to
  the part of the DSA that processes the 20-byte hash value; it does not
  compute the hash value. */
#define CKM_DSA                        0x00000011
/** DSA with SHA-1 mechanism. Supports single- and multiple-part signatures and
  verification based on the Digital Signature Algorithm. */
#define CKM_DSA_SHA1                   0x00000012
/** PKCS #3 Diffie-Hellman key pair generation mechanism. Generates
  Diffie-Hellman public/private key pairs with a particular prime and base. */
#define CKM_DH_PKCS_KEY_PAIR_GEN       0x00000020
/** PKCS #3 Diffie-Hellman key derivation mechanism. Derives a secret key from
  a Diffie-Hellman private key and the public value of the other party. */
#define CKM_DH_PKCS_DERIVE             0x00000021

/** X9.42 Diffie-Hellman key pair generation mechanism. Generates X9.42
  Diffie-Hellman public/private key pairs with a particular prime, base and
  subprime.
*/
#define CKM_X9_42_DH_KEY_PAIR_GEN      0x00000030
/** X9.42 Diffie-Hellman key derivation mechanism. Derives a secret value and
  truncates the result.
*/
#define CKM_X9_42_DH_DERIVE            0x00000031
/** X9.42 Diffie-Hellman hybrid key derivation mechanism. Derives a secret
  value and truncates the result.
*/
#define CKM_X9_42_DH_HYBRID_DERIVE     0x00000032
/** X9.42 Diffie-Hellman Menezes-Qu-Vanstone key derivation mechanism. */
#define CKM_X9_42_MQV_DERIVE           0x00000033
/** @endcond */

/** PKCS #1 v1.5 RSA signature with SHA-256 mechanism. */
#define CKM_SHA256_RSA_PKCS            0x00000040

/** @cond
*/
/** PKCS #1 v1.5 RSA signature with SHA-384 mechanism */
#define CKM_SHA384_RSA_PKCS            0x00000041
/** PKCS #1 v1.5 RSA signature with SHA-512 mechanism. */
#define CKM_SHA512_RSA_PKCS            0x00000042
/** @endcond */

/** PKCS #1 RSA PSS signature with SHA-256 mechanism. */
#define CKM_SHA256_RSA_PKCS_PSS        0x00000043

/** @cond
*/
/** PKCS #1 RSA PSS signature with SHA-348 mechanism. */
#define CKM_SHA384_RSA_PKCS_PSS        0x00000044
/** PKCS #1 RSA PSS signature with SHA-512 mechanism. */
#define CKM_SHA512_RSA_PKCS_PSS        0x00000045

/** PKCS #1 v1.5 RSA signature with SHA-224 mechanism. */
#define CKM_SHA224_RSA_PKCS            0x00000046
/** PKCS #1 RSA PSS signature with SHA-224 mechanism.  */
#define CKM_SHA224_RSA_PKCS_PSS        0x00000047
/** @endcond */

/** @cond
*/
/** Key generation mechanism for RSA block cipher RC2. */
#define CKM_RC2_KEY_GEN                0x00000100
/** Mechanism based on RSA block cipher RC2 and Electronic Codebook mode.
  Supports:
  - Single- and multiple-part encryption and decryption
  - Key wrapping
  - Key unwrapping
*/
#define CKM_RC2_ECB                    0x00000101
/** Mechanism based on RSA block cipher RC2 and cipher-block chaining mode.
  Supports:
  - Single- and multiple-part encryption and decryption
  - Key wrapping
  - Key unwrapping */
#define CKM_RC2_CBC                    0x00000102
/** Mechanism based on CKM_RC2_MAC_GENERAL; however, it only contains the
  effective number of bits in the RC2 search space. */
#define CKM_RC2_MAC                    0x00000103

/** Mechanism based on RSA block cipher RC2 and data authentication that
  supports single- and multiple-part signatures and verification.
*/
#define CKM_RC2_MAC_GENERAL            0x00000104
/** RC2-CBC with PKCS padding. Supports:
  - Single- and multiple-part encryption and decryption
  - Key wrapping
  - Key unwrapping
*/
#define CKM_RC2_CBC_PAD                0x00000105

/** RC4 key generation mechanism. */
#define CKM_RC4_KEY_GEN                0x00000110
/** Mechanism based on RSA stream cipher RC4 that supports single- and
  multiple-part encryption and decryption. */
#define CKM_RC4                        0x00000111
/** @endcond */


/** Data Encryption Standard key generation mechanism. */
#define CKM_DES_KEY_GEN                0x00000120
/** Electronic CodeBook encryption with a DES key. */
#define CKM_DES_ECB                    0x00000121
/** Cypher-Block Chaining encryption with a DES key. */
#define CKM_DES_CBC                    0x00000122

/** @cond
*/
/** Comment needed here. */
#define CKM_DES_MAC                    0x00000123

/** Comment needed here. */
#define CKM_DES_MAC_GENERAL            0x00000124
/** @endcond */

/** Cypher-Block Chaining encryption with padding and a DES key. */
#define CKM_DES_CBC_PAD                0x00000125

/** @cond
*/
/** Double-length DES key generation mechanism. */
#define CKM_DES2_KEY_GEN               0x00000130
/** @endcond */

/** Triple-length DES key generation mechanism. */
#define CKM_DES3_KEY_GEN               0x00000131
/** Electronic CodeBook encryption with a triple-length DES key. */
#define CKM_DES3_ECB                   0x00000132
/** Cypher-Block Chaining encryption with a triple-length DES key. */
#define CKM_DES3_CBC                   0x00000133

/** @cond
*/
/** Comment needed here. */
#define CKM_DES3_MAC                   0x00000134
/** Comment needed here. */
#define CKM_DES3_MAC_GENERAL           0x00000135
/** @endcond */

/** Cypher-block chaining encryption with padding with a triple-length DES key.
*/
#define CKM_DES3_CBC_PAD               0x00000136

/** @cond
*/
/** Comment needed here. */
#define CKM_CDMF_KEY_GEN               0x00000140
/** Comment needed here. */
#define CKM_CDMF_ECB                   0x00000141
/** Comment needed here. */
#define CKM_CDMF_CBC                   0x00000142
/** Comment needed here. */
#define CKM_CDMF_MAC                   0x00000143
/** Comment needed here. */
#define CKM_CDMF_MAC_GENERAL           0x00000144
/** Comment needed here. */
#define CKM_CDMF_CBC_PAD               0x00000145

/** Output feedback mode that supports single and multiple-part encryption
  and decryption with DES.
*/
#define CKM_DES_OFB64                  0x00000150
/** Output feedback mode that supports single and multiple-part encryption
  and decryption with DES.
*/
#define CKM_DES_OFB8                   0x00000151
/** Cipher feedback mode that supports single and multiple-part encryption
  and decryption with DES.
*/
#define CKM_DES_CFB64                  0x00000152
/** Cipher feedback mode that supports single and multiple-part encryption
  and decryption with DES.
*/
#define CKM_DES_CFB8                   0x00000153

/** Mechanism for message digesting that follows the MD2 message-digest
  algorithm. */
#define CKM_MD2                        0x00000200

/** Comment needed here. */
#define CKM_MD2_HMAC                   0x00000201
/** Mechanism for signatures and verification. It uses the HMAC construction,
  based on the MD2 hash function.
*/
#define CKM_MD2_HMAC_GENERAL           0x00000202

/** Mechanism for message digesting that follows the MD5 message-digest
  algorithm. */
#define CKM_MD5                        0x00000210

/** Comment needed here.
*/
#define CKM_MD5_HMAC                   0x00000211
/** Mechanism for signatures and verification. It uses the HMAC construction,
  based on the MD2 hash function.
*/
#define CKM_MD5_HMAC_GENERAL           0x00000212
/** @endcond */

/** Mechanism for message digesting that follows the Secure Hash Algorithm with
  a 160-bit message digest. */
#define CKM_SHA_1                      0x00000220

/** Mechanism for signatures and verification that uses the HMAC construction
  based on the SHA-1 hash function.
*/
#define CKM_SHA_1_HMAC                 0x00000221
/** Mechanism for signatures and verification that uses the HMAC construction
  based on the SHA-1 hash function. The length in bytes of the desired output
  can be passed as a parameter.
*/
#define CKM_SHA_1_HMAC_GENERAL         0x00000222

/** @cond
*/
/** Mechanism for message digesting that follows the RIPE-MD 128 message-digest
  algorithm.
*/
#define CKM_RIPEMD128                  0x00000230
/** Mechanism for signatures and verification. It uses the HMAC construction,
  based on the RIPE-MD 128 hash function.
*/
#define CKM_RIPEMD128_HMAC             0x00000231
/** Comment needed here. */
#define CKM_RIPEMD128_HMAC_GENERAL     0x00000232
/** Mechanism for message digesting that follows the RIPE-MD 160 message-digest
  algorithm.
*/
#define CKM_RIPEMD160                  0x00000240
/** Comment needed here.  */
#define CKM_RIPEMD160_HMAC             0x00000241
/** Mechanism for signatures and verification that uses the HMAC construction
  based on the RIPE-MD 160 hash function.
*/
#define CKM_RIPEMD160_HMAC_GENERAL     0x00000242
/** @endcond */

/** Mechanism for message digesting that follows the Secure Hash Algorithm with
  a 256-bit message digest. Refer to FIPS PUB 180-2 @ref S11 "[S11]".
*/
#define CKM_SHA256                     0x00000250
/** CKM_SHA256_HMAC_GENERAL with a 32-byte output length.
*/
#define CKM_SHA256_HMAC                0x00000251
/** Mechanism for signatures and verification that uses the HMAC construction
  based on the SHA-256 hash function. The length in bytes of the desired output
  can be passed as a parameter.
*/
#define CKM_SHA256_HMAC_GENERAL        0x00000252

/** @cond
*/
/** Mechanism for message digesting that follows the Secure Hash Algorithm with
  a 224-bit message digest.
*/
#define CKM_SHA224                     0x00000255
/** Comment needed here. */
#define CKM_SHA224_HMAC                0x00000256
/** General-length SHA-224-HMAC mechanism. It uses the HMAC construction based
  on the SHA-224 hash function. Output length range: 0 to 28.
*/
#define CKM_SHA224_HMAC_GENERAL        0x00000257

/** Mechanism for message digesting that follows the Secure Hash Algorithm with
  a 384-bit message digest.
*/
#define CKM_SHA384                     0x00000260
/** Comment needed here.  */
#define CKM_SHA384_HMAC                0x00000261
/** Comment needed here. */
#define CKM_SHA384_HMAC_GENERAL        0x00000262
/** Mechanism for message digesting that follows the Secure Hash Algorithm with
  a 512-bit message digest.
*/
#define CKM_SHA512                     0x00000270
/** Comment needed here.  */
#define CKM_SHA512_HMAC                0x00000271
/** General-length SHA-512-HMAC mechanism. It uses the HMAC construction based
  on the SHA-512 hash function. Output length range: 0 to 64.
*/
#define CKM_SHA512_HMAC_GENERAL        0x00000272

/** RSA SecurID key generation mechanism. */
#define CKM_SECURID_KEY_GEN            0x00000280
/** Mechanism that retrieves and verifies RSA SecurID OTP values. */
#define CKM_SECURID                    0x00000282
/** @endcond */

/** Key generation for the HOTP  algorithm. */
#define CKM_HOTP_KEY_GEN    0x00000290
/** Mechanism that retrieves and verifies HOTP OTP values based on the current
  internal counter or a provided counter.
*/
#define CKM_HOTP            0x00000291

/** @cond
*/
/** Mechanism that retrieves and verifies ACTI OTP values. */
#define CKM_ACTI            0x000002A0
/** Key generation mechanism for the ACTI algorithm. */
#define CKM_ACTI_KEY_GEN    0x000002A1
/** @endcond */

/* All of the following mechanisms are new for v2.0 */
/* Note that CAST128 and CAST5 are the same algorithm */
/** @cond
*/
/** Comment needed here. */
#define CKM_CAST_KEY_GEN               0x00000300
/** Comment needed here. */
#define CKM_CAST_ECB                   0x00000301
/** Comment needed here. */
#define CKM_CAST_CBC                   0x00000302
/** Comment needed here. */
#define CKM_CAST_MAC                   0x00000303
/** Comment needed here. */
#define CKM_CAST_MAC_GENERAL           0x00000304
/** Comment needed here. */
#define CKM_CAST_CBC_PAD               0x00000305
/** Comment needed here. */
#define CKM_CAST3_KEY_GEN              0x00000310
/** Comment needed here. */
#define CKM_CAST3_ECB                  0x00000311
/** Comment needed here. */
#define CKM_CAST3_CBC                  0x00000312
/** Comment needed here. */
#define CKM_CAST3_MAC                  0x00000313
/** Comment needed here. */
#define CKM_CAST3_MAC_GENERAL          0x00000314
/** Comment needed here. */
#define CKM_CAST3_CBC_PAD              0x00000315
/** Comment needed here. */
#define CKM_CAST5_KEY_GEN              0x00000320
/** Comment needed here. \n
  @note1hang CAST128 and CAST5 are the same algorithm.
*/
#define CKM_CAST128_KEY_GEN            0x00000320
/** Comment needed here. \n
  @note1hang CAST128 and CAST5 are the same algorithm.
*/
#define CKM_CAST5_ECB                  0x00000321
/** Comment needed here. \n
  @note1hang CAST128 and CAST5 are the same algorithm.
*/
#define CKM_CAST128_ECB                0x00000321
/** Comment needed here. \n
  @note1hang CAST128 and CAST5 are the same algorithm.
*/
#define CKM_CAST5_CBC                  0x00000322
/** Comment needed here. \n
  @note1hang CAST128 and CAST5 are the same algorithm.
*/
#define CKM_CAST128_CBC                0x00000322
/** Comment needed here. \n
  @note1hang CAST128 and CAST5 are the same algorithm.
*/
#define CKM_CAST5_MAC                  0x00000323
/** Comment needed here. \n
  @note1hang CAST128 and CAST5 are the same algorithm.
*/
#define CKM_CAST128_MAC                0x00000323
/** Comment needed here. \n
  @note1hang CAST128 and CAST5 are the same algorithm.
*/
#define CKM_CAST5_MAC_GENERAL          0x00000324
/** Comment needed here. \n
  @note1hang CAST128 and CAST5 are the same algorithm.
*/
#define CKM_CAST128_MAC_GENERAL        0x00000324
/** Comment needed here. \n
  @note1hang CAST128 and CAST5 are the same algorithm.
*/
#define CKM_CAST5_CBC_PAD              0x00000325
/** Comment needed here. \n
  @note1hang CAST128 and CAST5 are the same algorithm.
*/
#define CKM_CAST128_CBC_PAD            0x00000325
/** RC5 key generation mechanism. */
#define CKM_RC5_KEY_GEN                0x00000330
/** Mechanism based on RSA block cipher RC5 and Electronic Codebook mode.
  Supports: \n
  - Single- and multiple-part encryption and decryption
  - Key wrapping
  - Key unwrapping
*/
#define CKM_RC5_ECB                    0x00000331
/** Mechanism based on RSA block cipher RC5 and Cipher-Block Chaining mode.
  Supports: \n
  - Single- and multiple-part encryption and decryption
  - Key wrapping
  - Key unwrapping
*/
#define CKM_RC5_CBC                    0x00000332
/** Comment needed here. */
#define CKM_RC5_MAC                    0x00000333
/** Mechanism based on RSA block cipher RC5 and data authentication that
  supports single- and multiple-part signatures and verification.
*/
#define CKM_RC5_MAC_GENERAL            0x00000334
/** Mechanism based on RSA block cipher RC5, Cipher-Block Chaining mode, and
  the block cipher padding method. Supports: \n
  - Single- and multiple-part encryption and decryption
  - Key wrapping
  - Key unwrapping
*/
#define CKM_RC5_CBC_PAD                0x00000335
/** Comment needed here. */
#define CKM_IDEA_KEY_GEN               0x00000340
/** Comment needed here.  */
#define CKM_IDEA_ECB                   0x00000341
/** Comment needed here. */
#define CKM_IDEA_CBC                   0x00000342
/** Comment needed here. */
#define CKM_IDEA_MAC                   0x00000343
/** Comment needed here. */
#define CKM_IDEA_MAC_GENERAL           0x00000344
/** Comment needed here. */
#define CKM_IDEA_CBC_PAD               0x00000345
/** @endcond */


/** Generic secret key generation mechanism. */
#define CKM_GENERIC_SECRET_KEY_GEN     0x00000350
/** Mechanism that derives a secret key from the concatenation of two existing
  secret keys.
*/
#define CKM_CONCATENATE_BASE_AND_KEY   0x00000360
/** Mechanism that derives a secret key by concatenating data onto the end of a
  specified secret key.
*/
#define CKM_CONCATENATE_BASE_AND_DATA  0x00000362
/** Mechanism that derives a secret key by prepending data to the start of a
  specified secret key.
*/
#define CKM_CONCATENATE_DATA_AND_BASE  0x00000363
/** Mechanism that enables the derivation of a secret key by performing a bit
  XORing of a key pointed to by a base key handle and some data.
*/
#define CKM_XOR_BASE_AND_DATA          0x00000364
/** Mechanism that enables the creation of one secret key from the bits of
  another secret key.
*/
#define CKM_EXTRACT_KEY_FROM_KEY       0x00000365

/** @cond
*/
/** Mechanism that generates a 48-byte generic secret key used to produce the
  pre_master key for RSA-like cipher suites in SSL version 3.0.
*/
#define CKM_SSL3_PRE_MASTER_KEY_GEN    0x00000370
/** Mechanism that derives one 48-byte generic secret key from another 48-byte
  generic secret key; produces the master_secret key used in the SSL protocol
  from the pre_master key.
*/
#define CKM_SSL3_MASTER_KEY_DERIVE     0x00000371
/** Mechanism that derives the appropriate cryptographic keying material used
  by a cipher suite from the master_secret key and random data.
*/
#define CKM_SSL3_KEY_AND_MAC_DERIVE    0x00000372

/** Master key derivation for Diffie-Hellman in SSL 3.0. A mechanism that
  derives one 48-byte generic secret key from another arbitrary length generic
  secret key; produces the master_secret key used in the SSL protocol from the
  pre_master key.
*/
#define CKM_SSL3_MASTER_KEY_DERIVE_DH  0x00000373
/** Mechanism that generates a 48-byte generic secret key; produces the
  pre_master key used in TLS version 1.0 for RSA-like cipher suites.
*/
#define CKM_TLS_PRE_MASTER_KEY_GEN     0x00000374
/** Mechanism that derives one 48-byte generic secret key from another 48-byte
  generic secret key; produces the master_secret key used in the TLS protocol
  from the pre_master key.
*/
#define CKM_TLS_MASTER_KEY_DERIVE      0x00000375
/** Mechanism that derives the cryptographic keying material used by a cipher
  suite from the master_secret key and random data.
*/
#define CKM_TLS_KEY_AND_MAC_DERIVE     0x00000376
/** Mechanism that derives one 48-byte generic secret key from another
  arbitrary length generic secret key; produces the master_secret key used in
  the TLS protocol from the pre_master key.
*/
#define CKM_TLS_MASTER_KEY_DERIVE_DH   0x00000377

/** Mechanism that produces securely generated pseudo-random output of
  arbitrary length.
*/
#define CKM_TLS_PRF                    0x00000378

/** Mechanism based on the SSL 3.0 protocol that supports single- and
  multiple-part signatures (data authentication) and verification using the MD5
  algorithm. */
#define CKM_SSL3_MD5_MAC               0x00000380
/** Mechanism based on the SSL 3.0 protocol that supports single- and
  multiple-part signatures (data authentication) and verification using the
  SHA-1 algorithm. */
#define CKM_SSL3_SHA1_MAC              0x00000381
/** Mechanism that enables the derivation of a secret key by digesting the
  value of another secret key with the MD5 algorithm. */
#define CKM_MD5_KEY_DERIVATION         0x00000390
/** Mechanism that enables the derivation of a secret key by digesting the
  value of another secret key with the MD2 algorithm.  */
#define CKM_MD2_KEY_DERIVATION         0x00000391
/** @endcond */

/** Mechanism that enables the derivation of a secret key by digesting the
  value of another secret key with the SHA-1 algorithm. */
#define CKM_SHA1_KEY_DERIVATION        0x00000392

/** Mechanism that enables the derivation of a secret key by digesting the
  value of another secret key with the SHA-256 hash function and a relevant
  length of 32 bytes.
*/
#define CKM_SHA256_KEY_DERIVATION      0x00000393

/** @cond
*/
/** Mechanism that enables the derivation of a secret key by digesting the
  value of another secret key with the SHA-384 hash function and the relevant
  length is 48 bytes.
*/
#define CKM_SHA384_KEY_DERIVATION      0x00000394
/** Mechanism that enables the derivation of a secret key by digesting the
  value of another secret key with the SHA-512 hash function and the relevant
  length is 64 bytes.
*/
#define CKM_SHA512_KEY_DERIVATION      0x00000395

/** Mechanism that enables the derivation of a secret key by digesting the
  value of another secret key with the SHA-224 hash function and the relevant
  length is 28 bytes.
*/
#define CKM_SHA224_KEY_DERIVATION      0x00000396

/** Comment needed here. */
#define CKM_PBE_MD2_DES_CBC            0x000003A0
/** Comment needed here. */
#define CKM_PBE_MD5_DES_CBC            0x000003A1
/** Comment needed here. */
#define CKM_PBE_MD5_CAST_CBC           0x000003A2
/** Comment needed here. */
#define CKM_PBE_MD5_CAST3_CBC          0x000003A3
/** Comment needed here. */
#define CKM_PBE_MD5_CAST5_CBC          0x000003A4
/** Comment needed here. */
#define CKM_PBE_MD5_CAST128_CBC        0x000003A4
/** Comment needed here. */
#define CKM_PBE_SHA1_CAST5_CBC         0x000003A5
/** Comment needed here. */
#define CKM_PBE_SHA1_CAST128_CBC       0x000003A5
/** Comment needed here. */
#define CKM_PBE_SHA1_RC4_128           0x000003A6
/** Comment needed here. */
#define CKM_PBE_SHA1_RC4_40            0x000003A7
/** Comment needed here. */
#define CKM_PBE_SHA1_DES3_EDE_CBC      0x000003A8
/** Comment needed here. */
#define CKM_PBE_SHA1_DES2_EDE_CBC      0x000003A9
/** Comment needed here. */
#define CKM_PBE_SHA1_RC2_128_CBC       0x000003AA
/** Comment needed here. */
#define CKM_PBE_SHA1_RC2_40_CBC        0x000003AB
/** @endcond */

/** Mechanism for generating a secret key from a password and a salt value.
  This functionality is defined in PKCS #5 as PBKDF2.
*/
#define CKM_PKCS5_PBKD2                0x000003B0

/** @cond
*/
/** Comment needed here. */
#define CKM_PBA_SHA1_WITH_SHA1_HMAC    0x000003C0

/** Comment needed here. */
#define CKM_WTLS_PRE_MASTER_KEY_GEN         0x000003D0
/** Comment needed here. */
#define CKM_WTLS_MASTER_KEY_DERIVE          0x000003D1
/** Comment needed here. */
#define CKM_WTLS_MASTER_KEY_DERIVE_DH_ECC   0x000003D2
/** Comment needed here. */
#define CKM_WTLS_PRF                        0x000003D3
/** Comment needed here. */
#define CKM_WTLS_SERVER_KEY_AND_MAC_DERIVE  0x000003D4
/** Comment needed here. */
#define CKM_WTLS_CLIENT_KEY_AND_MAC_DERIVE  0x000003D5

/** Comment needed here. */
#define CKM_KEY_WRAP_LYNKS             0x00000400
/** Comment needed here. */
#define CKM_KEY_WRAP_SET_OAEP          0x00000401

/** Comment needed here. */
#define CKM_CMS_SIG                    0x00000500

/** Comment needed here. */
#define CKM_KIP_DERIVE	               0x00000510
/** Comment needed here. */
#define CKM_KIP_WRAP	               0x00000511
/** Comment needed here. */
#define CKM_KIP_MAC	               0x00000512

/* Camellia is new for PKCS #11 v2.20 amendment 3 */
/** Comment needed here. */
#define CKM_CAMELLIA_KEY_GEN           0x00000550
/** Comment needed here. */
#define CKM_CAMELLIA_ECB               0x00000551
/** Comment needed here. */
#define CKM_CAMELLIA_CBC               0x00000552
/** Comment needed here. */
#define CKM_CAMELLIA_MAC               0x00000553
/** Comment needed here. */
#define CKM_CAMELLIA_MAC_GENERAL       0x00000554
/** Comment needed here. */
#define CKM_CAMELLIA_CBC_PAD           0x00000555
/** Comment needed here. */
#define CKM_CAMELLIA_ECB_ENCRYPT_DATA  0x00000556
/** Comment needed here. */
#define CKM_CAMELLIA_CBC_ENCRYPT_DATA  0x00000557
/** Comment needed here. */
#define CKM_CAMELLIA_CTR               0x00000558

/* ARIA is new for PKCS #11 v2.20 amendment 3 */
/** Comment needed here. */
#define CKM_ARIA_KEY_GEN               0x00000560
/** Comment needed here. */
#define CKM_ARIA_ECB                   0x00000561
/** Comment needed here. */
#define CKM_ARIA_CBC                   0x00000562
/** Comment needed here. */
#define CKM_ARIA_MAC                   0x00000563
/** Comment needed here. */
#define CKM_ARIA_MAC_GENERAL           0x00000564
/** Comment needed here.
*/
#define CKM_ARIA_CBC_PAD               0x00000565
/** Comment needed here.
*/
#define CKM_ARIA_ECB_ENCRYPT_DATA      0x00000566
/** Comment needed here. */
#define CKM_ARIA_CBC_ENCRYPT_DATA      0x00000567

/* Fortezza mechanisms */
/** Comment needed here.*/
#define CKM_SKIPJACK_KEY_GEN           0x00001000
/** Comment needed here.*/
#define CKM_SKIPJACK_ECB64             0x00001001
/** Comment needed here.*/
#define CKM_SKIPJACK_CBC64             0x00001002
/** Comment needed here.*/
#define CKM_SKIPJACK_OFB64             0x00001003
/** Comment needed here.*/
#define CKM_SKIPJACK_CFB64             0x00001004
/** Comment needed here.*/
#define CKM_SKIPJACK_CFB32             0x00001005
/** Comment needed here.*/
#define CKM_SKIPJACK_CFB16             0x00001006
/** Comment needed here.*/
#define CKM_SKIPJACK_CFB8              0x00001007
/** Comment needed here.*/
#define CKM_SKIPJACK_WRAP              0x00001008
/** Comment needed here.*/
#define CKM_SKIPJACK_PRIVATE_WRAP      0x00001009
/** Comment needed here.*/
#define CKM_SKIPJACK_RELAYX            0x0000100a
/** Comment needed here.*/
#define CKM_KEA_KEY_PAIR_GEN           0x00001010
/** Comment needed here.*/
#define CKM_KEA_KEY_DERIVE             0x00001011
/** Comment needed here.*/
#define CKM_FORTEZZA_TIMESTAMP         0x00001020
/** Comment needed here.*/
#define CKM_BATON_KEY_GEN              0x00001030
/** Comment needed here.*/
#define CKM_BATON_ECB128               0x00001031
/** Comment needed here.*/
#define CKM_BATON_ECB96                0x00001032
/** Comment needed here.*/
#define CKM_BATON_CBC128               0x00001033
/** Comment needed here.*/
#define CKM_BATON_COUNTER              0x00001034
/** Comment needed here.*/
#define CKM_BATON_SHUFFLE              0x00001035
/** Comment needed here.*/
#define CKM_BATON_WRAP                 0x00001036

/* CKM_ECDSA_KEY_PAIR_GEN is deprecated in v2.11,
 * CKM_EC_KEY_PAIR_GEN is preferred */
/** @deprecated This define is deprecated in v2.11.
                See #CKM_EC_KEY_PAIR_GEN.

  Comment needed here.
*/
#define CKM_ECDSA_KEY_PAIR_GEN         0x00001040
/** Comment needed here. */
#define CKM_EC_KEY_PAIR_GEN            0x00001040

/** Comment needed here. */
#define CKM_ECDSA                      0x00001041
/** Comment needed here. */
#define CKM_ECDSA_SHA1                 0x00001042

/* CKM_ECDH1_DERIVE, CKM_ECDH1_COFACTOR_DERIVE, and CKM_ECMQV_DERIVE
 * are new for v2.11 */
/** Comment needed here. */
#define CKM_ECDH1_DERIVE               0x00001050
/** Comment needed here. */
#define CKM_ECDH1_COFACTOR_DERIVE      0x00001051
/** Comment needed here. */
#define CKM_ECMQV_DERIVE               0x00001052

/** Comment needed here. */
#define CKM_JUNIPER_KEY_GEN            0x00001060
/** Comment needed here. */
#define CKM_JUNIPER_ECB128             0x00001061
/** Comment needed here. */
#define CKM_JUNIPER_CBC128             0x00001062
/** Comment needed here. */
#define CKM_JUNIPER_COUNTER            0x00001063
/** Comment needed here. */
#define CKM_JUNIPER_SHUFFLE            0x00001064
/** Comment needed here. */
#define CKM_JUNIPER_WRAP               0x00001065
/** Comment needed here. */
#define CKM_FASTHASH                   0x00001070
/** @endcond */


/* CKM_AES_KEY_GEN, CKM_AES_ECB, CKM_AES_CBC, CKM_AES_MAC,
 * CKM_AES_MAC_GENERAL, CKM_AES_CBC_PAD, CKM_DSA_PARAMETER_GEN,
 * CKM_DH_PKCS_PARAMETER_GEN, and CKM_X9_42_DH_PARAMETER_GEN are
 * new for v2.11 */
/** Key generation mechanism for the NIST Advanced Encryption Standard. */
#define CKM_AES_KEY_GEN                0x00001080
/** Electronic CodeBook encryption with an AES key. */
#define CKM_AES_ECB                    0x00001081
/** Cypher-Block Chaining encryption with an AES key. */
#define CKM_AES_CBC                    0x00001082
/** CKM_AES_MAC_GENERAL with signatures that are half the block
  size in length.
*/
#define CKM_AES_MAC                    0x00001083
/** Mechanism for signature generation and verification based on NIST AES as
  defined in FIPS PUB 197 @ref S12 "[S12]" and data authentication as defined
  in FIPS PUB 113 @ref S13 "[S13]".
*/
#define CKM_AES_MAC_GENERAL            0x00001084
/** Cypher-Block Chaining encryption with PKCS #7 @ref S14 "[S14]" padding and
  an AES key.  */
#define CKM_AES_CBC_PAD                0x00001085

/** @cond
*/
/** Comment needed here. */
#define CKM_AES_CTR                    0x00001086

/** Comment needed here. */
#define CKM_BLOWFISH_KEY_GEN           0x00001090
/** Comment needed here. */
#define CKM_BLOWFISH_CBC               0x00001091
/** Comment needed here. */
#define CKM_TWOFISH_KEY_GEN            0x00001092
/** Comment needed here. */
#define CKM_TWOFISH_CBC                0x00001093
/** @endcond */

/** Key derivation mechanism in which the new key is derived by performing DES
  ECB encryption with the base key on the passed data.
*/
#define CKM_DES_ECB_ENCRYPT_DATA       0x00001100
/** Key derivation mechanism in which the new key is derived by performing DES
  CBC encryption with the base key on the passed data.
*/
#define CKM_DES_CBC_ENCRYPT_DATA       0x00001101
/** Key derivation mechanism in which the new key is derived by performing DES3
  ECB encryption with the base key on the passed data.
*/
#define CKM_DES3_ECB_ENCRYPT_DATA      0x00001102
/** Key derivation mechanism in which the new key is derived by performing DES3
  CBC encryption with the base key on the passed data.
*/
#define CKM_DES3_CBC_ENCRYPT_DATA      0x00001103
/** Key derivation mechanism in which the new key is derived by performing AES
  ECB encryption with the base key on the passed data.
*/
#define CKM_AES_ECB_ENCRYPT_DATA       0x00001104
/** Key derivation mechanism in which the new key is derived by performing AES
  CBC encryption with the base key on the passed data.
*/
#define CKM_AES_CBC_ENCRYPT_DATA       0x00001105

/** @cond
*/
/** Comment needed here. */
#define CKM_DSA_PARAMETER_GEN          0x00002000
/** Comment needed here. */
#define CKM_DH_PKCS_PARAMETER_GEN      0x00002001
/** Comment needed here. */
#define CKM_X9_42_DH_PARAMETER_GEN     0x00002002
/** @endcond */

/** Base define for vendor-specific mechanisms. */
#define CKM_VENDOR_DEFINED             0x80000000
/** Password-based key generation using a generic secret key generated from the
  secure key pad. This define is a QTI PKCS #11 extension and uses the PKCS #5
  PBKD2 mechanism as its PRF function.  */
#define CKM_PKCS5_PBKD2_GENERIC_SECRET_KEY      (CKM_VENDOR_DEFINED+0x0000000C)

/** Unwraps from secure objects. This define is a QTI PKCS #11 extension.

  The wrapping and unwrapping of secret and private keys with ancillary data
  binds a key being wrapped (or unwrapped) with a subset of its attributes.
  Both the key and the attributes are signed to guarantee their integrity. The
  token can be configured so that this is the only allowed key wrapping and
  unwrapping mechanism (besides #CKM_SO_KEY_WRAP). This mechanism can be
  invoked with the wrapping/unwrapping key being an AES 256 secret key.

  This mechanism takes a parameter in the form of CK_WRAPKEY_AES_CBC_PAD_PARAMS.
*/
#define CKM_WRAPKEY_AES_CBC_PAD       (CKM_VENDOR_DEFINED+0x0000000F)

/** Mechanism for generating a secret key from a password and a salt value.
  This functionality is defined in PKCS #5 as PBKDF1.
*/
#define CKM_PKCS5_PBKD1                       (CKM_VENDOR_DEFINED+0x00000010)

/** Mechanism for generating a secret key from a password and a salt value.
  The password is entered using the Secure UI.
  This define is a QTI PKCS #11 extension.
  This functionality is defined in PKCS #5 as PBKDF2.

  This mechanism takes a parameter in the form of CK_PKCS5_PBKD2_KEYPAD_PARAMS.
*/
#define CKM_PKCS5_PBKD2_KEYPAD                (CKM_VENDOR_DEFINED+0x00000011)

/** Mechanism for generating a secret key from a password and a salt value.
  The password is entered using the Secure UI.
  This define is a QTI PKCS #11 extension.
  This functionality is defined in PKCS #5 as PBKDF1.

  This mechanism takes a parameter in the form of CK_PKCS5_PBKD1_KEYPAD_PARAMS.
*/
#define CKM_PKCS5_PBKD1_KEYPAD                (CKM_VENDOR_DEFINED+0x00000012)

/** Mechanism for generating a secret key whose content is a password entered
  via the Secure UI. This key can only be used as parameter for CKM_PKCS5_PBKD1_VPN.
  This define is a QTI PKCS #11 extension.

  This mechanism takes a parameter in the form of CK_GENERIC_SECRET_KEY_KEYPAD_PARAMS.
*/
#define CKM_GENERIC_SECRET_KEY_GEN_VPN_KEYPAD (CKM_VENDOR_DEFINED+0x00000013)

/** Mechanism for generating a secret key from a password and a salt value.
  The password is provided in a key generated with CKM_GENERIC_SECRET_KEY_GEN_VPN_KEYPAD.
  This define is a QTI PKCS #11 extension.
  This functionality is defined in PKCS #5 as PBKDF1.

  This mechanism takes a parameter in the form of CK_PKCS5_PBKD1_KEYPAD_PARAMS.
*/
#define CKM_PKCS5_PBKD1_VPN                   (CKM_VENDOR_DEFINED+0x00000014)

/** Mechanism for deriving the ClientProof and ServerSignature associated to
  a SCRAM authentication session.
  The password is entered via the Secure UI.
  This define is a QTI PKCS #11 extension.

  This mechanism can be used in a signature generation operation, taking its parameter
  in the form of CK_SCRAM_KEYPAD_PARAMS.
  Content signed by the operation has to be in the form of CK_SCRAM_KEYPAD_INPUT_PARAMS,
  and the result of the operation will be in the form of CK_SCRAM_KEYPAD_OUTPUT_INFO.
*/
#define CKM_SCRAM_KEYPAD                      (CKM_VENDOR_DEFINED+0x00000015)
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** Pointer to #CK_MECHANISM_TYPE. */
typedef CK_MECHANISM_TYPE CK_PTR CK_MECHANISM_TYPE_PTR;


/** Specifies a particular mechanism. \n
*/
typedef struct CK_MECHANISM {
  CK_MECHANISM_TYPE mechanism;  /**< Mechanism. */
  CK_VOID_PTR       pParameter; /**< Pointer to the parameter.*/

  CK_ULONG          ulParameterLen;  /**< Length of the parameter in bytes. */
} CK_MECHANISM;

/** Pointer to the mechanism specified in CK_MECHANISM. */
typedef CK_MECHANISM CK_PTR CK_MECHANISM_PTR;


/** Provides information about a particular mechanism. */
typedef struct CK_MECHANISM_INFO {
    CK_ULONG    ulMinKeySize;  /**< Minimum key size. */
    CK_ULONG    ulMaxKeySize;  /**< Maximum key size. */
    CK_FLAGS    flags;         /**< Bit flags. */
} CK_MECHANISM_INFO;
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** Performed by the hardware. */
#define CKF_HW                 0x00000001

/** Mechnaism can be used for encryption. */
#define CKF_ENCRYPT            0x00000100
/** Mechanism can be used for decryption. */
#define CKF_DECRYPT            0x00000200
/** Mechanism can be used for digesting. */
#define CKF_DIGEST             0x00000400
/** Mechanism can be used for signing. */
#define CKF_SIGN               0x00000800
/** Mechanism can be used for recovery. */
#define CKF_SIGN_RECOVER       0x00001000
/** Mechanism can be used for verification. */
#define CKF_VERIFY             0x00002000
/** Mechanism can be used to verify recovery. */
#define CKF_VERIFY_RECOVER     0x00004000
/** Mechanism can be used for key generation. */
#define CKF_GENERATE           0x00008000
/** Mechanism can be used for key pair generation. */
#define CKF_GENERATE_KEY_PAIR  0x00010000
/** Mechanism can be used for key wrapping. */
#define CKF_WRAP               0x00020000
/** Mechanism can be used for key unwrapping. */
#define CKF_UNWRAP             0x00040000
/** Mechanism can be used for key derivation. */
#define CKF_DERIVE             0x00080000

/** @cond
*/
/* CKF_EC_F_P, CKF_EC_F_2M, CKF_EC_ECPARAMETERS, CKF_EC_NAMEDCURVE,
 * CKF_EC_UNCOMPRESS, and CKF_EC_COMPRESS are new for v2.11. They
 * describe a token's EC capabilities not available in mechanism
 * information. */
/** Comment needed here. */
#define CKF_EC_F_P             0x00100000
/** Comment needed here. */
#define CKF_EC_F_2M            0x00200000
/** Comment needed here. */
#define CKF_EC_ECPARAMETERS    0x00400000
/** Comment needed here. */
#define CKF_EC_NAMEDCURVE      0x00800000
/** Comment needed here. */
#define CKF_EC_UNCOMPRESS      0x01000000
/** Comment needed here.*/
#define CKF_EC_COMPRESS        0x02000000

/** Comment needed here. Value: FALSE for this version. */
#define CKF_EXTENSION          0x80000000
/** @endcond */
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** Pointer to the mechanism information specified in CK_MECHANISM_INFO. */
typedef CK_MECHANISM_INFO CK_PTR CK_MECHANISM_INFO_PTR;

/** Identifies the return value of a Cryptoki function. \n
*/
typedef CK_ULONG          CK_RV;
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** Command completed successfully. */
#define CKR_OK                                0x00000000
/** Command cancelled. */
#define CKR_CANCEL                            0x00000001
/** Insufficient memory to run the command. */
#define CKR_HOST_MEMORY                       0x00000002
/** Slot ID is invalid.*/
#define CKR_SLOT_ID_INVALID                   0x00000003

/* CKR_FLAGS_INVALID was removed for v2.0 */

/** General failure. */
#define CKR_GENERAL_ERROR                     0x00000005
/** Function failed. */
#define CKR_FUNCTION_FAILED                   0x00000006

/** One of the supplied arguments was inappropriate. */
#define CKR_ARGUMENTS_BAD                     0x00000007
/** @cond
*/

/** Comment needed here. */
#define CKR_NO_EVENT                          0x00000008
/** @endcond */

/** Failed to spawn new threads. */
#define CKR_NEED_TO_CREATE_THREADS            0x00000009
/** Requested locking type is not available. */
#define CKR_CANT_LOCK                         0x0000000A

/** Attribute is read-only. */
#define CKR_ATTRIBUTE_READ_ONLY               0x00000010
/** Attribute is sensitive. */
#define CKR_ATTRIBUTE_SENSITIVE               0x00000011
/** Attribute type is invalid. */
#define CKR_ATTRIBUTE_TYPE_INVALID            0x00000012
/** Attribute value is invalid. */
#define CKR_ATTRIBUTE_VALUE_INVALID           0x00000013
/** Data is invalid. */
#define CKR_DATA_INVALID                      0x00000020
/** Plaintext input has an unsupported length. */
#define CKR_DATA_LEN_RANGE                    0x00000021
/** Device error occurred. */
#define CKR_DEVICE_ERROR                      0x00000030
/** Insufficient memory to perform the requested operation. */
#define CKR_DEVICE_MEMORY                     0x00000031
/** Device was removed. */
#define CKR_DEVICE_REMOVED                    0x00000032
/** Encrypted data is invalid. */
#define CKR_ENCRYPTED_DATA_INVALID            0x00000040
/** Ciphertext input has an unsupported length. */
#define CKR_ENCRYPTED_DATA_LEN_RANGE          0x00000041
/** Function was cancelled. */
#define CKR_FUNCTION_CANCELED                 0x00000050
/** Function is not parallel. */
#define CKR_FUNCTION_NOT_PARALLEL             0x00000051

/** Function is not supported. */
#define CKR_FUNCTION_NOT_SUPPORTED            0x00000054

/** Key handle is invalid. */
#define CKR_KEY_HANDLE_INVALID                0x00000060

/* CKR_KEY_SENSITIVE was removed for v2.0 */

/** Unsupported key size. */
#define CKR_KEY_SIZE_RANGE                    0x00000062
/** Key type is inconsistent. */
#define CKR_KEY_TYPE_INCONSISTENT             0x00000063

/** Key is not needed. */
#define CKR_KEY_NOT_NEEDED                    0x00000064
/** Key has changed. */
#define CKR_KEY_CHANGED                       0x00000065
/** Key is needed. */
#define CKR_KEY_NEEDED                        0x00000066
/** Key cannot be digested. */
#define CKR_KEY_INDIGESTIBLE                  0x00000067
/** Key function is not permitted. */
#define CKR_KEY_FUNCTION_NOT_PERMITTED        0x00000068
/** Key cannot be wrapped. */
#define CKR_KEY_NOT_WRAPPABLE                 0x00000069
/** Key is unextractable. */
#define CKR_KEY_UNEXTRACTABLE                 0x0000006A

/** Mechanism is invalid. */
#define CKR_MECHANISM_INVALID                 0x00000070
/** Mechanism parameter is invalid. */
#define CKR_MECHANISM_PARAM_INVALID           0x00000071

/* CKR_OBJECT_CLASS_INCONSISTENT and CKR_OBJECT_CLASS_INVALID
 * were removed for v2.0 */
/** Object handle is invalid. */
#define CKR_OBJECT_HANDLE_INVALID             0x00000082
/** Operation is active. */
#define CKR_OPERATION_ACTIVE                  0x00000090
/** Operation is not initialized. */
#define CKR_OPERATION_NOT_INITIALIZED         0x00000091
/** PIN is incorrect. */
#define CKR_PIN_INCORRECT                     0x000000A0
/** PIN is invalid. */
#define CKR_PIN_INVALID                       0x000000A1
/** PIN too short or too long. */
#define CKR_PIN_LEN_RANGE                     0x000000A2

/** PIN has expired. */
#define CKR_PIN_EXPIRED                       0x000000A3
/** PIN is locked. */
#define CKR_PIN_LOCKED                        0x000000A4

/** Session is closed. */
#define CKR_SESSION_CLOSED                    0x000000B0
/** Too many read-only or read/write opened sessions. */
#define CKR_SESSION_COUNT                     0x000000B1
/** Session handle is invalid. */
#define CKR_SESSION_HANDLE_INVALID            0x000000B3
/** Parallel sessions are not supported. */
#define CKR_SESSION_PARALLEL_NOT_SUPPORTED    0x000000B4
/** Session is read-only. */
#define CKR_SESSION_READ_ONLY                 0x000000B5
/** Session exists. */
#define CKR_SESSION_EXISTS                    0x000000B6

/** Read-only session exists. */
#define CKR_SESSION_READ_ONLY_EXISTS          0x000000B7
/** Security officer session with read/write permission exists. */
#define CKR_SESSION_READ_WRITE_SO_EXISTS      0x000000B8

/** Signature is invalid. */
#define CKR_SIGNATURE_INVALID                 0x000000C0
/** Signature is invalid because of its length. */
#define CKR_SIGNATURE_LEN_RANGE               0x000000C1
/** Template is incomplete. */
#define CKR_TEMPLATE_INCOMPLETE               0x000000D0
/** Template is inconsistent. */
#define CKR_TEMPLATE_INCONSISTENT             0x000000D1
/** Token is not present. */
#define CKR_TOKEN_NOT_PRESENT                 0x000000E0
/** Token is not recognized. */
#define CKR_TOKEN_NOT_RECOGNIZED              0x000000E1
/** Token is write-protected. */
#define CKR_TOKEN_WRITE_PROTECTED             0x000000E2
/** Unwrapping key handle is invalid. */
#define CKR_UNWRAPPING_KEY_HANDLE_INVALID     0x000000F0
/** Unwrapping key length is outside of supported range. */
#define CKR_UNWRAPPING_KEY_SIZE_RANGE         0x000000F1
/** Unwrapping key type is inconsistent. */
#define CKR_UNWRAPPING_KEY_TYPE_INCONSISTENT  0x000000F2
/** User is already logged in. */
#define CKR_USER_ALREADY_LOGGED_IN            0x00000100
/** User is not logged in. */
#define CKR_USER_NOT_LOGGED_IN                0x00000101
/** User PIN has not been initialized. */
#define CKR_USER_PIN_NOT_INITIALIZED          0x00000102
/** User type is invalid. */
#define CKR_USER_TYPE_INVALID                 0x00000103

/** Another user is already logged in. */
#define CKR_USER_ANOTHER_ALREADY_LOGGED_IN    0x00000104
/** Too many user types. */
#define CKR_USER_TOO_MANY_TYPES               0x00000105

/** Wrapped key is invalid. */
#define CKR_WRAPPED_KEY_INVALID               0x00000110
/** Wrapped key is invalid solely on the basis of its length. */
#define CKR_WRAPPED_KEY_LEN_RANGE             0x00000112
/** Wrapping key handle is invalid. */
#define CKR_WRAPPING_KEY_HANDLE_INVALID       0x00000113
/** Wrapping key size outside of supported range. */
#define CKR_WRAPPING_KEY_SIZE_RANGE           0x00000114
/** Wrapping key type is inconsistent. */
#define CKR_WRAPPING_KEY_TYPE_INCONSISTENT    0x00000115
/** Random seed is not supported. */
#define CKR_RANDOM_SEED_NOT_SUPPORTED         0x00000120

/** No random number generator. */
#define CKR_RANDOM_NO_RNG                     0x00000121

/** Domain parameters are invalid. */
#define CKR_DOMAIN_PARAMS_INVALID             0x00000130

/** Buffer is too small. */
#define CKR_BUFFER_TOO_SMALL                  0x00000150
/** Saved state is invalid. */
#define CKR_SAVED_STATE_INVALID               0x00000160
/** Requested information is considered sensitive. */
#define CKR_INFORMATION_SENSITIVE             0x00000170
/** State cannot be saved. */
#define CKR_STATE_UNSAVEABLE                  0x00000180

/** Cryptoki is not initialized. */
#define CKR_CRYPTOKI_NOT_INITIALIZED          0x00000190
/** Cryptoki is already initialized. */
#define CKR_CRYPTOKI_ALREADY_INITIALIZED      0x00000191
/** Mutex object is bad. */
#define CKR_MUTEX_BAD                         0x000001A0
/** Mutex object is not locked. */
#define CKR_MUTEX_NOT_LOCKED                  0x000001A1

/** Supplied OTP was rejected and the library requests a new OTP using a new
  PIN.
*/
#define CKR_NEW_PIN_MODE                      0x000001B0
/** Larger than normal drift in the token internal state. */
#define CKR_NEXT_OTP                          0x000001B1

/** Function was rejected. */
#define CKR_FUNCTION_REJECTED                 0x00000200

/** Vendor is defined. */
#define CKR_VENDOR_DEFINED                    0x80000000
/** @} */ /* end_addtogroup pkcs11_constants */


/** @ingroup m_CK_NOTIFY

  Processes events.

  @datatypes
  #CK_SESSION_HANDLE \n
  #CK_NOTIFICATION \n
  #CK_VOID_PTR

  @param[in] hSession      Handle of the session.
  @param[in] event         Event for which notification is to be sent.
  @param[in] pApplication  Pointer to the application. Passed to
                           @ref C_OpenSession "C_OpenSession()".

  @return
  CKR_OK    -- Success. \n
  Otherwise -- Application-specific error codes.

  @dependencies
  None.
*/
typedef CK_CALLBACK_FUNCTION(CK_RV, CK_NOTIFY)(
  CK_SESSION_HANDLE hSession,
  CK_NOTIFICATION   event,
  CK_VOID_PTR       pApplication
);

/** @addtogroup pkcs11_datatypes
 @{ */

/** Provides the Cryptoki specification version and pointers of appropriate
  types to all the Cryptoki functions. */
typedef struct CK_FUNCTION_LIST CK_FUNCTION_LIST;

/** Pointer to #CK_FUNCTION_LIST. */
typedef CK_FUNCTION_LIST CK_PTR CK_FUNCTION_LIST_PTR;

/** Pointer to #CK_FUNCTION_LIST_PTR. */
typedef CK_FUNCTION_LIST_PTR CK_PTR CK_FUNCTION_LIST_PTR_PTR;
/** @} */  /* end_addtogroup pkcs11_datatypes */

/** @ingroup m_CK_CREATEMUTEX

  Creates a mutex object.

  @datatypes
  #CK_VOID_PTR_PTR

  @param[in] ppMutex   Pointer to the location to receive the pointer to a
                       mutex object.

  @return
  CKR_OK  -- Success. \n
  CKR_GENERAL_ERROR -- General failure. \n
  CKR_HOST_MEMORY -- Out of memory.

  @dependencies
  None.
*/
typedef CK_CALLBACK_FUNCTION(CK_RV, CK_CREATEMUTEX)(
  CK_VOID_PTR_PTR ppMutex
);

/** @ingroup m_CK_DESTROYMUTEX

  Destroys a mutex object.

  @datatypes
  #CK_VOID_PTR

  @param[in] pMutex Pointer to a mutex object.

  @return
  CKR_OK    -- Success. \n
  CKR_GENERAL_ERROR  -- General failure. \n
  CKR_HOST_MEMORY  -- Out of memory. \n
  CKR_MUTEX_BAD  -- Bad mutex object.

  @dependencies
  None.
*/
typedef CK_CALLBACK_FUNCTION(CK_RV, CK_DESTROYMUTEX)(
  CK_VOID_PTR pMutex
);

/** @ingroup m_CK_LOCKMUTEX

  Locks a mutex object.

  @datatypes
  #CK_VOID_PTR

  @param[in] pMutex Pointer to a mutex object.

  @return
  CKR_OK    -- Success. \n
  CKR_GENERAL_ERROR  -- General failure. \n
  CKR_HOST_MEMORY  -- Out of memory. \n
  CKR_MUTEX_BAD  -- Bad mutex object.

  @dependencies
  None.
*/
typedef CK_CALLBACK_FUNCTION(CK_RV, CK_LOCKMUTEX)(
  CK_VOID_PTR pMutex
);

/** @ingroup m_CK_UNLOCKMUTEX

  Unlocks a mutex object.

  @datatypes
  #CK_VOID_PTR

  @param[in] pMutex Pointer to a mutex object.

  @return
  CKR_OK    -- Success. \n
  CKR_MUTEX_NOT_LOCKED  -- Mutex object is not locked.

  @dependencies
  None.
*/
typedef CK_CALLBACK_FUNCTION(CK_RV, CK_UNLOCKMUTEX)(
  CK_VOID_PTR pMutex
);


/** @addtogroup pkcs11_datatypes
 @{ */

/** Provides the optional arguments to @ref C_Initialize "C_Initialize()". */
typedef struct CK_C_INITIALIZE_ARGS {
  CK_CREATEMUTEX CreateMutex;   /**< Creates a mutex object. */
  CK_DESTROYMUTEX DestroyMutex; /**< Destroys a mutex object. */
  CK_LOCKMUTEX LockMutex;       /**< Locks a mutex object. */
  CK_UNLOCKMUTEX UnlockMutex;   /**< Unlocks a mutex object. */
  CK_FLAGS flags;               /**< Provides slot capabilities. */
  CK_VOID_PTR pReserved;        /**< Reserved. */
}CK_C_INITIALIZE_ARGS;
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/* flags: bit flags that provide capabilities of the slot
 */
/** @name Slot Capabilities Bit Flags
  @{ */
/** Library cannot create the OS threads. */
#define CKF_LIBRARY_CANT_CREATE_OS_THREADS 0x00000001
/** Locking the OS is OK.*/
#define CKF_OS_LOCKING_OK                  0x00000002
/** @} */ /* end_name Slot Capabilities Bit Flags */
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */
/** Pointer to #CK_C_INITIALIZE_ARGS. */
typedef CK_C_INITIALIZE_ARGS CK_PTR CK_C_INITIALIZE_ARGS_PTR;
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/* additional flags for parameters to functions */

/** @cond
*/
/** Does not block the message. Used for the C_WaitForSlotEvent() function. */
#define CKF_DONT_BLOCK     1
/** @endcond */

/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/* CK_RSA_PKCS_MGF_TYPE  is used to indicate the Message
 * Generation Function (MGF) applied to a message block when
 * formatting a message block for the PKCS #1 OAEP encryption
 * scheme. */
typedef CK_ULONG CK_RSA_PKCS_MGF_TYPE;

typedef CK_RSA_PKCS_MGF_TYPE CK_PTR CK_RSA_PKCS_MGF_TYPE_PTR;
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/* The following MGFs are defined */
/* CKG_MGF1_SHA256, CKG_MGF1_SHA384, and CKG_MGF1_SHA512
 * are new for v2.20 */
/** Message generation function using SHA1. */
#define CKG_MGF1_SHA1         0x00000001
/** Message generation function using SHA256. */
#define CKG_MGF1_SHA256       0x00000002

/** @cond
*/
/** Message generation function using SHA384. */
#define CKG_MGF1_SHA384       0x00000003
/** Message generation function using SHA512. */
#define CKG_MGF1_SHA512       0x00000004
/* SHA-224 is new for PKCS #11 v2.20 amendment 3 */
/** Message generation function using SHA224. */
#define CKG_MGF1_SHA224       0x00000005
/** @endcond */
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */
/** @cond
*/
/** Indicates the source of the encoding parameter when formatting a message block
  for the PKCS #1 OAEP encryption scheme.
*/
typedef CK_ULONG CK_RSA_PKCS_OAEP_SOURCE_TYPE;

/* Pointer to the encoding parameter source. */
typedef CK_RSA_PKCS_OAEP_SOURCE_TYPE CK_PTR CK_RSA_PKCS_OAEP_SOURCE_TYPE_PTR;
/** @endcond */
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** Defines the encoding parameter sources. */
#define CKZ_DATA_SPECIFIED    0x00000001
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** @cond
*/
/** Provides the parameters to the #CKM_RSA_PKCS_OAEP mechanism. */
typedef struct CK_RSA_PKCS_OAEP_PARAMS {
        CK_MECHANISM_TYPE hashAlg;           /**< Hash algorithm. */
        CK_RSA_PKCS_MGF_TYPE mgf;            /**< Manufacturer. */
        CK_RSA_PKCS_OAEP_SOURCE_TYPE source; /**< Source. */
        CK_VOID_PTR pSourceData;             /**< Pointer to the source data. */
        CK_ULONG ulSourceDataLen;            /**< Length of the source data. */
} CK_RSA_PKCS_OAEP_PARAMS;

typedef CK_RSA_PKCS_OAEP_PARAMS CK_PTR CK_RSA_PKCS_OAEP_PARAMS_PTR;
/** @endcond */

/** Provides the parameters to the #CKM_RSA_PKCS_PSS mechanism(s). */
typedef struct CK_RSA_PKCS_PSS_PARAMS {
        CK_MECHANISM_TYPE    hashAlg;  /**< Hash algorithm. */
        CK_RSA_PKCS_MGF_TYPE mgf;      /**< Manufacturer. */
        CK_ULONG             sLen;     /**< Length in bytes of the salt value
		                                    used in the PSS encoding. */
} CK_RSA_PKCS_PSS_PARAMS;

typedef CK_RSA_PKCS_PSS_PARAMS CK_PTR CK_RSA_PKCS_PSS_PARAMS_PTR;

/** @cond
*/
/** Comment needed here. */
typedef CK_ULONG CK_EC_KDF_TYPE;
/** @endcond */

/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** @cond
*/
/** @name EC Key Derivation Functions
  @{ */
/** EC key derivation function is NULL. */
#define CKD_NULL                 0x00000001
/** EC key derivation function is using SHA1. */
#define CKD_SHA1_KDF             0x00000002
/** @} */ /* end_name EC Key Derivation Functions */
/** @endcond */
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */
/** @cond
*/
/** Provides the parameters to the CKM_ECDH1_DERIVE and
  CKM_ECDH1_COFACTOR_DERIVE mechanisms, where each party contributes one key
  pair.
*/
typedef struct CK_ECDH1_DERIVE_PARAMS {
  CK_EC_KDF_TYPE kdf;       /**< Key derivation function. */
  CK_ULONG ulSharedDataLen; /**< Length of the shared data. */
  CK_BYTE_PTR pSharedData;  /**< Pointer to the shared data. */
  CK_ULONG ulPublicDataLen; /**< Length of the public data. */
  CK_BYTE_PTR pPublicData;  /**< Pointer to the public data. */
} CK_ECDH1_DERIVE_PARAMS;

typedef CK_ECDH1_DERIVE_PARAMS CK_PTR CK_ECDH1_DERIVE_PARAMS_PTR;


/** Provides the parameters to the CK_ECDH2_DERIVE_PARAMS mechanism and the
  CK_ECMQV_DERIVE_PARAMS mechanism, where each party contributes two key
  pairs.
*/
typedef struct CK_ECDH2_DERIVE_PARAMS {
  CK_EC_KDF_TYPE kdf;             /**< Key derivation function. */
  CK_ULONG ulSharedDataLen;       /**< Length of the shared data. */
  CK_BYTE_PTR pSharedData;        /**< Pointer to the shared data. */
  CK_ULONG ulPublicDataLen;       /**< Length of the public data. */
  CK_BYTE_PTR pPublicData;        /**< Pointer to the public data. */
  CK_ULONG ulPrivateDataLen;      /**< Length of the private data. */
  CK_OBJECT_HANDLE hPrivateData;  /**< Handle to the private data. */
  CK_ULONG ulPublicDataLen2;      /**< Length of the second set of public data. */
  CK_BYTE_PTR pPublicData2;       /**< Pointer to the second set of public data. */
} CK_ECDH2_DERIVE_PARAMS;

typedef CK_ECDH2_DERIVE_PARAMS CK_PTR CK_ECDH2_DERIVE_PARAMS_PTR;

/** Provides the parameters to the CK_ECMQV_DERIVE_PARAMS mechanism, where each party
  contributes two key pairs.
*/
typedef struct CK_ECMQV_DERIVE_PARAMS {
  CK_EC_KDF_TYPE kdf;             /**< Key derivation function. */
  CK_ULONG ulSharedDataLen;       /**< Length of the shared data. */
  CK_BYTE_PTR pSharedData;        /**< Pointer to the shared data. */
  CK_ULONG ulPublicDataLen;       /**< Length of the public data. */
  CK_BYTE_PTR pPublicData;        /**< Pointer to the public data. */
  CK_ULONG ulPrivateDataLen;      /**< Length of the private data. */
  CK_OBJECT_HANDLE hPrivateData;  /**< Handle to the private data. */
  CK_ULONG ulPublicDataLen2;      /**< Length of the second set of public
                                       data. */
  CK_BYTE_PTR pPublicData2;       /**< Pointer to the second set of public
                                       data. */
  CK_OBJECT_HANDLE publicKey;     /**< Handle to the public key. */
} CK_ECMQV_DERIVE_PARAMS;

typedef CK_ECMQV_DERIVE_PARAMS CK_PTR CK_ECMQV_DERIVE_PARAMS_PTR;

/* Typedefs and defines for the CKM_X9_42_DH_KEY_PAIR_GEN and the
 * CKM_X9_42_DH_PARAMETER_GEN mechanisms (new for PKCS #11 v2.11) */
/** Comment needed here. CK_X9_42_DH_KDF_TYPE is defined for the
  CKM_X9_42_DH_KEY_PAIR_GEN and the CKM_X9_42_DH_PARAMETER_GEN mechanisms.
*/
typedef CK_ULONG CK_X9_42_DH_KDF_TYPE;

typedef CK_X9_42_DH_KDF_TYPE CK_PTR CK_X9_42_DH_KDF_TYPE_PTR;
/** @endcond */
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** @name X9.42 DH Key Derivation Functions
  @sa CKD_NULL
  @{ */
/** @cond
*/
/** Comment needed here. */
#define CKD_SHA1_KDF_ASN1        0x00000003
/** Comment needed here. */
#define CKD_SHA1_KDF_CONCATENATE 0x00000004
/** @endcond */
/** @} */ /* end_name X9.42 DH Key Derivation Functions */
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */
/** @cond
*/
/** Provides the parameters to the CKM_X9_42_DH_DERIVE key derivation
  mechanism, where each party contributes one key pair.
*/
typedef struct CK_X9_42_DH1_DERIVE_PARAMS {
  CK_X9_42_DH_KDF_TYPE kdf;     /**< Key derivation function. */
  CK_ULONG ulOtherInfoLen;      /**< Length of the other information. */
  CK_BYTE_PTR pOtherInfo;       /**< Pointer to the other information. */
  CK_ULONG ulPublicDataLen;     /**< Length of the public data. */
  CK_BYTE_PTR pPublicData;      /**< Pointer to the public data. */
} CK_X9_42_DH1_DERIVE_PARAMS;

typedef struct CK_X9_42_DH1_DERIVE_PARAMS CK_PTR CK_X9_42_DH1_DERIVE_PARAMS_PTR;

/** Provides the parameters to the CKM_X9_42_DH_HYBRID_DERIVE key derivation
  mechanisms, where each party contributes two key pairs.
*/
typedef struct CK_X9_42_DH2_DERIVE_PARAMS {
  CK_X9_42_DH_KDF_TYPE kdf;      /**< Key derivation function. */
  CK_ULONG ulOtherInfoLen;       /**< Length of the other information. */
  CK_BYTE_PTR pOtherInfo;        /**< Pointer to the other information. */
  CK_ULONG ulPublicDataLen;      /**< Length of the public data. */
  CK_BYTE_PTR pPublicData;       /**< Pointer to the public data. */
  CK_ULONG ulPrivateDataLen;     /**< Length of the private data. */
  CK_OBJECT_HANDLE hPrivateData; /**< Handle to the private data. */
  CK_ULONG ulPublicDataLen2;     /**< Length of the second set of public
                                      data. */
  CK_BYTE_PTR pPublicData2;      /**< Pointer to the second set of public
                                      data. */
} CK_X9_42_DH2_DERIVE_PARAMS;

typedef CK_X9_42_DH2_DERIVE_PARAMS CK_PTR CK_X9_42_DH2_DERIVE_PARAMS_PTR;

/** Provides the parameters to the CKM_X9_42_MQV_DERIVE key derivation
  mechanisms, where each party contributes two key pairs.
*/
typedef struct CK_X9_42_MQV_DERIVE_PARAMS {
  CK_X9_42_DH_KDF_TYPE kdf;      /**< Key derivation function. */
  CK_ULONG ulOtherInfoLen;       /**< Length of the other information. */
  CK_BYTE_PTR pOtherInfo;        /**< Pointer to the other information. */
  CK_ULONG ulPublicDataLen;      /**< Length of the public data. */
  CK_BYTE_PTR pPublicData;       /**< Pointer to the public data. */
  CK_ULONG ulPrivateDataLen;     /**< Length of the private data. */
  CK_OBJECT_HANDLE hPrivateData; /**< Handle to the private data. */
  CK_ULONG ulPublicDataLen2;     /**< Length of the second set of public
                                      data. */
  CK_BYTE_PTR pPublicData2;      /**< Pointer to the second set of public
                                      data. */
  CK_OBJECT_HANDLE publicKey;    /**< Handle to the public key. */
} CK_X9_42_MQV_DERIVE_PARAMS;

typedef CK_X9_42_MQV_DERIVE_PARAMS CK_PTR CK_X9_42_MQV_DERIVE_PARAMS_PTR;

/** Provides the parameters to the CKM_KEA_DERIVE mechanism. */
typedef struct CK_KEA_DERIVE_PARAMS {
  CK_BBOOL      isSender;        /**< Comment needed here. */
  CK_ULONG      ulRandomLen;     /**< Comment needed here. */
  CK_BYTE_PTR   pRandomA;        /**< Comment needed here. */
  CK_BYTE_PTR   pRandomB;        /**< Comment needed here. */
  CK_ULONG      ulPublicDataLen; /**< Length of the public data. */
  CK_BYTE_PTR   pPublicData;     /**< Pointer to the public data. */
} CK_KEA_DERIVE_PARAMS;

typedef CK_KEA_DERIVE_PARAMS CK_PTR CK_KEA_DERIVE_PARAMS_PTR;


/** Provides the parameters to the CKM_RC2_ECB and CKM_RC2_MAC mechanisms. An
  instance of CK_RC2_PARAMS holds the effective key size only. */
typedef CK_ULONG          CK_RC2_PARAMS;

typedef CK_RC2_PARAMS CK_PTR CK_RC2_PARAMS_PTR;


/** Provides the parameters to the CKM_RC2_CBC mechanism.  */
typedef struct CK_RC2_CBC_PARAMS {
  CK_ULONG      ulEffectiveBits;  /**< Effective bits. Value: 1 to 1024. */

  CK_BYTE       iv[8];            /**< Initialization vector for CBC mode. */
} CK_RC2_CBC_PARAMS;

typedef CK_RC2_CBC_PARAMS CK_PTR CK_RC2_CBC_PARAMS_PTR;


/** Provides the parameters for the CKM_RC2_MAC_GENERAL mechanism. */
typedef struct CK_RC2_MAC_GENERAL_PARAMS {
  CK_ULONG      ulEffectiveBits;  /**< Effective bits. Value: 1 to 1024. */
  CK_ULONG      ulMacLength;      /**< Length of message authentication code in
                                       bytes. */
} CK_RC2_MAC_GENERAL_PARAMS;

typedef CK_RC2_MAC_GENERAL_PARAMS CK_PTR \
  CK_RC2_MAC_GENERAL_PARAMS_PTR;


/** Provides the parameters to the CKM_RC5_ECB and CKM_RC5_MAC mechanisms. */
typedef struct CK_RC5_PARAMS {
  CK_ULONG      ulWordsize;  /**< Word size in bits. */
  CK_ULONG      ulRounds;    /**< Number of rounds. */
} CK_RC5_PARAMS;

typedef CK_RC5_PARAMS CK_PTR CK_RC5_PARAMS_PTR;


/** Provides the parameters to the CKM_RC5_CBC mechanism. */
typedef struct CK_RC5_CBC_PARAMS {
  CK_ULONG      ulWordsize;  /**< Word size in bits. */
  CK_ULONG      ulRounds;    /**< Number of rounds. */
  CK_BYTE_PTR   pIv;         /**< Pointer to the initialization vector. */
  CK_ULONG      ulIvLen;     /**< Length of the initialization vector in
                                  bytes. */
} CK_RC5_CBC_PARAMS;

typedef CK_RC5_CBC_PARAMS CK_PTR CK_RC5_CBC_PARAMS_PTR;


/** Provides the parameters for the CKM_RC5_MAC_GENERAL mechanism. */
typedef struct CK_RC5_MAC_GENERAL_PARAMS {
  CK_ULONG      ulWordsize;   /**< Word size in bits. */
  CK_ULONG      ulRounds;     /**< Number of rounds. */
  CK_ULONG      ulMacLength;  /**< Length of message authentication code in
                                   bytes. */
} CK_RC5_MAC_GENERAL_PARAMS;

typedef CK_RC5_MAC_GENERAL_PARAMS CK_PTR \
  CK_RC5_MAC_GENERAL_PARAMS_PTR;
/** @endcond */

/** Provides the parameters to block most of the ciphers' MAC_GENERAL
  mechanisms. Value: Length of the message authentication code.
*/
typedef CK_ULONG          CK_MAC_GENERAL_PARAMS;

typedef CK_MAC_GENERAL_PARAMS CK_PTR CK_MAC_GENERAL_PARAMS_PTR;

/* CK_DES/AES_ECB/CBC_ENCRYPT_DATA_PARAMS are new for v2.20 */
/** Provides the parameters for the #CKM_DES_CBC_ENCRYPT_DATA and
  #CKM_DES3_CBC_ENCRYPT_DATA mechanisms.
*/
typedef struct CK_DES_CBC_ENCRYPT_DATA_PARAMS {
  CK_BYTE      iv[8];  /**< Initialization vector. */
  CK_BYTE_PTR  pData;  /**< Pointer to the data. */
  CK_ULONG     length; /**< Length of the data. */
} CK_DES_CBC_ENCRYPT_DATA_PARAMS;

typedef CK_DES_CBC_ENCRYPT_DATA_PARAMS CK_PTR CK_DES_CBC_ENCRYPT_DATA_PARAMS_PTR;

/** Provides the parameters for the #CKM_AES_CBC_ENCRYPT_DATA mechanism. */
typedef struct CK_AES_CBC_ENCRYPT_DATA_PARAMS {
  CK_BYTE      iv[16];  /**< Initialization vector. */
  CK_BYTE_PTR  pData;   /**< Pointer to the data. */
  CK_ULONG     length;  /**< Length of the data. */
} CK_AES_CBC_ENCRYPT_DATA_PARAMS;

typedef CK_AES_CBC_ENCRYPT_DATA_PARAMS CK_PTR CK_AES_CBC_ENCRYPT_DATA_PARAMS_PTR;

/** @cond
*/
/** Provides the parameters to the CKM_SKIPJACK_PRIVATE_WRAP mechanism. */
typedef struct CK_SKIPJACK_PRIVATE_WRAP_PARAMS {
  CK_ULONG      ulPasswordLen;   /**< Length of the password. */
  CK_BYTE_PTR   pPassword;       /**< Pointer to the password. */
  CK_ULONG      ulPublicDataLen; /**< Length of the public data. */
  CK_BYTE_PTR   pPublicData;     /**< Pointer to the public data. */
  CK_ULONG      ulPAndGLen;      /**< Comment needed here. */
  CK_ULONG      ulQLen;          /**< Comment needed here. */
  CK_ULONG      ulRandomLen;     /**< Comment needed here. */
  CK_BYTE_PTR   pRandomA;        /**< Comment needed here. */
  CK_BYTE_PTR   pPrimeP;         /**< Comment needed here. */
  CK_BYTE_PTR   pBaseG;          /**< Comment needed here. */
  CK_BYTE_PTR   pSubprimeQ;      /**< Comment needed here. */
} CK_SKIPJACK_PRIVATE_WRAP_PARAMS;

typedef CK_SKIPJACK_PRIVATE_WRAP_PARAMS CK_PTR \
  CK_SKIPJACK_PRIVATE_WRAP_PTR;


/** Provides the parameters to the CKM_SKIPJACK_RELAYX mechanism. */
typedef struct CK_SKIPJACK_RELAYX_PARAMS {
  CK_ULONG      ulOldWrappedXLen;    /**< Comment needed here. */
  CK_BYTE_PTR   pOldWrappedX;        /**< Comment needed here. */
  CK_ULONG      ulOldPasswordLen;    /**< Length of the old password. */
  CK_BYTE_PTR   pOldPassword;        /**< Pointer to the old password. */
  CK_ULONG      ulOldPublicDataLen;  /**< Length of the old public data. */
  CK_BYTE_PTR   pOldPublicData;      /**< Pointer to the old public data. */
  CK_ULONG      ulOldRandomLen;      /**< Comment needed here. */
  CK_BYTE_PTR   pOldRandomA;         /**< Comment needed here. */
  CK_ULONG      ulNewPasswordLen;    /**< Length of the new password. */
  CK_BYTE_PTR   pNewPassword;        /**< Pointer to the new password. */
  CK_ULONG      ulNewPublicDataLen;  /**< Length of the new public data. */
  CK_BYTE_PTR   pNewPublicData;      /**< Pointer to the new public data. */
  CK_ULONG      ulNewRandomLen;      /**< Comment needed here. */
  CK_BYTE_PTR   pNewRandomA;         /**< Comment needed here. */
} CK_SKIPJACK_RELAYX_PARAMS;

typedef CK_SKIPJACK_RELAYX_PARAMS CK_PTR \
  CK_SKIPJACK_RELAYX_PARAMS_PTR;

/** Comment needed here. */
typedef struct CK_PBE_PARAMS {
  CK_BYTE_PTR      pInitVector;      /**< Pointer to the initialization vector. */
  CK_UTF8CHAR_PTR  pPassword;        /**< Pointer to the password. */
  CK_ULONG         ulPasswordLen;    /**< Length of the password. */
  CK_BYTE_PTR      pSalt;            /**< Comment needed here. */
  CK_ULONG         ulSaltLen;        /**< Comment needed here. */
  CK_ULONG         ulIteration;      /**< Comment needed here. */
} CK_PBE_PARAMS;

typedef CK_PBE_PARAMS CK_PTR CK_PBE_PARAMS_PTR;

/** @endcond */

/** Provides the parameters to the #CKM_WRAPKEY_AES_CBC_PAD mechanism.
  This structure is a QTI PKCS #11 extension.
*/
typedef struct CK_WRAPKEY_AES_CBC_PAD_PARAMS {
  CK_BYTE  iv[16];
    /**< Initialization vector used in the encryption/decryption operation. */
  CK_ULONG flags;
    /**< Attributes specified as flags. The following Boolean key attributes
         are grouped via the corresponding flags:
         - CKA_ENCRYPT -- CKF_ENCRYPT
         - CKA_DECRYPT -- CKF_DECRYPT
         - CKA_WRAP -- CKF_WRAP
         - CKA_UNWRAP -- CKF_UNWRAP
         - CKA_SIGN -- CKF_SIGN
         - CKA_VERIFY -- CKF_VERIFY
         @tablebulletend
         */
  CK_BYTE  mac[32];
    /**< SHA256 HMAC of (Key+flags+wrapping_key) with KMac. */
  CK_BYTE  eKmac[48];
    /**< KMac encrypted with wrapping_key. The structure content is generated
         and returned during a key wrapping operation and is expected to be
         passed back unmodified for the corresponding key unwrapping
         operation.
    */
} CK_WRAPKEY_AES_CBC_PAD_PARAMS;

typedef CK_WRAPKEY_AES_CBC_PAD_PARAMS CK_PTR \
    CK_WRAPKEY_AES_CBC_PAD_PARAMS_PTR;

/** @cond
*/
/** Provides the parameters to the CKM_KEY_WRAP_SET_OAEP mechanism. */
typedef struct CK_KEY_WRAP_SET_OAEP_PARAMS {
  CK_BYTE       bBC;     /**< Block contents byte. */
  CK_BYTE_PTR   pX;      /**< Extra data. */
  CK_ULONG      ulXLen;  /**< Length of extra data in bytes. */
} CK_KEY_WRAP_SET_OAEP_PARAMS;

typedef CK_KEY_WRAP_SET_OAEP_PARAMS CK_PTR \
  CK_KEY_WRAP_SET_OAEP_PARAMS_PTR;

/** Comment needed here. */
typedef struct CK_SSL3_RANDOM_DATA {
  CK_BYTE_PTR  pClientRandom;      /**< Comment needed here. */
  CK_ULONG     ulClientRandomLen;  /**< Comment needed here. */
  CK_BYTE_PTR  pServerRandom;      /**< Comment needed here. */
  CK_ULONG     ulServerRandomLen;  /**< Comment needed here. */
} CK_SSL3_RANDOM_DATA;


/** Comment needed here. */
typedef struct CK_SSL3_MASTER_KEY_DERIVE_PARAMS {
  CK_SSL3_RANDOM_DATA RandomInfo;   /**< Comment needed here. */
  CK_VERSION_PTR pVersion;          /**< Comment needed here. */
} CK_SSL3_MASTER_KEY_DERIVE_PARAMS;

typedef struct CK_SSL3_MASTER_KEY_DERIVE_PARAMS CK_PTR \
  CK_SSL3_MASTER_KEY_DERIVE_PARAMS_PTR;


/** Comment needed here. */
typedef struct CK_SSL3_KEY_MAT_OUT {
  CK_OBJECT_HANDLE hClientMacSecret;  /**< Comment needed here. */
  CK_OBJECT_HANDLE hServerMacSecret;  /**< Comment needed here. */
  CK_OBJECT_HANDLE hClientKey;        /**< Handle to the client key. */
  CK_OBJECT_HANDLE hServerKey;        /**< Handle to the server key. */
  CK_BYTE_PTR      pIVClient;         /**< Pointer to the initialization vector
                                           client. */
  CK_BYTE_PTR      pIVServer;         /**< Pointer to the initialization vector
                                           server. */
} CK_SSL3_KEY_MAT_OUT;

typedef CK_SSL3_KEY_MAT_OUT CK_PTR CK_SSL3_KEY_MAT_OUT_PTR;


/** Comment needed here. */
typedef struct CK_SSL3_KEY_MAT_PARAMS {
  CK_ULONG                ulMacSizeInBits;      /**< Size of the MAC in
                                                     bits. */
  CK_ULONG                ulKeySizeInBits;      /**< Size of the key in
                                                     bits. */
  CK_ULONG                ulIVSizeInBits;       /**< Size of the initialization
                                                     vector in bits. */
  CK_BBOOL                bIsExport;            /**< Comment needed here. */
  CK_SSL3_RANDOM_DATA     RandomInfo;           /**< Comment needed here. */
  CK_SSL3_KEY_MAT_OUT_PTR pReturnedKeyMaterial; /**< Pointer to the returned
                                                     key material. */
} CK_SSL3_KEY_MAT_PARAMS;

typedef CK_SSL3_KEY_MAT_PARAMS CK_PTR CK_SSL3_KEY_MAT_PARAMS_PTR;

/* CK_TLS_PRF_PARAMS is new for version 2.20 */
/** Comment needed here. */
typedef struct CK_TLS_PRF_PARAMS {
  CK_BYTE_PTR  pSeed;        /**< Pointer to the seed. */
  CK_ULONG     ulSeedLen;    /**< Length of the seed. */
  CK_BYTE_PTR  pLabel;       /**< Pointer to the label. */
  CK_ULONG     ulLabelLen;   /**< Length of the label. */
  CK_BYTE_PTR  pOutput;      /**< Pointer to the output. */
  CK_ULONG_PTR pulOutputLen; /**< Pointer to the length of the output. */
} CK_TLS_PRF_PARAMS;

typedef CK_TLS_PRF_PARAMS CK_PTR CK_TLS_PRF_PARAMS_PTR;

/* WTLS is new for version 2.20 */
/** Comment needed here. */
typedef struct CK_WTLS_RANDOM_DATA {
  CK_BYTE_PTR pClientRandom;      /**< Pointer to the client random data. */
  CK_ULONG    ulClientRandomLen;  /**< Length of the client random data. */
  CK_BYTE_PTR pServerRandom;      /**< Pointer to the server random data. */
  CK_ULONG    ulServerRandomLen;  /**< Length of the server random data. */
} CK_WTLS_RANDOM_DATA;

typedef CK_WTLS_RANDOM_DATA CK_PTR CK_WTLS_RANDOM_DATA_PTR;

/** Comment needed here. */
typedef struct CK_WTLS_MASTER_KEY_DERIVE_PARAMS {
  CK_MECHANISM_TYPE   DigestMechanism; /**< Comment needed here. */
  CK_WTLS_RANDOM_DATA RandomInfo;      /**< Comment needed here. */
  CK_BYTE_PTR         pVersion;        /**< Pointer to the version of the
                                            master key. */
} CK_WTLS_MASTER_KEY_DERIVE_PARAMS;

typedef CK_WTLS_MASTER_KEY_DERIVE_PARAMS CK_PTR \
  CK_WTLS_MASTER_KEY_DERIVE_PARAMS_PTR;

/** Comment needed here. */
typedef struct CK_WTLS_PRF_PARAMS {
  CK_MECHANISM_TYPE DigestMechanism; /**< Comment needed here. */
  CK_BYTE_PTR       pSeed;           /**< Pointer to the seed. */
  CK_ULONG          ulSeedLen;       /**< Length of the seed. */
  CK_BYTE_PTR       pLabel;          /**< Pointer to the label. */
  CK_ULONG          ulLabelLen;      /**< Length of the label. */
  CK_BYTE_PTR       pOutput;         /**< Pointer to the output. */
  CK_ULONG_PTR      pulOutputLen;    /**< Pointer to the length of the
                                          output. */
} CK_WTLS_PRF_PARAMS;

typedef CK_WTLS_PRF_PARAMS CK_PTR CK_WTLS_PRF_PARAMS_PTR;

/** Comment needed here. */
typedef struct CK_WTLS_KEY_MAT_OUT {
  CK_OBJECT_HANDLE hMacSecret; /**< Handle to the MAC secret key. */
  CK_OBJECT_HANDLE hKey;       /**< Handle to the key. */
  CK_BYTE_PTR      pIV;        /**< Pointer to the initialization vector. */
} CK_WTLS_KEY_MAT_OUT;

typedef CK_WTLS_KEY_MAT_OUT CK_PTR CK_WTLS_KEY_MAT_OUT_PTR;

/** Comment needed here. */
typedef struct CK_WTLS_KEY_MAT_PARAMS {
  CK_MECHANISM_TYPE       DigestMechanism;      /**< Comment needed here. */
  CK_ULONG                ulMacSizeInBits;      /**< Size of the MAC in bits. */
  CK_ULONG                ulKeySizeInBits;      /**< Size of the key in bits. */
  CK_ULONG                ulIVSizeInBits;       /**< Size of the initialization
                                                     vector in bits. */
  CK_ULONG                ulSequenceNumber;     /**< Number of the sequence.*/
  CK_BBOOL                bIsExport;            /**< Comment needed here. */
  CK_WTLS_RANDOM_DATA     RandomInfo;           /**< Comment needed here. */
  CK_WTLS_KEY_MAT_OUT_PTR pReturnedKeyMaterial; /**< Pointer to the returned
                                                     key material. */
} CK_WTLS_KEY_MAT_PARAMS;

typedef CK_WTLS_KEY_MAT_PARAMS CK_PTR CK_WTLS_KEY_MAT_PARAMS_PTR;

/* CMS is new for version 2.20 */
/** Comment needed here. */
typedef struct CK_CMS_SIG_PARAMS {
  CK_OBJECT_HANDLE      certificateHandle;        /**< Handle of the
                                                       certificate. */
  CK_MECHANISM_PTR      pSigningMechanism;        /**< Pointer to the signing
                                                       mechanism. */
  CK_MECHANISM_PTR      pDigestMechanism;         /**< Pointer to the digest
                                                       mechanism. */
  CK_UTF8CHAR_PTR       pContentType;             /**< Pointer to the content
                                                       type. */
  CK_BYTE_PTR           pRequestedAttributes;     /**< Pointer to the requested
                                                       attributes. */
  CK_ULONG              ulRequestedAttributesLen; /**< Length of the requested
                                                       attributes. */
  CK_BYTE_PTR           pRequiredAttributes;      /**< Pointer to the required
                                                       attributes. */
  CK_ULONG              ulRequiredAttributesLen;  /**< Length of the required
                                                       attributes. */
} CK_CMS_SIG_PARAMS;

typedef CK_CMS_SIG_PARAMS CK_PTR CK_CMS_SIG_PARAMS_PTR;
/** @endcond */


/** Provides the parameters for the #CKM_DES_ECB_ENCRYPT_DATA,
  #CKM_DES3_ECB_ENCRYPT_DATA and #CKM_AES_ECB_ENCRYPT_DATA mechanisms.
*/
typedef struct CK_KEY_DERIVATION_STRING_DATA {
  CK_BYTE_PTR pData;  /**< Pointer to the data. */
  CK_ULONG    ulLen;  /**< Length of the byte string. */
} CK_KEY_DERIVATION_STRING_DATA;

typedef CK_KEY_DERIVATION_STRING_DATA CK_PTR \
  CK_KEY_DERIVATION_STRING_DATA_PTR;


/** Provides the parameters for the #CKM_EXTRACT_KEY_FROM_KEY mechanism. It
  specifies which bit of the base key is to be used as the first bit of the
  derived key.
*/
typedef CK_ULONG CK_EXTRACT_PARAMS;

typedef CK_EXTRACT_PARAMS CK_PTR CK_EXTRACT_PARAMS_PTR;

/** Indicates the pseudorandom function used to generate key bits using
  PKCS #5 PBKDF2.
*/
typedef CK_ULONG CK_PKCS5_PBKD2_PSEUDO_RANDOM_FUNCTION_TYPE;

typedef CK_PKCS5_PBKD2_PSEUDO_RANDOM_FUNCTION_TYPE CK_PTR CK_PKCS5_PBKD2_PSEUDO_RANDOM_FUNCTION_TYPE_PTR;
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** Pseudorandom number generation function using HMAC and SHA1. Refer to
  PKCS #5 v2.0 @ref S2 "[S2]". */
#define CKP_PKCS5_PBKD2_HMAC_SHA1 0x00000001

/** Pseudorandom number generation function using HMAC and SHA256 that is used
  to implement other PKCS mechanisms such as TLS. Refer to PKCS #5 v2.0
  @ref S2 "[S2]".  */
#define CKP_PKCS5_PBKD2_HMAC_SHA256 0x00000002
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** Indicates the source of the salt value when deriving a key using PKCS #5
  PBKDF2.
*/
typedef CK_ULONG CK_PKCS5_PBKDF2_SALT_SOURCE_TYPE;

typedef CK_PKCS5_PBKDF2_SALT_SOURCE_TYPE CK_PTR CK_PKCS5_PBKDF2_SALT_SOURCE_TYPE_PTR;
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/** Salt value source. Refer to PKCS #5 v2.0 @ref S2 "[S2]". */
#define CKZ_SALT_SPECIFIED        0x00000001
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */

/** Provides the parameters to the #CKM_PKCS5_PBKD2 mechanism. */
typedef struct CK_PKCS5_PBKD2_PARAMS {
        CK_PKCS5_PBKDF2_SALT_SOURCE_TYPE           saltSource;
        /**< Source of the salt value. */
        CK_VOID_PTR                                pSaltSourceData;
        /**< Pointer to the salt source data. */
        CK_ULONG                                   ulSaltSourceDataLen;
        /**< Length of the salt source data. */
        CK_ULONG                                   iterations;
        /**< Number of iterations. */
        CK_PKCS5_PBKD2_PSEUDO_RANDOM_FUNCTION_TYPE prf;
        /**< Type of pseudorandom function. */
        CK_VOID_PTR                                pPrfData;
        /**< Pointer to the pseudorandom function data. */
        CK_ULONG                                   ulPrfDataLen;
        /**< Length of the pseudorandom function data. */
        CK_UTF8CHAR_PTR                            pPassword;
        /**< Pointer to the password. */
        CK_ULONG_PTR                               ulPasswordLen;
        /**< Length of the password. */
} CK_PKCS5_PBKD2_PARAMS;

typedef CK_PKCS5_PBKD2_PARAMS CK_PTR CK_PKCS5_PBKD2_PARAMS_PTR;

/** Provides the parameters for the #CKM_PKCS5_PBKD2_KEYPAD mechanism. \n */
typedef struct CK_PKCS5_PBKD2_KEYPAD_PARAMS {
        CK_PKCS5_PBKDF2_SALT_SOURCE_TYPE           saltSource;
        /**< Source of the salt value. */
        CK_VOID_PTR                                pSaltSourceData;
        /**< Pointer to the salt source data. */
        CK_ULONG                                   ulSaltSourceDataLen;
        /**< Length of the salt source data. */
        CK_ULONG                                   iterations;
        /**< Number of iterations. */
        CK_PKCS5_PBKD2_PSEUDO_RANDOM_FUNCTION_TYPE prf;
        /**< Type of pseudorandom function. */
        CK_VOID_PTR                                pPrfData;
        /**< Pointer to the pseudorandom function data. */
        CK_ULONG                                   ulPrfDataLen;
        /**< Length of the pseudorandom function data. */
        CK_ULONG                                   ulMinPasswordLen;
        /**< Minimum length of the password. */
        CK_ULONG                                   ulMaxPasswordLen;
        /**< Maximum length of the password. */
} CK_PKCS5_PBKD2_KEYPAD_PARAMS;

typedef CK_PKCS5_PBKD2_KEYPAD_PARAMS CK_PTR CK_PKCS5_PBKD2_KEYPAD_PARAMS_PTR;
/** Provides the parameters for the #CK_PKCS5_PBKD1 mechanism. \n */
typedef struct CK_PKCS5_PBKD1_PARAMS {
        CK_VOID_PTR        pSaltSourceData;
        /**< Pointer to the salt source data. */
        CK_ULONG           ulSaltSourceDataLen;
        /**< Length of the salt source data. */
        CK_ULONG           iterations;
        /**< Number of iterations. */
        CK_MECHANISM_TYPE  hashAlg;
        /**< Hash algorithm. */
        CK_UTF8CHAR_PTR    pPassword;
        /**< Pointer to the password. */
        CK_ULONG           ulPasswordLen;
        /**< Length of the password. */
} CK_PKCS5_PBKD1_PARAMS;

typedef CK_PKCS5_PBKD1_PARAMS CK_PTR CK_PKCS5_PBKD1_PARAMS_PTR;

/** Provides the parameters for the #CKM_PKCS5_PBKD1_KEYPAD mechanism. \n*/
typedef struct CK_PKCS5_PBKD1_KEYPAD_PARAMS {
        CK_VOID_PTR        pSaltSourceData;
        /**< Pointer to the salt source data. */
        CK_ULONG           ulSaltSourceDataLen;
        /**< Length of the salt source data. */
        CK_ULONG           iterations;
        /**< Number of iterations. */
        CK_MECHANISM_TYPE  hashAlg;
        /**< Hash algorithm. */
        CK_ULONG           ulMinPasswordLen;
        /**< Minimum length of the password. */
        CK_ULONG           ulMaxPasswordLen;
        /**< Maximum length of the password. */
} CK_PKCS5_PBKD1_KEYPAD_PARAMS;

typedef CK_PKCS5_PBKD1_KEYPAD_PARAMS CK_PTR CK_PKCS5_PBKD1_KEYPAD_PARAMS_PTR;

/** Provides the parameters for the #CKM_PKCS5_PBKD1_VPN mechanism.\n */
typedef struct CK_PKCS5_PBKD1_VPN_PARAMS {
        CK_VOID_PTR        pSaltSourceData;
        /**< Pointer to the salt source data. */
        CK_ULONG           ulSaltSourceDataLen;
        /**< Length of the salt source data. */
        CK_ULONG           iterations;
        /**< Number of iterations. */
        CK_MECHANISM_TYPE  hashAlg;
        /**< Hash algorithm. */
        CK_OBJECT_HANDLE   hGenericSecretKey;
        /**< Handle of the generic secret key. */
} CK_PKCS5_PBKD1_VPN_PARAMS;

typedef CK_PKCS5_PBKD1_VPN_PARAMS CK_PTR CK_PKCS5_PBKD1_VPN_PARAMS_PTR;

/** Provides the parameters for the #CKM_GENERIC_SECRET_KEY_KEYPAD and
  #CKM_GENERIC_SECRET_KEY_KEYPAD_VPN mechanisms. \n */
typedef struct CK_GENERIC_SECRET_KEY_KEYPAD_PARAMS {
        CK_ULONG  ulMinPasswordLen;
        /**< Minimum length of the password. */
        CK_ULONG  ulMaxPasswordLen;
        /**< Maximum length of the password. */
} CK_GENERIC_SECRET_KEY_KEYPAD_PARAMS;

typedef CK_GENERIC_SECRET_KEY_KEYPAD_PARAMS CK_PTR CK_GENERIC_SECRET_KEY_KEYPAD_PARAMS_PTR;

/** Provides the parameters for the #CKM_SCRAM_KEYPAD mechanism. \n*/
typedef struct CK_SCRAM_KEYPAD_PARAMS {
        CK_BYTE_PTR  pClientRandom;
        /**< Length of client random data */
        CK_ULONG     ulClientRandomLen;
        /**< Client random. */
        CK_MECHANISM_TYPE hmac;
        /**< HMAC mechanism to use for the SCRAM authentication */
} CK_SCRAM_KEYPAD_PARAMS;

typedef CK_SCRAM_KEYPAD_PARAMS CK_PTR CK_SCRAM_KEYPAD_PARAMS_PTR;

/** Input to the CKM_SCRAM_KEYPAD mechanism authentication call.
  @ref C_Sign "C_Sign()" (@ref C_SignFinal "C_SignFinal()"). */
typedef struct CK_SCRAM_KEYPAD_INPUT_PARAMS {
    CK_BYTE_PTR pClientFirstMessageBare;
    CK_ULONG ulClientFirstMessageBare;
    CK_BYTE_PTR pServerFirstMessageBare;
    CK_ULONG ulServerFirstMessageBare;
    CK_BYTE_PTR pClientFinalMessageWithoutProof;
    CK_ULONG ulClientFinalMessageWithoutProof;
} CK_SCRAM_KEYPAD_INPUT_PARAMS;

/** Output to the CKM_SCRAM_KEYPAD mechanism authentication call.
  @ref C_Sign "C_Sign()" (@ref C_SignFinal "C_SignFinal()"). */
typedef struct CK_SCRAM_KEYPAD_OUTPUT_INFO {
    CK_BYTE_PTR pClientProof;
    CK_ULONG ulClientProof;
    CK_BYTE_PTR pServerSignature;
    CK_ULONG ulServerSignature;
} CK_SCRAM_KEYPAD_OUTPUT_INFO;

/* All CK_OTP structs are new for PKCS #11 v2.20 amendment 3 */

/** Type for OTP parameters. */
typedef CK_ULONG CK_OTP_PARAM_TYPE;

typedef CK_OTP_PARAM_TYPE CK_PARAM_TYPE; /* B/w compatibility */

/** Represents an OTP parameter as a Type-Length-Value structure. */
typedef struct CK_OTP_PARAM {
    CK_OTP_PARAM_TYPE type; /**< Parameter type. */
    CK_VOID_PTR pValue;     /**< Pointer to the value of the parameter. */
    CK_ULONG ulValueLen;    /**< Length in bytes of the value. */
} CK_OTP_PARAM;

typedef CK_OTP_PARAM CK_PTR CK_OTP_PARAM_PTR;

/** Provides parameters for OTP mechanisms in a generic fashion. */
typedef struct CK_OTP_PARAMS {
    CK_OTP_PARAM_PTR pParams;  /**< Pointer to an array of OTP parameters. */
    CK_ULONG ulCount;          /**< Number of parameters in the array. */
} CK_OTP_PARAMS;

typedef CK_OTP_PARAMS CK_PTR CK_OTP_PARAMS_PTR;

/** Returned by all OTP mechanisms in successful calls to
  @ref C_Sign "C_Sign()" (@ref C_SignFinal "C_SignFinal()"). */
typedef struct CK_OTP_SIGNATURE_INFO {
    CK_OTP_PARAM_PTR pParams;  /**< Pointer to an array of OTP parameter values. */
    CK_ULONG ulCount;           /**< The number of parameters in the array. */
} CK_OTP_SIGNATURE_INFO;

typedef CK_OTP_SIGNATURE_INFO CK_PTR CK_OTP_SIGNATURE_INFO_PTR;
/** @} */ /* end_addtogroup pkcs11_datatypes */

/** @addtogroup pkcs11_constants
 @{ */

/* The following OTP-related defines are new for PKCS #11 v2.20 amendment 1 */
/** Actual OTP value. */
#define CK_OTP_VALUE          0
/** UTF8 string containing a PIN for use when computing or verifying PIN-based
  OTP values.
*/
#define CK_OTP_PIN            1
/** Challenge to use when computing or verifying challenge-based OTP values. */
#define CK_OTP_CHALLENGE      2
/** UTC time value (as CK_ULONG) to use when computing or verifying time-based
  OTP-values.
*/
#define CK_OTP_TIME           3
/** Counter value to use when computing or verifying counter-based OTP values. */
#define CK_OTP_COUNTER        4
/** Bit flags indicating the characteristics of the OTP. */
#define CK_OTP_FLAGS          5
/** Desired output length (overrides any default value). */
#define CK_OTP_OUTPUT_LENGTH  6
/** Desired OTP format. */
#define CK_OTP_OUTPUT_FORMAT  7
/* QTI PKCS #11 extension */
/** Time step when computing time-based OTP values. This define is a QTI PKCS #11 extension. */
#define CK_OTP_TIME_STEP      8

/* The following OTP-related defines are new for PKCS #11 v2.20 amendment 1 */
/** OTP computation must be for the next OTP. */
#define CKF_NEXT_OTP          0x00000001
/** OTP computation must not include a time value. */
#define CKF_EXCLUDE_TIME      0x00000002
/** OTP computation must not include a counter value. */
#define CKF_EXCLUDE_COUNTER   0x00000004
/** OTP computation must not include a challenge value. */
#define CKF_EXCLUDE_CHALLENGE 0x00000008
/** OTP computation must not include a PIN value. */
#define CKF_EXCLUDE_PIN       0x00000010
/** Returned OTP must be in a form suitable for human consumption.
*/
#define CKF_USER_FRIENDLY_OTP 0x00000020
/** @} */ /* end_addtogroup pkcs11_constants */

/** @addtogroup pkcs11_datatypes
 @{ */
/** @cond
*/
/* CK_KIP_PARAMS is new for PKCS #11 v2.20 amendment 2 */
/** Comment needed here. */
typedef struct CK_KIP_PARAMS {
    CK_MECHANISM_PTR  pMechanism; /**< Pointer to the mechanism. */
    CK_OBJECT_HANDLE  hKey;       /**< Handle to the key. */
    CK_BYTE_PTR       pSeed;      /**< Pointer to the seed. */
    CK_ULONG          ulSeedLen;  /**< Length of the seed. */
} CK_KIP_PARAMS;

typedef CK_KIP_PARAMS CK_PTR CK_KIP_PARAMS_PTR;

/* CK_AES_CTR_PARAMS is new for PKCS #11 v2.20 amendment 3 */
/** Comment needed here. */
typedef struct CK_AES_CTR_PARAMS {
    CK_ULONG ulCounterBits; /**< Comment needed here. */
    CK_BYTE cb[16];         /**< Comment needed here. */
} CK_AES_CTR_PARAMS;

typedef CK_AES_CTR_PARAMS CK_PTR CK_AES_CTR_PARAMS_PTR;

/* CK_CAMELLIA_CTR_PARAMS is new for PKCS #11 v2.20 amendment 3 */
/** Comment needed here. */
typedef struct CK_CAMELLIA_CTR_PARAMS {
    CK_ULONG ulCounterBits; /**< Comment needed here. */
    CK_BYTE cb[16];         /**< Comment needed here. */
} CK_CAMELLIA_CTR_PARAMS;

typedef CK_CAMELLIA_CTR_PARAMS CK_PTR CK_CAMELLIA_CTR_PARAMS_PTR;

/* CK_CAMELLIA_CBC_ENCRYPT_DATA_PARAMS is new for PKCS #11 v2.20 amendment 3 */
/** Comment needed here. */
typedef struct CK_CAMELLIA_CBC_ENCRYPT_DATA_PARAMS {
    CK_BYTE      iv[16];  /**< Initialization vector. */
    CK_BYTE_PTR  pData;   /**< Pointer to the data. */
    CK_ULONG     length;  /**< Comment needed here. */
} CK_CAMELLIA_CBC_ENCRYPT_DATA_PARAMS;

typedef CK_CAMELLIA_CBC_ENCRYPT_DATA_PARAMS CK_PTR CK_CAMELLIA_CBC_ENCRYPT_DATA_PARAMS_PTR;

/* CK_ARIA_CBC_ENCRYPT_DATA_PARAMS is new for PKCS #11 v2.20 amendment 3 */
/** Comment needed here. */
typedef struct CK_ARIA_CBC_ENCRYPT_DATA_PARAMS {
    CK_BYTE      iv[16];  /**< Initialization vector. */
    CK_BYTE_PTR  pData;   /**< Pointer to the data. */
    CK_ULONG     length;  /**< Comment needed here. */
} CK_ARIA_CBC_ENCRYPT_DATA_PARAMS;

typedef CK_ARIA_CBC_ENCRYPT_DATA_PARAMS CK_PTR CK_ARIA_CBC_ENCRYPT_DATA_PARAMS_PTR;
/** @endcond */
/** @} */ /* end_addtogroup pkcs11_datatypes */


#endif
