#ifndef __DRMRESULTS_H__
#define __DRMRESULTS_H__

/*===========================================================================
**
** Microsoft PlayReady
** Copyright (c) Microsoft Corporation. All rights reserved.
**
===========================================================================*/
/*===========================================================================

                      EDIT HISTORY FOR FILE

when       who     what, where, why
--------   ---     ----------------------------------------------------------
11/06/13    rk     Modified error codes for video protection
07/30/13    dm     Modified the error code for DRM_E_HDCP_NON_SECURE
07/10/13    kr     Added new error codes for HDCP/HDMI
01/02/13    kr     Added all playready DRM error codes
11/16/12    cz     Added more error codes
===========================================================================*/

typedef long  DRM_RESULT;
/*
 * MessageId: DRM_SUCCESS
 *
 * MessageText:
 *
 * Operation was successful.
 *
 */
#define DRM_SUCCESS                      ((DRM_RESULT)0x00000000L)

/*
 * MessageId: DRM_S_FALSE
 *
 * MessageText:
 *
 * Operation was successful, but returned a FALSE test condition.
 *
 */
#define DRM_S_FALSE                      ((DRM_RESULT)0x00000001L)

/*
 * MessageId: DRM_S_MORE_DATA
 *
 * MessageText:
 *
 * Operation was successful, but more data is available.
 *
 */
#define DRM_S_MORE_DATA                  ((DRM_RESULT)0x00000002L)



/* ============================================================
**
** Standard error messages (0x8000xxxx)
**
** ============================================================
*/

/*
 * MessageId: DRM_E_OUTOFMEMORY
 *
 * MessageText:
 *
 * Insufficient resources exist to complete the request.
 *
 */
#define DRM_E_OUTOFMEMORY                ((DRM_RESULT)0x80000002L)

/*
 * MessageId: DRM_E_NOTIMPL
 *
 * MessageText:
 *
 * The requested operation is not implemented.
 *
 */
#define DRM_E_NOTIMPL                    ((DRM_RESULT)0x80004001L)

/*
 * MessageId: DRM_E_FAIL
 *
 * MessageText:
 *
 * The requested operation failed.
 *
 */
#define DRM_E_FAIL                       ((DRM_RESULT)0x80004005L)

/*
 * MessageId: DRM_E_HLOS_TAMPERED
 *
 * MessageText:
 *
 * HLOS tampered.
 *
 */
#define DRM_E_HLOS_TAMPERED              ((DRM_RESULT)0x80004006L)

/*
 * MessageId: DRM_E_OPL_BLOCKED
 *
 * MessageText:
 *
 * OPL blocked.
 *
 */
#define DRM_E_OPL_BLOCKED                ((DRM_RESULT)0x80004007L)

/*
 * MessageId: DRM_E_LOAD_IMG
 *
 * MessageText:
 *
 * Failed on loading playready image.
 *
 */
#define DRM_E_LOAD_IMG                   ((DRM_RESULT)0x80004008L)

/*
 * MessageId: DRM_E_VER_MISMATCH
 *
 * MessageText:
 *
 * Version mismatch between HLOS and TZ.
 *
 */
#define DRM_E_VER_MISMATCH               ((DRM_RESULT)0x80004009L)

/*
 * MessageId: DRM_E_SET_BANDWIDTH
 *
 * MessageText:
 *
 * Failed on setting bandwidth.
 *
 */
#define DRM_E_SET_BANDWIDTH              ((DRM_RESULT)0x8000400AL)

/*
 * MessageId: DRM_E_OUT_OF_BOUND
 *
 * MessageText:
 *
 * Out of bound.
 *
 */
#define DRM_E_OUT_OF_BOUND               ((DRM_RESULT)0x8000400BL)

/*
 * MessageId: DRM_E_PLAY_ENABLER_BLOCKED
 *
 * MessageText:
 *
 * WFD is blocked as play enabler object for HDCP doesn't exist in license
 *
 */
#define DRM_E_PLAY_ENABLER_BLOCKED       ((DRM_RESULT)0x8000400CL)

/*
 * MessageId: DRM_E_OEM_CONSTRAINT_1_FAIL
 *
 * MessageText:
 * Failed while OEM CONSTRAINT-1 check
 *
 */

#define DRM_E_OEM_CONSTRAINT_1_FAIL       ((DRM_RESULT)0x8004DFFC)

/*
 * MessageId: DRM_E_OEM_CONSTRAINT_2_FAIL
 *
 * MessageText:
 * Failed while OEM CONSTRAINT-2 check
 *
 */
#define DRM_E_OEM_CONSTRAINT_2_FAIL    ((DRM_RESULT)0x8004DFFD)

/*
 * MessageId: DRM_E_OEM_CONSTRAINT_3_FAIL
 *
 * MessageText:
 * Failed while OEM CONSTRAINT-3 check
 *
 */
#define DRM_E_OEM_CONSTRAINT_3_FAIL       ((DRM_RESULT)0x8004DFFE)

/*
 * MessageId: DRM_E_HDCP_NON_SECURE
 *
 * MessageText:
 *
 * Attempt to play back through insecure HDMI output path
 *
 */
#define DRM_E_HDCP_NON_SECURE       ((DRM_RESULT)0x8004DFFF)

/*
 * MessageId: DRM_E_HDMI_READ_FAIL
 *
 * MessageText:
 * Failed to read HDMI Status
 *
 */
#define DRM_E_HDMI_READ_FAIL       ((DRM_RESULT)0x8000400EL)

/* ============================================================
**
** Error messages shared with Win32 (0x8007xxxx)
**
** ============================================================
*/

/*
 * MessageId: DRM_E_WIN32_FILE_NOT_FOUND
 *
 * MessageText:
 *
 * The system cannot find the file specified.
 *
 */
#define DRM_E_WIN32_FILE_NOT_FOUND       ((DRM_RESULT)0x80070002L)

/*
 * MessageId: DRM_E_WIN32_NO_MORE_FILES
 *
 * MessageText:
 *
 * There are no more files.
 *
 */
#define DRM_E_WIN32_NO_MORE_FILES        ((DRM_RESULT)0x80070012L)

/*
 * MessageId: DRM_E_INVALIDARG
 *
 * MessageText:
 *
 * The parameter is incorrect.
 *
 */
#define DRM_E_INVALIDARG                 ((DRM_RESULT)0x80070057L)

/*
 * MessageId: DRM_E_BUFFERTOOSMALL
 *
 * MessageText:
 *
 * The data area passed to a function is too small.
 *
 */
#define DRM_E_BUFFERTOOSMALL             ((DRM_RESULT)0x8007007AL)

/*
 * MessageId: DRM_E_NOMORE
 *
 * MessageText:
 *
 * No more data is available.
 *
 */
#define DRM_E_NOMORE                     ((DRM_RESULT)0x80070103L)

/*
 * MessageId: DRM_E_ARITHMETIC_OVERFLOW
 *
 * MessageText:
 *
 * Arithmetic result exceeded maximum value.
 *
 */
#define DRM_E_ARITHMETIC_OVERFLOW        ((DRM_RESULT)0x80070216L)

/*
 * MessageId: DRM_E_NOT_FOUND
 *
 * MessageText:
 *
 * Element not found.
 *
 */
#define DRM_E_NOT_FOUND                  ((DRM_RESULT)0x80070490L)

/*
 * MessageId: DRM_E_INVALID_COMMAND_LINE
 *
 * MessageText:
 *
 * Invalid command line argument.
 *
 */
#define DRM_E_INVALID_COMMAND_LINE       ((DRM_RESULT)0x80070667L)


/* ============================================================
**
** Error messages shared with COM Storage (mostly file errors)
** (0x8003xxxx)
**
** ============================================================
*/

/*
 * MessageId: DRM_E_FILENOTFOUND
 *
 * MessageText:
 *
 * A requested file could not be found.
 *
 */
#define DRM_E_FILENOTFOUND               ((DRM_RESULT)0x80030002L)

/*
 * MessageId: DRM_E_FILEOPEN
 *
 * MessageText:
 *
 * A request failed due to a file being open.
 *
 */
#define DRM_E_FILEOPEN                   ((DRM_RESULT)0x8003006EL)


/* ============================================================
**
** NetShow Errors from NsError.h (0xc00Dxxxx)
**
** ============================================================
*/

/*
 * MessageId: DRM_E_PARAMETERS_MISMATCHED
 *
 * MessageText:
 *
 * A problem has occurred in the Digital Rights Management component.
 *
 */
#define DRM_E_PARAMETERS_MISMATCHED      ((DRM_RESULT)0xC00D272FL)

/*
 * MessageId: DRM_E_FAILED_TO_STORE_LICENSE
 *
 * MessageText:
 *
 * License storage is not working.
 *
 */
#define DRM_E_FAILED_TO_STORE_LICENSE    ((DRM_RESULT)0xC00D2712L)

/*
 * MessageId: DRM_E_NOT_ALL_STORED
 *
 * MessageText:
 *
 * Some of the licenses could not be stored.
 *
 */
#define DRM_E_NOT_ALL_STORED             ((DRM_RESULT)0xC00D275FL)


/* ============================================================
**
** Vista crypto errors, 0x80040e80-0x80040e8f.
**
** ============================================================
*/

/*
 * MessageId: DRM_E_VERIFICATION_FAILURE
 *
 * MessageText:
 *
 * Validation of a Longhorn certificate failed.
 *
 */
#define DRM_E_VERIFICATION_FAILURE       ((DRM_RESULT)0x80040E80L)

/*
 * MessageId: DRM_E_RSA_SIGNATURE_ERROR
 *
 * MessageText:
 *
 * Error in RSA(PSS) signature.
 *
 */
#define DRM_E_RSA_SIGNATURE_ERROR        ((DRM_RESULT)0x80040E82L)

/*
 * MessageId: DRM_E_BAD_RSA_EXPONENT
 *
 * MessageText:
 *
 * An incorrect RSA exponent was supplied for a public key.
 *
 */
#define DRM_E_BAD_RSA_EXPONENT           ((DRM_RESULT)0x80040E86L)

/*
 * MessageId: DRM_E_P256_CONVERSION_FAILURE
 *
 * MessageText:
 *
 * An error occurred while converting between P256 types.
 *
 */
#define DRM_E_P256_CONVERSION_FAILURE    ((DRM_RESULT)0x80040E87L)

/*
 * MessageId: DRM_E_P256_PKCRYPTO_FAILURE
 *
 * MessageText:
 *
 * An error occurred in an asymmetric P256 cryptographic operation.
 *
 */
#define DRM_E_P256_PKCRYPTO_FAILURE      ((DRM_RESULT)0x80040E88L)

/*
 * MessageId: DRM_E_P256_PLAINTEXT_MAPPING_FAILURE
 *
 * MessageText:
 *
 * An error occurred while attempting to map a plaintext array to a EC Point: There is no conversion for this byte array to a EC Point.
 *
 */
#define DRM_E_P256_PLAINTEXT_MAPPING_FAILURE ((DRM_RESULT)0x80040E89L)

/*
 * MessageId: DRM_E_P256_INVALID_SIGNATURE
 *
 * MessageText:
 *
 * The ECDSA signature to be verified was not a valid signature format.
 *
 */
#define DRM_E_P256_INVALID_SIGNATURE     ((DRM_RESULT)0x80040E8AL)

/*
 * MessageId: DRM_E_P256_ECDSA_VERIFICATION_ERROR
 *
 * MessageText:
 *
 * The ECDSA verification algorithm encountered an unknown error.
 *
 */
#define DRM_E_P256_ECDSA_VERIFICATION_ERROR ((DRM_RESULT)0x80040E8BL)

/*
 * MessageId: DRM_E_P256_ECDSA_SIGNING_ERROR
 *
 * MessageText:
 *
 * The ECDSA signature algorithm encountered an unknown error.
 *
 */
#define DRM_E_P256_ECDSA_SIGNING_ERROR   ((DRM_RESULT)0x80040E8CL)

/*
 * MessageId: DRM_E_P256_HMAC_KEYGEN_FAILURE
 *
 * MessageText:
 *
 * Could not generate a valid HMAC key under constraint where CK || HMACK is a valid x coord on the EC (P256).
 *
 */
#define DRM_E_P256_HMAC_KEYGEN_FAILURE   ((DRM_RESULT)0x80040E8DL)


/* ============================================================
**
** IContentHeader errors: error codes from DRM_E_CH_BASECODE+0
** to DRM_E_CH_BASECODE+0xFF, 0x80041100-0x800411ff.
**
** ============================================================
*/

#define DRM_E_CH_BASECODE                ((DRM_RESULT)0x80041100L)

/*
 * MessageId: DRM_E_CH_VERSION_MISSING
 *
 * MessageText:
 *
 * Missing content header version.
 *
 */
#define DRM_E_CH_VERSION_MISSING         ((DRM_RESULT)0x80041103L)

/*
 * MessageId: DRM_E_CH_KID_MISSING
 *
 * MessageText:
 *
 * Missing KID attribute in content header.
 *
 */
#define DRM_E_CH_KID_MISSING             ((DRM_RESULT)0x80041104L)

/*
 * MessageId: DRM_E_CH_LAINFO_MISSING
 *
 * MessageText:
 *
 * Missing LAINFO attribute in content header.
 *
 */
#define DRM_E_CH_LAINFO_MISSING          ((DRM_RESULT)0x80041105L)

/*
 * MessageId: DRM_E_CH_CHECKSUM_MISSING
 *
 * MessageText:
 *
 * Missing content header checksum.
 *
 */
#define DRM_E_CH_CHECKSUM_MISSING        ((DRM_RESULT)0x80041106L)

/*
 * MessageId: DRM_E_CH_ATTR_MISSING
 *
 * MessageText:
 *
 * Missing content header attribute.
 *
 */
#define DRM_E_CH_ATTR_MISSING            ((DRM_RESULT)0x80041107L)

/*
 * MessageId: DRM_E_CH_INVALID_HEADER
 *
 * MessageText:
 *
 * Invalid content header.
 *
 */
#define DRM_E_CH_INVALID_HEADER          ((DRM_RESULT)0x80041108L)

/*
 * MessageId: DRM_E_CH_INVALID_CHECKSUM
 *
 * MessageText:
 *
 * Invalid checksum in the header.
 *
 */
#define DRM_E_CH_INVALID_CHECKSUM        ((DRM_RESULT)0x80041109L)

/*
 * MessageId: DRM_E_CH_UNABLE_TO_VERIFY
 *
 * MessageText:
 *
 * Unable to verify signature of content header.
 *
 */
#define DRM_E_CH_UNABLE_TO_VERIFY        ((DRM_RESULT)0x8004110AL)

/*
 * MessageId: DRM_E_CH_UNSUPPORTED_VERSION
 *
 * MessageText:
 *
 * Unsupported content header version.
 *
 */
#define DRM_E_CH_UNSUPPORTED_VERSION     ((DRM_RESULT)0x8004110BL)

/*
 * MessageId: DRM_E_CH_UNSUPPORTED_HASH_ALGORITHM
 *
 * MessageText:
 *
 * Unsupported hash algorithm.
 *
 */
#define DRM_E_CH_UNSUPPORTED_HASH_ALGORITHM ((DRM_RESULT)0x8004110CL)

/*
 * MessageId: DRM_E_CH_UNSUPPORTED_SIGN_ALGORITHM
 *
 * MessageText:
 *
 * Unsupported signature algorithm.
 *
 */
#define DRM_E_CH_UNSUPPORTED_SIGN_ALGORITHM ((DRM_RESULT)0x8004110DL)

/*
 * MessageId: DRM_E_CH_BAD_KEY
 *
 * MessageText:
 *
 * Invalid key.
 *
 */
#define DRM_E_CH_BAD_KEY                 ((DRM_RESULT)0x8004110EL)

/*
 * MessageId: DRM_E_CH_INCOMPATIBLE_HEADER_TYPE
 *
 * MessageText:
 *
 * Incompatible content header type.
 *
 */
#define DRM_E_CH_INCOMPATIBLE_HEADER_TYPE ((DRM_RESULT)0x8004110FL)

/*
 * MessageId: DRM_E_HEADER_ALREADY_SET
 *
 * MessageText:
 *
 * Content header type is already set. Reinitialize is required.
 *
 */
#define DRM_E_HEADER_ALREADY_SET         ((DRM_RESULT)0x80041110L)

/*
 * MessageId: DRM_E_CH_NOT_SIGNED
 *
 * MessageText:
 *
 * The header was not signed.
 *
 */
#define DRM_E_CH_NOT_SIGNED              ((DRM_RESULT)0x80041113L)

/*
 * MessageId: DRM_E_CH_UNKNOWN_ERROR
 *
 * MessageText:
 *
 * Unknown Error.
 *
 */
#define DRM_E_CH_UNKNOWN_ERROR           ((DRM_RESULT)0x80041116L)


/* ============================================================
**
** License parsing results: error codes from 0x80041200-0x800412ff.
**
** ============================================================
*/

#define LIC_BASECODE           ((DRM_RESULT)0x80041200L)

/*
 * MessageId: LIC_INIT_FAILURE
 *
 * MessageText:
 *
 *  LIC_INIT_FAILURE
 *
 */
#define LIC_INIT_FAILURE                 ((DRM_RESULT)0x80041201L)

/*
 * MessageId: LIC_LICENSE_NOTSET
 *
 * MessageText:
 *
 *  LIC_LICENSE_NOTSET
 *
 */
#define LIC_LICENSE_NOTSET               ((DRM_RESULT)0x80041202L)

/*
 * MessageId: LIC_PARAM_NOT_OPTIONAL
 *
 * MessageText:
 *
 *  LIC_PARAM_NOT_OPTIONAL
 *
 */
#define LIC_PARAM_NOT_OPTIONAL           ((DRM_RESULT)0x80041203L)

/*
 * MessageId: LIC_MEMORY_ALLOCATION_ERROR
 *
 * MessageText:
 *
 *  LIC_MEMORY_ALLOCATION_ERROR
 *
 */
#define LIC_MEMORY_ALLOCATION_ERROR      ((DRM_RESULT)0x80041204L)

/*
 * MessageId: LIC_INVALID_LICENSE
 *
 * MessageText:
 *
 *  LIC_INVALID_LICENSE
 *
 */
#define LIC_INVALID_LICENSE              ((DRM_RESULT)0x80041205L)

/*
 * MessageId: LIC_FIELD_MISSING
 *
 * MessageText:
 *
 *  LIC_FIELD_MISSING
 *
 */
#define LIC_FIELD_MISSING                ((DRM_RESULT)0x80041206L)

/*
 * MessageId: LIC_UNSUPPORTED_VALUE
 *
 * MessageText:
 *
 *  LIC_UNSUPPORTED_VALUE
 *
 */
#define LIC_UNSUPPORTED_VALUE            ((DRM_RESULT)0x80041207L)

/*
 * MessageId: LIC_UNKNOWN_ERROR
 *
 * MessageText:
 *
 *  LIC_UNKNOWN_ERROR
 *
 */
#define LIC_UNKNOWN_ERROR                ((DRM_RESULT)0x80041208L)

/*
 * MessageId: LIC_INVALID_REVLIST
 *
 * MessageText:
 *
 *  LIC_INVALID_REVLIST
 *
 */
#define LIC_INVALID_REVLIST              ((DRM_RESULT)0x80041209L)

/*
 * MessageId: LIC_EXPIRED_CERT
 *
 * MessageText:
 *
 *  LIC_EXPIRED_CERT
 *
 */
#define LIC_EXPIRED_CERT                 ((DRM_RESULT)0x8004120AL)


/* ============================================================
**
** Expression evaluator results: error codes from 0x80041400-0x800414ff.
**
** ============================================================
*/

#define CPRMEXP_BASECODE       ((DRM_RESULT)0x80041400L)

/*
 * MessageId: CPRMEXP_NOERROR
 *
 * MessageText:
 *
 *  CPRMEXP_NOERROR
 *
 */
#define CPRMEXP_NOERROR                  ((DRM_RESULT)0x00041400L)

/*
 * MessageId: CPRMEXP_PARAM_NOT_OPTIONAL
 *
 * MessageText:
 *
 *  CPRMEXP_PARAM_NOT_OPTIONAL
 *
 */
#define CPRMEXP_PARAM_NOT_OPTIONAL       ((DRM_RESULT)0x80041401L)

/*
 * MessageId: CPRMEXP_MEMORY_ALLOCATION_ERROR
 *
 * MessageText:
 *
 *  CPRMEXP_MEMORY_ALLOCATION_ERROR
 *
 */
#define CPRMEXP_MEMORY_ALLOCATION_ERROR  ((DRM_RESULT)0x80041402L)

/*
 * MessageId: CPRMEXP_NO_OPERANDS_IN_EXPRESSION
 *
 * MessageText:
 *
 *  CPRMEXP_NO_OPERANDS_IN_EXPRESSION
 *
 */
#define CPRMEXP_NO_OPERANDS_IN_EXPRESSION ((DRM_RESULT)0x80041403L)

/*
 * MessageId: CPRMEXP_INVALID_TOKEN
 *
 * MessageText:
 *
 *  CPRMEXP_INVALID_TOKEN
 *
 */
#define CPRMEXP_INVALID_TOKEN            ((DRM_RESULT)0x80041404L)

