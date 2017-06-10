/* =======================================================================
                               oscl_string_utils.cpp
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.  Include any initialization and synchronizing
  requirements.

EXTERNALIZED FUNCTIONS
  List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
  Detail how to initialize and use this service.  The sequencing aspect
  is only needed if the order of operations is important.

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/src/oscl_string_utils.cpp#20 $
$DateTime: 2013/08/02 06:28:55 $
$Change: 4209946 $


========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "qtv_msg.h"
#include <math.h>
#include "oscl_string_utils.h"
#include <AEEstd.h>
#include <ctype.h>
#include "string.h"
#include "IPStreamSourceUtils.h"
/* ==========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
const int base64tab[256]=
{
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
    52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                            Function Definitions
** ======================================================================= */

#if 0
/* ======================================================================
FUNCTION
  PV_atoi

DESCRIPTION
  Extracts an integer from the input string.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  Enumerate possible return values

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool PV_atoi(const char *buf,const char new_format, uint32& value)
{
  return PV_atoi(buf, new_format, strlen(buf), value);
}

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
             uint32& value)
{
  uint32 old, temp;
  const char *ptr = buf;
  value = 0;
  switch ( new_format )
  {
  case 'x':{
      while ( ptr-buf < length )
      {
        if ( (*ptr == 'a')||(*ptr == 'A') )
          temp = 10;
        else if ( (*ptr == 'b')||(*ptr == 'B') )
          temp = 11;
        else if ( (*ptr == 'c')||(*ptr == 'C') )
          temp = 12;
        else if ( (*ptr == 'd')||(*ptr == 'D') )
          temp = 13;
        else if ( (*ptr == 'e')||(*ptr == 'E') )
          temp = 14;
        else if ( (*ptr == 'f')||(*ptr == 'F') )
          temp = 15;
        else if ( (*ptr >= 48) && (*ptr <= 57) )
          temp = (int)(*ptr-48);
        else return false;
        ++ptr;
        old = value;
        value = value*16 + temp;
        if ( old > value )
        {
          // overflow
          return false;
        }
      }
    }break;

    case 'd':{
      int numDigits = 0;
      while ( ptr-buf < length )
      {
        if ( (*ptr >= 48) && (*ptr <= 57) )
        {
          numDigits++;
          temp=(int)(*ptr-48);
          ++ptr;
          old = value;
          value = value*10 + temp;
          if ( old > value )
          {
            // overflow
            return false;
          }
        }
        else
        {
          /* Ignore remaining input until next whitespace */
          while ( ptr-buf < length )
          {
            ++ptr;
          }
          if (numDigits)
          {
            return true;
          }
          else
          {
            return false;
          }
        }
      }
    }break;

  default :{
      while ( ptr-buf < length )
      {
        if ( (*ptr >= 48) && (*ptr <= 57) )
        {
          temp=(int)(*ptr-48);
          ++ptr;
          old = value;
          value = value*10 + temp;
          if ( old > value )
          {
            // overflow
            return false;
          }
        }
        else
        {
          return false;
        }
      }
    }break;
  }

  return true;
}

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
             uint64& value)
{
  uint64 old, temp;
  const char *ptr = buf;
  value = 0;
  switch ( new_format )
  {
  case 'x':{
      while ( ptr-buf < length )
      {
        if ( (*ptr == 'a')||(*ptr == 'A') )
          temp = 10;
        else if ( (*ptr == 'b')||(*ptr == 'B') )
          temp = 11;
        else if ( (*ptr == 'c')||(*ptr == 'C') )
          temp = 12;
        else if ( (*ptr == 'd')||(*ptr == 'D') )
          temp = 13;
        else if ( (*ptr == 'e')||(*ptr == 'E') )
          temp = 14;
        else if ( (*ptr == 'f')||(*ptr == 'F') )
          temp = 15;
        else if ( (*ptr >= 48) && (*ptr <= 57) )
          temp = (int)(*ptr-48);
        else return false;
        ++ptr;
        old = value;
        value = value*16 + temp;
        if ( old > value )
        {
          // overflow
          return false;
        }
      }
    }break;

    case 'd':{
      int numDigits = 0;
      while ( ptr-buf < length )
      {
        if ( (*ptr >= 48) && (*ptr <= 57) )
        {
          numDigits++;
          temp=(int)(*ptr-48);
          ++ptr;
          old = value;
          value = value*10 + temp;
          if ( old > value )
          {
            // overflow
            return false;
          }
        }
        else
        {
          /* Ignore remaining input until next whitespace */
          while ( ptr-buf < length )
          {
            ++ptr;
          }
          if (numDigits)
          {
            return true;
          }
          else
          {
            return false;
          }
        }
      }
    }break;

  default :{
      while ( ptr-buf < length )
      {
        if ( (*ptr >= 48) && (*ptr <= 57) )
        {
          temp=(int)(*ptr-48);
          ++ptr;
          old = value;
          value = value*10 + temp;
          if ( old > value )
          {
            // overflow
            return false;
          }
        }
        else
        {
          return false;
        }
      }
    }break;
  }

  return true;
}
#endif

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
const char* skip_whitespace(const char *ptr)
{
  while ( ptr && *ptr )
  {
    if ( *ptr != ' ' && *ptr != '\t' )
    {
      break;
    }

    ++ptr;
  }

  return ptr;
}

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
char * skip_whitespace( char * ptr )
{
  while ( ptr && *ptr )
  {
    if ( *ptr != ' ' && *ptr != '\t' )
    {
      break;
    }

    ++ptr;
  }

  return ptr;
}

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
const char* skip_whitespace(const char *start, const char *end)
{
  while ( start && (start < end) )
  {
    if ( *start != ' ' && *start != '\t' )
    {
      break;
    }

    ++start;
  }

  return start;
}

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
const char* skip_to_whitespace(const char *start, const char *end)
{
  while ( start && (start < end) )
  {
    if ( oscl_iswhite(*start) )
    {
      break;
    }
    ++start;
  }

  return start;
}

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
const char* skip_to_char(const char *start, const char *end, char c)
{
  while (start && (start <= end))
  {

    if (*start == c)
      break;

    ++start;
  }

  return start;
}

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
const char * skip_to_line_term(const char *start, const char *end)
{
  while ( start && (start < end) )
  {
    if ( *start == '\r' || *start == '\n' )
    {
      return start;
    }
    ++start;
  }

  return start;
}

