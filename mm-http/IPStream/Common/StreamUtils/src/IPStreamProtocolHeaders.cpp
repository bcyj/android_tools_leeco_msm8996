/* =======================================================================
                               IPStreamProtocolHeaders.cpp
DESCRIPTION

EXTERNALIZED FUNCTIONS

INITIALIZATION AND SEQUENCING REQUIREMENTS

COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
All rights reserved. Qualcomm Technologies proprietary and confidential.
========================================================================== */

/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/src/IPStreamProtocolHeaders.cpp#5 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
/* Includes custmp4.h. The following 2 includes must be the first includes in this file! */
#include "IPStreamProtocolHeaders.h"
#include "qtv_msg.h"
#include "SourceMemDebug.h"
#include "string.h"

/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define NO_IPSTREAM_PROTOCOL_HEADER_FOUND -1

const char CHAR_CR        = 13;
const char CHAR_LF        = 10;

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

/* -----------------------------------------------------------------------
** Static/private  Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Function Declarations
** ======================================================================= */

/* ======================================================================
FUNCTION
  IPStreamProtocolHeaders::HeaderCount

DESCRIPTION
  Determines the number of oem-defined headers currently defined
DEPENDENCIES
  None

RETURN VALUE
  # of headers.  0 if none.

SIDE EFFECTS
========================================================================== */
int IPStreamProtocolHeaders::HeaderCount()
{
   int result = 0;
   IPStreamProtocolHeaderEntry *entry = &IPStreamProtocolHeaderTable[0];
   for (int itemIx = 0; itemIx < IPSTREAM_PROTOCOL_HEADER_TABLE_SIZE; ++itemIx)
   {
      if (entry->affectedMethods != PROTO_METHOD_NONE)
         ++result;
      ++entry;
   }
   return result;
}


/* ======================================================================
FUNCTION
  IPStreamProtocolHeaders::EditIPStreamProtocolHeaders

DESCRIPTION
   This is the main function of this class.  QtvPlayer uses it to
  add and remove entries from the oem headers list.

DEPENDENCIES
  None

RETURN VALUE
  Result of the requested operation

SIDE EFFECTS
  The oem headers array is updated
========================================================================== */

IPStreamProtocolHeaderResult IPStreamProtocolHeaders::EditIPStreamProtocolHeaders(
   IPStreamProtocolHeaderCommand whatToDo,
   int whichMethodsAffected, // AffectedMethod, dc on delete
   const char *headerName,  // don't care for delete_all
   const char *headerValue  // don't care for delete_all, delete
)
{
   IPStreamProtocolHeaderResult result = IPSTREAM_PROTOCOL_RESULT_OK; // assume ok
   switch (whatToDo)
   {
      case IPSTREAM_PROTOCOL_HEADER_DELETE_ALL:
         QTV_MSG(QTVDIAG_GENERAL, "Deleting all oem headers");
         freeAllHeaders();
         break;
      case IPSTREAM_PROTOCOL_HEADER_DELETE:
         QTV_MSG_SPRINTF_2(QTVDIAG_GENERAL, "Deleting oem header '%s' for method %d",
                           headerName, whichMethodsAffected);
         if (freeHeader(whichMethodsAffected, headerName) == NO_IPSTREAM_PROTOCOL_HEADER_FOUND)
         {
            // print error
            result = IPSTREAM_PROTOCOL_RESULT_NOT_FOUND;
         }
         break;
      case IPSTREAM_PROTOCOL_HEADER_ADD:
         result = addHeader(whichMethodsAffected,headerName, headerValue);
         break;
      case IPSTREAM_PROTOCOL_HEADER_REPLACE:
         // first delete, then add
         QTV_MSG_SPRINTF_2(QTVDIAG_GENERAL, "Replacing oem header '%s' for method %d",
                           headerName, whichMethodsAffected);
         (void)freeHeader(whichMethodsAffected, headerName);
         result = addHeader(whichMethodsAffected, headerName, headerValue);
         break;
      default:
         QTV_MSG1(QTVDIAG_GENERAL, "Illegal Oem Header command: %d", whatToDo);
         result = IPSTREAM_PROTOCOL_RESULT_BAD_COMMAND;
         break;
   }

   if (result != IPSTREAM_PROTOCOL_RESULT_OK)
   {
      QTV_MSG2(QTVDIAG_GENERAL, "Failure in command %d, =%d",
         whatToDo, result);
   }
   return result;
}