/*
 * MessageId: CPRMEXP_INVALID_CONSTANT
 *
 * MessageText:
 *
 *  CPRMEXP_INVALID_CONSTANT
 *
 */
#define CPRMEXP_INVALID_CONSTANT         ((DRM_RESULT)0x80041405L)

/*
 * MessageId: CPRMEXP_INVALID_VARIABLE
 *
 * MessageText:
 *
 *  CPRMEXP_INVALID_VARIABLE
 *
 */
#define CPRMEXP_INVALID_VARIABLE         ((DRM_RESULT)0x80041406L)

/*
 * MessageId: CPRMEXP_INVALID_FUNCTION
 *
 * MessageText:
 *
 *  CPRMEXP_INVALID_FUNCTION
 *
 */
#define CPRMEXP_INVALID_FUNCTION         ((DRM_RESULT)0x80041407L)

/*
 * MessageId: CPRMEXP_INVALID_ARGUMENT
 *
 * MessageText:
 *
 *  CPRMEXP_INVALID_ARGUMENT
 *
 */
#define CPRMEXP_INVALID_ARGUMENT         ((DRM_RESULT)0x80041408L)

/*
 * MessageId: CPRMEXP_INVALID_CONTEXT
 *
 * MessageText:
 *
 *  CPRMEXP_INVALID_CONTEXT
 *
 */
#define CPRMEXP_INVALID_CONTEXT          ((DRM_RESULT)0x80041409L)

/*
 * MessageId: CPRMEXP_ENDOFBUFFER
 *
 * MessageText:
 *
 *  CPRMEXP_ENDOFBUFFER
 *
 */
#define CPRMEXP_ENDOFBUFFER              ((DRM_RESULT)0x8004140AL)

/*
 * MessageId: CPRMEXP_MISSING_OPERAND
 *
 * MessageText:
 *
 *  CPRMEXP_MISSING_OPERAND
 *
 */
#define CPRMEXP_MISSING_OPERAND          ((DRM_RESULT)0x8004140BL)

/*
 * MessageId: CPRMEXP_OVERFLOW
 *
 * MessageText:
 *
 *  CPRMEXP_OVERFLOW
 *
 */
#define CPRMEXP_OVERFLOW                 ((DRM_RESULT)0x8004140CL)

/*
 * MessageId: CPRMEXP_UNDERFLOW
 *
 * MessageText:
 *
 *  CPRMEXP_UNDERFLOW
 *
 */
#define CPRMEXP_UNDERFLOW                ((DRM_RESULT)0x8004140DL)

/*
 * MessageId: CPRMEXP_INCORRECT_NUM_ARGS
 *
 * MessageText:
 *
 *  CPRMEXP_INCORRECT_NUM_ARGS
 *
 */
#define CPRMEXP_INCORRECT_NUM_ARGS       ((DRM_RESULT)0x8004140EL)

/*
 * MessageId: CPRMEXP_VARIABLE_EXPECTED
 *
 * MessageText:
 *
 *  CPRMEXP_VARIABLE_EXPECTED
 *
 */
#define CPRMEXP_VARIABLE_EXPECTED        ((DRM_RESULT)0x8004140FL)

/*
 * MessageId: CPRMEXP_RETRIEVAL_FAILURE
 *
 * MessageText:
 *
 *  CPRMEXP_RETRIEVAL_FAILURE
 *
 */
#define CPRMEXP_RETRIEVAL_FAILURE        ((DRM_RESULT)0x80041410L)

/*
 * MessageId: CPRMEXP_UPDATE_FAILURE
 *
 * MessageText:
 *
 *  CPRMEXP_UPDATE_FAILURE
 *
 */
#define CPRMEXP_UPDATE_FAILURE           ((DRM_RESULT)0x80041411L)

/*
 * MessageId: CPRMEXP_STRING_UNTERMINATED
 *
 * MessageText:
 *
 *  CPRMEXP_STRING_UNTERMINATED
 *
 */
#define CPRMEXP_STRING_UNTERMINATED      ((DRM_RESULT)0x80041412L)

/*
 * MessageId: CPRMEXP_UPDATE_UNSUPPORTED
 *
 * MessageText:
 *
 *  CPRMEXP_UPDATE_UNSUPPORTED
 *
 */
#define CPRMEXP_UPDATE_UNSUPPORTED       ((DRM_RESULT)0x80041413L)

/*
 * MessageId: CPRMEXP_ISOLATED_OPERAND_OR_OPERATOR
 *
 * MessageText:
 *
 *  CPRMEXP_ISOLATED_OPERAND_OR_OPERATOR
 *
 */
#define CPRMEXP_ISOLATED_OPERAND_OR_OPERATOR ((DRM_RESULT)0x80041414L)

/*
 * MessageId: CPRMEXP_UNMATCHED
 *
 * MessageText:
 *
 *  CPRMEXP_UNMATCHED
 *
 */
#define CPRMEXP_UNMATCHED                ((DRM_RESULT)0x80041415L)

/*
 * MessageId: CPRMEXP_WRONG_TYPE_OPERAND
 *
 * MessageText:
 *
 *  CPRMEXP_WRONG_TYPE_OPERAND
 *
 */
#define CPRMEXP_WRONG_TYPE_OPERAND       ((DRM_RESULT)0x80041416L)

/*
 * MessageId: CPRMEXP_TOO_MANY_OPERANDS
 *
 * MessageText:
 *
 *  CPRMEXP_TOO_MANY_OPERANDS
 *
 */
#define CPRMEXP_TOO_MANY_OPERANDS        ((DRM_RESULT)0x80041417L)

/*
 * MessageId: CPRMEXP_UNKNOWN_PARSE_ERROR
 *
 * MessageText:
 *
 *  CPRMEXP_UNKNOWN_PARSE_ERROR
 *
 */
#define CPRMEXP_UNKNOWN_PARSE_ERROR      ((DRM_RESULT)0x80041418L)

/*
 * MessageId: CPRMEXP_UNSUPPORTED_FUNCTION
 *
 * MessageText:
 *
 *  CPRMEXP_UNSUPPORTED_FUNCTION
 *
 */
#define CPRMEXP_UNSUPPORTED_FUNCTION     ((DRM_RESULT)0x80041419L)

/*
 * MessageId: CPRMEXP_CLOCK_REQUIRED
 *
 * MessageText:
 *
 *  CPRMEXP_CLOCK_REQUIRED
 *
 */
#define CPRMEXP_CLOCK_REQUIRED           ((DRM_RESULT)0x8004141AL)


/* ============================================================
**
** Legacy errors: error codes from 0x80048000-0x800480ff.
**
** ============================================================
*/

#define DRM_E_LEGACY_BASECODE                ((DRM_RESULT)0x80048000L)

/*
 * MessageId: DRM_E_LIC_KEY_DECODE_FAILURE
 *
 * MessageText:
 *
 * Key decode failure.
 *
 */
#define DRM_E_LIC_KEY_DECODE_FAILURE     ((DRM_RESULT)0x80048007L)

/*
 * MessageId: DRM_E_LIC_SIGNATURE_FAILURE
 *
 * MessageText:
 *
 * License signature failure.
 *
 */
#define DRM_E_LIC_SIGNATURE_FAILURE      ((DRM_RESULT)0x80048008L)

/*
 * MessageId: DRM_E_LIC_KEY_AND_CERT_MISMATCH
 *
 * MessageText:
 *
 * Key and cert mismatch.
 *
 */
#define DRM_E_LIC_KEY_AND_CERT_MISMATCH  ((DRM_RESULT)0x80048013L)

/*
 * MessageId: DRM_E_KEY_MISMATCH
 *
 * MessageText:
 *
 * A public/private keypair is mismatched.
 *
 */
#define DRM_E_KEY_MISMATCH               ((DRM_RESULT)0x80048014L)

/*
 * MessageId: DRM_E_INVALID_SIGNATURE
 *
 * MessageText:
 *
 * License signature failure.
 *
 */
#define DRM_E_INVALID_SIGNATURE          ((DRM_RESULT)0x800480CFL)

/*
 * MessageId: DRM_E_SYNC_ENTRYNOTFOUND
 *
 * MessageText:
 *
 * An entry was not found in the sync store.
 *
 */
#define DRM_E_SYNC_ENTRYNOTFOUND         ((DRM_RESULT)0x800480D0L)

/*
 * MessageId: DRM_E_STACKTOOSMALL
 *
 * MessageText:
 *
 * The stack supplied to the DRM API was too small.
 *
 */
#define DRM_E_STACKTOOSMALL              ((DRM_RESULT)0x800480D1L)

/*
 * MessageId: DRM_E_CIPHER_NOTINITIALIZED
 *
 * MessageText:
 *
 * The DRM Cipher routines were not correctly initialized before calling encryption/decryption routines.
 *
 */
#define DRM_E_CIPHER_NOTINITIALIZED      ((DRM_RESULT)0x800480D2L)

/*
 * MessageId: DRM_E_DECRYPT_NOTINITIALIZED
 *
 * MessageText:
 *
 * The DRM decrypt routines were not correctly initialized before trying to decrypt data.
 *
 */
#define DRM_E_DECRYPT_NOTINITIALIZED     ((DRM_RESULT)0x800480D3L)

/*
 * MessageId: DRM_E_SECURESTORE_LOCKNOTOBTAINED
 *
 * MessageText:
 *
 * Before reading or writing data to securestore in raw mode, first the lock must be obtained using DRM_SST_OpenData.
 *
 */
#define DRM_E_SECURESTORE_LOCKNOTOBTAINED ((DRM_RESULT)0x800480D4L)

/*
 * MessageId: DRM_E_PKCRYPTO_FAILURE
 *
 * MessageText:
 *
 * An error occurred in an asymmetric cryptographic operation.
 *
 */
#define DRM_E_PKCRYPTO_FAILURE           ((DRM_RESULT)0x800480D5L)

/*
 * MessageId: DRM_E_INVALID_DSTSLOTSIZE
 *
 * MessageText:
 *
 * Invalid DST slot size is specified.
 *
 */
#define DRM_E_INVALID_DSTSLOTSIZE        ((DRM_RESULT)0x800480D6L)


/* ============================================================
**
** DRM utility results: error codes from 0x80049000-0x800490ff.
**
** ============================================================
*/

#define DRMUTIL_BASECODE       ((DRM_RESULT)0x80049000L)

/*
 * MessageId: DRMUTIL_UNSUPPORTED_VERSION
 *
 * MessageText:
 *
 *  DRMUTIL_UNSUPPORTED_VERSION
 *
 */
#define DRMUTIL_UNSUPPORTED_VERSION      ((DRM_RESULT)0x80049005L)

/*
 * MessageId: DRMUTIL_EXPIRED_CERT
 *
 * MessageText:
 *
 *  DRMUTIL_EXPIRED_CERT
 *
 */
#define DRMUTIL_EXPIRED_CERT             ((DRM_RESULT)0x80049006L)

/*
 * MessageId: DRMUTIL_INVALID_CERT
 *
 * MessageText:
 *
 *  DRMUTIL_INVALID_CERT
 *
 */
#define DRMUTIL_INVALID_CERT             ((DRM_RESULT)0x80049007L)


/* ============================================================
**
** PK specific errors (from 0x8004a000 to 0x8004bfff)
**
** ============================================================
*/

/*
 * MessageId: DRM_E_DEVICE_NOT_REGISTERED
 *
 * MessageText:
 *
 * The DEVICEID does not exist in the device store
 *
 */
#define DRM_E_DEVICE_NOT_REGISTERED      ((DRM_RESULT)0x8004A000L)

/*
 * MessageId: DRM_E_TOO_MANY_INCLUSION_GUIDS
 *
 * MessageText:
 *
 * The license contained more than DRM_MAX_INCLUSION_GUIDS entries in its inclusion list
 *
 */
#define DRM_E_TOO_MANY_INCLUSION_GUIDS   ((DRM_RESULT)0x8004A001L)

/*
 * MessageId: DRM_E_REVOCATION_GUID_NOT_RECOGNIZED
 *
 * MessageText:
 *
 * The revocation list type GUID was not recognized
 *
 */
#define DRM_E_REVOCATION_GUID_NOT_RECOGNIZED ((DRM_RESULT)0x8004A002L)

/*
 * MessageId: DRM_E_LIC_CHAIN_TOO_DEEP
 *
 * MessageText:
 *
 * The license chained deeper than this implementation can handle
 *
 */
#define DRM_E_LIC_CHAIN_TOO_DEEP         ((DRM_RESULT)0x8004A003L)

/*
 * MessageId: DRM_E_DEVICE_SECURITY_LEVEL_TOO_LOW
 *
 * MessageText:
 *
 * The security level of the remote device is too low to receive the license
 *
 */
#define DRM_E_DEVICE_SECURITY_LEVEL_TOO_LOW ((DRM_RESULT)0x8004A004L)

/*
 * MessageId: DRM_E_DST_BLOCK_CACHE_CORRUPT
 *
 * MessageText:
 *
 * The block header cache returned invalid data
 *
 */
#define DRM_E_DST_BLOCK_CACHE_CORRUPT    ((DRM_RESULT)0x8004A005L)

/*
 * MessageId: DRM_E_CONTRACT_FAILED
 *
 * MessageText:
 *
 * The error code returned by the API is not present in the contract
 *
 */
#define DRM_E_CONTRACT_FAILED            ((DRM_RESULT)0x8004A006L)

/*
 * MessageId: DRM_E_DST_BLOCK_CACHE_MISS
 *
 * MessageText:
 *
 * The block header cache didn't contain the requested block header
 *
 */
#define DRM_E_DST_BLOCK_CACHE_MISS       ((DRM_RESULT)0x8004A007L)

/*
 * MessageId: DRM_E_INVALID_METERRESPONSE_SIGNATURE
 *
 * MessageText:
 *
 * Invalid signature in meter response
 *
 */
#define DRM_E_INVALID_METERRESPONSE_SIGNATURE ((DRM_RESULT)0x8004A013L)

/*
 * MessageId: DRM_E_INVALID_LICENSE_REVOCATION_LIST_SIGNATURE
 *
 * MessageText:
 *
 * Invalid signature in license revocation list.
 *
 */
#define DRM_E_INVALID_LICENSE_REVOCATION_LIST_SIGNATURE ((DRM_RESULT)0x8004A014L)

/*
 * MessageId: DRM_E_INVALID_METERCERT_SIGNATURE
 *
 * MessageText:
 *
 * Invalid signature in metering certificate
 *
 */
#define DRM_E_INVALID_METERCERT_SIGNATURE ((DRM_RESULT)0x8004A015L)

/*
 * MessageId: DRM_E_METERSTORE_DATA_NOT_FOUND
 *
 * MessageText:
 *
 * Metering data slot not found due to bad data in response file
 *
 */
#define DRM_E_METERSTORE_DATA_NOT_FOUND  ((DRM_RESULT)0x8004A016L)

/*
 * MessageId: DRM_E_NO_LICENSES_TO_SYNC
 *
 * MessageText:
 *
 * No more licenses to sync
 *
 */
#define DRM_E_NO_LICENSES_TO_SYNC        ((DRM_RESULT)0x8004A017L)

/*
 * MessageId: DRM_E_INVALID_REVOCATION_LIST
 *
 * MessageText:
 *
 * The revocation list version does not match the current revocation version
 *
 */
#define DRM_E_INVALID_REVOCATION_LIST    ((DRM_RESULT)0x8004A018L)

/*
 * MessageId: DRM_E_ENVELOPE_CORRUPT
 *
 * MessageText:
 *
 * The envelope archive or file is corrupt
 *
 */
#define DRM_E_ENVELOPE_CORRUPT           ((DRM_RESULT)0x8004A019L)

/*
 * MessageId: DRM_E_ENVELOPE_FILE_NOT_COMPATIBLE
 *
 * MessageText:
 *
 * The envelope file is not compatible with this version of the porting kit
 *
 */
#define DRM_E_ENVELOPE_FILE_NOT_COMPATIBLE ((DRM_RESULT)0x8004A01AL)

/*
 * MessageId: DRM_E_EXTENDED_RESTRICTION_NOT_UNDERSTOOD
 *
 * MessageText:
 *
 * An extensible restriction was not understood by the app, and is mark as being required
 *
 */
#define DRM_E_EXTENDED_RESTRICTION_NOT_UNDERSTOOD ((DRM_RESULT)0x8004A01BL)

/*
 * MessageId: DRM_E_INVALID_SLK
 *
 * MessageText:
 *
 * An ILA SLK (symmetric session key) was found, but did not contain valid data
 *
 */
#define DRM_E_INVALID_SLK                ((DRM_RESULT)0x8004A01CL)

/*
 * MessageId: DRM_E_DEVCERT_MODEL_MISMATCH
 *
 * MessageText:
 *
 * The model string in the certificate does not match the model of the device and so the cert must be re-generated.
 *
 */
#define DRM_E_DEVCERT_MODEL_MISMATCH     ((DRM_RESULT)0x8004A01DL)


/* ============================================================
**
** Drm Core errors (from 0x8004c000 to 0x8004dfff)
**
** ============================================================
*/

/*
 * MessageId: DRM_E_DEVICENOTINIT
 *
 * MessageText:
 *
 * This device has not been initialized against a DRM init service
 *
 */
#define DRM_E_DEVICENOTINIT              ((DRM_RESULT)0x8004C001L)

/*
 * MessageId: DRM_E_DRMNOTINIT
 *
 * MessageText:
 *
 * The app has not call DRM_Init properly
 *
 */
#define DRM_E_DRMNOTINIT                 ((DRM_RESULT)0x8004C002L)

/*
 * MessageId: DRM_E_INVALIDRIGHT
 *
 * MessageText:
 *
 * A right in the license in invalid
 *
 */
#define DRM_E_INVALIDRIGHT               ((DRM_RESULT)0x8004C003L)

/*
 * MessageId: DRM_E_INCOMPATABLELICENSESIZE
 *
 * MessageText:
 *
 * The size of the license is incompatable. DRM doesn't understand this license
 *
 */
#define DRM_E_INCOMPATABLELICENSESIZE    ((DRM_RESULT)0x8004C004L)

/*
 * MessageId: DRM_E_INVALIDLICENSEFLAGS
 *
 * MessageText:
 *
 * The flags in the license are invalid. DRM either doesn't understand them or they are conflicting
 *
 */
#define DRM_E_INVALIDLICENSEFLAGS        ((DRM_RESULT)0x8004C005L)

/*
 * MessageId: DRM_E_INVALIDLICENSE
 *
 * MessageText:
 *
 * The license is invalid
 *
 */
#define DRM_E_INVALIDLICENSE             ((DRM_RESULT)0x8004C006L)

/*
 * MessageId: DRM_E_CONDITIONFAIL
 *
 * MessageText:
 *
 * A condition in the license failed to pass
 *
 */
#define DRM_E_CONDITIONFAIL              ((DRM_RESULT)0x8004C007L)

/*
 * MessageId: DRM_E_CONDITIONNOTSUPPORTED
 *
 * MessageText:
 *
 * A condition in the license is not supported by this verison of DRM
 *
 */
#define DRM_E_CONDITIONNOTSUPPORTED      ((DRM_RESULT)0x8004C008L)

/*
 * MessageId: DRM_E_LICENSEEXPIRED
 *
 * MessageText:
 *
 * The license has expired either by depleting a play count or via an end time.
 *
 */
#define DRM_E_LICENSEEXPIRED             ((DRM_RESULT)0x8004C009L)

/*
 * MessageId: DRM_E_LICENSENOTYETVALID
 *
 * MessageText:
 *
 * The license start time had not come to pass yet.
 *
 */
#define DRM_E_LICENSENOTYETVALID         ((DRM_RESULT)0x8004C00AL)

/*
 * MessageId: DRM_E_RIGHTSNOTAVAILABLE
 *
 * MessageText:
 *
 * The rights the app has requested are not available in the license
 *
 */
#define DRM_E_RIGHTSNOTAVAILABLE         ((DRM_RESULT)0x8004C00BL)

/*
 * MessageId: DRM_E_LICENSEMISMATCH
 *
 * MessageText:
 *
 * The license content id/ sku id doesn't match that requested by the app
 *
 */
#define DRM_E_LICENSEMISMATCH            ((DRM_RESULT)0x8004C00CL)

/*
 * MessageId: DRM_E_WRONGTOKENTYPE
 *
 * MessageText:
 *
 * The token parameter was of an incompatible type.
 *
 */
#define DRM_E_WRONGTOKENTYPE             ((DRM_RESULT)0x8004C00DL)

/*
 * MessageId: DRM_E_NORIGHTSREQUESTED
 *
 * MessageText:
 *
 * The app has not requested any rights before trying to bind
 *
 */
#define DRM_E_NORIGHTSREQUESTED          ((DRM_RESULT)0x8004C00EL)

/*
 * MessageId: DRM_E_LICENSENOTBOUND
 *
 * MessageText:
 *
 * A license has not been bound to. Decrypt can not happen without a successful bind call
 *
 */
#define DRM_E_LICENSENOTBOUND            ((DRM_RESULT)0x8004C00FL)

/*
 * MessageId: DRM_E_HASHMISMATCH
 *
 * MessageText:
 *
 * A Keyed Hash check failed.
 *
 */
#define DRM_E_HASHMISMATCH               ((DRM_RESULT)0x8004C010L)

