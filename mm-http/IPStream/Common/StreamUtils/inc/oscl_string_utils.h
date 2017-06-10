#ifndef OSCL_STRING_UTILS_H
#define OSCL_STRING_UTILS_H
/* =======================================================================
                               oscl_string_utils.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/oscl_string_utils.h#12 $
$DateTime: 2013/08/02 06:28:55 $
$Change: 4209946 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "oscl_types.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define MAX_UINT64_VAL 0xffffffffffffffffULL


/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
MACRO MYOBJ

ARGS
  xx_obj - this is the xx argument

DESCRIPTION:
  Complete description of what this macro does
========================================================================== */

/* =======================================================================
**                        Function Declarations
** ======================================================================= */

/* ======================================================================
FUNCTION
  skip_whitespace

DESCRIPTION
  Skips over any leading whitespace (i.e., a space or
  horizontal tab character) in the input string and
  returns the pointer to the first non-whitespace
  character.

DEPENDENCIES
  None

RETURN VALUE
  pointer to first non-whitespace character

SIDE EFFECTS
  None

========================================================================== */
const char* skip_whitespace(const char *ptr);

/* ======================================================================
FUNCTION
  skip_whitespace

DESCRIPTION
  Skips over any leading whitespace (i.e., a space or
  horizontal tab character) in the input string and
  returns the pointer to the first non-whitespace
  character.

DEPENDENCIES
  None

RETURN VALUE
  pointer to first non-whitespace character

SIDE EFFECTS
  None

========================================================================== */
char * skip_whitespace(char * ptr);

/* ======================================================================
FUNCTION
  skip_whitespace

DESCRIPTION
  Skips over any leading whitespace (i.e., a space or
  horizontal tab character) in the input string and
  returns the pointer to the first non-whitespace
  character.  The input string is represented by
  starting and ending pointers and does not need to be
  NULL terminated.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  pointer to first non-whitespace character

SIDE EFFECTS
  None.

========================================================================== */
const char* skip_whitespace(const char *start, const char *end);

/* ======================================================================
FUNCTION
  skip_to_whitespace

DESCRIPTION
  Skips over any leading non-whitespace in the input string and
  returns the pointer to the first whitespace character.  The input string
  is represented by starting and ending pointers and does not need to be
  NULL terminated.

DEPENDENCIES
  None

RETURN VALUE
  pointer to first whitespace character

SIDE EFFECTS
  None

========================================================================== */
const char* skip_to_whitespace(const char *start, const char *end);

/* ======================================================================
FUNCTION
  skip_to_char

DESCRIPTION
  Skips over all the characters in the input string until it finds the
  first occurrance of specified char 'c'. The input string is represented
  by starting and ending pointers and need not be NULL terminated.

DEPENDENCIES
  None

RETURN VALUE
  Returns pointer to 'c', if 'c' was found and the pointer after end,
  if 'c' was not found.

SIDE EFFECTS
  None

========================================================================== */
const char* skip_to_char(const char *start, const char *end, char c);

/* ======================================================================
FUNCTION
  skip_to_line_term

DESCRIPTION
  Skips over any characters to the next line terminator
  (i.e., \r and \n) and
  returns the pointer to the line term character.
  The input string is represented by
  starting and ending pointers and does not need to be
  NULL terminated.

DEPENDENCIES
  None

RETURN VALUE
  pointer to line terminator character

SIDE EFFECTS
  None

========================================================================== */
const char * skip_to_line_term(const char *start_ptr, const char *end_ptr);

/* ======================================================================
FUNCTION
  skip_whitespace_and_line_term

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
const char* skip_whitespace_and_line_term(const char *start, const char *end);


/* ======================================================================
FUNCTION
  strip_control_chars

DESCRIPTION
  The function is passed a standard null-terminated string.  Starting
 at the end, it turns all chars <0x20 (space) into nulls.  It stops at
 the first non-control character, or beginning of string.  In
 worst-case scenario, the entire string is cleared.

DEPENDENCIES
 None

RETURN VALUE
  The new length of the string

SIDE EFFECTS
  Passed string is possibly modified

========================================================================== */
int strip_control_chars(char *str, int length);

