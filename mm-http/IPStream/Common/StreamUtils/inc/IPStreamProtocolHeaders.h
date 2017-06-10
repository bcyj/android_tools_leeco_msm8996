#ifndef _IPSTREAM_PROTOCOL_HEADERS_H_
#define _IPSTREAM_PROTOCOL_HEADERS_H_
/* =======================================================================
                               IPStreamProcotolHeaders.h
DESCRIPTION
COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/IPStreamProtocolHeaders.h#6 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */
#include "AEEStdDef.h"

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

// # of oem headers which can be defined.  Change max as necessary.
#define IPSTREAM_PROTOCOL_HEADER_TABLE_SIZE 10

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

struct IPStreamProtocolHeaderEntry
{
   // OR of values in AffectedMethods. 0=unused
   int  affectedMethods;
   char *headerName;
   char *headerValue;
};

enum IPStreamProtocolHeaderResult
{
  IPSTREAM_PROTOCOL_RESULT_OK          = 0, // no problem
  IPSTREAM_PROTOCOL_RESULT_TOO_MANY    = 1, // all headers already used
  IPSTREAM_PROTOCOL_RESULT_NOT_FOUND   = 2, // request to replace non-existent
  IPSTREAM_PROTOCOL_RESULT_BAD_DATA    = 3, // methods, name or value is empty
  IPSTREAM_PROTOCOL_RESULT_NO_MEMORY   = 4, // unable to alloc memory for item
  IPSTREAM_PROTOCOL_RESULT_BAD_COMMAND = 5, // command not OemHeaderCommand
};


//What operation should be performed on the Oem headers?
enum IPStreamProtocolHeaderCommand
{
  IPSTREAM_PROTOCOL_HEADER_NONE       = 0,
  IPSTREAM_PROTOCOL_HEADER_DELETE_ALL = 1,
  IPSTREAM_PROTOCOL_HEADER_DELETE     = 2,
  IPSTREAM_PROTOCOL_HEADER_ADD        = 3,
  IPSTREAM_PROTOCOL_HEADER_REPLACE    = 4
};

enum AffectedMethod
{
  PROTO_METHOD_NONE            =  0,
  RTSP_METHOD_NONE             =  0,
  RTSP_METHOD_DESCRIBE         =  1,
  RTSP_METHOD_SETUP            =  2,
  RTSP_METHOD_PLAY             =  4,
  RTSP_METHOD_PAUSE            =  8,
  RTSP_METHOD_TEARDOWN         = 16,
  RTSP_METHOD_OPTIONS          = 32,
  RTSP_METHOD_SET_PARAMETER    = 1024,
  HTTP_METHOD_NONE             =  0,
  HTTP_METHOD_GET              = 64,
  HTTP_METHOD_POST             = 128,
  HTTP_METHOD_HEAD             = 256,
  HTTP_METHOD_OPTIONS          = 512,
  // all of the above, rtsp
  RTSP_METHOD_ALL              =
  RTSP_METHOD_DESCRIBE |
  RTSP_METHOD_SETUP    |
  RTSP_METHOD_PLAY     |
  RTSP_METHOD_PAUSE    |
  RTSP_METHOD_TEARDOWN |
  RTSP_METHOD_OPTIONS  |
  RTSP_METHOD_SET_PARAMETER,
  // all of the above, http
  HTTP_METHOD_ALL              =
  HTTP_METHOD_GET      |
  HTTP_METHOD_HEAD     |
  HTTP_METHOD_POST     |
  HTTP_METHOD_OPTIONS
};

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
**                        Class Declarations
** ======================================================================= */
class IPStreamProtocolHeaders
{
public:
   IPStreamProtocolHeaders();
   ~IPStreamProtocolHeaders();

   /**
    * @brief
    *   This is the main function of this class.  Client
    *   uses it to add and remove entries from the oem headers
    *   list.
    *   SIDE EFFECTS: The oem headers array is updated
    *
    * @param whatToDo
    * @param whichMethodsAffected     AffectedMethod,  dc on delete
    * @param headerName               don't care for delete_all
    * @param headerValue              don't care for delete_all, delete
    *
    * @return IPStreamProtocolHeaderResult
    */
   IPStreamProtocolHeaderResult EditIPStreamProtocolHeaders(
     IPStreamProtocolHeaderCommand whatToDo,
     int whichMethodsAffected,
     const char *headerName,
     const char *headerValue);

   /**
    * @brief
    *   Return the first value for the given method and header
    *   name. If none found, return NULL.
    *
    * @param method   AffectedMethod
    * @param name
    *
    * @return const char*
    */
   const char *ValueFor(int method,
                        const char *name);

   int  HeaderCount();

   /**
     * @brief
     *  Add all the IPStreamProtocol header key-value pairs to the buffer in
     *  header format
     * @param oemMethod
     * @param buffer
     * @param bufSize
     *
     * @return int
     */
   int AddIPStreamProtocolHeaders(AffectedMethod oemMethod, char *buffer, int bufSize);

   /**
    * @brief
    *   Find the first entry in the headers which applies to the
    *   passed method.
    *
    * @param whichMethods   AffectedMethod
    * @param name
    * @param value
    *
    * @return bool
    *   returns true if one found, and name/value pair filled in.
    *   Else false, and name/value are 0
    */
   bool FindFirst(int whichMethods, const char *&name, const char *&value);

   bool FindNext(const char *&name, const char *&value); // only after FindFirst()

private:

   IPStreamProtocolHeaderEntry IPStreamProtocolHeaderTable[IPSTREAM_PROTOCOL_HEADER_TABLE_SIZE];

   int findHeaderEntry(const int whichMethodsAffected, const char *nameToFind);
   int findFreeHeaderEntry();
   void freeAllHeaders();
   int freeHeader(const int whichMethodsAffected, const char *nameToFree);
   void freeHeaderItem(IPStreamProtocolHeaderEntry *item);

   /**
    * @brief
    *   Helper function. add the specified name/value to the list.
    *
    *   SIDE EFFECTS
    *     Memory is allocated for a copy of the passed name & value,
    *     if the add is totally successful.  If there is any error,
    *     the addition is undone.
    *
    * @param whichMethodsAffected     AffectedMethod
    * @param headerName
    * @param headerValue
    *
    * @return OemHeaderResult
    */
   IPStreamProtocolHeaderResult addHeader(
     int whichMethodsAffected,
     const char *headerName,
     const char *headerValue);

   /**
    * items for FindFirst(),FindNext()
    */
   uint32 searchCriteria; // if != METHOD_NONE

   /**
    * valid only if searchCriteria is valid
    */
   int lastItemFound;
};

#endif /* _IPSTREAM_PROTOCOL_HEADERS_H_ */