/*
 * MessageId: DRM_E_INVALIDTIME
 *
 * MessageText:
 *
 * A time structure is invalid.
 *
 */
#define DRM_E_INVALIDTIME                ((DRM_RESULT)0x8004C011L)

/*
 * MessageId: DRM_E_LICENSESTORENOTFOUND
 *
 * MessageText:
 *
 * The external license store was not found.
 *
 */
#define DRM_E_LICENSESTORENOTFOUND       ((DRM_RESULT)0x8004C012L)

/*
 * MessageId: DRM_E_LICENSENOTFOUND
 *
 * MessageText:
 *
 * A license was not found in the license store.
 *
 */
#define DRM_E_LICENSENOTFOUND            ((DRM_RESULT)0x8004C013L)

/*
 * MessageId: DRM_E_LICENSEVERSIONNOTSUPPORTED
 *
 * MessageText:
 *
 * The DRM license version is not supported by the DRM version on the device.
 *
 */
#define DRM_E_LICENSEVERSIONNOTSUPPORTED ((DRM_RESULT)0x8004C014L)

/*
 * MessageId: DRM_E_INVALIDBINDID
 *
 * MessageText:
 *
 * The bind id is invalid.
 *
 */
#define DRM_E_INVALIDBINDID              ((DRM_RESULT)0x8004C015L)

/*
 * MessageId: DRM_E_UNSUPPORTEDALGORITHM
 *
 * MessageText:
 *
 * The encryption algorithm required for this operation is not supported.
 *
 */
#define DRM_E_UNSUPPORTEDALGORITHM       ((DRM_RESULT)0x8004C016L)

/*
 * MessageId: DRM_E_ALGORITHMNOTSET
 *
 * MessageText:
 *
 * The encryption algorithm required for this operation is not supported.
 *
 */
#define DRM_E_ALGORITHMNOTSET            ((DRM_RESULT)0x8004C017L)

/*
 * MessageId: DRM_E_LICENSESERVERNEEDSKEY
 *
 * MessageText:
 *
 * The license server needs a version of the device bind key from the init service.
 *
 */
#define DRM_E_LICENSESERVERNEEDSKEY      ((DRM_RESULT)0x8004C018L)

/*
 * MessageId: DRM_E_INVALIDLICENSESTORE
 *
 * MessageText:
 *
 * The license store version number is incorrect, or the store is invalid in some other way.
 *
 */
#define DRM_E_INVALIDLICENSESTORE        ((DRM_RESULT)0x8004C019L)

/*
 * MessageId: DRM_E_FILEREADERROR
 *
 * MessageText:
 *
 * There was an error reading a file.
 *
 */
#define DRM_E_FILEREADERROR              ((DRM_RESULT)0x8004C01AL)

/*
 * MessageId: DRM_E_FILEWRITEERROR
 *
 * MessageText:
 *
 * There was an error writing a file.
 *
 */
#define DRM_E_FILEWRITEERROR             ((DRM_RESULT)0x8004C01BL)

/*
 * MessageId: DRM_E_CLIENTTIMEINVALID
 *
 * MessageText:
 *
 * The time/clock on the device is not in sync with the license server within tolerance.
 *
 */
#define DRM_E_CLIENTTIMEINVALID          ((DRM_RESULT)0x8004C01CL)

/*
 * MessageId: DRM_E_DSTSTOREFULL
 *
 * MessageText:
 *
 * The data store is full.
 *
 */
#define DRM_E_DSTSTOREFULL               ((DRM_RESULT)0x8004C01DL)

/*
 * MessageId: DRM_E_NOXMLOPENTAG
 *
 * MessageText:
 *
 * XML open tag not found
 *
 */
#define DRM_E_NOXMLOPENTAG               ((DRM_RESULT)0x8004C01EL)

/*
 * MessageId: DRM_E_NOXMLCLOSETAG
 *
 * MessageText:
 *
 * XML close tag not found
 *
 */
#define DRM_E_NOXMLCLOSETAG              ((DRM_RESULT)0x8004C01FL)

/*
 * MessageId: DRM_E_INVALIDXMLTAG
 *
 * MessageText:
 *
 * Invalid XML tag
 *
 */
#define DRM_E_INVALIDXMLTAG              ((DRM_RESULT)0x8004C020L)

/*
 * MessageId: DRM_E_NOXMLCDATA
 *
 * MessageText:
 *
 * No XML CDATA found
 *
 */
#define DRM_E_NOXMLCDATA                 ((DRM_RESULT)0x8004C021L)

/*
 * MessageId: DRM_E_DSTNAMESPACEFULL
 *
 * MessageText:
 *
 * No more room for DST Namespace
 *
 */
#define DRM_E_DSTNAMESPACEFULL           ((DRM_RESULT)0x8004C022L)

/*
 * MessageId: DRM_E_DSTNAMESPACENOTFOUND
 *
 * MessageText:
 *
 * No DST Namespace found
 *
 */
#define DRM_E_DSTNAMESPACENOTFOUND       ((DRM_RESULT)0x8004C023L)

/*
 * MessageId: DRM_E_DSTSLOTNOTFOUND
 *
 * MessageText:
 *
 * DST Dataslot not found
 *
 */
#define DRM_E_DSTSLOTNOTFOUND            ((DRM_RESULT)0x8004C024L)

/*
 * MessageId: DRM_E_DSTSLOTEXIST
 *
 * MessageText:
 *
 * DST Dataslot already exists
 *
 */
#define DRM_E_DSTSLOTEXIST               ((DRM_RESULT)0x8004C025L)

/*
 * MessageId: DRM_E_DSTCORRUPTED
 *
 * MessageText:
 *
 * The data store is corrupted
 *
 */
#define DRM_E_DSTCORRUPTED               ((DRM_RESULT)0x8004C026L)

/*
 * MessageId: DRM_E_DSTSEEKERROR
 *
 * MessageText:
 *
 * There was an error attempting to seek in the Data Store
 *
 */
#define DRM_E_DSTSEEKERROR               ((DRM_RESULT)0x8004C027L)

/*
 * MessageId: DRM_E_DSTNAMESPACEINUSE
 *
 * MessageText:
 *
 * No DST Namespace in use
 *
 */
#define DRM_E_DSTNAMESPACEINUSE          ((DRM_RESULT)0x8004C028L)

/*
 * MessageId: DRM_E_INVALID_SECURESTORE_PASSWORD
 *
 * MessageText:
 *
 * The password used to open the secure store key was not able to validate the secure store hash.
 *
 */
#define DRM_E_INVALID_SECURESTORE_PASSWORD ((DRM_RESULT)0x8004C029L)

/*
 * MessageId: DRM_E_SECURESTORE_CORRUPT
 *
 * MessageText:
 *
 * The secure store is corrupt
 *
 */
#define DRM_E_SECURESTORE_CORRUPT        ((DRM_RESULT)0x8004C02AL)

/*
 * MessageId: DRM_E_SECURESTORE_FULL
 *
 * MessageText:
 *
 * The current secure store key is full. No more data can be added.
 *
 */
#define DRM_E_SECURESTORE_FULL           ((DRM_RESULT)0x8004C02BL)

/*
 * MessageId: DRM_E_NOACTIONINLICENSEREQUEST
 *
 * MessageText:
 *
 * No action(s) added for license request
 *
 */
#define DRM_E_NOACTIONINLICENSEREQUEST   ((DRM_RESULT)0x8004C02CL)

/*
 * MessageId: DRM_E_DUPLICATEDHEADERATTRIBUTE
 *
 * MessageText:
 *
 * Duplicated attribute in Header
 *
 */
#define DRM_E_DUPLICATEDHEADERATTRIBUTE  ((DRM_RESULT)0x8004C02DL)

/*
 * MessageId: DRM_E_NOKIDINHEADER
 *
 * MessageText:
 *
 * No KID attribute in Header
 *
 */
#define DRM_E_NOKIDINHEADER              ((DRM_RESULT)0x8004C02EL)

/*
 * MessageId: DRM_E_NOLAINFOINHEADER
 *
 * MessageText:
 *
 * No LAINFO attribute in Header
 *
 */
#define DRM_E_NOLAINFOINHEADER           ((DRM_RESULT)0x8004C02FL)

/*
 * MessageId: DRM_E_NOCHECKSUMINHEADER
 *
 * MessageText:
 *
 * No Checksum attribute in Header
 *
 */
#define DRM_E_NOCHECKSUMINHEADER         ((DRM_RESULT)0x8004C030L)

/*
 * MessageId: DRM_E_DSTBLOCKMISMATCH
 *
 * MessageText:
 *
 * DST block mismatch
 *
 */
#define DRM_E_DSTBLOCKMISMATCH           ((DRM_RESULT)0x8004C031L)

/*
 * MessageId: DRM_E_BACKUP_EXISTS
 *
 * MessageText:
 *
 * Backup file already exist.
 *
 */
#define DRM_E_BACKUP_EXISTS              ((DRM_RESULT)0x8004C032L)

/*
 * MessageId: DRM_E_LICENSE_TOOLONG
 *
 * MessageText:
 *
 * License size is too long
 *
 */
#define DRM_E_LICENSE_TOOLONG            ((DRM_RESULT)0x8004C033L)

/*
 * MessageId: DRM_E_DSTEXISTS
 *
 * MessageText:
 *
 * A DST already exists in the specified location
 *
 */
#define DRM_E_DSTEXISTS                  ((DRM_RESULT)0x8004C034L)

/*
 * MessageId: DRM_E_INVALIDDEVICECERTIFICATE
 *
 * MessageText:
 *
 * The device certificate is invalid.
 *
 */
#define DRM_E_INVALIDDEVICECERTIFICATE   ((DRM_RESULT)0x8004C035L)

/*
 * MessageId: DRM_E_DSTLOCKFAILED
 *
 * MessageText:
 *
 * Locking a segment of the DST failed.
 *
 */
#define DRM_E_DSTLOCKFAILED              ((DRM_RESULT)0x8004C036L)

/*
 * MessageId: DRM_E_FILESEEKERROR
 *
 * MessageText:
 *
 * File Seek Error
 *
 */
#define DRM_E_FILESEEKERROR              ((DRM_RESULT)0x8004C037L)

/*
 * MessageId: DRM_E_DSTNOTLOCKEDEXCLUSIVE
 *
 * MessageText:
 *
 * Existing lock is not exclusive
 *
 */
#define DRM_E_DSTNOTLOCKEDEXCLUSIVE      ((DRM_RESULT)0x8004C038L)

/*
 * MessageId: DRM_E_DSTEXCLUSIVELOCKONLY
 *
 * MessageText:
 *
 * Only exclusive lock is accepted
 *
 */
#define DRM_E_DSTEXCLUSIVELOCKONLY       ((DRM_RESULT)0x8004C039L)

/*
 * MessageId: DRM_E_DSTRESERVEDKEYDETECTED
 *
 * MessageText:
 *
 * DST reserved key value detected in UniqueKey
 *
 */
#define DRM_E_DSTRESERVEDKEYDETECTED     ((DRM_RESULT)0x8004C03AL)

/*
 * MessageId: DRM_E_V1_NOT_SUPPORTED
 *
 * MessageText:
 *
 * V1 Lic Acquisition is not supported
 *
 */
#define DRM_E_V1_NOT_SUPPORTED           ((DRM_RESULT)0x8004C03BL)

/*
 * MessageId: DRM_E_HEADER_NOT_SET
 *
 * MessageText:
 *
 * Content header is not set
 *
 */
#define DRM_E_HEADER_NOT_SET             ((DRM_RESULT)0x8004C03CL)

/*
 * MessageId: DRM_E_NEEDDEVCERTINDIV
 *
 * MessageText:
 *
 * The device certificate is template. It need Devcert Indiv
 *
 */
#define DRM_E_NEEDDEVCERTINDIV           ((DRM_RESULT)0x8004C03DL)

/*
 * MessageId: DRM_E_MACHINEIDMISMATCH
 *
 * MessageText:
 *
 * The device has Machine Id different from that in devcert.
 *
 */
#define DRM_E_MACHINEIDMISMATCH          ((DRM_RESULT)0x8004C03EL)

/*
 * MessageId: DRM_E_CLK_INVALID_RESPONSE
 *
 * MessageText:
 *
 * The secure clock response is invalid.
 *
 */
#define DRM_E_CLK_INVALID_RESPONSE       ((DRM_RESULT)0x8004C03FL)

/*
 * MessageId: DRM_E_CLK_INVALID_DATE
 *
 * MessageText:
 *
 * The secure clock response is invalid.
 *
 */
#define DRM_E_CLK_INVALID_DATE           ((DRM_RESULT)0x8004C040L)

/*
 * MessageId: DRM_E_CLK_UNSUPPORTED_VALUE
 *
 * MessageText:
 *
 * The secure clock response has unsupported value.
 *
 */
#define DRM_E_CLK_UNSUPPORTED_VALUE      ((DRM_RESULT)0x8004C041L)

/*
 * MessageId: DRM_E_INVALIDDEVICECERTIFICATETEMPLATE
 *
 * MessageText:
 *
 * The device certificate is invalid.
 *
 */
#define DRM_E_INVALIDDEVICECERTIFICATETEMPLATE ((DRM_RESULT)0x8004C042L)

/*
 * MessageId: DRM_E_DEVCERTEXCEEDSIZELIMIT
 *
 * MessageText:
 *
 * The device certificate exceeds max size
 *
 */
#define DRM_E_DEVCERTEXCEEDSIZELIMIT     ((DRM_RESULT)0x8004C043L)

/*
 * MessageId: DRM_E_DEVCERTTEMPLATEEXCEEDSSIZELIMIT
 *
 * MessageText:
 *
 * The device certificate template exceeds max size
 *
 */
#define DRM_E_DEVCERTTEMPLATEEXCEEDSSIZELIMIT ((DRM_RESULT)0x8004C044L)

/*
 * MessageId: DRM_E_DEVCERTREADERROR
 *
 * MessageText:
 *
 * Can't get the device certificate
 *
 */
#define DRM_E_DEVCERTREADERROR           ((DRM_RESULT)0x8004C045L)

/*
 * MessageId: DRM_E_DEVCERTWRITEERROR
 *
 * MessageText:
 *
 * Can't store the device certificate
 *
 */
#define DRM_E_DEVCERTWRITEERROR          ((DRM_RESULT)0x8004C046L)

/*
 * MessageId: DRM_E_PRIVKEYREADERROR
 *
 * MessageText:
 *
 * Can't get device private key
 *
 */
#define DRM_E_PRIVKEYREADERROR           ((DRM_RESULT)0x8004C047L)

/*
 * MessageId: DRM_E_PRIVKEYWRITEERROR
 *
 * MessageText:
 *
 * Can't store device private key
 *
 */
#define DRM_E_PRIVKEYWRITEERROR          ((DRM_RESULT)0x8004C048L)

/*
 * MessageId: DRM_E_DEVCERTTEMPLATEREADERROR
 *
 * MessageText:
 *
 * Can't get the device certificate template
 *
 */
#define DRM_E_DEVCERTTEMPLATEREADERROR   ((DRM_RESULT)0x8004C049L)

/*
 * MessageId: DRM_E_CLK_NOT_SUPPORTED
 *
 * MessageText:
 *
 * The secure clock is not supported.
 *
 */
#define DRM_E_CLK_NOT_SUPPORTED          ((DRM_RESULT)0x8004C04AL)

/*
 * MessageId: DRM_E_DEVCERTINDIV_NOT_SUPPORTED
 *
 * MessageText:
 *
 * The Devcert Indiv is not supported.
 *
 */
#define DRM_E_DEVCERTINDIV_NOT_SUPPORTED ((DRM_RESULT)0x8004C04BL)

/*
 * MessageId: DRM_E_METERING_NOT_SUPPORTED
 *
 * MessageText:
 *
 * The Metering is not supported.
 *
 */
#define DRM_E_METERING_NOT_SUPPORTED     ((DRM_RESULT)0x8004C04CL)

/*
 * MessageId: DRM_E_CLK_RESETSTATEREADERROR
 *
 * MessageText:
 *
 * Can not read Secure clock Reset State.
 *
 */
#define DRM_E_CLK_RESETSTATEREADERROR    ((DRM_RESULT)0x8004C04DL)

/*
 * MessageId: DRM_E_CLK_RESETSTATEWRITEERROR
 *
 * MessageText:
 *
 * Can not write Secure clock Reset State.
 *
 */
#define DRM_E_CLK_RESETSTATEWRITEERROR   ((DRM_RESULT)0x8004C04EL)

/*
 * MessageId: DRM_E_XMLNOTFOUND
 *
 * MessageText:
 *
 * a required XML tag was not found
 *
 */
#define DRM_E_XMLNOTFOUND                ((DRM_RESULT)0x8004C04FL)

/*
 * MessageId: DRM_E_METERING_WRONG_TID
 *
 * MessageText:
 *
 * wrong TID sent on metering response
 *
 */
#define DRM_E_METERING_WRONG_TID         ((DRM_RESULT)0x8004C050L)

/*
 * MessageId: DRM_E_METERING_INVALID_COMMAND
 *
 * MessageText:
 *
 * wrong command sent on metering response
 *
 */
#define DRM_E_METERING_INVALID_COMMAND   ((DRM_RESULT)0x8004C051L)

/*
 * MessageId: DRM_E_METERING_STORE_CORRUPT
 *
 * MessageText:
 *
 * The metering store is corrupt
 *
 */
#define DRM_E_METERING_STORE_CORRUPT     ((DRM_RESULT)0x8004C052L)

/*
 * MessageId: DRM_E_CERTIFICATE_REVOKED
 *
 * MessageText:
 *
 * A certificate given to DRM was revoked.
 *
 */
#define DRM_E_CERTIFICATE_REVOKED        ((DRM_RESULT)0x8004C053L)

/*
 * MessageId: DRM_E_CRYPTO_FAILED
 *
 * MessageText:
 *
 * A cryptographic operation failed.
 *
 */
#define DRM_E_CRYPTO_FAILED              ((DRM_RESULT)0x8004C054L)

/*
 * MessageId: DRM_E_STACK_CORRUPT
 *
 * MessageText:
 *
 * The stack allocator context is corrupt. Likely a buffer overrun problem.
 *
 */
#define DRM_E_STACK_CORRUPT              ((DRM_RESULT)0x8004C055L)

/*
 * MessageId: DRM_E_UNKNOWN_BINDING_KEY
 *
 * MessageText:
 *
 * A matching binding key could not be found for the license.
 *
 */
#define DRM_E_UNKNOWN_BINDING_KEY        ((DRM_RESULT)0x8004C056L)

/*
 * MessageId: DRM_E_V1_LICENSE_CHAIN_NOT_SUPPORTED
 *
 * MessageText:
 *
 * License chaining with V1 content is not supported.
 *
 */
#define DRM_E_V1_LICENSE_CHAIN_NOT_SUPPORTED ((DRM_RESULT)0x8004C057L)

/*
 * MessageId: DRM_E_WRONG_TOKEN_TYPE
 *
 * MessageText:
 *
 * The wrong type of token was used.
 *
 */
#define DRM_E_WRONG_TOKEN_TYPE           ((DRM_RESULT)0x8004C058L)

/*
 * MessageId: DRM_E_POLICY_METERING_DISABLED
 *
 * MessageText:
 *
 * Metering code was called but metering is disabled by group or user policy
 *
 */
#define DRM_E_POLICY_METERING_DISABLED   ((DRM_RESULT)0x8004C059L)

/*
 * MessageId: DRM_E_POLICY_ONLINE_DISABLED
 *
 * MessageText:
 *
 * online communication is disabled by group policy
 *
 */
#define DRM_E_POLICY_ONLINE_DISABLED     ((DRM_RESULT)0x8004C05AL)

/*
 * MessageId: DRM_E_CLK_NOT_SET
 *
 * MessageText:
 *
 * License may be there but can not be used as secure clock not set.
 *
 */
#define DRM_E_CLK_NOT_SET                ((DRM_RESULT)0x8004C05BL)

/*
 * MessageId: DRM_E_NO_CLK_SUPPORTED
 *
 * MessageText:
 *
 * This device does not support any Clock. So time bound licenses can not be played
 *
 */
#define DRM_E_NO_CLK_SUPPORTED           ((DRM_RESULT)0x8004C05CL)

/*
 * MessageId: DRM_E_NO_URL
 *
 * MessageText:
 *
 * Can not find URL info.
 *
 */
#define DRM_E_NO_URL                     ((DRM_RESULT)0x8004C05DL)

/*
 * MessageId: DRM_E_UNKNOWN_DEVICE_PROPERTY
 *
 * MessageText:
 *
 * Unknown device property.
 *
 */
#define DRM_E_UNKNOWN_DEVICE_PROPERTY    ((DRM_RESULT)0x8004C05EL)

/*
 * MessageId: DRM_E_METERING_MID_MISMATCH
 *
 * MessageText:
 *
 * The metering ID is not same in Metering Cert and metering response data
 *
 */
#define DRM_E_METERING_MID_MISMATCH      ((DRM_RESULT)0x8004C05FL)

/*
 * MessageId: DRM_E_METERING_RESPONSE_DECRYPT_FAILED
 *
 * MessageText:
 *
 * The encrypted section of metering response can not be decrypted
 *
 */