/* ======================================================================
FUNCTION
  IPStreamProtocolHeaders::addHeader

DESCRIPTION
   Helper function.
   add the specified name/value to the list.

DEPENDENCIES
  None

RETURN VALUE
  Result of the add
SIDE EFFECTS
  Memory is allocated for a copy of the passed name & value, if the
 add is totally successful.  If there is any error, the addition is
 undone.
========================================================================== */
IPStreamProtocolHeaderResult IPStreamProtocolHeaders::addHeader(
   int whichMethodsAffected, // AffectedMethod
   const char *headerName,
   const char *headerValue
)
{
   QTV_MSG_SPRINTF_3(QTVDIAG_GENERAL, "Add oem header, meth=%d, '%s'='%s'",
      whichMethodsAffected, headerName, headerValue);

   // sanity checks
   IPStreamProtocolHeaderResult result = IPSTREAM_PROTOCOL_RESULT_OK; // assume ok
   if ( (whichMethodsAffected == PROTO_METHOD_NONE) || (headerName == 0) ||
        (headerValue == 0) ||
        (strlen(headerName) == 0) || (strlen(headerValue) == 0) ||
        (findHeaderEntry(whichMethodsAffected, headerName) >= 0) )
   {
      result = IPSTREAM_PROTOCOL_RESULT_BAD_DATA;
   }
   if (result == IPSTREAM_PROTOCOL_RESULT_OK)
   {
      int itemToAdd = findFreeHeaderEntry();

      if (itemToAdd == NO_IPSTREAM_PROTOCOL_HEADER_FOUND)
      {
         result = IPSTREAM_PROTOCOL_RESULT_TOO_MANY;
      }
      else
      {
         IPStreamProtocolHeaderEntry *entry = &IPStreamProtocolHeaderTable[itemToAdd];
         entry->affectedMethods = whichMethodsAffected;
         entry->headerName = (char*)QTV_Malloc(strlen(headerName) + 1);
         entry->headerValue = (char*)QTV_Malloc(strlen(headerValue) + 1);
         // check success.  If not, clear back out
         if ( (entry->headerName == 0) || (entry->headerValue == 0) )
         {
            freeHeaderItem(entry);
            result = IPSTREAM_PROTOCOL_RESULT_NO_MEMORY;
            QTV_MSG(QTVDIAG_GENERAL, "Unable to alloc memory for new header");
         }
         else
         {
            // copy over
            (void)std_strlcpy(entry->headerName,
                              headerName,
                              (strlen(headerName) + 1));
            (void)std_strlcpy(entry->headerValue,
                              headerValue,
                              (strlen(headerValue) + 1));
         }
      }
   }
   return result;
}


/* ======================================================================
FUNCTION
  IPStreamProtocolHeaders::IPStreamProtocolHeaders

DESCRIPTION
   Class constructor.

DEPENDENCIES
  None

RETURN VALUE

SIDE EFFECTS
  The IPStreamProtocol header table is made empty
========================================================================== */

IPStreamProtocolHeaders::IPStreamProtocolHeaders()
{
  // preset the table empty
  IPStreamProtocolHeaderEntry *entry = &IPStreamProtocolHeaderTable[0];
  for (int itemIx = 0; itemIx < IPSTREAM_PROTOCOL_HEADER_TABLE_SIZE; ++itemIx)
  {
    entry->affectedMethods = PROTO_METHOD_NONE;
    entry->headerName = entry->headerValue = 0;
    ++entry;
  }

  lastItemFound = 0;
  // other inits
  searchCriteria = PROTO_METHOD_NONE; // == FindNext() invalid
}