#if 0
/* ======================================================================
FUNCTION:
  stripTrailingSlashFromURL

DESCRIPTION:
  If the URL has an extraneous slash at the end, strip it.

INPUT/OUTPUT PARAMETERS:
  url - The URL to modify

RETURN VALUE:
  The input URL.

SIDE EFFECTS:
  None.
======================================================================*/
char *stripTrailingSlashFromURL(char *url);

/* ======================================================================
FUNCTION:
  getFilenameFromURL

DESCRIPTION:
  Return the filename part (after the last /) of the URL.

INPUT/OUTPUT PARAMETERS:
  url - The URL to search.

RETURN VALUE:
  See description.

SIDE EFFECTS:
  None.
======================================================================*/
const char *getFilenameFromURL(const char *url);

/* ======================================================================
FUNCTION
  extract_string

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int extract_string(const char * in_ptr, char *outstring, int maxsize);

/* ======================================================================
FUNCTION
  extract_string

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int extract_string(const char * start, const char *end, char *outstring, int maxsize);

/* ======================================================================
FUNCTION
  PV_atoi

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool PV_atoi(const char *buf,const char new_format, uint32& value);

/* ======================================================================
FUNCTION
  PV_atoi

DESCRIPTION
  Thorough, meaningful description of what this function does.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool PV_atoi(const char *buf,const char new_format, int length,
             uint32& value);

/* ======================================================================
FUNCTION
  RTPSource_atoull

DESCRIPTION
  Converts the string to unsigned long long (uint64).

DEPENDENCIES
  None.

RETURN VALUE
  false on failure in conversion
  true if the conversion went through

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool RTPSource_atoull(const char *buf,const char new_format, int length,
             uint64& value);

bool retrieveValue(const char* line_start_ptr,
                   const char* line_end_ptr,
                   const char*& temp_ptr,
                   int& value,
                   const char format = 10);

bool retrieveDouble(const char* line_start_ptr,
                    const char* line_end_ptr,
                    const char*& temp_ptr,
                    double &value);

/* ======================================================================
FUNCTION
  strncpy_upper

DESCRIPTION
  Copy at most n characters from src to dest. No null characters are
  implicitly copied. If n is greater than the length of src, null
  characters are appended until the length of src is reached. Copy
  the upper-case version of any characters in src.

DEPENDENCIES
  None

RETURN VALUE
  dest

SIDE EFFECTS
  None

========================================================================== */
char *strncpy_upper(char *dest, const char *src, int n);

/* ======================================================================
FUNCTION
  strnstr

DESCRIPTION
  Search for the first occurrance of str_to_find in src, which must occur
  within the first n - strlen(str_to_find) in src.

DEPENDENCIES
  None

RETURN VALUE
  If found, return a pointer to the location in src. Otherwise, return
  NULL.

SIDE EFFECTS
  None

========================================================================== */
const char *oscl_strnstr(const char *src, const char *str_to_find, int n);
#endif
/* =======================================================================
FUNCTION oscl_iswhite

ARGS
  c - The character to test.

DESCRIPTION:
  Returns true if the character is whitespace

RETURN VALUE:
  See description

SIDE EFFECTS
  None

========================================================================== */
bool oscl_iswhite(char c);
#if 0
/* ======================================================================
FUNCTION:
  expect

DESCRIPTION:
  Returns true iff a C-string begins with another string (the expecatation).
  Optionally skips whitespace before and after the expectation.
  If the expectation is found, the string pointer is updated to just after the
  occurrance of the expecation, and any trailing whitespace if specified.

INPUT/OUTPUT PARAMETERS:
  expectation The string that must match, must be null-terminated.
  sptr A pointer to a C-string. The value of the pointer is updated to after the
  expectation if found.
  eptr - The end boundary for the C-string. If the match doesn't fit before the
  end boundary, this function returns false.
  skipBeginningWhitespace - If true, initial whitespace will be skipped before
  expecting the match.
  skipEndingWhitespace - If true, initial whitespace will be skipped after
  expecting the match.

RETURN VALUE:
  Returns true iff the expectation is found immediately (after skipping any
  beginning whitespace if specified), and before the end boundary.

SIDE EFFECTS:
  The value of sptr is updated if the match succeeds (see above).
======================================================================*/
bool expect(const char *expectation, const char **sptr, const char *eptr,
                   bool skipBeginningWhitespace = true,
                   bool skipEndingWhitespace = true);