/* ======================================================================
FUNCTION
  skip_whitespace_and_line_term

DESCRIPTION
  Skips over any leading whitespace (i.e., a space or
  horizontal tab character) or line terminator (i.e., \r
  and \n) and
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
  Detail any side effects.

========================================================================== */
const char* skip_whitespace_and_line_term(const char *start, const char *end)
{
  while ( start && (start < end) )
  {
    if ( !oscl_iswhite(*start) )
    {
      break;
    }

    ++start;
  }

  return start;
}

#if 0
/* ======================================================================
FUNCTION
  extract_string

DESCRIPTION
  Extracts string of a maximum size after skipping any
  leading whitespace.  The input string is represented by
  starting and ending pointers and does not need to be
  NULL terminated.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  length of the extracted string

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int extract_string(const char * start, const char *end, char *outstring, int maxsize)
{
  int len = 0;

  if ( ! outstring )
  {
    return 0;
  }

  start = skip_whitespace(start, end);

  for ( ; start && (start < end) ; ++start )
  {

    if (oscl_iswhite(*start))
    {
      // whitespace so stop copying
      break;
    }

    if ( len < maxsize )
    {
      *outstring++ = *start;
    }
    else if ( len == maxsize )
    {
      // too long so just terminate the string
      *(outstring-1) = '\0';
    }
    ++len;

  }

  if ( len < maxsize )
  {
    // terminate the string
    *outstring = '\0';
  }

  return len;
}

/* ======================================================================
FUNCTION
  extract_string

DESCRIPTION
  Extracts string of a maximum size after skipping any
  leading whitespace.

DEPENDENCIES
  List any dependencies for this function, global variables, state,
  resource availability, etc.

RETURN VALUE
  length of the extracted string

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
int extract_string(const char * in_ptr, char *outstring, int maxsize)
{
  int len = 0;

  if ( ! outstring )
  {
    return 0;
  }

  in_ptr = skip_whitespace(in_ptr, in_ptr+strlen(in_ptr));

  for ( ; in_ptr && *in_ptr ; ++in_ptr )
  {

    if ( *in_ptr == ' ' || *in_ptr == '\t' || *in_ptr == '\n' || *in_ptr == '\r' )
    {
      // whitespace so stop copying
      break;
    }

    if ( len < maxsize )
    {
      *outstring++ = *in_ptr;
    }
    else if ( len == maxsize )
    {
      // too long so just terminate the string
      *(outstring-1) = '\0';
    }
    ++len;

  }

  if ( len < maxsize )
  {
    // terminate the string
    *outstring = '\0';
  }

  return len;
}

#endif
/* ======================================================================
FUNCTION
  strip_control_chars

DESCRIPTION
  The function is passed a standard null-terminated string. It scans the
  string for any chars <0x20 (space), and removes them from the string.
  This operation uses nested loops and hence should not be used for
  time critical operations.

DEPENDENCIES
  None

RETURN VALUE
  The new length of the string

SIDE EFFECTS
  Passed string is possibly modified

========================================================================== */
int strip_control_chars(char *str, int length)
{
  // look for degenerate case
  if ((length <= 0) || (str == NULL) )
  {
    return 0;
  }

  int len = length;

   //perform the strip
   int i = 0;
   while ((i < len) && (len > 0))
   {
     if (str[i] <= ' ')
     {
        for (int j = i; j < (len - 1); j++)
        {
          str[j] = str[j + 1];
        }
        len--;
     }
     else
     {
       i++;
     }
   }
   str[len] = '\0';

   return len;
}

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
char *stripTrailingSlashFromURL(char *url)
{
  if (std_strnicmp(url, "rtsp://", strlen("rtsp://")) != 0)
  {
    return url;
  }

  int i = -1;
  int slashes = 0;
  int prevSlashIndex = -1;
  for (i = strlen("rtsp://"); url[i] != '\0'; i++)
  {
    if (url[i] == '/')
    {
      prevSlashIndex = i;
      slashes++;
    }
  }

  // i is now the length of url
  if ((slashes > 1) && (prevSlashIndex == i - 1))
  {
    url[prevSlashIndex] = '\0';
  }

  return url;
}