#define DRM_E_METERING_RESPONSE_DECRYPT_FAILED ((DRM_RESULT)0x8004C060L)

/*
 * MessageId: DRM_E_RIV_TOO_SMALL
 *
 * MessageText:
 *
 * RIV on the machine is too small.
 *
 */
#define DRM_E_RIV_TOO_SMALL              ((DRM_RESULT)0x8004C063L)

/*
 * MessageId: DRM_E_STACK_ALREADY_INITIALIZED
 *
 * MessageText:
 *
 * DRM_STK_Init called for initialized stack
 *
 */
#define DRM_E_STACK_ALREADY_INITIALIZED  ((DRM_RESULT)0x8004C064L)

/*
 * MessageId: DRM_E_DEVCERT_REVOKED
 *
 * MessageText:
 *
 * The device certificate given to DRM is revoked.
 *
 */
#define DRM_E_DEVCERT_REVOKED            ((DRM_RESULT)0x8004C065L)

/*
 * MessageId: DRM_E_OEM_RSA_DECRYPTION_ERROR
 *
 * MessageText:
 *
 * Error in OEM RSA Decryption.
 *
 */
#define DRM_E_OEM_RSA_DECRYPTION_ERROR   ((DRM_RESULT)0x8004C066L)

/*
 * MessageId: DRM_E_INVALID_DEVSTORE_ATTRIBUTE
 *
 * MessageText:
 *
 * Invalid device attributes in the device store
 *
 */
#define DRM_E_INVALID_DEVSTORE_ATTRIBUTE ((DRM_RESULT)0x8004C067L)

/*
 * MessageId: DRM_E_INVALID_DEVSTORE_ENTRY
 *
 * MessageText:
 *
 * The device store data entry is corrupted
 *
 */
#define DRM_E_INVALID_DEVSTORE_ENTRY     ((DRM_RESULT)0x8004C068L)

/*
 * MessageId: DRM_E_OEM_RSA_ENCRYPTION_ERROR
 *
 * MessageText:
 *
 * Error in OEM RSA Encryption process
 *
 */
#define DRM_E_OEM_RSA_ENCRYPTION_ERROR   ((DRM_RESULT)0x8004C069L)

/*
 * MessageId: DRM_E_DSTNAMESPACEEXIST
 *
 * MessageText:
 *
 * The DST Namespace already exists.
 *
 */
#define DRM_E_DSTNAMESPACEEXIST          ((DRM_RESULT)0x8004C06AL)

/*
 * MessageId: DRM_E_PERF_SCOPING_ERROR
 *
 * MessageText:
 *
 * Error in performance scope context
 *
 */
#define DRM_E_PERF_SCOPING_ERROR         ((DRM_RESULT)0x8004C06BL)

/*
 * MessageId: DRM_E_PRECISION_ARITHMETIC_FAIL
 *
 * MessageText:
 *
 * Operation involving multiple precision arithmetic fails
 *
 */
#define DRM_E_PRECISION_ARITHMETIC_FAIL  ((DRM_RESULT)0x8004C06CL)

/*
 * MessageId: DRM_E_OEM_RSA_INVALID_PRIVATE_KEY
 *
 * MessageText:
 *
 * Invalid private key.
 *
 */
#define DRM_E_OEM_RSA_INVALID_PRIVATE_KEY ((DRM_RESULT)0x8004C06DL)

/*
 * MessageId: DRM_E_NO_OPL_CALLBACK
 *
 * MessageText:
 *
 * There is no callback function to process the output restrictions specified in the license
 *
 */
#define DRM_E_NO_OPL_CALLBACK            ((DRM_RESULT)0x8004C06EL)

/*
 * MessageId: DRM_E_INVALID_PLAYREADY_OBJECT
 *
 * MessageText:
 *
 * Structure of PlayReady object is invalid
 *
 */
#define DRM_E_INVALID_PLAYREADY_OBJECT   ((DRM_RESULT)0x8004C06FL)

/*
 * MessageId: DRM_E_DUPLICATE_LICENSE
 *
 * MessageText:
 *
 * There is already a license in the store with the same KID & LID
 *
 */
#define DRM_E_DUPLICATE_LICENSE          ((DRM_RESULT)0x8004C070L)

/*
 * MessageId: DRM_E_REVOCATION_NOT_SUPPORTED
 *
 * MessageText:
 *
 * Device does not support revocation, while revocation data was placed into license policy structure.
 *
 */
#define DRM_E_REVOCATION_NOT_SUPPORTED   ((DRM_RESULT)0x8004C071L)

/*
 * MessageId: DRM_E_RECORD_NOT_FOUND
 *
 * MessageText:
 *
 * Record with requested type was not found in PlayReady object.
 *
 */
#define DRM_E_RECORD_NOT_FOUND           ((DRM_RESULT)0x8004C072L)

/*
 * MessageId: DRM_E_BUFFER_BOUNDS_EXCEEDED
 *
 * MessageText:
 *
 * An array is being referenced outside of it's bounds.
 *
 */
#define DRM_E_BUFFER_BOUNDS_EXCEEDED     ((DRM_RESULT)0x8004C073L)

/*
 * MessageId: DRM_E_INVALID_BASE64
 *
 * MessageText:
 *
 * An input string contains invalid Base64 characters.
 *
 */
#define DRM_E_INVALID_BASE64             ((DRM_RESULT)0x8004C074L)

/*
 * MessageId: DRM_E_PROTOCOL_VERSION_NOT_SUPPORTED
 *
 * MessageText:
 *
 * The protocol version is not supported.
 *
 */
#define DRM_E_PROTOCOL_VERSION_NOT_SUPPORTED ((DRM_RESULT)0x8004C075L)

/*
 * MessageId: DRM_E_INVALID_LICENSE_RESPONSE_SIGNATURE
 *
 * MessageText:
 *
 * Cannot verify license acquisition's response because signature is invalid.
 *
 */
#define DRM_E_INVALID_LICENSE_RESPONSE_SIGNATURE ((DRM_RESULT)0x8004C076L)

/*
 * MessageId: DRM_E_INVALID_LICENSE_RESPONSE_RESPONSEID
 *
 * MessageText:
 *
 * Cannot verify license acquisition's response because response ID is invalid.
 *
 */
#define DRM_E_INVALID_LICENSE_RESPONSE_RESPONSEID ((DRM_RESULT)0x8004C077L)

/*
 * MessageId: DRM_E_LICENSE_RESPONSE_SIGNATURE_MISSING
 *
 * MessageText:
 *
 * Cannot verify license acquisition's response because either response ID, license nonce or signature is missing.
 *
 */
#define DRM_E_LICENSE_RESPONSE_SIGNATURE_MISSING ((DRM_RESULT)0x8004C078L)

/*
 * MessageId: DRM_E_INVALID_DOMAIN_JOIN_RESPONSE_SIGNATURE
 *
 * MessageText:
 *
 * Cannot verify domain join response because signature is invalid.
 *
 */
#define DRM_E_INVALID_DOMAIN_JOIN_RESPONSE_SIGNATURE ((DRM_RESULT)0x8004C079L)

/*
 * MessageId: DRM_E_DOMAIN_JOIN_RESPONSE_SIGNATURE_MISSING
 *
 * MessageText:
 *
 * Cannot verify domain join response because either signing certificate chain or signature is missing.
 *
 */
#define DRM_E_DOMAIN_JOIN_RESPONSE_SIGNATURE_MISSING ((DRM_RESULT)0x8004C07AL)

/*
 * MessageId: DRM_E_ACTIVATION_REQUIRED
 *
 * MessageText:
 *
 * The device must be activated before initialization can succeed.
 *
 */
#define DRM_E_ACTIVATION_REQUIRED        ((DRM_RESULT)0x8004C07BL)

/*
 * MessageId: DRM_E_ACTIVATION_INTERNAL_ERROR
 *
 * MessageText:
 *
 * A server error occurred during device activation.
 *
 */
#define DRM_E_ACTIVATION_INTERNAL_ERROR  ((DRM_RESULT)0x8004C07CL)

/*
 * MessageId: DRM_E_ACTIVATION_GROUP_CERT_REVOKED_ERROR
 *
 * MessageText:
 *
 * The activation group cert has been revoked and the application must be updated with a new client lib.
 *
 */
#define DRM_E_ACTIVATION_GROUP_CERT_REVOKED_ERROR ((DRM_RESULT)0x8004C07DL)

/*
 * MessageId: DRM_E_ACTIVATION_NEW_CLIENT_LIB_REQUIRED_ERROR
 *
 * MessageText:
 *
 * The client lib used by the application is not supported and must be updated.
 *
 */
#define DRM_E_ACTIVATION_NEW_CLIENT_LIB_REQUIRED_ERROR ((DRM_RESULT)0x8004C07EL)

/*
 * MessageId: DRM_E_ACTIVATION_BAD_REQUEST
 *
 * MessageText:
 *
 * The activation request is invalid
 *
 */
#define DRM_E_ACTIVATION_BAD_REQUEST     ((DRM_RESULT)0x8004C07FL)

/*
 * MessageId: DRM_E_FILEIO_ERROR
 *
 * MessageText:
 *
 * Encountered a system error during file I/O.
 *
 */
#define DRM_E_FILEIO_ERROR               ((DRM_RESULT)0x8004C080L)

/*
 * MessageId: DRM_E_DISKSPACE_ERROR
 *
 * MessageText:
 *
 * Out of disk space for storing playready files.
 *
 */
#define DRM_E_DISKSPACE_ERROR            ((DRM_RESULT)0x8004C081L)

/*
 * MessageId: DRM_E_UPLINKLICENSENOTFOUND
 *
 * MessageText:
 *
 * A license was found in the license store but no license was found for its uplink ID.
 *
 */
#define DRM_E_UPLINKLICENSENOTFOUND      ((DRM_RESULT)0x8004C082L)


/* ------------------------------------------------------------
**
** License revocation errors: error codes from DRM_E_BASECODE+0xA0 (+160) to
** DRM_E_BASECODE+0xBF, 0x8004c0a0-0x8004c0bf.
**
** ------------------------------------------------------------
*/

#define DRM_E_LRB_BASECODE                      DRM_E_BASECODE+0xA0

/*
 * MessageId: DRM_E_LRB_NOLGPUBKEY
 *
 * MessageText:
 *
 * LRB does not contain a valid LGPUBKEY.
 *
 */
#define DRM_E_LRB_NOLGPUBKEY             ((DRM_RESULT)0x8004C0A0L)

/*
 * MessageId: DRM_E_LRB_INVALIDSIGNATURE
 *
 * MessageText:
 *
 * Signature inside LRB is invalid.
 *
 */
#define DRM_E_LRB_INVALIDSIGNATURE       ((DRM_RESULT)0x8004C0A1L)

/*
 * MessageId: DRM_E_LRB_LGPUBKEY_MISMATCH
 *
 * MessageText:
 *
 * LRB is signed with a pubkey different from LGPUBKEY
 *
 */
#define DRM_E_LRB_LGPUBKEY_MISMATCH      ((DRM_RESULT)0x8004C0A2L)

/*
 * MessageId: DRM_E_LRB_INVALIDLICENSEDATA
 *
 * MessageText:
 *
 * LRB is signed with a pubkey different from LGPUBKEY
 *
 */
#define DRM_E_LRB_INVALIDLICENSEDATA     ((DRM_RESULT)0x8004C0A3L)


/* ------------------------------------------------------------
**
** License evaluator errors: error codes from DRM_E_BASECODE+0xC0 to
** DRM_E_BASECODE+0xDF, 0x8004c0c0-0x8004c0df.
**
** ------------------------------------------------------------
*/

#define DRM_E_LICEVAL_BASECODE                  DRM_E_BASECODE+0xC0

/*
 * MessageId: DRM_E_LICEVAL_LICENSE_NOT_SUPPLIED
 *
 * MessageText:
 *
 * License not supplied in the liceval context
 *
 */
#define DRM_E_LICEVAL_LICENSE_NOT_SUPPLIED ((DRM_RESULT)0x8004C0C0L)

/*
 * MessageId: DRM_E_LICEVAL_KID_MISMATCH
 *
 * MessageText:
 *
 * Mismatch between KID from header and the one inside license
 *
 */
#define DRM_E_LICEVAL_KID_MISMATCH       ((DRM_RESULT)0x8004C0C1L)

/*
 * MessageId: DRM_E_LICEVAL_LICENSE_REVOKED
 *
 * MessageText:
 *
 * License for this content has been revoked
 *
 */
#define DRM_E_LICEVAL_LICENSE_REVOKED    ((DRM_RESULT)0x8004C0C2L)

/*
 * MessageId: DRM_E_LICEVAL_UPDATE_FAILURE
 *
 * MessageText:
 *
 * Failed to update content revocation
 *
 */
#define DRM_E_LICEVAL_UPDATE_FAILURE     ((DRM_RESULT)0x8004C0C3L)

/*
 * MessageId: DRM_E_LICEVAL_REQUIRED_REVOCATION_LIST_NOT_AVAILABLE
 *
 * MessageText:
 *
 * Failed to update content revocation
 *
 */
#define DRM_E_LICEVAL_REQUIRED_REVOCATION_LIST_NOT_AVAILABLE ((DRM_RESULT)0x8004C0C4L)


/* ------------------------------------------------------------
**
** XMR parser and builder errors: error codes from DRM_E_BASECODE+0xE0 to
** DRM_E_BASECODE+0xFF, 0x8004c0e0-0x8004c0ff.
**
** ------------------------------------------------------------
*/

#define DRM_E_XMR_BASECODE                      DRM_E_BASECODE+0xE0

/*
 * MessageId: DRM_E_XMR_OBJECT_ALREADY_EXISTS
 *
 * MessageText:
 *
 * XMR builder context already has this object.
 *
 */
#define DRM_E_XMR_OBJECT_ALREADY_EXISTS  ((DRM_RESULT)0x8004C0E0L)

/*
 * MessageId: DRM_E_XMR_OBJECT_NOTFOUND
 *
 * MessageText:
 *
 * XMR object was not found.
 *
 */
#define DRM_E_XMR_OBJECT_NOTFOUND        ((DRM_RESULT)0x8004C0E1L)

/*
 * MessageId: DRM_E_XMR_REQUIRED_OBJECT_MISSING
 *
 * MessageText:
 *
 * XMR license doesn't have one or more required objects.
 *
 */
#define DRM_E_XMR_REQUIRED_OBJECT_MISSING ((DRM_RESULT)0x8004C0E2L)

/*
 * MessageId: DRM_E_XMR_INVALID_UNKNOWN_OBJECT
 *
 * MessageText:
 *
 * Invalid unknown object
 *
 */
#define DRM_E_XMR_INVALID_UNKNOWN_OBJECT ((DRM_RESULT)0x8004C0E3L)

/*
 * MessageId: DRM_E_XMR_LICENSE_BINDABLE
 *
 * MessageText:
 *
 * XMR license does not contain the Cannot Bind right
 *
 */
#define DRM_E_XMR_LICENSE_BINDABLE       ((DRM_RESULT)0x8004C0E4L)

/*
 * MessageId: DRM_E_XMR_LICENSE_NOT_BINDABLE
 *
 * MessageText:
 *
 * XMR license cannot be bound to because of the Cannot Bind right
 *
 */
#define DRM_E_XMR_LICENSE_NOT_BINDABLE   ((DRM_RESULT)0x8004C0E5L)

/*
 * MessageId: DRM_E_XMR_UNSUPPORTED_XMR_VERSION
 *
 * MessageText:
 *
 * The version of XMR license is not supported for the current action
 *
 */
#define DRM_E_XMR_UNSUPPORTED_XMR_VERSION ((DRM_RESULT)0x8004C0E6L)


/* ------------------------------------------------------------
**
** CRL parsing and validation errors: error codes from DRM_E_BASECODE+0x100 to
** DRM_E_BASECODE+0x1FF, 0x8004c100-0x8004c1ff.
**
** ------------------------------------------------------------
*/

#define DRM_E_CRL_BASECODE                  DRM_E_BASECODE+0x100

/*
 * MessageId: DRM_E_NOT_CRL_BLOB
 *
 * MessageText:
 *
 * CRL blob provided for parsing does not start with CBLB. It means file is not CRL blob at all.
 *
 */
#define DRM_E_NOT_CRL_BLOB               ((DRM_RESULT)0x8004C100L)

/*
 * MessageId: DRM_E_BAD_CRL_BLOB
 *
 * MessageText:
 *
 * The file is structured as CRL blob, but there is some error in file structure or one of CRLs inside is invalid.
 *
 */
#define DRM_E_BAD_CRL_BLOB               ((DRM_RESULT)0x8004C101L)


/* ------------------------------------------------------------
**
** Device certificate errors: error codes from DRM_E_BASECODE+0x200 to
** DRM_E_BASECODE+0x4FF, 0x8004c200-0x8004c2ff.
**
** ------------------------------------------------------------
*/

#define DRM_E_CERT_BASECODE                  DRM_E_BASECODE+0x200

/*
 * MessageId: DRM_E_INVALID_DEVCERT_ATTRIBUTE
 *
 * MessageText:
 *
 * The attributes in the Device certificate are invalid
 *
 */
#define DRM_E_INVALID_DEVCERT_ATTRIBUTE  ((DRM_RESULT)0x8004C200L)


/* ------------------------------------------------------------
**
** Test errors: error codes from DRM_E_BASECODE+0x300 to
** DRM_E_BASECODE+0x3E7, 0x8004c300-0x8004c3e7.
**
** ------------------------------------------------------------
*/

#define DRM_E_TEST_BASECODE        DRM_E_BASECODE+0x300
#define DRM_S_TEST_BASECODE        DRM_S_BASECODE+0x300

/*
 * MessageId: DRM_S_TEST_SKIP_FILE
 *
 * MessageText:
 *
 * Skip processing this file, not an eror.
 *
 */
#define DRM_S_TEST_SKIP_FILE             ((DRM_RESULT)0x0004C300L)

/*
 * MessageId: DRM_S_TEST_CONVERTED_FILE
 *
 * MessageText:
 *
 * The file was converted to a PlayReady file during the action.
 *
 */
#define DRM_S_TEST_CONVERTED_FILE        ((DRM_RESULT)0x0004C301L)

/*
 * MessageId: DRM_E_TEST_PKCRYPTO_FAILURE
 *
 * MessageText:
 *
 * Error in PK encryption/decryption crypto test cases.
 *
 */
#define DRM_E_TEST_PKCRYPTO_FAILURE      ((DRM_RESULT)0x8004C300L)

/*
 * MessageId: DRM_E_TEST_PKSIGN_VERIFY_ERROR
 *
 * MessageText:
 *
 * Digital signature verification failed.
 *
 */
#define DRM_E_TEST_PKSIGN_VERIFY_ERROR   ((DRM_RESULT)0x8004C301L)

/*
 * MessageId: DRM_E_TEST_ENCRYPT_ERROR
 *
 * MessageText:
 *
 * Error in encryption of cipher text.
 *
 */
#define DRM_E_TEST_ENCRYPT_ERROR         ((DRM_RESULT)0x8004C302L)

/*
 * MessageId: DRM_E_TEST_RC4KEY_FAILED
 *
 * MessageText:
 *
 * RC4 key failed during crypto operations.
 *
 */
#define DRM_E_TEST_RC4KEY_FAILED         ((DRM_RESULT)0x8004C303L)

/*
 * MessageId: DRM_E_TEST_DECRYPT_ERROR
 *
 * MessageText:
 *
 * Error in cipher text decryption.
 *
 */
#define DRM_E_TEST_DECRYPT_ERROR         ((DRM_RESULT)0x8004C304L)

/*
 * MessageId: DRM_E_TEST_DESKEY_FAILED
 *
 * MessageText:
 *
 * Decrypted data not equal to original data in a DES operation.
 *
 */
#define DRM_E_TEST_DESKEY_FAILED         ((DRM_RESULT)0x8004C305L)

/*
 * MessageId: DRM_E_TEST_CBC_INVERSEMAC_FAILURE
 *
 * MessageText:
 *
 * Decrypted data not equal to original in Inverse MAC operation.
 *
 */
#define DRM_E_TEST_CBC_INVERSEMAC_FAILURE ((DRM_RESULT)0x8004C306L)

/*
 * MessageId: DRM_E_TEST_HMAC_FAILURE
 *
 * MessageText:
 *
 * Error in hashed data in HMAC operation.
 *
 */
#define DRM_E_TEST_HMAC_FAILURE          ((DRM_RESULT)0x8004C307L)

/*
 * MessageId: DRM_E_TEST_INVALIDARG
 *
 * MessageText:
 *
 * Error in the number of arguments or argument data in Test files.
 *
 */
#define DRM_E_TEST_INVALIDARG            ((DRM_RESULT)0x8004C308L)

/*
 * MessageId: DRM_E_TEST_DEVICE_PRIVATE_KEY_INCORRECTLY_STORED
 *
 * MessageText:
 *
 * DRMManager context should not contain the device private key.
 *
 */
#define DRM_E_TEST_DEVICE_PRIVATE_KEY_INCORRECTLY_STORED ((DRM_RESULT)0x8004C30AL)

/*
 * MessageId: DRM_E_TEST_DRMMANAGER_CONTEXT_NULL
 *
 * MessageText:
 *
 * DRMManager context is NULL.
 *
 */