/* ======================================================================
FUNCTION:
  parseDelimitedList

DESCRIPTION:
  Given a string which is a bunch of strings separated by a separator character
  and optional whitespace, call the callback function for each element string.

INPUT/OUTPUT PARAMETERS:
  sptr - A string pointing to the beginning of the list.
  eptr - A string pointing to the end of the list. If NULL, it will the
  location of the first null character after sptr.
  separator - The character that separates the elements
  handleElementCallback - The callback function that is called for each element.
  cbData - Data given to the callback function.

RETURN VALUE:
  None

SIDE EFFECTS:
  None
======================================================================*/
void parseDelimitedList(const char *sptr, const char *eptr, char separator,
  bool (*handleElementCallback)(const char *start, const char *end, void *cbData),
  void *cbData);

int base64_decode( const char * CodedData, int TotalData, unsigned char * DecodedData );

/* ======================================================================
FUNCTION:
  indexOf

DESCRIPTION:
  Finds the first occurance of a character (among a string of choices).

INPUT/OUTPUT PARAMETERS:
  choices A C-string containing all the characters to search for.
  sptr The C-string to search.
  eptr The end boundary for sptr. If none of the choice characters are found before
  reaching eptr, this function fails.
  choiceIndexPtr A pointer to a int which is set with the index of choice character
  that is actually found. May be NULL, in which case nothing is set.

RETURN VALUE:
  Returns the index of the first occurance of a choice character. If no choice
  character is found before eptr, returns -1.

SIDE EFFECTS:
  The value pointed to by choiceIndexPtr may be updated (see above).
======================================================================*/
int indexOf(const char *choices, const char *sptr, const char *eptr,
            int *choiceIndexPtr = NULL);

/* ======================================================================
FUNCTION:
  parseUnsignedInt32

DESCRIPTION:
  Parse a (positive) integer

INPUT/OUTPUT PARAMETERS:
  sptr A pointer to a C-string that may begin with an unsigned base-10 number
  string. If so, the value of the pointer will be updated to just after the
  integer string.
  eptr The end boundary for the C-string to parse. Parsing will stop if the boundary
  is hit, even if more number characters follow.
  valueptr A pointer to a 32-bit integer which is updated with the parsed value
  value if successful.

RETURN VALUE:
  Return true iff successful.

SIDE EFFECTS:
  The value pointed to by valueptr is updated (see above).
======================================================================*/
bool parseUnsignedInt32(const char **sptr, const char *eptr, int32 *valueptr);
#endif

/* ======================================================================
FUNCTION
  std_strstri

DESCRIPTION
 Find a pattern in a string, return starting point or npos. Does a case
 insensitive search (not supported by component services)

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  NULL if the the pattern is not found else the pos in the string

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
char *std_strstri(const char *string, const char *pattern);


/* ======================================================================
FUNCTION:
  parseByteRange

DESCRIPTION:
  parse Byte Range to provide start offset and end offset

INPUT/OUTPUT PARAMETERS:
  byteRange   - [In], byte range string
  startOffset - [OUT], start offset
  endOffset   - [OUT], end offset

RETURN VALUE:
 void

SIDE EFFECTS:

======================================================================*/
void parseByteRange(char *byteRange, int64 &startOffset, int64 &endOffset);

/* ======================================================================
FUNCTION:
  std_scanull

DESCRIPTION:
  converts string to 64 bit value

INPUT/OUTPUT PARAMETERS:
  inputStr   - [In], input string
  endp       - [OUT]
  base       - [IN], base for conversiob
  retcode    - [OUT], ret code
RETURN VALUE:
 64 bit unsignged value

SIDE EFFECTS:

======================================================================*/

uint64 std_scanull(const char *pchBuf, int nRadix,
                    const char **ppchEnd, int *pnError);

/* ======================================================================
FUNCTION:
  std_strtoullVal

DESCRIPTION:
  converts interger value

INPUT/OUTPUT PARAMETERS:
  charval    - [In]
RETURN VALUE:
 unsignged int value

SIDE EFFECTS:

======================================================================*/

static unsigned int std_strtoullVal ( unsigned int charval);

/* ======================================================================
FUNCTION:
  std_trim

DESCRIPTION:
  trim leading and trailing whitespace (' ', '\t', '\r', and '\n' )
  from a string

INPUT/OUTPUT PARAMETERS:
  char *pc: string to trim [IN]
  int nLen: length of pc   [IN]

RETURN VALUE:
 length of trimmed string (not including null-term)

SIDE EFFECTS:
  the passed-in string needn't be null-terminated.
  if no spaces are trimmed from the end of the string, but
  some are trimmed from the beginning, the *result* will not
  be null-terminated.
======================================================================*/
size_t std_trim(char *pc, size_t nLen);