/* ======================================================================
FUNCTION:
  getFilenameFromURL

DESCRIPTION:
  Return the filename part (including the path) of the URL. The
  returned string points to within the argument url, starting from a
  / character. If the argument is not a URL, or does not contain a
  slash after ://, return NULL.

INPUT/OUTPUT PARAMETERS:
  url - The URL to search.

RETURN VALUE:
  See description.

SIDE EFFECTS:
  None.
======================================================================*/
const char *getFilenameFromURL(const char *url)
{
  const char *temp = strstr(url, "://");

  if (temp == NULL)
  {
    return NULL;
  }

  return strstr(temp + 3, "/");
}

/* ======================================================================
FUNCTION
  retrieveValue

DESCRIPTION
 Skip white space from line_start_ptr and then convert following string to
 integer, then skip white space until line_end_ptr.

DEPENDENCIES
 None

RETURN VALUE
 bool - whether conversion succeedes.

SIDE EFFECTS
 temp_ptr returns the first non-whitespace character after conversion.

========================================================================== */
bool retrieveValue(const char* line_start_ptr,
                   const char* line_end_ptr,
                   const char*& temp_ptr,
                   int &value,
                   const char format)
{
   temp_ptr = skip_whitespace( line_start_ptr, line_end_ptr );
   if (temp_ptr >= line_end_ptr)
   {
      // empty line
      return false;
   }
   // retrieve value
   const char * end_ptr = NULL;
   int nErr;
   value = (int)std_scanul(temp_ptr, format, &end_ptr, &nErr);
   if (nErr == STD_NEGATIVE)
   {
     value = -value;
   }
   temp_ptr = skip_whitespace( end_ptr, line_end_ptr );
   return true;
}

