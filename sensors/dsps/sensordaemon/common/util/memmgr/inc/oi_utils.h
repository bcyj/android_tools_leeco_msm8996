#ifndef _OI_UTILS_H
#define _OI_UTILS_H
/**
 * @file
 *
 * This file provides the interface for utility functions.
 * Among the utilities are strlen (string length), strcmp (string compare), and
 * other string manipulation functions. These are provided for those plaforms
 * where this functionality is not available in stdlib.
 */

/**********************************************************************************
  $AccuRev-Revision: 1109/4 $
  Copyright 2002 - 2004 Open Interface North America, Inc. All rights reserved.
***********************************************************************************/

#include <stdarg.h>
#include "oi_common.h"
#include "oi_string.h"
//#include "oi_bt_spec.h"

/** \addtogroup Misc Miscellaneous APIs */
/**@{*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque type for a callback function handle. See OI_ScheduleCallbackFunction().
 */
typedef OI_UINT32 OI_CALLBACK_HANDLE;


/**
 * Function prototype for a timed procedure callback.
 *
 * @param arg                 Value that was passed into the OI_ScheduleCallbackFunction() function
 *
 */
typedef void (*OI_SCHEDULED_CALLBACK)(void *arg);


/**
 * This function registers a function to be called when a timeout expires. This API uses BLUEmagic's internal
 * function dispatch mechanism, so applications that make extensive use of this facility may need to
 * increase the value of DispatchTableSize in the configuration block for the dispatcher (see
 * oi_bt_stack_config.h).
 *
 * @param callbackFunction    Specifies the function that will be called when the timeout expires.
 *
 * @param arg                 Value that will be returned as the parameter to the callback function.
 *
 * @param timeout             Timeout value expressed in OI_INTERVALs (tenths of seconds); can be
 *                            zero in which case the callback function will be called as soon as
 *                            possible.
 *
 * @param handle              NULL or a pointer to receive the callback handle.
 *
 * @return                    OI_OK if the function was reqistered, or an error status.
 */
//sean
/* OI_STATUS OI_ScheduleCallbackFunction(OI_SCHEDULED_CALLBACK callbackFunction,
                                      void                 *arg,
                                      OI_INTERVAL           timeout,
                                      OI_CALLBACK_HANDLE   *handle);

*/
/**
 * This function cancels a function registered with OI_ScheduleCallbackFunction() before its timer expires.
 *
 * @param handle              Handle returned by OI_ScheduleCallbackFunction().
 *
 * @return                    OI_OK if the function was cancelled, or an error status.
 */
OI_STATUS OI_CancelCallbackFunction(OI_CALLBACK_HANDLE handle);

/**
 * This function parses a Bluetooth device address from the specified string.
 *
 * @param str   String to parse
 * @param addr  Parsed address, if successful
 *
 * @return TRUE if an address was successfully parsed, FALSE otherwise
 */