/**
  || Function
  || --------
  || bool parseuint32(const char *cpsz, int nRadix, uint32 *pnNum);
  ||
  || Description
  || -----------
  || parse an unsigned long into pnNum
  ||
  || Parameters
  || ----------
  || const char *cpsz: null-terminated string to parse
  || int nRadix: number base
  || uint32 *pnNum: number to fill
  ||
  || Returns
  || -------
  || whether parsing succeeded
  ||
  || Remarks
  || -------
  || cpsz must contain only number contents,
  || these will fail, even though they might work in strtoul():
  ||
  ||   " 1"
  ||   "1 "
  ||   "1g"
  ||
  */
bool parseuint32(const char *cpsz, int nRadix, unsigned int *pnNum);

/**
  || Function
  || --------
  || int strchop(char *pszToChop, const char *cpszChopChars)
  ||
  || Description
  || -----------
  || chop a string into its component words, using cpszChopChars
  ||   as delimters
  ||
  || Parameters
  || ----------
  || char *pszToChop: string to chop
  || const char *cpszChopChars: delimiting characters
  ||
  || Returns
  || -------
  || number of words the string was chopped into
  ||
  || Remarks
  || -------
  || chopping is effected by replacing all occurences
  ||  of characters in cpszChopChars with '\0'
  ||
  */
int strchop(char *pszToChop, const char *cpszChopChars);

/**
  || Function
  || --------
  || char *strchopped_nth(const char *cpsz, int nNth)
  ||
  || Description
  || -----------
  || returns a pointer to the nNth string in a chopped string
  ||
  || Parameters
  || ----------
  || const char *cpsz: chopped string
  || int nNth: 0-based index into cpsz
  ||
  || Returns
  || -------
  || pointer to the nNth string
  ||
  || Remarks
  || -------
  || you must know the contents of cpsz, and must not pass an index
  ||  higher than the number of null terminated strings in cpsz.
  ||
  || if cpsz has leading nulls, they're skipped, so for example:
  ||
  ||   strchopped_nth("\0\0hi\0there\0",1) returns "there"
  ||
  */
char* strchopped_nth(const char *cpsz, int nNth);

#if 0
/**
  || Function
  || --------
  || bool parseuint16(const char *cpsz, int nRadix, uint16 *pnNum);
  ||
  || Description
  || -----------
  || parse an unsigned long into pnNum
  ||
  || Parameters
  || ----------
  || const char *cpsz: null-terminated string to parse
  || int nRadix: number base
  || uint32 *pnNum: number to fill
  ||
  || Returns
  || -------
  || whether parsing succeeded
  ||
  || Remarks
  || -------
  || cpsz must contain only number contents,
  || these will fail, even though they might work in strtoul():
  ||
  ||   " 1"
  ||   "1 "
  ||   "1g"
  ||
  */
bool parseuint16(const char *cpsz, int nRadix, unsigned short int *pnNum);
#endif

/**
  || Function
  || --------
  || boolean strmxnstr(const char *cpszHaystack,
  ||                   const char *cpszNeedle,
  ||                   int m, int *pn)
  ||
  || Description
  || -----------
  || find cpszNeedle in cpszHaystack, where cpszHaystack is a list of
  ||  strings of equal length m, set *pn to the zero-based index where
  ||  needle begins in haystack divided by exactly m.  Return TRUE if
  ||  match found exactly
  ||
  || Parameters
  || ----------
  || const char *cpszHaystack: null-terminated list of potential matches
  || const char *cpszNeedle: string to find
  || int m: length of each potential match, cpszNeedle needn't be exactly
  ||         m, but cpszNeedle must match cpszHaystack at an offset that
  ||         is a multiple of m
  || int *pn: output, which multiple of m cpszNeedle was found
  ||
  || Returns
  || -------
  || TRUE if a proper match was found
  ||
  || Remarks
  || -------
  ||
  */
bool strmxnstr(const char *cpszHaystack, const char *cpszNeedle,
               int m, unsigned int *pn);

#endif /* OSCL_STRING_UTILS_H */