/* ======================================================================
FUNCTION
  retrieveDouble

DESCRIPTION
 Skip white space from line_start_ptr and then convert following string to
 double, then skip white space until line_end_ptr.

DEPENDENCIES
 None

RETURN VALUE
 bool - whether conversion succeeded.

SIDE EFFECTS
 temp_ptr returns the first non-whitespace character after conversion.

========================================================================== */
bool retrieveDouble(const char* line_start_ptr,
                    const char* line_end_ptr,
                    const char*& temp_ptr,
                    double &value)
{
   temp_ptr = skip_whitespace( line_start_ptr, line_end_ptr );
   if (temp_ptr >= line_end_ptr)
   {
      // empty line
      return false;
   }
   // retrieve value
   char *end_ptr = NULL;
   value = strtod( temp_ptr, &end_ptr );

   if ((value == HUGE_VAL) || (value == -HUGE_VAL))
   {
     temp_ptr = line_start_ptr;
     return false;
   }

   temp_ptr = skip_whitespace( end_ptr, line_end_ptr );
   return true;
}

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
char *strncpy_upper(char *dest, const char *src, int n)
{
   bool reached_end = false;

   for (int i = 0; i < n; i++)
   {
      if (!reached_end)
      {
         reached_end = (src[i] == '\0');
      }

      if (reached_end)
      {
         dest[i] = '\0';
      }
      else
      {
         char c = src[i];

         if ((c >= 'a') && (c <= 'z'))
         {
            c = c - 'a' + 'A';
         }

         dest[i] = c;
      }
   }
   return dest;
}

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
const char *oscl_strnstr(const char *src, const char *str_to_find, int n)
{
   if ((src == NULL) || (str_to_find == NULL) || (n < 1))
   {
      return NULL;
   }

   int find_length = strlen(str_to_find);

   // If we're looking for the null terminator ...
   if (find_length == 0)
   {
      return src + strlen(src);
   }

   for (int i = 0; i <= n - find_length; i++)
   {
      if (src[i] == '\0')
      {
         return NULL;
      }

      for (int j = 0; j < find_length; j++)
      {
         char c = src[i + j];

         if (c == '\0')
         {
             // rest of src is shorter than str_to_find
             return NULL;
         }

         if (c != str_to_find[j])
         {
            goto SearchNextIndex;
         }
      }

      // Hooray, we have a match!
      return src + i;

SearchNextIndex:
      ;
   }
   return NULL;
}
#endif

bool oscl_iswhite(char c)
{
  switch (c)
  {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    return true;

    default:
    return false;
  }
}

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
            bool skipBeginningWhitespace, bool skipEndingWhitespace)
{
   const char *ptr = *sptr;

   if (skipBeginningWhitespace)
   {
      while ((ptr < eptr) && isspace(*ptr))
      {
         ptr++;
      }
   }

   int charsLeft = eptr - ptr;
   int charsToExpect = strlen(expectation);

   // If we don't have enough chars to match, fail.
   if (charsLeft < charsToExpect)
   {
      return false;
   }

   // Match the expecation now, but don't match the empty string.
   if ((charsLeft > 0) && (strncmp(ptr, expectation, charsToExpect) != 0))
   {
      return false;
   }

   // We matched, so skip the expectation.
   ptr += charsToExpect;

   if (skipEndingWhitespace)
   {
      while ((ptr < eptr) && isspace(*ptr))
      {
         ptr++;
      }
   }

   // Update start pointer
   *sptr = ptr;
   return true;
}

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
  void *cbData)
{
  // Figure out eptr if NULL.
  if (eptr == NULL)
  {
    eptr = sptr + strlen(sptr);
  }

  while (sptr < eptr)
  {
    // Location of next comma or end of string
    const char *delim = sptr;

    while (delim && delim < eptr)
    {
      if (*delim == separator)
      {
        break;
      }
      delim++;
    }

    // Stop parsing if the callback returns true.
    if (handleElementCallback && (*handleElementCallback)(sptr, delim, cbData))
    {
      return;
    }

    sptr = skip_whitespace(delim + 1, eptr);
  }
}