#define DRM_E_TEST_DRMMANAGER_CONTEXT_NULL ((DRM_RESULT)0x8004C30BL)

/*
 * MessageId: DRM_E_TEST_UNEXPECTED_REVINFO_RESULT
 *
 * MessageText:
 *
 * Revocation cache result was not as expected.
 *
 */
#define DRM_E_TEST_UNEXPECTED_REVINFO_RESULT ((DRM_RESULT)0x8004C30CL)

/*
 * MessageId: DRM_E_TEST_RIV_MISMATCH
 *
 * MessageText:
 *
 * Revocation Info Version(RIV) mismatch.
 *
 */
#define DRM_E_TEST_RIV_MISMATCH          ((DRM_RESULT)0x8004C30DL)

/*
 * MessageId: DRM_E_TEST_URL_ERROR
 *
 * MessageText:
 *
 * There is an error in the URL from the challenge generated.
 *
 */
#define DRM_E_TEST_URL_ERROR             ((DRM_RESULT)0x8004C310L)

/*
 * MessageId: DRM_E_TEST_MID_MISMATCH
 *
 * MessageText:
 *
 * The MIDs returned from the DRM_MANAGER_CONTEXT does not match the test input.
 *
 */
#define DRM_E_TEST_MID_MISMATCH          ((DRM_RESULT)0x8004C311L)

/*
 * MessageId: DRM_E_TEST_METER_CERTIFICATE_MISMATCH
 *
 * MessageText:
 *
 * The input data does not match with the Metering certificate returned from the license.
 *
 */
#define DRM_E_TEST_METER_CERTIFICATE_MISMATCH ((DRM_RESULT)0x8004C312L)

/*
 * MessageId: DRM_E_TEST_LICENSE_STATE_MISMATCH
 *
 * MessageText:
 *
 * The input data and license state returned from the license do not match.
 *
 */
#define DRM_E_TEST_LICENSE_STATE_MISMATCH ((DRM_RESULT)0x8004C313L)

/*
 * MessageId: DRM_E_TEST_SOURCE_ID_MISMATCH
 *
 * MessageText:
 *
 * The input data and license state returned from the license do not match.
 *
 */
#define DRM_E_TEST_SOURCE_ID_MISMATCH    ((DRM_RESULT)0x8004C316L)

/*
 * MessageId: DRM_E_TEST_UNEXPECTED_LICENSE_COUNT
 *
 * MessageText:
 *
 * The input data and the number of license from the KID do not match.
 *
 */
#define DRM_E_TEST_UNEXPECTED_LICENSE_COUNT ((DRM_RESULT)0x8004C317L)

/*
 * MessageId: DRM_E_TEST_UNEXPECTED_DEVICE_PROPERTY
 *
 * MessageText:
 *
 * Unknown device property.
 *
 */
#define DRM_E_TEST_UNEXPECTED_DEVICE_PROPERTY ((DRM_RESULT)0x8004C318L)

/*
 * MessageId: DRM_E_TEST_DRMMANAGER_MISALIGNED_BYTES
 *
 * MessageText:
 *
 * Error due to misalignment of bytes.
 *
 */
#define DRM_E_TEST_DRMMANAGER_MISALIGNED_BYTES ((DRM_RESULT)0x8004C319L)

/*
 * MessageId: DRM_E_TEST_LICENSE_RESPONSE_ERROR
 *
 * MessageText:
 *
 * The license response callbacks did not provide the expected data.
 *
 */
#define DRM_E_TEST_LICENSE_RESPONSE_ERROR ((DRM_RESULT)0x8004C31AL)

/*
 * MessageId: DRM_E_TEST_OPL_MISMATCH
 *
 * MessageText:
 *
 * The minimum levels of the compressed/uncompressed Digital and Analog Video do not match the OPL.
 *
 */
#define DRM_E_TEST_OPL_MISMATCH          ((DRM_RESULT)0x8004C31BL)

/*
 * MessageId: DRM_E_TEST_INVALID_OPL_CALLBACK
 *
 * MessageText:
 *
 * The callback type supplied is not valid.
 *
 */
#define DRM_E_TEST_INVALID_OPL_CALLBACK  ((DRM_RESULT)0x8004C31CL)

/*
 * MessageId: DRM_E_TEST_INCOMPLETE
 *
 * MessageText:
 *
 * The test function failed to complete.
 *
 */
#define DRM_E_TEST_INCOMPLETE            ((DRM_RESULT)0x8004C31DL)

/*
 * MessageId: DRM_E_TEST_UNEXPECTED_OUTPUT
 *
 * MessageText:
 *
 * The output of the function being tested does not match the expected output.
 *
 */
#define DRM_E_TEST_UNEXPECTED_OUTPUT     ((DRM_RESULT)0x8004C31EL)

/*
 * MessageId: DRM_E_TEST_DLA_NO_CONTENT_HEADER
 *
 * MessageText:
 *
 * Content Header Information was not retrieved correctly in DLA Sync Tests.
 *
 */
#define DRM_E_TEST_DLA_NO_CONTENT_HEADER ((DRM_RESULT)0x8004C31FL)

/*
 * MessageId: DRM_E_TEST_DLA_CONTENT_HEADER_FOUND
 *
 * MessageText:
 *
 * Content Header Information was found when it should not have been in DLA Sync Tests.
 *
 */
#define DRM_E_TEST_DLA_CONTENT_HEADER_FOUND ((DRM_RESULT)0x8004C320L)

/*
 * MessageId: DRM_E_TEST_SYNC_LSD_INCORRECT
 *
 * MessageText:
 *
 * DRM_SNC_GetSyncStoreEntry returned incorrect License State Data.
 *
 */
#define DRM_E_TEST_SYNC_LSD_INCORRECT    ((DRM_RESULT)0x8004C321L)

/*
 * MessageId: DRM_E_TEST_TOO_SLOW
 *
 * MessageText:
 *
 * The performance test failed because DRM took longer than its maximum time.
 *
 */
#define DRM_E_TEST_TOO_SLOW              ((DRM_RESULT)0x8004C322L)

/*
 * MessageId: DRM_E_TEST_LICENSESTORE_NOT_OPEN
 *
 * MessageText:
 *
 * The License Store contexts in the App Manager context are not open.
 *
 */
#define DRM_E_TEST_LICENSESTORE_NOT_OPEN ((DRM_RESULT)0x8004C323L)

/*
 * MessageId: DRM_E_TEST_DEVICE_NOT_INITED
 *
 * MessageText:
 *
 * The device instance has not been initialized prior to use.
 *
 */
#define DRM_E_TEST_DEVICE_NOT_INITED     ((DRM_RESULT)0x8004C324L)

/*
 * MessageId: DRM_E_TEST_VARIABLE_NOT_SET
 *
 * MessageText:
 *
 * A global variable needed for test execution has not been set correctly.
 *
 */
#define DRM_E_TEST_VARIABLE_NOT_SET      ((DRM_RESULT)0x8004C325L)

/*
 * MessageId: DRM_E_TEST_NOMORE
 *
 * MessageText:
 *
 * The same as DRM_E_NOMORE, only explicitly used in test code.
 *
 */
#define DRM_E_TEST_NOMORE                ((DRM_RESULT)0x8004C326L)

/*
 * MessageId: DRM_E_TEST_FILE_LOAD_ERROR
 *
 * MessageText:
 *
 * There was an error loading a test data file.
 *
 */
#define DRM_E_TEST_FILE_LOAD_ERROR       ((DRM_RESULT)0x8004C327L)

/*
 * MessageId: DRM_E_TEST_LICENSE_ACQ_FAILED
 *
 * MessageText:
 *
 * The attempt to acquire a license failed.
 *
 */
#define DRM_E_TEST_LICENSE_ACQ_FAILED    ((DRM_RESULT)0x8004C328L)

/*
 * MessageId: DRM_E_TEST_UNSUPPORTED_FILE_FORMAT
 *
 * MessageText:
 *
 * A file format is being used which is not supported by the test function.
 *
 */
#define DRM_E_TEST_UNSUPPORTED_FILE_FORMAT ((DRM_RESULT)0x8004C329L)

/*
 * MessageId: DRM_E_TEST_PARSING_ERROR
 *
 * MessageText:
 *
 * There was an error parsing input parameter.
 *
 */
#define DRM_E_TEST_PARSING_ERROR         ((DRM_RESULT)0x8004C32AL)

/*
 * MessageId: DRM_E_TEST_NOTIMPL
 *
 * MessageText:
 *
 * The specified test API is not implemented.
 *
 */
#define DRM_E_TEST_NOTIMPL               ((DRM_RESULT)0x8004C32BL)

/*
 * MessageId: DRM_E_TEST_VARIABLE_NOTFOUND
 *
 * MessageText:
 *
 * The specified test varaible was not found in the shared variable table.
 *
 */
#define DRM_E_TEST_VARIABLE_NOTFOUND     ((DRM_RESULT)0x8004C32CL)

/*
 * MessageId: DRM_E_TEST_VARIABLE_LISTFULL
 *
 * MessageText:
 *
 * The shared test variable table is full.
 *
 */
#define DRM_E_TEST_VARIABLE_LISTFULL     ((DRM_RESULT)0x8004C32DL)

/*
 * MessageId: DRM_E_TEST_UNEXPECTED_CONTENT_PROPERTY
 *
 * MessageText:
 *
 * Unknown content property.
 *
 */
#define DRM_E_TEST_UNEXPECTED_CONTENT_PROPERTY ((DRM_RESULT)0x8004C32EL)

/*
 * MessageId: DRM_E_TEST_PRO_HEADER_NOT_SET
 *
 * MessageText:
 *
 * PlayReady Object Header not set.
 *
 */
#define DRM_E_TEST_PRO_HEADER_NOT_SET    ((DRM_RESULT)0x8004C32FL)

/*
 * MessageId: DRM_E_TEST_NON_PRO_HEADER_TYPE
 *
 * MessageText:
 *
 * Incompatible header - PlayReady Object Header expected.
 *
 */
#define DRM_E_TEST_NON_PRO_HEADER_TYPE   ((DRM_RESULT)0x8004C330L)

/*
 * MessageId: DRM_E_TEST_INVALID_DEVICE_WRAPPER
 *
 * MessageText:
 *
 * The Device Simulator Device Wrapper is not valid.
 *
 */
#define DRM_E_TEST_INVALID_DEVICE_WRAPPER ((DRM_RESULT)0x8004C331L)

/*
 * MessageId: DRM_E_TEST_INVALID_WMDM_WRAPPER
 *
 * MessageText:
 *
 * The Device Simulator WMDM Wrapper is not valid.
 *
 */
#define DRM_E_TEST_INVALID_WMDM_WRAPPER  ((DRM_RESULT)0x8004C332L)

/*
 * MessageId: DRM_E_TEST_INVALID_WPD_WRAPPER
 *
 * MessageText:
 *
 * The Device Simulator WPD Wrapper is not valid.
 *
 */
#define DRM_E_TEST_INVALID_WPD_WRAPPER   ((DRM_RESULT)0x8004C333L)

/*
 * MessageId: DRM_E_TEST_INVALID_FILE
 *
 * MessageText:
 *
 * The data file given was invalid.
 *
 */
#define DRM_E_TEST_INVALID_FILE          ((DRM_RESULT)0x8004C334L)

/*
 * MessageId: DRM_E_TEST_PROPERTY_NOT_FOUND
 *
 * MessageText:
 *
 * The object did not have the property which was queried.
 *
 */
#define DRM_E_TEST_PROPERTY_NOT_FOUND    ((DRM_RESULT)0x8004C335L)

/*
 * MessageId: DRM_E_TEST_METERING_DATA_INCORRECT
 *
 * MessageText:
 *
 * The metering data reported is incorrect.
 *
 */
#define DRM_E_TEST_METERING_DATA_INCORRECT ((DRM_RESULT)0x8004C336L)

/*
 * MessageId: DRM_E_TEST_FILE_ALREADY_OPEN
 *
 * MessageText:
 *
 * The handle variable for a test file is not NULL. This indicates that a file was opened and not closed properly.
 *
 */
#define DRM_E_TEST_FILE_ALREADY_OPEN     ((DRM_RESULT)0x8004C337L)

/*
 * MessageId: DRM_E_TEST_FILE_NOT_OPEN
 *
 * MessageText:
 *
 * The handle variable for a test file is NULL. This indicates that a file was not opened.
 *
 */
#define DRM_E_TEST_FILE_NOT_OPEN         ((DRM_RESULT)0x8004C338L)

/*
 * MessageId: DRM_E_TEST_PICT_COLUMN_TOO_WIDE
 *
 * MessageText:
 *
 * The PICT input file contains a column which is too wide for the test parser to handle.
 *
 */
#define DRM_E_TEST_PICT_COLUMN_TOO_WIDE  ((DRM_RESULT)0x8004C339L)

/*
 * MessageId: DRM_E_TEST_PICT_COLUMN_MISMATCH
 *
 * MessageText:
 *
 * The PICT input file contains a row which doesn't have the same number of columns as the header row.
 *
 */
#define DRM_E_TEST_PICT_COLUMN_MISMATCH  ((DRM_RESULT)0x8004C33AL)

/*
 * MessageId: DRM_E_TEST_TUX_TEST_SKIPPED
 *
 * MessageText:
 *
 * TUX cannot find the speficied test case in target dll. Test Skipped.
 *
 */
#define DRM_E_TEST_TUX_TEST_SKIPPED      ((DRM_RESULT)0x8004C33BL)

/*
 * MessageId: DRM_E_TEST_KEYFILE_VERIFICATION_FAILURE
 *
 * MessageText:
 *
 * Verification of the Keyfile context failed.
 *
 */
#define DRM_E_TEST_KEYFILE_VERIFICATION_FAILURE ((DRM_RESULT)0x8004C33CL)

/*
 * MessageId: DRM_E_TEST_DATA_VERIFICATION_FAILURE
 *
 * MessageText:
 *
 * Data does not match expected value and failed verification.
 *
 */
#define DRM_E_TEST_DATA_VERIFICATION_FAILURE ((DRM_RESULT)0x8004C33DL)

/*
 * MessageId: DRM_E_TEST_NET_FAIL
 *
 * MessageText:
 *
 * The Test failed to perform Network I/O.
 *
 */
#define DRM_E_TEST_NET_FAIL              ((DRM_RESULT)0x8004C33EL)


/* ------------------------------------------------------------
**
** Errors of the range 0x8004c3e8-0x8004c3f8 (range is where
** *decimal* +1000 starts.
**
** ------------------------------------------------------------
*/

/*
 * MessageId: DRM_E_LOGICERR
 *
 * MessageText:
 *
 * DRM code has a logic error in it.  This result should never be returned.  There is an unhandled code path if it is returned.
 *
 */
#define DRM_E_LOGICERR                   ((DRM_RESULT)0x8004C3E8L)

/*
 * MessageId: DRM_E_INVALIDREVINFO
 *
 * MessageText:
 *
 * The device certificate is invalid.
 *
 */
#define DRM_E_INVALIDREVINFO             ((DRM_RESULT)0x8004C3E9L)

/*
 * MessageId: DRM_E_SYNCLISTNOTSUPPORTED
 *
 * MessageText:
 *
 * The device does not support synclist.
 *
 */
#define DRM_E_SYNCLISTNOTSUPPORTED       ((DRM_RESULT)0x8004C3EAL)

/*
 * MessageId: DRM_E_REVOCATION_BUFFERTOOSMALL
 *
 * MessageText:
 *
 * The revocation buffer is too small.
 *
 */
#define DRM_E_REVOCATION_BUFFERTOOSMALL  ((DRM_RESULT)0x8004C3EBL)

/*
 * MessageId: DRM_E_DEVICE_ALREADY_REGISTERED
 *
 * MessageText:
 *
 * There exists already a device in the device store with the same DEVICEID that was given.
 *
 */
#define DRM_E_DEVICE_ALREADY_REGISTERED  ((DRM_RESULT)0x8004C3ECL)

/*
 * MessageId: DRM_E_DST_NOT_COMPATIBLE
 *
 * MessageText:
 *
 * The data store version is incompatible with this version of DRM.
 *
 */
#define DRM_E_DST_NOT_COMPATIBLE         ((DRM_RESULT)0x8004C3EDL)

/*
 * MessageId: DRM_E_RSA_DECRYPTION_ERROR
 *
 * MessageText:
 *
 * The data block/Encoded message used in OAEP decoding is incorrect.
 *
 */
#define DRM_E_RSA_DECRYPTION_ERROR       ((DRM_RESULT)0x8004C3F0L)

/*
 * MessageId: DRM_E_OEM_RSA_MESSAGE_TOO_BIG
 *
 * MessageText:
 *
 * The base message buffer is larger than the given modulus.
 *
 */
#define DRM_E_OEM_RSA_MESSAGE_TOO_BIG    ((DRM_RESULT)0x8004C3F1L)

/*
 * MessageId: DRM_E_METERCERTNOTFOUND
 *
 * MessageText:
 *
 * The metering certificate was not found in the store.
 *
 */
#define DRM_E_METERCERTNOTFOUND          ((DRM_RESULT)0x8004C3F2L)

/*
 * MessageId: DRM_E_MODULAR_ARITHMETIC_FAILURE
 *
 * MessageText:
 *
 * A failure occurred in bignum modular arithmetic.
 *
 */
#define DRM_E_MODULAR_ARITHMETIC_FAILURE ((DRM_RESULT)0x8004C3F3L)

/*
 * MessageId: DRM_E_FEATURE_NOT_SUPPORTED
 *
 * MessageText:
 *
 * The feature is not supported in this release.
 *
 */
#define DRM_E_FEATURE_NOT_SUPPORTED      ((DRM_RESULT)0x8004C3F4L)

/*
 * MessageId: DRM_E_REVOCATION_INVALID_PACKAGE
 *
 * MessageText:
 *
 * The revocation package is invalid
 *
 */
#define DRM_E_REVOCATION_INVALID_PACKAGE ((DRM_RESULT)0x8004C3F5L)

/*
 * MessageId: DRM_E_HWID_ERROR
 *
 * MessageText:
 *
 * Failed to get the hardware ID.
 *
 */
#define DRM_E_HWID_ERROR                 ((DRM_RESULT)0x8004C3F6L)


/* ------------------------------------------------------------
**
** Domain errors: error codes from DRM_E_BASECODE+0x500 to
** DRM_E_BASECODE+0x57F, 0x8004c500-0x8004c57f.
**
** ------------------------------------------------------------
*/

#define DRM_E_DOMAIN_BASECODE       DRM_E_BASECODE + 0x500

/*
 * MessageId: DRM_E_DOMAIN_INVALID_GUID
 *
 * MessageText:
 *
 * Not a correct GUID.
 *
 */
#define DRM_E_DOMAIN_INVALID_GUID        ((DRM_RESULT)0x8004C500L)

/*
 * MessageId: DRM_E_DOMAIN_INVALID_CUSTOM_DATA_TYPE
 *
 * MessageText:
 *
 * Not a valid custom data type.
 *
 */
#define DRM_E_DOMAIN_INVALID_CUSTOM_DATA_TYPE ((DRM_RESULT)0x8004C501L)

/*
 * MessageId: DRM_E_DOMAIN_STORE_ADD_DATA
 *
 * MessageText:
 *
 * Failed to add data into the domain store.
 *
 */
#define DRM_E_DOMAIN_STORE_ADD_DATA      ((DRM_RESULT)0x8004C502L)

/*
 * MessageId: DRM_E_DOMAIN_STORE_GET_DATA
 *
 * MessageText:
 *
 * Failed to retrieve data from the domain store.
 *
 */
#define DRM_E_DOMAIN_STORE_GET_DATA      ((DRM_RESULT)0x8004C503L)

/*
 * MessageId: DRM_E_DOMAIN_STORE_DELETE_DATA
 *
 * MessageText:
 *
 * Failed to delete data from the domain store.
 *
 */
#define DRM_E_DOMAIN_STORE_DELETE_DATA   ((DRM_RESULT)0x8004C504L)

/*
 * MessageId: DRM_E_DOMAIN_STORE_OPEN_STORE
 *
 * MessageText:
 *
 * Failed to open the domain store.
 *
 */
#define DRM_E_DOMAIN_STORE_OPEN_STORE    ((DRM_RESULT)0x8004C505L)

/*
 * MessageId: DRM_E_DOMAIN_STORE_CLOSE_STORE
 *
 * MessageText:
 *
 * Failed to close the domain store.
 *
 */
#define DRM_E_DOMAIN_STORE_CLOSE_STORE   ((DRM_RESULT)0x8004C506L)

/*
 * MessageId: DRM_E_DOMAIN_BIND_LICENSE
 *
 * MessageText:
 *
 * Failed to bind to the domain license.
 *
 */
#define DRM_E_DOMAIN_BIND_LICENSE        ((DRM_RESULT)0x8004C507L)

/*
 * MessageId: DRM_E_DOMAIN_INVALID_CUSTOM_DATA
 *
 * MessageText:
 *
 * Not a valid custom data.
 *
 */
#define DRM_E_DOMAIN_INVALID_CUSTOM_DATA ((DRM_RESULT)0x8004C508L)