/* ======================================================================
FUNCTION
  IPStreamProtocolHeaders::~IPStreamProtocolHeaders

DESCRIPTION
   Class destructor
   Because this is a singleton that is never freed, the destructor should
 never be invoked.  It's here for completeness only.

DEPENDENCIES
  None

RETURN VALUE

SIDE EFFECTS
  The IPStreamProtocol header table is made empty
========================================================================== */
IPStreamProtocolHeaders::~IPStreamProtocolHeaders()
{
/*we need not handle exceptions in destructors because our functions are
  return based and they never raise any exception which we need to catch
  by 'catch'*/
  freeAllHeaders();/*lint !e1551 /above comment*/
}


/* ======================================================================
FUNCTION
  IPStreamProtocolHeaders::freeAllHeaders

DESCRIPTION
   Helper function.
   Clear out any and all currently defined oem headers

DEPENDENCIES
  None

RETURN VALUE

SIDE EFFECTS
  The oem header table is made empty.  All dynamically allocated memory
 is freed.
========================================================================== */
void IPStreamProtocolHeaders::freeAllHeaders()
{
   IPStreamProtocolHeaderEntry *entry = &IPStreamProtocolHeaderTable[0];
   for (int itemIx = 0; itemIx < IPSTREAM_PROTOCOL_HEADER_TABLE_SIZE; ++itemIx)
   {
      if (entry->affectedMethods != 0)
      {
         entry->affectedMethods = PROTO_METHOD_NONE;
         QTV_Free(entry->headerName);
         QTV_Free(entry->headerValue);
         entry->headerName = entry->headerValue = 0;
      }
      ++entry;
   }
}


/* ======================================================================
FUNCTION
  IPStreamProtocolHeaders::freeHeader

DESCRIPTION
   Helper function.
   Remove the specified name from the list.  Memory freed.

DEPENDENCIES
  None

RETURN VALUE
  Index of item made free, else NO_IPSTREAM_PROTOCOL_HEADER_FOUND
SIDE EFFECTS
  The oem header table entry is made available.  Associated dynamically
 allocated memory is freed.
========================================================================== */
int IPStreamProtocolHeaders::freeHeader(const int whichMethodsAffected, const char *nameToFree)
{
   //Header name is the key to the table and it could have multiple entries with the
   //same header name but with different affectedMethods (e.g. Add(62, "User-Agent"),
   //Add(1, "User-Agent")). Loop through the entire table to first find the header
   //name and clear the input method from entry->affectedMethods
   IPStreamProtocolHeaderEntry *entry = &IPStreamProtocolHeaderTable[0];
   int result = NO_IPSTREAM_PROTOCOL_HEADER_FOUND;
   for (int itemIx = 0; itemIx < IPSTREAM_PROTOCOL_HEADER_TABLE_SIZE; ++itemIx)
   {
      if ( (entry->affectedMethods != PROTO_METHOD_NONE) &&
           (std_strcmp(entry->headerName, nameToFree) == 0) )
      {
        if ((entry->affectedMethods & whichMethodsAffected) != 0)
        {
          int currentMethod = entry->affectedMethods;
          entry->affectedMethods &= ~whichMethodsAffected;
          QTV_MSG_SPRINTF_3(QTVDIAG_GENERAL, "Clearing method %d/%d for oem header '%s'",
                            whichMethodsAffected, currentMethod, nameToFree);
          if (entry->affectedMethods == PROTO_METHOD_NONE)
          {
            //Free the node as no more valid methods
            QTV_MSG_SPRINTF_2(QTVDIAG_GENERAL, "Free oem header '%s' for method %d",
                              nameToFree, currentMethod);
            freeHeaderItem(entry);
          }
          result = itemIx;
        }
      }
      ++entry;
   }

   return result;
}