int base64_decode( const char * CodedData, int TotalData, unsigned char * DecodedData )
{
    int BytesDecoded, BytesEncoded;
    const char *InData = CodedData;
    unsigned char *OutData;

    if(DecodedData == 0L)
    {
        return ((TotalData+3)/4) * 3;
    }

    InData = CodedData;
    while(base64tab[(int)(*(InData++))] <= 63);
    BytesEncoded = (int)(InData - CodedData - 1);
    if (BytesEncoded < TotalData)
    {
      TotalData = BytesEncoded;
    }
    BytesDecoded = ((TotalData+3)/4) * 3;

    InData = CodedData;
    OutData = DecodedData;

    while (TotalData > 0)
    {
        *(OutData++) =
            (unsigned char) (base64tab[(int)(*InData)] << 2 | base64tab[(int)(InData[1])] >> 4);
        if (base64tab[(int)(InData[2])] <= 63)
        {
          *(OutData++) =
              (unsigned char) (base64tab[(int)(InData[1])] << 4 | base64tab[(int)(InData[2])] >> 2);
        }
        if (base64tab[(int)(InData[3])] <= 63)
        {
          *(OutData++) =
              (unsigned char) (base64tab[(int)(InData[2])] << 6 | base64tab[(int)(InData[3])]);
        }
        InData += 4;
        TotalData -= 4;
    }

    if(TotalData & 03)
    {
        if(base64tab[(int)(InData[-2])] > 63)
            BytesDecoded -= 2;
        else
            BytesDecoded -= 1;
    }

    return BytesDecoded;
}

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
            int *choiceIndexPtr)
{
   int numChoices = strlen(choices);

   for (const char *ptr = sptr; ptr < eptr; ptr++)
   {
      char c = *ptr;

      for (int i = 0; i < numChoices; i++)
      {
         if (c == choices[i])
         {
            if (choiceIndexPtr != NULL) {
               *choiceIndexPtr  = i;
            }
            // Calculate offset from beginning.
            return (ptr - sptr);
         }
      }
   }

   // Not found
   return -1;
}

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
bool parseUnsignedInt32(const char **sptr, const char *eptr, int32 *valueptr)
{
   bool anyDigitsFound = false;
   int32 value = 0;
   const char *ptr = *sptr;
   bool done = false;

   while ((ptr < eptr) && !done)
   {
      char c = *ptr;

      if (isdigit(c))
      {
         anyDigitsFound = true;
         value *= 10;
         value += (c - '0');
         ptr++;
      } else {
         done = true;
         // terminate loop, don't increment ptr
      }
   }

   if (!anyDigitsFound)
   {
      return false;
   }

   *valueptr = value;
   *sptr = ptr;
   return true;
}
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
char *std_strstri(const char *string, const char *pattern)
{
  /* Check for the null pattern  */
  if (*pattern == '\0')
    return (char *) string;

  while ( *string != '\0' )
  {
    if ( std_stribegins(string, pattern) )
       return (char *) string;
    string++;
  }
  return NULL;
}

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

void parseByteRange(char *byteRange, int64 &startOffset, int64 &endOffset)
{
  size_t byteRangeLen = 0;
  startOffset = 0;
  endOffset = -1;
  if (byteRange)
  {
    byteRangeLen = std_strlen(byteRange);
    size_t posDash = 0;
    while (byteRange[posDash] != '-' && byteRange[posDash] != '\0')
    {
       posDash++;
    }
    //byte range ends with '-'.e.g. "200-"
    if (posDash == byteRangeLen - 1)
    {
          sscanf(byteRange, "%lld-", &startOffset);
    }
    //byte range starts with '-'.e.g. "-200"
    else if (posDash == 0)
    {
      sscanf(byteRange, "-%lld", &endOffset);
    }
    //byte range does  not contains '-'. e.g. "200"
    else if (posDash == byteRangeLen)
    {
      endOffset =  atoi(byteRange);
    }
    //"100-200"
    else
    {
      sscanf(byteRange, "%lld-%lld", &startOffset, &endOffset);
    }
  }
}