/*
 * MessageId: DRM_E_DOMAIN_NOT_FOUND
 *
 * MessageText:
 *
 * No domain information is found.
 *
 */
#define DRM_E_DOMAIN_NOT_FOUND           ((DRM_RESULT)0x8004C509L)

/*
 * MessageId: DRM_E_DOMAIN_INVALID_DOMKEYXMR_DATA
 *
 * MessageText:
 *
 * The domain join response contains invalid domain privkey XMR data.
 *
 */
#define DRM_E_DOMAIN_INVALID_DOMKEYXMR_DATA ((DRM_RESULT)0x8004C50AL)

/*
 * MessageId: DRM_E_DOMAIN_STORE_INVALID_KEY_RECORD
 *
 * MessageText:
 *
 * Invalid format of domain private key record read from the domain store.
 *
 */
#define DRM_E_DOMAIN_STORE_INVALID_KEY_RECORD ((DRM_RESULT)0x8004C50BL)


/* ------------------------------------------------------------
**
** PC errors returned by core logic: error codes from DRM_E_BASECODE+0x580 to
** DRM_E_BASECODE+0x5FF, 0x8004c580-0x8004c5ff.
**
** ------------------------------------------------------------
*/

#define DRM_E_PC_BASECODE           DRM_E_BASECODE + 0x580

/*
 * MessageId: DRM_E_DEVICE_DOMAIN_JOIN_REQUIRED
 *
 * MessageText:
 *
 * This error code communicates to the application that the device is not a member of a domain. The app can uses this error code in turn to decide whether it needs to join the domain or not
 *
 */
#define DRM_E_DEVICE_DOMAIN_JOIN_REQUIRED ((DRM_RESULT)0x8004C580L)


/* ------------------------------------------------------------
**
** Server errors returned by core logic: error codes from DRM_E_BASECODE+0x600
** to DRM_E_BASECODE+0x6FF, 0x8004c600-0x8004c6ff.
**
** ------------------------------------------------------------
*/

#define DRM_E_SERVER_BASECODE       DRM_E_BASECODE + 0x600

/*
 * MessageId: DRM_E_SERVER_INTERNAL_ERROR
 *
 * MessageText:
 *
 * An internal server error occurred.
 *
 */
#define DRM_E_SERVER_INTERNAL_ERROR      ((DRM_RESULT)0x8004C600L)

/*
 * MessageId: DRM_E_SERVER_INVALID_MESSAGE
 *
 * MessageText:
 *
 * The message sent to the server was invalid.
 *
 */
#define DRM_E_SERVER_INVALID_MESSAGE     ((DRM_RESULT)0x8004C601L)

/*
 * MessageId: DRM_E_SERVER_DEVICE_LIMIT_REACHED
 *
 * MessageText:
 *
 * The device limit for the domain has been reached.
 *
 */
#define DRM_E_SERVER_DEVICE_LIMIT_REACHED ((DRM_RESULT)0x8004C602L)

/*
 * MessageId: DRM_E_SERVER_INDIV_REQUIRED
 *
 * MessageText:
 *
 * Individualization of the client is required.
 *
 */
#define DRM_E_SERVER_INDIV_REQUIRED      ((DRM_RESULT)0x8004C603L)

/*
 * MessageId: DRM_E_SERVER_SERVICE_SPECIFIC
 *
 * MessageText:
 *
 * An error specific to the service has occurred.
 *
 */
#define DRM_E_SERVER_SERVICE_SPECIFIC    ((DRM_RESULT)0x8004C604L)

/*
 * MessageId: DRM_E_SERVER_DOMAIN_REQUIRED
 *
 * MessageText:
 *
 * A Domain certificate is required.
 *
 */
#define DRM_E_SERVER_DOMAIN_REQUIRED     ((DRM_RESULT)0x8004C605L)

/*
 * MessageId: DRM_E_SERVER_RENEW_DOMAIN
 *
 * MessageText:
 *
 * The Domain certificate needs to be renewed.
 *
 */
#define DRM_E_SERVER_RENEW_DOMAIN        ((DRM_RESULT)0x8004C606L)

/*
 * MessageId: DRM_E_SERVER_UNKNOWN_METERINGID
 *
 * MessageText:
 *
 * The metering identifier is unknown.
 *
 */
#define DRM_E_SERVER_UNKNOWN_METERINGID  ((DRM_RESULT)0x8004C607L)

/*
 * MessageId: DRM_E_SERVER_COMPUTER_LIMIT_REACHED
 *
 * MessageText:
 *
 * The computer limit for the domain has been reached.
 *
 */
#define DRM_E_SERVER_COMPUTER_LIMIT_REACHED ((DRM_RESULT)0x8004C608L)

/*
 * MessageId: DRM_E_SERVER_PROTOCOL_FALLBACK
 *
 * MessageText:
 *
 * The client should fallback to the V2 license acquisition protocol.
 *
 */
#define DRM_E_SERVER_PROTOCOL_FALLBACK   ((DRM_RESULT)0x8004C609L)

/*
 * MessageId: DRM_E_SERVER_NOT_A_MEMBER
 *
 * MessageText:
 *
 * The client was removed from the domain in an offline fashion and thus still has a domain cert, but not a valid domain membership.
 *
 */
#define DRM_E_SERVER_NOT_A_MEMBER        ((DRM_RESULT)0x8004C60AL)

/*
 * MessageId: DRM_E_SERVER_PROTOCOL_VERSION_MISMATCH
 *
 * MessageText:
 *
 * The protocol version specified was not supported by the server.
 *
 */
#define DRM_E_SERVER_PROTOCOL_VERSION_MISMATCH ((DRM_RESULT)0x8004C60BL)

/*
 * MessageId: DRM_E_SERVER_UNKNOWN_ACCOUNTID
 *
 * MessageText:
 *
 * The account identifier is unknown.
 *
 */
#define DRM_E_SERVER_UNKNOWN_ACCOUNTID   ((DRM_RESULT)0x8004C60CL)

/*
 * MessageId: DRM_E_SERVER_PROTOCOL_REDIRECT
 *
 * MessageText:
 *
 * The protocol has a redirect.
 *
 */
#define DRM_E_SERVER_PROTOCOL_REDIRECT   ((DRM_RESULT)0x8004C60DL)

/*
 * MessageId: DRM_E_SERVER_UNKNOWN_TRANSACTIONID
 *
 * MessageText:
 *
 * The transaction identifier is unknown.
 *
 */
#define DRM_E_SERVER_UNKNOWN_TRANSACTIONID ((DRM_RESULT)0x8004C610L)

/*
 * MessageId: DRM_E_SERVER_INVALID_LICENSEID
 *
 * MessageText:
 *
 * The license identifier is invalid.
 *
 */
#define DRM_E_SERVER_INVALID_LICENSEID   ((DRM_RESULT)0x8004C611L)

/*
 * MessageId: DRM_E_SERVER_MAXIMUM_LICENSEID_EXCEEDED
 *
 * MessageText:
 *
 * The maximum number of license identifiers in the request was exceeded.
 *
 */
#define DRM_E_SERVER_MAXIMUM_LICENSEID_EXCEEDED ((DRM_RESULT)0x8004C612L)


/* ------------------------------------------------------------
** DRM_E_BASECODE + 0x680 - DRM_E_BASECODE + 0x6ff (0x8004c680-0x8004c6ff)
** are reserved for DRM Services.
**
** See source\common\services\inc\svcerrors.h for Services error codes.
**
** ------------------------------------------------------------
*/

#define DRM_E_SERVICES_BASECODE     (DRM_E_BASECODE + 0x680)

/* ------------------------------------------------------------
**
** License acquisition protocol errors: error codes from DRM_E_BASECODE+0x700
** to DRM_E_BASECODE+0x77F, 0x8004c700-0x8004c77f.
**
** ------------------------------------------------------------
*/

#define DRM_E_LICACQ_BASECODE       DRM_E_BASECODE + 0x700

/*
 * MessageId: DRM_E_LICACQ_TOO_MANY_LICENSES
 *
 * MessageText:
 *
 * There are too many licenses in the license response.
 *
 */
#define DRM_E_LICACQ_TOO_MANY_LICENSES   ((DRM_RESULT)0x8004C700L)

/*
 * MessageId: DRM_E_LICACQ_ACK_TRANSACTIONID_TOO_BIG
 *
 * MessageText:
 *
 * The Transaction ID specified by the server exceeds the allocated buffer.
 *
 */
#define DRM_E_LICACQ_ACK_TRANSACTIONID_TOO_BIG ((DRM_RESULT)0x8004C701L)

/*
 * MessageId: DRM_E_LICACQ_ACK_MESSAGE_NOT_CREATED
 *
 * MessageText:
 *
 * The license acquisition acknowledgement message could not be created.
 *
 */
#define DRM_E_LICACQ_ACK_MESSAGE_NOT_CREATED ((DRM_RESULT)0x8004C702L)


/* ------------------------------------------------------------
**
** PlayReady initiator format errors: error codes from DRM_E_BASECODE+0x780
** to DRM_E_BASECODE+0x7FF, 0x8004c780-0x8004c7ff.
**
** ------------------------------------------------------------
*/

#define DRM_E_INITIATORS_BASECODE      DRM_E_BASECODE + 0x780

/*
 * MessageId: DRM_E_INITIATORS_UNKNOWN_TYPE
 *
 * MessageText:
 *
 * The initiator type is unknown.
 *
 */
#define DRM_E_INITIATORS_UNKNOWN_TYPE    ((DRM_RESULT)0x8004C780L)

/*
 * MessageId: DRM_E_INITIATORS_INVALID_SERVICEID
 *
 * MessageText:
 *
 * The service ID data is not valid.
 *
 */
#define DRM_E_INITIATORS_INVALID_SERVICEID ((DRM_RESULT)0x8004C781L)

/*
 * MessageId: DRM_E_INITIATORS_INVALID_ACCOUNTID
 *
 * MessageText:
 *
 * The account ID data is not valid.
 *
 */
#define DRM_E_INITIATORS_INVALID_ACCOUNTID ((DRM_RESULT)0x8004C782L)

/*
 * MessageId: DRM_E_INITIATORS_INVALID_MID
 *
 * MessageText:
 *
 * The account ID data is not valid.
 *
 */
#define DRM_E_INITIATORS_INVALID_MID     ((DRM_RESULT)0x8004C783L)

/*
 * MessageId: DRM_E_INITIATORS_MISSING_DC_URL
 *
 * MessageText:
 *
 * Domain Controller URL is missing.
 *
 */
#define DRM_E_INITIATORS_MISSING_DC_URL  ((DRM_RESULT)0x8004C784L)

/*
 * MessageId: DRM_E_INITIATORS_MISSING_CONTENT_HEADER
 *
 * MessageText:
 *
 * Content header is missing.
 *
 */
#define DRM_E_INITIATORS_MISSING_CONTENT_HEADER ((DRM_RESULT)0x8004C785L)

/*
 * MessageId: DRM_E_INITIATORS_MISSING_LAURL_IN_CONTENT_HEADER
 *
 * MessageText:
 *
 * Missing license acquisition URL in content header.
 *
 */
#define DRM_E_INITIATORS_MISSING_LAURL_IN_CONTENT_HEADER ((DRM_RESULT)0x8004C786L)

/*
 * MessageId: DRM_E_INITIATORS_MISSING_METERCERT_URL
 *
 * MessageText:
 *
 * Meter certificate server URL is missing.
 *
 */
#define DRM_E_INITIATORS_MISSING_METERCERT_URL ((DRM_RESULT)0x8004C787L)


/* ------------------------------------------------------------
**
** Binary certificate errors: error codes from DRM_E_BASECODE+0x800
** to DRM_E_BASECODE+0x8FF, 0x8004c800-0x8004c8ff.
**
** ------------------------------------------------------------
*/

#define DRM_E_BCERT_BASECODE     DRM_E_BASECODE + 0x800

/*
 * MessageId: DRM_E_BCERT_INVALID_SIGNATURE_TYPE
 *
 * MessageText:
 *
 * An invalid signature type was encountered
 *
 */
#define DRM_E_BCERT_INVALID_SIGNATURE_TYPE ((DRM_RESULT)0x8004C800L)

/*
 * MessageId: DRM_E_BCERT_CHAIN_TOO_DEEP
 *
 * MessageText:
 *
 * There are, or there would be, too many certificates in the certificate chain
 *
 */
#define DRM_E_BCERT_CHAIN_TOO_DEEP       ((DRM_RESULT)0x8004C801L)

/*
 * MessageId: DRM_E_BCERT_INVALID_CERT_TYPE
 *
 * MessageText:
 *
 * An invalid certificate type was encountered
 *
 */
#define DRM_E_BCERT_INVALID_CERT_TYPE    ((DRM_RESULT)0x8004C802L)

/*
 * MessageId: DRM_E_BCERT_INVALID_FEATURE
 *
 * MessageText:
 *
 * An invalid feature entry was encountered
 *
 */
#define DRM_E_BCERT_INVALID_FEATURE      ((DRM_RESULT)0x8004C803L)

/*
 * MessageId: DRM_E_BCERT_INVALID_KEY_USAGE
 *
 * MessageText:
 *
 * An invalid public key usage was encountered
 *
 */
#define DRM_E_BCERT_INVALID_KEY_USAGE    ((DRM_RESULT)0x8004C804L)

/*
 * MessageId: DRM_E_BCERT_INVALID_SECURITY_VERSION
 *
 * MessageText:
 *
 * An invalid Indiv Box security version was encountered
 *
 */
#define DRM_E_BCERT_INVALID_SECURITY_VERSION ((DRM_RESULT)0x8004C805L)

/*
 * MessageId: DRM_E_BCERT_INVALID_KEY_TYPE
 *
 * MessageText:
 *
 * An invalid public key type was encountered
 *
 */
#define DRM_E_BCERT_INVALID_KEY_TYPE     ((DRM_RESULT)0x8004C806L)

/*
 * MessageId: DRM_E_BCERT_INVALID_KEY_LENGTH
 *
 * MessageText:
 *
 * An invalid public key length was encountered
 *
 */
#define DRM_E_BCERT_INVALID_KEY_LENGTH   ((DRM_RESULT)0x8004C807L)

/*
 * MessageId: DRM_E_BCERT_INVALID_MAX_LICENSE_SIZE
 *
 * MessageText:
 *
 * An invalid maximum license size value was encountered
 *
 */
#define DRM_E_BCERT_INVALID_MAX_LICENSE_SIZE ((DRM_RESULT)0x8004C808L)

/*
 * MessageId: DRM_E_BCERT_INVALID_MAX_HEADER_SIZE
 *
 * MessageText:
 *
 * An invalid maximum license header size value was encountered
 *
 */
#define DRM_E_BCERT_INVALID_MAX_HEADER_SIZE ((DRM_RESULT)0x8004C809L)

/*
 * MessageId: DRM_E_BCERT_INVALID_MAX_LICENSE_CHAIN_DEPTH
 *
 * MessageText:
 *
 * An invalid maximum license chain depth was encountered
 *
 */
#define DRM_E_BCERT_INVALID_MAX_LICENSE_CHAIN_DEPTH ((DRM_RESULT)0x8004C80AL)

/*
 * MessageId: DRM_E_BCERT_INVALID_SECURITY_LEVEL
 *
 * MessageText:
 *
 * An invalid security level was encountered
 *
 */
#define DRM_E_BCERT_INVALID_SECURITY_LEVEL ((DRM_RESULT)0x8004C80BL)

/*
 * MessageId: DRM_E_BCERT_PRIVATE_KEY_NOT_SPECIFIED
 *
 * MessageText:
 *
 * A private key for signing the certificate was not provided to the builder
 *
 */
#define DRM_E_BCERT_PRIVATE_KEY_NOT_SPECIFIED ((DRM_RESULT)0x8004C80CL)

/*
 * MessageId: DRM_E_BCERT_ISSUER_KEY_NOT_SPECIFIED
 *
 * MessageText:
 *
 * An issuer key was not provided to the builder
 *
 */
#define DRM_E_BCERT_ISSUER_KEY_NOT_SPECIFIED ((DRM_RESULT)0x8004C80DL)

/*
 * MessageId: DRM_E_BCERT_ACCOUNT_ID_NOT_SPECIFIED
 *
 * MessageText:
 *
 * An account ID was not provided to the builder
 *
 */
#define DRM_E_BCERT_ACCOUNT_ID_NOT_SPECIFIED ((DRM_RESULT)0x8004C80EL)

/*
 * MessageId: DRM_E_BCERT_SERVICE_ID_NOT_SPECIFIED
 *
 * MessageText:
 *
 * A service provider ID was not provided to the builder
 *
 */
#define DRM_E_BCERT_SERVICE_ID_NOT_SPECIFIED ((DRM_RESULT)0x8004C80FL)

/*
 * MessageId: DRM_E_BCERT_CLIENT_ID_NOT_SPECIFIED
 *
 * MessageText:
 *
 * A client ID was not provided to the builder
 *
 */
#define DRM_E_BCERT_CLIENT_ID_NOT_SPECIFIED ((DRM_RESULT)0x8004C810L)

/*
 * MessageId: DRM_E_BCERT_DOMAIN_URL_NOT_SPECIFIED
 *
 * MessageText:
 *
 * A domain URL was not provided to the builder
 *
 */
#define DRM_E_BCERT_DOMAIN_URL_NOT_SPECIFIED ((DRM_RESULT)0x8004C811L)

/*
 * MessageId: DRM_E_BCERT_DOMAIN_URL_TOO_LONG
 *
 * MessageText:
 *
 * The domain URL contains too many ASCII characters
 *
 */
#define DRM_E_BCERT_DOMAIN_URL_TOO_LONG  ((DRM_RESULT)0x8004C812L)

/*
 * MessageId: DRM_E_BCERT_HARDWARE_ID_NOT_SPECIFIED
 *
 * MessageText:
 *
 * A hardware ID was not provided to the builder
 *
 */
#define DRM_E_BCERT_HARDWARE_ID_NOT_SPECIFIED ((DRM_RESULT)0x8004C813L)

/*
 * MessageId: DRM_E_BCERT_HARDWARE_ID_TOO_LONG
 *
 * MessageText:
 *
 * A hardware ID is longer than the maximum supported bytes
 *
 */
#define DRM_E_BCERT_HARDWARE_ID_TOO_LONG ((DRM_RESULT)0x8004C814L)

/*
 * MessageId: DRM_E_BCERT_SERIAL_NUM_NOT_SPECIFIED
 *
 * MessageText:
 *
 * A device serial number was not provided to the builder
 *
 */
#define DRM_E_BCERT_SERIAL_NUM_NOT_SPECIFIED ((DRM_RESULT)0x8004C815L)

/*
 * MessageId: DRM_E_BCERT_CERT_ID_NOT_SPECIFIED
 *
 * MessageText:
 *
 * A certificate ID was not provided to the builder
 *
 */
#define DRM_E_BCERT_CERT_ID_NOT_SPECIFIED ((DRM_RESULT)0x8004C816L)

/*
 * MessageId: DRM_E_BCERT_PUBLIC_KEY_NOT_SPECIFIED
 *
 * MessageText:
 *
 * A public key for the certificate was not provided to the builder or not found by the parser
 *
 */
#define DRM_E_BCERT_PUBLIC_KEY_NOT_SPECIFIED ((DRM_RESULT)0x8004C817L)

/*
 * MessageId: DRM_E_BCERT_KEY_USAGES_NOT_SPECIFIED
 *
 * MessageText:
 *
 * The public key usage information was not provided to the builder or not found by the parser
 *
 */
#define DRM_E_BCERT_KEY_USAGES_NOT_SPECIFIED ((DRM_RESULT)0x8004C818L)

/*
 * MessageId: DRM_E_BCERT_STRING_NOT_NULL_TERMINATED
 *
 * MessageText:
 *
 * Data string is not null-teminated
 *
 */
#define DRM_E_BCERT_STRING_NOT_NULL_TERMINATED ((DRM_RESULT)0x8004C819L)

/*
 * MessageId: DRM_E_BCERT_OBJECTHEADER_LEN_TOO_BIG
 *
 * MessageText:
 *
 * Object length in object header is too big
 *
 */
#define DRM_E_BCERT_OBJECTHEADER_LEN_TOO_BIG ((DRM_RESULT)0x8004C81AL)

/*
 * MessageId: DRM_E_BCERT_INVALID_ISSUERKEY_LENGTH
 *
 * MessageText:
 *
 * IssuerKey Length value is invalid
 *
 */
#define DRM_E_BCERT_INVALID_ISSUERKEY_LENGTH ((DRM_RESULT)0x8004C81BL)

/*
 * MessageId: DRM_E_BCERT_BASICINFO_CERT_EXPIRED
 *
 * MessageText:
 *
 * Certificate is expired
 *
 */
#define DRM_E_BCERT_BASICINFO_CERT_EXPIRED ((DRM_RESULT)0x8004C81CL)

/*
 * MessageId: DRM_E_BCERT_UNEXPECTED_OBJECT_HEADER
 *
 * MessageText:
 *
 * Object header has unexpected values
 *
 */
#define DRM_E_BCERT_UNEXPECTED_OBJECT_HEADER ((DRM_RESULT)0x8004C81DL)