/*
OI_BOOL OI_ParseBdAddr(const OI_CHAR *str,
                       OI_BD_ADDR    *addr) ;
*/
/**
 * Printf function for platforms which have no stdio or printf available.
 * OI_Printf supports the basic formatting types, with the exception of
 * floating point types. Additionally, OI_Printf supports several formats
 * specific to BLUEmagic 3.0 software:
 *
 * \%!   Prints the string for an #OI_STATUS value.
 *       @code OI_Printf("There was an error %!", status); @endcode
 *
 * \%@   Prints a hex dump of a buffer.
 *       Requires a pointer to the buffer and a signed integer length
 *       (0 for default length). If the buffer is large, only an excerpt will
 *       be printed.
 *       @code OI_Printf("Contents of buffer %@", buffer, sizeof(buffer)); @endcode
 *
 * \%:   Prints a Bluetooth address in the form "HH:HH:HH:HH:HH:HH".
 *       Requires a pointer to an #OI_BD_ADDR.
 *       @code OI_Printf("Bluetooth address %:", &bdaddr); @endcode
 *
 * \%^   Decodes and prints a data element as formatted XML.
 *       Requires a pointer to an #OI_DATAELEM.
 *       @code OI_Printf("Service attribute list is:\n%^", &attributes); @endcode
 *
 * \%=   Prints an OBEX header. 
 *       Requires a pointer to an #OI_OBEX_HEADER
 *       @code OI_Printf("OBEX header is:\n%:\n", &header); @endcode
 *
 * \%/   Prints the base file name of a path, that is, the final substring
 *       following a '/' or '\\' character. Requires a pointer to a null
 *       terminated string.
 *       @code OI_Printf("File %/", "c:\\dir1\\dir2\\file.txt"); @endcode
 *
 * \%~   Prints a string, escaping characters as needed to display it in
 *       ASCII. Requires a pointer to an #OI_PSTR and an #OI_UNICODE_ENCODING
 *       parameter.
 *       @code OI_Printf("Identifier %~", &id, OI_UNICODE_UTF16_BE); @endcode
 *
 * \%[   Inserts an ANSI color escape sequence. Requires a single character
 *       identifying the color to select. Colors are red (r/R), green (g/G),
 *       blue (b/B), yellow (y/Y), cyan (c/C), magenta (m/M), white (W),
 *       light-gray (l/L), dark-gray (d/D), and black (0). The lower case is
 *       dim, the upper case is bright (except in the case of light-gray and
 *       dark-gray, where bright and dim are identical). Any other value will
 *       select the default color.
 *       @code OI_Printf("%[red text %[black %[normal\n", 'r', '0', 0); @endcode
 *
 * \%a   Same as \%s, except '\\r' and '\\n' are output as "<cr>" and "<lf>".
 *       \%?a is valid, but \%la is not.
 *
 * \%b   Prints an integer in base 2.
 *       @code OI_Printf("Bits are %b", I); @endcode
 *
 * \%lb  Prints a long integer in base 2.
 *
 * \%?b  Prints the least significant N bits of an integer (or long integer)
 *       in base 2. Requires the integer and a length N.
 *       @code OI_Printf("Bottom 4 bits are: %?b", I, 4); @endcode
 *
 * \%B   Prints an integer as boolean text, "TRUE" or "FALSE".
 *       @code OI_Printf("The value 0 is %B, the value 1 is %B", 0, 1); @endcode
 *
 * \%?s  Prints a substring up to a specified maximum length.
 *       Requires a pointer to a string and a length parameter.
 *       @code OI_Printf("String prefix is %?s", str, 3); @endcode
 *
 * \%ls  Same as \%S.
 *
 * \%S   Prints a UTF16 string as UTF8 (plain ASCII, plus 8-bit char sequences
 *       where needed). Requires a pointer to #OI_CHAR16. \%?S is valid. The
 *       length parameter is in OI_CHAR16 characters.
 *
 * \%T   Prints time, formatted as "secs.msecs".
 *       Requires pointer to #OI_TIME struct, NULL pointer prints current time.
 *       @code OI_Printf("The time now is %T", NULL); @endcode
 *
 *  @param format   Specifies the format string
 *
 */
void OI_Printf(const OI_CHAR *format, ...);


/**
 * Var-args version OI_Printf
 *
 * @param format   Same as for OI_Printf.
 *
 * @param argp     Var-args list.
 */
void OI_VPrintf(const OI_CHAR *format, va_list argp);


/**
 * This function writes a formatted string to a buffer. This function supports the same format specifiers as
 * OI_Printf().
 *
 * @param buffer   Destination buffer for the formatted string.
 *
 * @param bufLen   Length of the destination buffer.
 *
 * @param format   Specifies the format string.
 *
 * @return   Number of characters written or -1 in the case of an error.
 */
/*
OI_INT32 OI_SNPrintf(OI_CHAR *buffer,
                    OI_UINT16 bufLen,
                    const OI_CHAR* format, ...);
*/

/**
 * Var-args version OI_SNPrintf
 *
 * @param buffer   Destination buffer for the formatted string.
 *
 * @param bufLen   Length of the destination buffer.
 *
 * @param format   Specifies the format string
 *
 * @param argp     Var-args list.
 *
 * @return   Number of characters written or -1 in the case of an error.
 */
/*
OI_INT32 OI_VSNPrintf(OI_CHAR *buffer,
                     OI_UINT16 bufLen,
                     const OI_CHAR *format, va_list argp);
*/

/**
 * This function converts a string to an integer.
 *
 * @param str  String to parse.
 *
 * @return Integer value of the string or 0 if the string could not be parsed.
 */
//OI_INT OI_atoi(const OI_CHAR *str);