/**
 * c'tor
 *
 * @param str   Null terminated string to tokenize. Will get
 *              modified by this class.
 * @param delimiters  Null terminated string of delimiters.
 */
IPStreamStringTokenizer::IPStreamStringTokenizer(
  char *str, const char *delimiters) :
    m_pStr(str), m_pDelimiters(delimiters), m_LenDelimStr(std_strlen(delimiters))
{
  while (IsDelimiter(*m_pStr))
  {
    ++m_pStr;
  }
}

/**
 * d'tor
 */
IPStreamStringTokenizer::~IPStreamStringTokenizer()
{

}

/**
 * Returns a pointer to the next token.
 */
char *IPStreamStringTokenizer::GetNextToken()
{
  char *pNextToken = m_pStr;

  while (!IsDelimiter(*m_pStr) && *m_pStr != '\0')
  {
    ++m_pStr;
  }

  if (*m_pStr)
  {
    // not null char
    *m_pStr = '\0';
    ++m_pStr;
  }

  while (IsDelimiter(*m_pStr))
  {
    ++m_pStr;
  }

  return pNextToken;
}

bool IPStreamStringTokenizer::IsDelimiter(char c) const
{
  bool rslt = false;

  for (size_t i = 0; i < m_LenDelimStr; ++i)
  {
    if (m_pDelimiters[i] == c)
    {
      rslt = true;
      break;
    }
  }

  return rslt;
}

/* ======================================================================
FUNCTION:
  std_scanull

DESCRIPTION:
  converts string to 64 bit value

INPUT/OUTPUT PARAMETERS:
  pchBuf   - [IN], input string
  nRadix   - [IN], base for conversion
  ppchEnd  - [OUT]
  pnError  - [OUT], ret code
RETURN VALUE:
 64 bit unsignged value

SIDE EFFECTS:

======================================================================*/

uint64 std_scanull( const char *pchBuf, int nRadix, const char **ppchEnd,  int *pnError )
{
  uint64 ret = 0;
  unsigned int charval;
  bool negative = false;
  int out_of_range = 0;
  *pnError = 0;
  if (pchBuf == NULL)
  {
    *pnError = STD_NODIGITS;
    return 0;
  }
  while ( isspace ( *pchBuf ) )
  {
    pchBuf++;
  }
  if(*pchBuf == '-')
  {
    negative = true;
    pchBuf++;
  }
  else if(*pchBuf == '+')
  {
    pchBuf++;
  }

  if((nRadix==0 || nRadix==16) && *pchBuf=='0' && tolower(*(pchBuf+1))=='x')
  {
     nRadix = 16;
     pchBuf += 2;
  }
  if ( nRadix == 0 )
  {
    if (*pchBuf == '0')
    {
      nRadix = 8;
    }
    else
    {
      nRadix = 10;
    }
  }

  if ((nRadix < 0) || (nRadix != 0 && (nRadix < 2 || nRadix > 36)))
  {
    *pnError = STD_BADPARAM;
    return MAX_UINT64_VAL;
  }

  if (!isalnum(*pchBuf))
  {
    *pnError = STD_NODIGITS;
    return 0;
  }

  while ( 1 )
  {
    charval = std_strtoullVal ( *pchBuf );
    if ( charval >= ( unsigned int ) nRadix )
    {
      break;
    }
    else
    {
      if (!out_of_range)
      {
        if ((ret > (MAX_UINT64_VAL / nRadix))  ||
            (ret * nRadix > (MAX_UINT64_VAL - charval)))
        {
          out_of_range = 1;
        }
        ret = ( ( ret * nRadix ) + charval );
      }
    }
    pchBuf++;
  }
  if (out_of_range)
  {
    *pnError = STD_OVERFLOW;
    return MAX_UINT64_VAL;
  }
  if ( ppchEnd )
  {
    *ppchEnd = ( char * ) pchBuf;
  }
  if (negative)
  {
    *pnError = STD_NEGATIVE;
  }
  return ret;
}

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