/*
 * MessageId: DRM_E_BCERT_ISSUERKEY_KEYINFO_MISMATCH
 *
 * MessageText:
 *
 * The cert's Issuer Key does not match key info in the next cert
 *
 */
#define DRM_E_BCERT_ISSUERKEY_KEYINFO_MISMATCH ((DRM_RESULT)0x8004C81EL)

/*
 * MessageId: DRM_E_BCERT_INVALID_MAX_KEY_USAGES
 *
 * MessageText:
 *
 * Number of key usage entries is invalid
 *
 */
#define DRM_E_BCERT_INVALID_MAX_KEY_USAGES ((DRM_RESULT)0x8004C81FL)

/*
 * MessageId: DRM_E_BCERT_INVALID_MAX_FEATURES
 *
 * MessageText:
 *
 * Number of features is invalid
 *
 */
#define DRM_E_BCERT_INVALID_MAX_FEATURES ((DRM_RESULT)0x8004C820L)

/*
 * MessageId: DRM_E_BCERT_INVALID_CHAIN_HEADER_TAG
 *
 * MessageText:
 *
 * Cert chain header tag is invalid
 *
 */
#define DRM_E_BCERT_INVALID_CHAIN_HEADER_TAG ((DRM_RESULT)0x8004C821L)

/*
 * MessageId: DRM_E_BCERT_INVALID_CHAIN_VERSION
 *
 * MessageText:
 *
 * Cert chain version is invalid
 *
 */
#define DRM_E_BCERT_INVALID_CHAIN_VERSION ((DRM_RESULT)0x8004C822L)

/*
 * MessageId: DRM_E_BCERT_INVALID_CHAIN_LENGTH
 *
 * MessageText:
 *
 * Cert chain length value is invalid
 *
 */
#define DRM_E_BCERT_INVALID_CHAIN_LENGTH ((DRM_RESULT)0x8004C823L)

/*
 * MessageId: DRM_E_BCERT_INVALID_CERT_HEADER_TAG
 *
 * MessageText:
 *
 * Cert header tag is invalid
 *
 */
#define DRM_E_BCERT_INVALID_CERT_HEADER_TAG ((DRM_RESULT)0x8004C824L)

/*
 * MessageId: DRM_E_BCERT_INVALID_CERT_VERSION
 *
 * MessageText:
 *
 * Cert version is invalid
 *
 */
#define DRM_E_BCERT_INVALID_CERT_VERSION ((DRM_RESULT)0x8004C825L)

/*
 * MessageId: DRM_E_BCERT_INVALID_CERT_LENGTH
 *
 * MessageText:
 *
 * Cert length value is invalid
 *
 */
#define DRM_E_BCERT_INVALID_CERT_LENGTH  ((DRM_RESULT)0x8004C826L)

/*
 * MessageId: DRM_E_BCERT_INVALID_SIGNEDCERT_LENGTH
 *
 * MessageText:
 *
 * Length of signed portion of certificate is invalid
 *
 */
#define DRM_E_BCERT_INVALID_SIGNEDCERT_LENGTH ((DRM_RESULT)0x8004C827L)

/*
 * MessageId: DRM_E_BCERT_INVALID_PLATFORM_IDENTIFIER
 *
 * MessageText:
 *
 * An invalid Platform Identifier was specified
 *
 */
#define DRM_E_BCERT_INVALID_PLATFORM_IDENTIFIER ((DRM_RESULT)0x8004C828L)

/*
 * MessageId: DRM_E_BCERT_INVALID_NUMBER_EXTDATARECORDS
 *
 * MessageText:
 *
 * An invalid number of extended data records
 *
 */
#define DRM_E_BCERT_INVALID_NUMBER_EXTDATARECORDS ((DRM_RESULT)0x8004C829L)

/*
 * MessageId: DRM_E_BCERT_INVALID_EXTDATARECORD
 *
 * MessageText:
 *
 * An invalid extended data record
 *
 */
#define DRM_E_BCERT_INVALID_EXTDATARECORD ((DRM_RESULT)0x8004C82AL)

/*
 * MessageId: DRM_E_BCERT_EXTDATA_LENGTH_MUST_PRESENT
 *
 * MessageText:
 *
 * Extended data record length must be present.
 *
 */
#define DRM_E_BCERT_EXTDATA_LENGTH_MUST_PRESENT ((DRM_RESULT)0x8004C82BL)

/*
 * MessageId: DRM_E_BCERT_EXTDATA_PRIVKEY_MUST_PRESENT
 *
 * MessageText:
 *
 * Extended data record length must be present.
 *
 */
#define DRM_E_BCERT_EXTDATA_PRIVKEY_MUST_PRESENT ((DRM_RESULT)0x8004C82CL)

/*
 * MessageId: DRM_E_BCERT_INVALID_EXTDATA_LENGTH
 *
 * MessageText:
 *
 * Calculated and written extended data object lengths do not match.
 *
 */
#define DRM_E_BCERT_INVALID_EXTDATA_LENGTH ((DRM_RESULT)0x8004C82DL)

/*
 * MessageId: DRM_E_BCERT_EXTDATA_IS_NOT_PROVIDED
 *
 * MessageText:
 *
 * Extended data is not provided, the cert builder cannot write it.
 *
 */
#define DRM_E_BCERT_EXTDATA_IS_NOT_PROVIDED ((DRM_RESULT)0x8004C82EL)

/*
 * MessageId: DRM_E_BCERT_HWIDINFO_IS_MISSING
 *
 * MessageText:
 *
 * The PC certificate is correct but is not ready to use because has no HWID information
 *
 */
#define DRM_E_BCERT_HWIDINFO_IS_MISSING  ((DRM_RESULT)0x8004C82FL)

/*
 * MessageId: DRM_E_BCERT_INVALID_EXTDATA_SIGNED_LENGTH
 *
 * MessageText:
 *
 * Length of signed portion of extended data info is invalid
 *
 */
#define DRM_E_BCERT_INVALID_EXTDATA_SIGNED_LENGTH ((DRM_RESULT)0x8004C830L)

/*
 * MessageId: DRM_E_BCERT_INVALID_EXTDATA_RECORD_TYPE
 *
 * MessageText:
 *
 * Extended data record type is invalid
 *
 */
#define DRM_E_BCERT_INVALID_EXTDATA_RECORD_TYPE ((DRM_RESULT)0x8004C831L)

/*
 * MessageId: DRM_E_BCERT_EXTDATAFLAG_CERT_TYPE_MISMATCH
 *
 * MessageText:
 *
 * Certificate of this type cannot have extended data flag set
 *
 */
#define DRM_E_BCERT_EXTDATAFLAG_CERT_TYPE_MISMATCH ((DRM_RESULT)0x8004C832L)

/*
 * MessageId: DRM_E_BCERT_METERING_ID_NOT_SPECIFIED
 *
 * MessageText:
 *
 * An metering ID was not provided to the builder
 *
 */
#define DRM_E_BCERT_METERING_ID_NOT_SPECIFIED ((DRM_RESULT)0x8004C833L)

/*
 * MessageId: DRM_E_BCERT_METERING_URL_NOT_SPECIFIED
 *
 * MessageText:
 *
 * A metering URL was not provided to the builder
 *
 */
#define DRM_E_BCERT_METERING_URL_NOT_SPECIFIED ((DRM_RESULT)0x8004C834L)

/*
 * MessageId: DRM_E_BCERT_METERING_URL_TOO_LONG
 *
 * MessageText:
 *
 * The metering URL contains too many ASCII characters
 *
 */
#define DRM_E_BCERT_METERING_URL_TOO_LONG ((DRM_RESULT)0x8004C835L)

/*
 * MessageId: DRM_E_BCERT_VERIFICATION_ERRORS
 *
 * MessageText:
 *
 * Verification errors are found while parsing cert chain
 *
 */
#define DRM_E_BCERT_VERIFICATION_ERRORS  ((DRM_RESULT)0x8004C836L)

/*
 * MessageId: DRM_E_BCERT_REQUIRED_KEYUSAGE_MISSING
 *
 * MessageText:
 *
 * Required key usage is missing
 *
 */
#define DRM_E_BCERT_REQUIRED_KEYUSAGE_MISSING ((DRM_RESULT)0x8004C837L)

/*
 * MessageId: DRM_E_BCERT_NO_PUBKEY_WITH_REQUESTED_KEYUSAGE
 *
 * MessageText:
 *
 * The certificate does not contain a public key with the requested key usage
 *
 */
#define DRM_E_BCERT_NO_PUBKEY_WITH_REQUESTED_KEYUSAGE ((DRM_RESULT)0x8004C838L)

/*
 * MessageId: DRM_E_BCERT_MANUFACTURER_STRING_TOO_LONG
 *
 * MessageText:
 *
 * The manufacturer string is too long
 *
 */
#define DRM_E_BCERT_MANUFACTURER_STRING_TOO_LONG ((DRM_RESULT)0x8004C839L)

/*
 * MessageId: DRM_E_BCERT_TOO_MANY_PUBLIC_KEYS
 *
 * MessageText:
 *
 * There are too many public keys in the certificate
 *
 */
#define DRM_E_BCERT_TOO_MANY_PUBLIC_KEYS ((DRM_RESULT)0x8004C83AL)

/*
 * MessageId: DRM_E_BCERT_OBJECTHEADER_LEN_TOO_SMALL
 *
 * MessageText:
 *
 * Object length in object header is too small
 *
 */
#define DRM_E_BCERT_OBJECTHEADER_LEN_TOO_SMALL ((DRM_RESULT)0x8004C83BL)

/*
 * MessageId: DRM_E_BCERT_INVALID_WARNING_DAYS
 *
 * MessageText:
 *
 * An invalid server certificate expiration warning days. Warning days must be greater than zero.
 *
 */
#define DRM_E_BCERT_INVALID_WARNING_DAYS ((DRM_RESULT)0x8004C83CL)


/* ------------------------------------------------------------
**
** XML Signature/Encryption errors: error codes from DRM_E_BASECODE+0x900
** to DRM_E_BASECODE+0x9FF, 0x8004c900-0x8004c9ff.
**
** ------------------------------------------------------------
*/

#define DRM_E_XMLSIG_BASECODE       DRM_E_BASECODE + 0x900

/*
 * MessageId: DRM_E_XMLSIG_ECDSA_VERIFY_FAILURE
 *
 * MessageText:
 *
 * Error in ECDSA signature verification.
 *
 */
#define DRM_E_XMLSIG_ECDSA_VERIFY_FAILURE ((DRM_RESULT)0x8004C900L)

/*
 * MessageId: DRM_E_XMLSIG_SHA_VERIFY_FAILURE
 *
 * MessageText:
 *
 * Error in SHA verification.
 *
 */
#define DRM_E_XMLSIG_SHA_VERIFY_FAILURE  ((DRM_RESULT)0x8004C901L)

/*
 * MessageId: DRM_E_XMLSIG_FORMAT
 *
 * MessageText:
 *
 * The format of XML signature or encryption segment is incorrect.
 *
 */
#define DRM_E_XMLSIG_FORMAT              ((DRM_RESULT)0x8004C902L)

/*
 * MessageId: DRM_E_XMLSIG_PUBLIC_KEY_ID
 *
 * MessageText:
 *
 * Invalud pre-shared public key ID.
 *
 */
#define DRM_E_XMLSIG_PUBLIC_KEY_ID       ((DRM_RESULT)0x8004C903L)

/*
 * MessageId: DRM_E_XMLSIG_INVALID_KEY_FORMAT
 *
 * MessageText:
 *
 * Invalid type of public/private key format.
 *
 */
#define DRM_E_XMLSIG_INVALID_KEY_FORMAT  ((DRM_RESULT)0x8004C904L)

/*
 * MessageId: DRM_E_XMLSIG_SHA_HASH_SIZE
 *
 * MessageText:
 *
 * Size of hash is unexpected.
 *
 */
#define DRM_E_XMLSIG_SHA_HASH_SIZE       ((DRM_RESULT)0x8004C905L)

/*
 * MessageId: DRM_E_XMLSIG_ECDSA_SIGNATURE_SIZE
 *
 * MessageText:
 *
 * Size of ECDSA signature is unexpected.
 *
 */
#define DRM_E_XMLSIG_ECDSA_SIGNATURE_SIZE ((DRM_RESULT)0x8004C906L)


/* ------------------------------------------------------------
**
** UTF8 encoding errors: error codes from DRM_E_BASECODE+0xA00
** to DRM_E_BASECODE+0xAFF, 0x8004ca00-0x8004caff.
**
** ------------------------------------------------------------
*/

#define DRM_E_UTF_BASECODE       DRM_E_BASECODE + 0xa00

/*
 * MessageId: DRM_E_UTF_UNEXPECTED_END
 *
 * MessageText:
 *
 * Unexpected end of data in the middle of multibyte character.
 *
 */
#define DRM_E_UTF_UNEXPECTED_END         ((DRM_RESULT)0x8004CA00L)

/*
 * MessageId: DRM_E_UTF_INVALID_CODE
 *
 * MessageText:
 *
 * UTF character maps into a code with invalid value.
 *
 */
#define DRM_E_UTF_INVALID_CODE           ((DRM_RESULT)0x8004CA01L)


/* ------------------------------------------------------------
**
** XML SOAP errors: error codes from DRM_E_BASECODE+0xB00
** to DRM_E_BASECODE+0xBFF, 0x8004cb00-0x8004cbff.
**
** ------------------------------------------------------------
*/

#define DRM_E_SOAPXML_BASECODE       DRM_E_BASECODE + 0xb00

/*
 * MessageId: DRM_E_SOAPXML_INVALID_STATUS_CODE
 *
 * MessageText:
 *
 * Status code contained in the server error response is invalid.
 *
 */
#define DRM_E_SOAPXML_INVALID_STATUS_CODE ((DRM_RESULT)0x8004CB00L)

/*
 * MessageId: DRM_E_SOAPXML_XML_FORMAT
 *
 * MessageText:
 *
 * Cannot parse out expected XML node.
 *
 */
#define DRM_E_SOAPXML_XML_FORMAT         ((DRM_RESULT)0x8004CB01L)

/*
 * MessageId: DRM_E_SOAPXML_WRONG_MESSAGE_TYPE
 *
 * MessageText:
 *
 * The message type associated with the soap message is wrong.
 *
 */
#define DRM_E_SOAPXML_WRONG_MESSAGE_TYPE ((DRM_RESULT)0x8004CB02L)

/*
 * MessageId: DRM_E_SOAPXML_SIGNATURE_MISSING
 *
 * MessageText:
 *
 * The message did not have a signature and needed one
 *
 */
#define DRM_E_SOAPXML_SIGNATURE_MISSING  ((DRM_RESULT)0x8004CB03L)

/*
 * MessageId: DRM_E_SOAPXML_PROTOCOL_NOT_SUPPORTED
 *
 * MessageText:
 *
 * The requested protocol is not supported by the DRM SOAP parser.
 *
 */
#define DRM_E_SOAPXML_PROTOCOL_NOT_SUPPORTED ((DRM_RESULT)0x8004CB04L)

/*
 * MessageId: DRM_E_SOAPXML_DATA_NOT_FOUND
 *
 * MessageText:
 *
 * The requested data is not found in the response.
 *
 */
#define DRM_E_SOAPXML_DATA_NOT_FOUND     ((DRM_RESULT)0x8004CB05L)


/* ------------------------------------------------------------
**
** Generic crypto errors: error codes from DRM_E_BASECODE+0xC00
** to DRM_E_BASECODE+0xCFF, 0x8004cc00-0x8004ccff.
**
** ------------------------------------------------------------
*/

#define DRM_E_CRYPTO_BASECODE       DRM_E_BASECODE + 0xc00

/*
 * MessageId: DRM_E_CRYPTO_PUBLIC_KEY_NOT_MATCH
 *
 * MessageText:
 *
 * The public key associated with an encrypted domain private from the server does not match any public key on the device.
 *
 */
#define DRM_E_CRYPTO_PUBLIC_KEY_NOT_MATCH ((DRM_RESULT)0x8004CC00L)

/*
 * MessageId: DRM_E_UNABLE_TO_RESOLVE_LOCATION_TREE
 *
 * MessageText:
 *
 * Unable to derive the key.  Maybe due to blackout or no rights to the service, etc.
 *
 */
#define DRM_E_UNABLE_TO_RESOLVE_LOCATION_TREE ((DRM_RESULT)0x8004CC01L)


/* ------------------------------------------------------------
**
** Secure tracing errors: error codes from DRM_E_BASECODE+0xD00
** to DRM_E_BASECODE+0xDFF, 0x8004cd00-0x8004cdff.
**
** ------------------------------------------------------------
*/

#define DRM_E_SECURETRACE_BASECODE       DRM_E_BASECODE + 0xd00

/*
 * MessageId: DRM_E_SECURE_TRACE_BAD_GLOBAL_DATA_POINTER
 *
 * MessageText:
 *
 * The secure trace global data pointer is NULL
 *
 */
#define DRM_E_SECURE_TRACE_BAD_GLOBAL_DATA_POINTER ((DRM_RESULT)0x8004CD00L)

/*
 * MessageId: DRM_E_SECURE_TRACE_INVALID_GLOBAL_DATA
 *
 * MessageText:
 *
 * The secure trace global data structure is invalid
 *
 */
#define DRM_E_SECURE_TRACE_INVALID_GLOBAL_DATA ((DRM_RESULT)0x8004CD01L)

/*
 * MessageId: DRM_E_SECURE_TRACE_FORMATTING_ERROR
 *
 * MessageText:
 *
 * An error occured in formatting the trace message
 *
 */
#define DRM_E_SECURE_TRACE_FORMATTING_ERROR ((DRM_RESULT)0x8004CD02L)

/*
 * MessageId: DRM_E_SECURE_TRACE_BAD_SCHEME_DATA_POINTER
 *
 * MessageText:
 *
 * A secure trace scheme data pointer is NULL
 *
 */
#define DRM_E_SECURE_TRACE_BAD_SCHEME_DATA_POINTER ((DRM_RESULT)0x8004CD03L)

/*
 * MessageId: DRM_E_SECURE_TRACE_BAD_PER_THREAD_AES_DATA_POINTER
 *
 * MessageText:
 *
 * The secure trace per thread AES data pointer is NULL
 *
 */
#define DRM_E_SECURE_TRACE_BAD_PER_THREAD_AES_DATA_POINTER ((DRM_RESULT)0x8004CD04L)

/*
 * MessageId: DRM_E_SECURE_TRACE_BAD_PER_THREAD_AES_BUFFER_POINTER
 *
 * MessageText:
 *
 * A secure trace per thread AES buffer pointer is NULL
 *
 */
#define DRM_E_SECURE_TRACE_BAD_PER_THREAD_AES_BUFFER_POINTER ((DRM_RESULT)0x8004CD05L)

/*
 * MessageId: DRM_E_SECURE_TRACE_AES_INSUFFICIENT_BUFFER
 *
 * MessageText:
 *
 * There is no space left in the secure trace AES buffer
 *
 */
#define DRM_E_SECURE_TRACE_AES_INSUFFICIENT_BUFFER ((DRM_RESULT)0x8004CD06L)

/*
 * MessageId: DRM_E_SECURE_TRACE_VERSION_MISMATCH
 *
 * MessageText:
 *
 * All drm dlls do not agree on the same secure trace version
 *
 */
#define DRM_E_SECURE_TRACE_VERSION_MISMATCH ((DRM_RESULT)0x8004CD07L)

/*
 * MessageId: DRM_E_SECURE_TRACE_UNEXPECTED_ERROR
 *
 * MessageText:
 *
 * An expected error was encountered in secure tracing system
 *
 */
#define DRM_E_SECURE_TRACE_UNEXPECTED_ERROR ((DRM_RESULT)0x8004CD08L)


/* ------------------------------------------------------------
**
** NDT/NDR results: error codes from DRM_E_BASECODE+0xE00
** to DRM_E_BASECODE+0xEFF, 0x8004ce00-0x8004ceff.
**
** ------------------------------------------------------------
*/

#define DRM_ND_BASECODE           DRM_E_BASECODE + 0xE00

/*
 * MessageId: DRM_E_ND_MUST_REVALIDATE
 *
 * MessageText:
 *
 * The client must be revalidated before executing the intended operation.
 *
 */
#define DRM_E_ND_MUST_REVALIDATE         ((DRM_RESULT)0x8004CE00L)

/*
 * MessageId: DRM_E_ND_INVALID_MESSAGE
 *
 * MessageText:
 *
 * A received message is garbled.
 *
 */
#define DRM_E_ND_INVALID_MESSAGE         ((DRM_RESULT)0x8004CE01L)

/*
 * MessageId: DRM_E_ND_INVALID_MESSAGE_TYPE
 *
 * MessageText:
 *
 * A received message contains an invalid message type.
 *
 */
#define DRM_E_ND_INVALID_MESSAGE_TYPE    ((DRM_RESULT)0x8004CE02L)

/*
 * MessageId: DRM_E_ND_INVALID_MESSAGE_VERSION
 *
 * MessageText:
 *
 * A received message contains an invalid message version.
 *
 */
#define DRM_E_ND_INVALID_MESSAGE_VERSION ((DRM_RESULT)0x8004CE03L)

/*
 * MessageId: DRM_E_ND_INVALID_SESSION
 *
 * MessageText:
 *
 * The requested session is invalid.
 *
 */