/**
 * This function parses a signed integer in a string.
 *
 * This function skips leading whitespace (space and tabs only) and parses a decimal or hex string. A hex string
 * must be prefixed by "0x". A pointer to the first character following the integer is returned. If the string does 
 * not describe an integer, the pointer passed in is returned.
 *
 * @param str    String to parse.
 *
 * @param val    Pointer to receive the parsed integer value.
 *
 * @return       Pointer to the first character following the integer or the pointer passed in.
 */
/*
const OI_CHAR* OI_ScanInt(const OI_CHAR *str,
                          OI_INT32 *val);
*/

/**
 * This function parses an unsigned integer in a string.
 *
 * This function skips leading whitespace (space and tabs only) and parses a decimal or hex string. A hex string
 * must be prefixed by "0x". A pointer to first character following the unsigned integer is returned. If the string does
 * not describe an integer, the pointer passed in is returned.
 *
 * @param str    String to parse.
 *
 * @param val    Pointer to receive the parsed unsigned integer value.
 *
 * @return       Pointer to the first character following the unsigned integer or the pointer passed in.
 */
/*
const OI_CHAR* OI_ScanUInt(const OI_CHAR *str,
                           OI_UINT32 *val);
*/
/**
 * This function parses a whitespace delimited substring out of a string.
 *
 * @param str     Input string to parse.
 * @param outStr  Buffer to return the substring.
 * @param len     Length of outStr.
 *
 *
 * @return       Pointer to the first character following the substring or the pointer passed in.
 */
/*
const OI_CHAR* OI_ScanStr(const OI_CHAR *str,
                          OI_CHAR *outStr,
                          OI_UINT16 len);
*/

/**
 * This function parses a string for one of a set of alternative value. This function skips leading whitespace (space and tabs
 * only) and parses text matching one of the alternative strings. A pointer to first character
 * following the matched text is returned.
 *
 * @param str    String to parse.
 *
 * @param alts   Alternative matching strings separated by '|'
 *
 * @param index  Pointer to receive the index of the matching alternative, return value is -1 if
 *               there is no match.
 *
 * @return       Pointer to the first character following the matched value or the pointer passed in 
 *               if there was no matching text.
 */
/*
const OI_CHAR* OI_ScanAlt(const OI_CHAR *str,
                          const OI_CHAR *alts,
                          OI_INT *index);
*/
/**
 * This function parses a string for a BD Addr. This function skips leading whitespace (space and tabs only) and parses a
 * Bluetooth device address with nibbles optionally separated by colons. A pointer to first
 * character following the BD Addr is returned.
 *
 * @param str    String to parse.
 *
 * @param addr   Pointer to receive the Bluetooth device address.
 *
 * @return       Pointer to the first character following the BD Addr or the pointer passed in.
 */
/*
const OI_CHAR* OI_ScanBdAddr(const OI_CHAR *str,
                             OI_BD_ADDR *addr);
*/

/** Get a character from a digit integer value (0 - 9). */
#define OI_DigitToChar(d) ((d) + '0')

/**
 * Determine the maximum and minimum between two arguments.
 *
 * @param a  1st value
 * @param b  2nd value
 *
 * @return Maximum or mininimum value between a and b
 */
#define OI_MAX(a, b) (((a) < (b)) ? (b) : (a) )
#define OI_MIN(a, b) (((a) > (b)) ? (b) : (a) )

/**
 * Compare two BD_ADDRs
 * SAME_BD_ADDR - Boolean: TRUE if they are the same address
 */

#define SAME_BD_ADDR(x, y)      (0 == OI_MemCmp((x),(y),OI_BD_ADDR_BYTE_SIZE) )

/**
 * Remove newlines from a string.  A newline on the end is replaced
 * with a null.  Newlines elsewhere are replaced with spaces.
 *
 * @param str    String to parse.
 */
//void OI_RemoveNewlines(OI_CHAR *str);

/**
 *  This function terminates or truncates a UTF8 string at a maximum length with a 
 *  null-terminator. This function ensures that termination/truncation does not occur 
 *  in the middle of a multi-byte sequence. As a result, the final length may be 
 *  shorter than the requested maxLen.
 *
 *  @note Null-termination occurs in-place; termination alters the caller's string.
 *
 * @param pStr      Pointer to the UTF8 string
 *
 * @param maxLen    Requested maximum length of the string after truncation
 *
 * @return          Returns the actual length of the string after termination/truncation 
                    (if any), i.e., returns strlen(str)
 *
 */
//OI_UINT OI_UTF8_Terminate(OI_UTF8 *pStr, OI_UINT maxLen);

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* _OI_UTILS_H */