/* ======================================================================
FUNCTION
  IPStreamProtocolHeaders::freeHeaderItem

DESCRIPTION
   Helper function.
   Remove the specified name from the list.  Memory freed.
DEPENDENCIES
  None

RETURN VALUE
  None
SIDE EFFECTS
  The IPStreamProtocol header table entry is made available.  Associated dynamically
 allocated memory is freed.
========================================================================== */
void IPStreamProtocolHeaders::freeHeaderItem(IPStreamProtocolHeaderEntry *item)
{
   QTV_Free(item->headerName);
   QTV_Free(item->headerValue);
   item->headerName = item->headerValue = 0;
   item->affectedMethods = PROTO_METHOD_NONE;
}

/* ======================================================================
FUNCTION
  IPStreamProtocolHeaders::findHeaderEntry

DESCRIPTION
   Helper function.
 Searches the table for a name matching that passed.

DEPENDENCIES
  None

RETURN VALUE
  Index of the entry, if found (0..N).  If not, NO_IPSTREAM_PROTOCOL_HEADER_FOUND
SIDE EFFECTS
  NONE
========================================================================== */
int IPStreamProtocolHeaders::findHeaderEntry(const int whichMethodsAffected, const char *nameToFind)
{
   int result = NO_IPSTREAM_PROTOCOL_HEADER_FOUND; // assume failed
   if (nameToFind != 0)
   {
      IPStreamProtocolHeaderEntry *entry = &IPStreamProtocolHeaderTable[0];
      for (int itemIx=0; itemIx<IPSTREAM_PROTOCOL_HEADER_TABLE_SIZE; ++itemIx)
      {
         if ( (entry->affectedMethods != PROTO_METHOD_NONE) &&
              (entry->headerName != 0) &&
              (std_strcmp(nameToFind, entry->headerName) == 0) &&
              ((entry->affectedMethods & whichMethodsAffected) != 0) )
         {
            result = itemIx;
            break;
         }
         ++entry;
      }
   }
   return result;
}


/* ======================================================================
FUNCTION
  IPStreamProtocolHeaders::findFreeHeaderEntry

DESCRIPTION
   Helper function.
 Find a free entry in the header table

DEPENDENCIES
  None

RETURN VALUE
  Index of the entry, if found (0..N).  If not, NO_IPSTREAM_PROTOCOL_HEADER_FOUND
SIDE EFFECTS
  NONE
========================================================================== */
int IPStreamProtocolHeaders::findFreeHeaderEntry()
{
   int result = NO_IPSTREAM_PROTOCOL_HEADER_FOUND; // assume failed
   IPStreamProtocolHeaderEntry *entry = &IPStreamProtocolHeaderTable[0];
   for (int itemIx=0; itemIx<IPSTREAM_PROTOCOL_HEADER_TABLE_SIZE; ++itemIx)
   {
      if (entry->affectedMethods == PROTO_METHOD_NONE)
      {
         result = itemIx;
         break;
      }
      ++entry;
   }
   return result;
}


/* ======================================================================
FUNCTION
  IPStreamProtocolHeaders::FindFirst

DESCRIPTION
   Find the first entry in the headers which applies to
 the passed method.

DEPENDENCIES
  None

RETURN VALUE
  returns true if one found, and name/value pair filled in.  Else
  false, and name/value are 0

SIDE EFFECTS
  NONE
========================================================================== */
bool IPStreamProtocolHeaders::FindFirst(
   int method, // AffectedMethod
   const char *&name, const char *&value)
{
   // assume fail
   bool result = false;
   name = value = 0;
   searchCriteria = PROTO_METHOD_NONE; // invalid FindNext()
   IPStreamProtocolHeaderEntry *entry = &IPStreamProtocolHeaderTable[0];
   for (int itemIx = 0; itemIx < IPSTREAM_PROTOCOL_HEADER_TABLE_SIZE; ++itemIx)
   {
      if ( (entry->affectedMethods & method) != 0)
      {
         searchCriteria = method;  // signals valid FindNext() ability
         lastItemFound = itemIx;
         name = entry->headerName;
         value = entry->headerValue;
         result = true;
         break;
      }
      ++entry;
   }
   return result;
}