#define DRM_E_ND_INVALID_SESSION         ((DRM_RESULT)0x8004CE04L)

/*
 * MessageId: DRM_E_ND_MEDIA_SESSION_LIMIT_REACHED
 *
 * MessageText:
 *
 * A new session cannot be opened because the maximum number of sessions has already been opened.
 *
 */
#define DRM_E_ND_MEDIA_SESSION_LIMIT_REACHED ((DRM_RESULT)0x8004CE05L)

/*
 * MessageId: DRM_E_ND_UNABLE_TO_VERIFY_PROXIMITY
 *
 * MessageText:
 *
 * The proximity detection procedure could not confirm that the receiver is near the transmitter in the network.
 *
 */
#define DRM_E_ND_UNABLE_TO_VERIFY_PROXIMITY ((DRM_RESULT)0x8004CE06L)

/*
 * MessageId: DRM_E_ND_INVALID_PROXIMITY_RESPONSE
 *
 * MessageText:
 *
 * The response to the proximity detection challenge is invalid.
 *
 */
#define DRM_E_ND_INVALID_PROXIMITY_RESPONSE ((DRM_RESULT)0x8004CE07L)

/*
 * MessageId: DRM_E_ND_DEVICE_LIMIT_REACHED
 *
 * MessageText:
 *
 * The maximum number of devices in use has been reached. Unable to open additional devices.
 *
 */
#define DRM_E_ND_DEVICE_LIMIT_REACHED    ((DRM_RESULT)0x8004CE08L)

/*
 * MessageId: DRM_E_ND_BAD_REQUEST
 *
 * MessageText:
 *
 * The message format is invalid.
 *
 */
#define DRM_E_ND_BAD_REQUEST             ((DRM_RESULT)0x8004CE09L)

/*
 * MessageId: DRM_E_ND_FAILED_SEEK
 *
 * MessageText:
 *
 * It is not possible to seek to the specified mark-in point.
 *
 */
#define DRM_E_ND_FAILED_SEEK             ((DRM_RESULT)0x8004CE0AL)

/*
 * MessageId: DRM_E_ND_INVALID_CONTEXT
 *
 * MessageText:
 *
 * Manager context or at least one of it's children is missing (or corrupt).
 *
 */
#define DRM_E_ND_INVALID_CONTEXT         ((DRM_RESULT)0x8004CE0BL)


/* ------------------------------------------------------------
**
** ASF results: error codes from DRM_E_BASECODE+0xF00
** to DRM_E_BASECODE+0xFFF, 0x8004ce00-0x8004cfff.
**
** ------------------------------------------------------------
*/

#define DRM_ASF_BASECODE           DRM_E_BASECODE + 0xF00

/* ASF Parsing Errors */
/* ------------------ */

/*
 * MessageId: DRM_E_ASF_BAD_ASF_HEADER
 *
 * MessageText:
 *
 * The ASF file has a bad ASF header.
 *
 */
#define DRM_E_ASF_BAD_ASF_HEADER         ((DRM_RESULT)0x8004CF00L)

/*
 * MessageId: DRM_E_ASF_BAD_PACKET_HEADER
 *
 * MessageText:
 *
 * The ASF file has a bad packet header.
 *
 */
#define DRM_E_ASF_BAD_PACKET_HEADER      ((DRM_RESULT)0x8004CF01L)

/*
 * MessageId: DRM_E_ASF_BAD_PAYLOAD_HEADER
 *
 * MessageText:
 *
 * The ASF file has a bad payload header.
 *
 */
#define DRM_E_ASF_BAD_PAYLOAD_HEADER     ((DRM_RESULT)0x8004CF02L)

/*
 * MessageId: DRM_E_ASF_BAD_DATA_HEADER
 *
 * MessageText:
 *
 * The ASF file has a bad data header.
 *
 */
#define DRM_E_ASF_BAD_DATA_HEADER        ((DRM_RESULT)0x8004CF03L)

/*
 * MessageId: DRM_E_ASF_INVALID_OPERATION
 *
 * MessageText:
 *
 * The intended operation is invalid given the current processing state of the ASF file.
 *
 */
#define DRM_E_ASF_INVALID_OPERATION      ((DRM_RESULT)0x8004CF04L)

/*
 * MessageId: DRM_E_ASF_AES_PAYLOAD_FOUND
 *
 * MessageText:
 *
 * ND payload extension system found; the file may be encrypted with AES already.
 *
 */
#define DRM_E_ASF_AES_PAYLOAD_FOUND      ((DRM_RESULT)0x8004CF05L)

/*
 * MessageId: DRM_E_ASF_EXTENDED_STREAM_PROPERTIES_OBJ_NOT_FOUND
 *
 * MessageText:
 *
 * Extended stream properties object is not found; the file may be in non-supported outdated format.
 *
 */
#define DRM_E_ASF_EXTENDED_STREAM_PROPERTIES_OBJ_NOT_FOUND ((DRM_RESULT)0x8004CF06L)


/* ASF Muxing Errors */
/* ----------------- */

/*
 * MessageId: DRM_E_ASF_INVALID_DATA
 *
 * MessageText:
 *
 * The packet is overstuffed with data.
 *
 */
#define DRM_E_ASF_INVALID_DATA           ((DRM_RESULT)0x8004CF20L)

/*
 * MessageId: DRM_E_ASF_TOO_MANY_PAYLOADS
 *
 * MessageText:
 *
 * The number of payloads in the packet is greater than the maximum allowed.
 *
 */
#define DRM_E_ASF_TOO_MANY_PAYLOADS      ((DRM_RESULT)0x8004CF21L)

/*
 * MessageId: DRM_E_ASF_BANDWIDTH_OVERRUN
 *
 * MessageText:
 *
 * An object is overflowing the leaky bucket.
 *
 */
#define DRM_E_ASF_BANDWIDTH_OVERRUN      ((DRM_RESULT)0x8004CF22L)

/*
 * MessageId: DRM_E_ASF_INVALID_STREAM_NUMBER
 *
 * MessageText:
 *
 * The stream number is invalid; it is either zero, greater than the maximum value allowed, or has no associated data.
 *
 */
#define DRM_E_ASF_INVALID_STREAM_NUMBER  ((DRM_RESULT)0x8004CF23L)

/*
 * MessageId: DRM_E_ASF_LATE_SAMPLE
 *
 * MessageText:
 *
 * A sample was encountered with a presentation time outside of the mux's send window.
 *
 */
#define DRM_E_ASF_LATE_SAMPLE            ((DRM_RESULT)0x8004CF24L)

/*
 * MessageId: DRM_E_ASF_NOT_ACCEPTING
 *
 * MessageText:
 *
 * The sample does not fit in the remaining payload space.
 *
 */
#define DRM_E_ASF_NOT_ACCEPTING          ((DRM_RESULT)0x8004CF25L)

/*
 * MessageId: DRM_E_ASF_UNEXPECTED
 *
 * MessageText:
 *
 * An unexpected error occurred.
 *
 */
#define DRM_E_ASF_UNEXPECTED             ((DRM_RESULT)0x8004CF26L)


/* ------------------------------------------------------------
**
** Nonce store errors: error codes from DRM_E_BASECODE+0x1000
** to DRM_E_BASECODE+0x10FF, 0x8004d000-0x8004d0ff.
**
** ------------------------------------------------------------
*/

#define DRM_E_NONCE_STORE_BASECODE       DRM_E_BASECODE + 0x1000

/*
 * MessageId: DRM_E_NONCE_STORE_TOKEN_NOT_FOUND
 *
 * MessageText:
 *
 * The matching nonce store token is not found.
 *
 */
#define DRM_E_NONCE_STORE_TOKEN_NOT_FOUND ((DRM_RESULT)0x8004D000L)

/*
 * MessageId: DRM_E_NONCE_STORE_OPEN_STORE
 *
 * MessageText:
 *
 * Fail to open nonce store.
 *
 */
#define DRM_E_NONCE_STORE_OPEN_STORE     ((DRM_RESULT)0x8004D001L)

/*
 * MessageId: DRM_E_NONCE_STORE_CLOSE_STORE
 *
 * MessageText:
 *
 * Fail to close nonce store.
 *
 */
#define DRM_E_NONCE_STORE_CLOSE_STORE    ((DRM_RESULT)0x8004D002L)

/*
 * MessageId: DRM_E_NONCE_STORE_ADD_LICENSE
 *
 * MessageText:
 *
 * There is already a license associated with the nonce store token.
 *
 */
#define DRM_E_NONCE_STORE_ADD_LICENSE    ((DRM_RESULT)0x8004D003L)


/* ------------------------------------------------------------
**
** License Generation errors: error codes from DRM_E_BASECODE+0x1100 to
** DRM_E_BASECODE+0x11FF, 0x8004d100-0x8004d1ff.
**
** ------------------------------------------------------------
*/

#define DRM_E_LICGEN_BASECODE                  DRM_E_BASECODE+0x1100

/*
 * MessageId: DRM_E_LICGEN_POLICY_NOT_SUPPORTED
 *
 * MessageText:
 *
 * The license generation policy combination is not supported.
 *
 */
#define DRM_E_LICGEN_POLICY_NOT_SUPPORTED ((DRM_RESULT)0x8004D100L)


/* ------------------------------------------------------------
**
** Policy State errors: error codes from DRM_E_BASECODE+0x1200 to
** DRM_E_BASECODE+0x12FF, 0x8004d200-0x8004d2ff.
**
** ------------------------------------------------------------
*/

#define DRM_E_POLICYSTATE_BASECODE              DRM_E_BASECODE+0x1200

/*
 * MessageId: DRM_E_POLICYSTATE_NOT_FOUND
 *
 * MessageText:
 *
 * The policy state is not found in the secure store.
 *
 */
#define DRM_E_POLICYSTATE_NOT_FOUND      ((DRM_RESULT)0x8004D200L)

/*
 * MessageId: DRM_E_POLICYSTATE_CORRUPTED
 *
 * MessageText:
 *
 * The policy state is not stored as a valid internal format in the secure store.
 *
 */
#define DRM_E_POLICYSTATE_CORRUPTED      ((DRM_RESULT)0x8004D201L)


/* ------------------------------------------------------------
**
** Move errors: error codes from DRM_E_BASECODE+0x1300 to
** DRM_E_BASECODE+0x13FF, 0x8004d300-0x8004d3ff.
**
** ------------------------------------------------------------
*/

#define DRM_E_MOVE_BASECODE              DRM_E_BASECODE+0x1300

/*
 * MessageId: DRM_E_MOVE_DENIED
 *
 * MessageText:
 *
 * The requested move operation was denied by the service.
 *
 */
#define DRM_E_MOVE_DENIED                ((DRM_RESULT)0x8004D300L)

/*
 * MessageId: DRM_E_INVALID_MOVE_RESPONSE
 *
 * MessageText:
 *
 * The move response was incorrectly formed.
 *
 */
#define DRM_E_INVALID_MOVE_RESPONSE      ((DRM_RESULT)0x8004D301L)

/*
 * MessageId: DRM_E_MOVE_NONCE_MISMATCH
 *
 * MessageText:
 *
 * The nonce in the repsonse did not match the expected value.
 *
 */
#define DRM_E_MOVE_NONCE_MISMATCH        ((DRM_RESULT)0x8004D302L)

/*
 * MessageId: DRM_E_MOVE_TXID_MISMATCH
 *
 * MessageText:
 *
 * The transaction id in the repsonse did not match the expected value.
 *
 */
#define DRM_E_MOVE_TXID_MISMATCH         ((DRM_RESULT)0x8004D303L)

/*
 * MessageId: DRM_E_MOVE_STORE_OPEN_STORE
 *
 * MessageText:
 *
 * Failed to open the move store.
 *
 */
#define DRM_E_MOVE_STORE_OPEN_STORE      ((DRM_RESULT)0x8004D304L)

/*
 * MessageId: DRM_E_MOVE_STORE_CLOSE_STORE
 *
 * MessageText:
 *
 * Failed to close the move store.
 *
 */
#define DRM_E_MOVE_STORE_CLOSE_STORE     ((DRM_RESULT)0x8004D305L)

/*
 * MessageId: DRM_E_MOVE_STORE_ADD_DATA
 *
 * MessageText:
 *
 * Failed to add data into the move store.
 *
 */
#define DRM_E_MOVE_STORE_ADD_DATA        ((DRM_RESULT)0x8004D306L)

/*
 * MessageId: DRM_E_MOVE_STORE_GET_DATA
 *
 * MessageText:
 *
 * Failed to retrieve data from the move store.
 *
 */
#define DRM_E_MOVE_STORE_GET_DATA        ((DRM_RESULT)0x8004D307L)

/*
 * MessageId: DRM_E_MOVE_FORMAT_INVALID
 *
 * MessageText:
 *
 * The format of a move page or index is invalid.
 *
 */
#define DRM_E_MOVE_FORMAT_INVALID        ((DRM_RESULT)0x8004D308L)

/*
 * MessageId: DRM_E_MOVE_SIGNATURE_INVALID
 *
 * MessageText:
 *
 * The signature of a move index is invalid.
 *
 */
#define DRM_E_MOVE_SIGNATURE_INVALID     ((DRM_RESULT)0x8004D309L)

/*
 * MessageId: DRM_E_COPY_DENIED
 *
 * MessageText:
 *
 * The requested copy operation was denied by the service.
 *
 */
#define DRM_E_COPY_DENIED                ((DRM_RESULT)0x8004D30AL)

/* ------------------------------------------------------------
**
** Extensible Binary errors: error codes from DRM_E_BASECODE+0x1400 to
** DRM_E_BASECODE+0x14FF, 0x8004d400-0x8004d4ff.
**
** ------------------------------------------------------------
*/

#define DRM_E_XB_BASECODE              DRM_E_BASECODE+0x1400

/*
 * MessageId: DRM_E_XB_OBJECT_NOTFOUND
 *
 * MessageText:
 *
 * The extensible binary object was not found.
 *
 */
#define DRM_E_XB_OBJECT_NOTFOUND         ((DRM_RESULT)0x8004D400L)

/*
 * MessageId: DRM_E_XB_INVALID_OBJECT
 *
 * MessageText:
 *
 * The extensible binary object format was invalid.
 *
 */
#define DRM_E_XB_INVALID_OBJECT          ((DRM_RESULT)0x8004D401L)

/*
 * MessageId: DRM_E_XB_OBJECT_ALREADY_EXISTS
 *
 * MessageText:
 *
 * A single instance extensible binary object was encountered more than once.
 *
 */
#define DRM_E_XB_OBJECT_ALREADY_EXISTS   ((DRM_RESULT)0x8004D402L)

/*
 * MessageId: DRM_E_XB_REQUIRED_OBJECT_MISSING
 *
 * MessageText:
 *
 * A required extensible binary object was not found during building.
 *
 */
#define DRM_E_XB_REQUIRED_OBJECT_MISSING ((DRM_RESULT)0x8004D403L)

/*
 * MessageId: DRM_E_XB_UNKNOWN_ELEMENT_TYPE
 *
 * MessageText:
 *
 * An extensible binary object description contained an element of an unknown type.
 *
 */
#define DRM_E_XB_UNKNOWN_ELEMENT_TYPE    ((DRM_RESULT)0x8004D404L)

/* ------------------------------------------------------------
**
** Keyfile errors: error codes from DRM_E_BASECODE+0x1500 to
** DRM_E_BASECODE+0x15FF, 0x8004d500-0x8004d5ff.
**
** ------------------------------------------------------------
*/

#define DRM_E_KEYFILE_BASECODE              DRM_E_BASECODE+0x1500

/*
 * MessageId: DRM_E_KEYFILE_INVALID_PLATFORM
 *
 * MessageText:
 *
 * The keyfile does not support the current platform.
 *
 */
#define DRM_E_KEYFILE_INVALID_PLATFORM   ((DRM_RESULT)0x8004D500L)

/*
 * MessageId: DRM_E_KEYFILE_TOO_LARGE
 *
 * MessageText:
 *
 * The keyfile is larger than the maximum supported size.
 *
 */
#define DRM_E_KEYFILE_TOO_LARGE          ((DRM_RESULT)0x8004D501L)

/*
 * MessageId: DRM_E_KEYFILE_PRIVATE_KEY_NOT_FOUND
 *
 * MessageText:
 *
 * The private key requested was not found in the keyfile.
 *
 */
#define DRM_E_KEYFILE_PRIVATE_KEY_NOT_FOUND ((DRM_RESULT)0x8004D502L)

/*
 * MessageId: DRM_E_KEYFILE_CERTIFICATE_CHAIN_NOT_FOUND
 *
 * MessageText:
 *
 * The certificate chain requested was not found in the keyfile.
 *
 */
#define DRM_E_KEYFILE_CERTIFICATE_CHAIN_NOT_FOUND ((DRM_RESULT)0x8004D503L)

/*
 * MessageId: DRM_E_KEYFILE_KEY_NOT_FOUND
 *
 * MessageText:
 *
 * The AES Key ID was not found in the keyfile.
 *
 */
#define DRM_E_KEYFILE_KEY_NOT_FOUND      ((DRM_RESULT)0x8004D504L)

/*
 * MessageId: DRM_E_KEYFILE_UNKNOWN_DECRYPTION_METHOD
 *
 * MessageText:
 *
 * Unknown keyfile decryption method.
 *
 */
#define DRM_E_KEYFILE_UNKNOWN_DECRYPTION_METHOD ((DRM_RESULT)0x8004D505L)

/*
 * MessageId: DRM_E_KEYFILE_INVALID_SIGNATURE
 *
 * MessageText:
 *
 * The keyfile signature was not valid.
 *
 */
#define DRM_E_KEYFILE_INVALID_SIGNATURE  ((DRM_RESULT)0x8004D506L)

/*
 * MessageId: DRM_E_KEYFILE_INTERNAL_DECRYPTION_BUFFER_TOO_SMALL
 *
 * MessageText:
 *
 * The internal decryption buffer is too small to hold the encrypted key from the keyfile.
 *
 */
#define DRM_E_KEYFILE_INTERNAL_DECRYPTION_BUFFER_TOO_SMALL ((DRM_RESULT)0x8004D507L)

/*
 * MessageId: DRM_E_KEYFILE_PLATFORMID_MISMATCH
 *
 * MessageText:
 *
 * Platform ID in the certificate does not match expected value.
 *
 */
#define DRM_E_KEYFILE_PLATFORMID_MISMATCH ((DRM_RESULT)0x8004D508L)

/*
 * MessageId: DRM_E_KEYFILE_CERTIFICATE_ISSUER_KEY_MISMATCH
 *
 * MessageText:
 *
 * Issuer key of the device certificate does not match public key of the model certificate.
 *
 */
#define DRM_E_KEYFILE_CERTIFICATE_ISSUER_KEY_MISMATCH ((DRM_RESULT)0x8004D509L)

/*
 * MessageId: DRM_E_KEYFILE_ROBUSTNESSVERSION_MISMATCH
 *
 * MessageText:
 *
 * Robustness version in the certificate does not match expected value.
 *
 */
#define DRM_E_KEYFILE_ROBUSTNESSVERSION_MISMATCH ((DRM_RESULT)0x8004D50AL)

/*
 * MessageId: DRM_E_KEYFILE_FILE_NOT_CLOSED
 *
 * MessageText:
 *
 * The KeyFile Close function was not called before trying to unintialize the KeyFile context.
 *
 */
#define DRM_E_KEYFILE_FILE_NOT_CLOSED    ((DRM_RESULT)0x8004D50BL)

/*
 * MessageId: DRM_E_KEYFILE_NOT_INITED
 *
 * MessageText:
 *
 * The KeyFile Context was not initialized before trying to use it.
 *
 */
#define DRM_E_KEYFILE_NOT_INITED         ((DRM_RESULT)0x8004D50CL)

/*
 * MessageId: DRM_E_KEYFILE_FORMAT_INVALID
 *
 * MessageText:
 *
 * The format of the KeyFile was invalid.
 *
 */
#define DRM_E_KEYFILE_FORMAT_INVALID     ((DRM_RESULT)0x8004D50DL)

/*
 * MessageId: DRM_E_KEYFILE_UPDATE_NOT_ALLOWED
 *
 * MessageText:
 *
 * The keyfile of the device is read only, and updates are not permitted.
 *
 */
#define DRM_E_KEYFILE_UPDATE_NOT_ALLOWED ((DRM_RESULT)0x8004D50EL)


/* ------------------------------------------------------------
** DRM_E_BASECODE + 0x1600 - DRM_E_BASECODE + 0x16ff (0x8004d600-0x8004d6ff)
** are reserved for additional DRM Services error codes.
**
** See source\common\services\inc\svcerrors.h for Services error codes.
**
** ------------------------------------------------------------
*/

#define DRM_E_SERVICES_BASECODE_EX     (DRM_E_BASECODE + 0x1600)

/* ------------------------------------------------------------
**
** Available range 0x8004d700-0x8004dfff.
**
** ------------------------------------------------------------
*/

/* Insert new PKCore sub-error facilities here */


/* ============================================================
**
** PC specific errors (from 0x8004e000 to 0x8004ffff)
**
** ============================================================
*/

/* Nothing should be added here - PC error codes are found in DrmPcErrors.h */

#endif /*__DRMRESULTS_H__ */