static unsigned int std_strtoullVal ( unsigned int  charval  )
{
  if ( charval >= 'a' )
  {
    charval = ( charval - 'a' + 10 );
  }
  else if ( charval >= 'A' )
  {
    charval = ( charval - 'A' + 10 );
  }
  else if ( charval <= '9' )
  {
    charval = ( charval - '0' );
  }
  return charval;
}

/**
  || Function
  || --------
  || int trim(char *pc, int nLen)
  ||
  || Description
  || -----------
  || trim leading and trailing whitespace (' ', '\t', '\r', and '\n' )
  ||    from a string
  ||
  || Parameters
  || ----------
  || char *pc: string to trim
  || int nLen: length of pc
  ||
  || Returns
  || -------
  || length of trimmed string (not including null-term)
  ||
  || Remarks
  || -------
  || the passed-in string needn't be null-terminated.
  ||
  || if no spaces are trimmed from the end of the string, but
  ||  some are trimmed from the beginning, the *result* will not
  ||  be null-terminated.
  ||
  */
size_t std_trim(char *pc, size_t nLen)
{
   char *pcSave = pc;
   char  c;

   if ((size_t)(-1) == nLen) {
      nLen = strlen(pc);
   }

   while ((0 < nLen) &&
          ((' ' == (c = *pc)) ||
           ('\t' == c) ||
           ('\r' == c) ||
           ('\n' == c))) {
      pc++;
      nLen--;
   }

   memmove(pcSave,pc,nLen);
   pc = pcSave;

   while ((0 < nLen) &&
          ((' ' == (c = pc[nLen-1])) ||
           ('\t' == c) ||
           ('\r' == c) ||
           ('\n' == c))) {
      pc[--nLen] = '\0';
   }

   return nLen;
}

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
bool parseuint32(const char *cpsz, int nRadix, unsigned int *pnNum)
{
   char *pc;

   *pnNum = (unsigned int)strtoul(cpsz,&pc,nRadix);

   return (pc != cpsz) && ('\0' == *pc);
}

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
int strchop(char *pszToChop, const char *cpszChopChars)
{
   size_t   nLen;
   int   nNumFields = 0;
   char *pc;
   bool   bOnWord = false;

   if ((char *)0 == pszToChop) {
      return 0;
   }

   if ((char *)0 == cpszChopChars) {
      return 1;
   }

   nLen = strlen(pszToChop);
   pc = pszToChop + nLen;

   while (--pc >= pszToChop) {
      if (strchr(cpszChopChars,*pc)) {

         *pc = '\0';
         bOnWord = false;

      } else {

         if (!bOnWord) {
            ++nNumFields;
         }
         bOnWord = true;

      }
   }

   return nNumFields;
}

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
char* strchopped_nth(const char *cpsz, int nNth)
{
   for (;;) {
      while ('\0' == *cpsz) {
         cpsz++; /* skip nulls (including leading) */
      }
      if (nNth-- == 0) {
         return (char *)cpsz;
      }
      while ('\0' != *cpsz++); /* skip word */
   }
}

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
#if 0
bool parseuint16(const char *cpsz, int nRadix, unsigned short int *pnNum)
{
   char *pc;

   *pnNum = (unsigned short int)strtoul(cpsz,&pc,nRadix);

   return (pc != cpsz) && ('\0' == *pc);
}
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
               int m, unsigned int *pn)
{
   size_t nOffset = 0;

   cpszNeedle = (const char *)strstr(cpszHaystack,cpszNeedle);

   if (cpszNeedle > cpszHaystack)
   {
   nOffset = cpszNeedle-cpszHaystack;
   }

   if ((unsigned int *)0 != pn) {
      *pn = (unsigned int)(nOffset/m);
   }

   return ((char *)0 != cpszNeedle) && (0 == nOffset%m);
}