/* ======================================================================
FUNCTION
  IPStreamProtocolHeaders::FindNext

DESCRIPTION
  Find the next matching entry for FindFirst()-specified method.

DEPENDENCIES
  Only valid after prior call to FindFirst()

RETURN VALUE
  returns true if find match, and name/value pair filled in.  Else
  returns false and name/value are zero

SIDE EFFECTS
  NONE
========================================================================== */
bool IPStreamProtocolHeaders::FindNext(
   const char *&name, const char *&value)
{
   // assume fail
   bool result = false;
   name = value = 0;

   if (searchCriteria != PROTO_METHOD_NONE)
   {
     int itemIx = lastItemFound + 1;

     IPStreamProtocolHeaderEntry *entry = &IPStreamProtocolHeaderTable[itemIx];
     for ( ; itemIx < IPSTREAM_PROTOCOL_HEADER_TABLE_SIZE; ++itemIx)
     {
       if ( (entry->affectedMethods & searchCriteria) != 0)
       {
         lastItemFound = itemIx;
         name = entry->headerName;
         value = entry->headerValue;
         result = true;
         break;
       }
       ++entry;
     }
   }

   // if no result, reset search
   if (result == false)
   {
      searchCriteria = PROTO_METHOD_NONE; // now invalid FindNext()
   }

   return result;
}

/**
* @brief
*   Return the first value for the given method and header
*   name. If none found, return NULL.
*
* @param method   QtvPlayer::AffectedMethod
* @param name
*
* @return const char*
*/
const char *IPStreamProtocolHeaders::ValueFor(
   int method, // AffectedMethod
   const char *name)
{
   if (NULL == name)
   {
      return NULL;
   }

   IPStreamProtocolHeaderEntry *entry = &IPStreamProtocolHeaderTable[0];

   for (int itemIx = 0; itemIx < IPSTREAM_PROTOCOL_HEADER_TABLE_SIZE; ++itemIx)
   {
      if ( ((entry->affectedMethods & method) != 0) &&
           !std_strcmp(name, entry->headerName))
      {
         return entry->headerValue;
      }
      ++entry;
   }
   return NULL;
}

/**
 * @brief
 *  Add all the oem header key-value pairs to the buffer in
 *  header format
 * @param oemMethod
 * @param buffer
 * @param bufSize
 *
 * @return int
 */
int
IPStreamProtocolHeaders::AddIPStreamProtocolHeaders(AffectedMethod oemMethod,
                              char *buffer,
                              int bufSize)
{
  const char *name = NULL;
  const char *value = NULL;
  int numWritten = 0;

  if (NULL == buffer || bufSize <= 0)
  {
    QTV_MSG_PRIO2(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                  "Invalid buffer 0x%p with size %d", (void *)buffer, bufSize);
  }
  else
  {

    // convert the method type to enum understood by oem headers object
    bool moreHeaders = FindFirst(oemMethod, name, value);

    while (moreHeaders)
    {
      QTV_MSG_SPRINTF_2(QTVDIAG_GENERAL,"Add OEM Hdr: '%s: %s", name, value);
      int len = std_strlprintf( buffer, bufSize,
                                "%s: %s%c%c",
                                name, value, CHAR_CR, CHAR_LF );

      if (len <= 0 || len >= bufSize)
      {
        QTV_MSG_SPRINTF_3(QTVDIAG_GENERAL,
                          "No space for OEM hdr '%s' rem=%d len=%d",
                          name, bufSize, len);
        numWritten = -1;
        break;
      }

      buffer += len;
      bufSize -= len;
      numWritten += len;

      moreHeaders = FindNext(name, value);
    }
  }

  return numWritten;
}
