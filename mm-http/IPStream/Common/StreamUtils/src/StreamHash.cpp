/************************************************************************* */
/**
 * @file
 * @brief
 *  A simple Hash structure for Streaming.
 *
 *  Possible key types:
 *    1. integer
 *    2. null terminated string.
 *
 * Value: generic void pointer.
 *
 * Characteristics:
 *  There are no deep mempry copies for pointers. So, caller should ensure that
 *    any object pointed at by the key or value should outlive the instance of
 *    the hash.
 *  Duplicates not allowed. Insert will overwrite value if key already exists.
 *  Strictly no throw. So useful where exceptions are no recommended in code.
 *
 * Note that as this is meant for internal use only, the caller should ensure
 * that string keys are null terminated.
 *
 COPYRIGHT 2011-2013 Qualcomm Technologies, Inc.
 All rights reserved. Qualcomm Technologies proprietary and confidential.
 *
 ************************************************************************* */
/* =======================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/src/StreamHash.cpp#7 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "qtv_msg.h"
#include "SourceMemDebug.h"
#include "StreamHash.h"

StreamHash::StreamHash(KeyType keyType) :
  m_KeyType(keyType),
  m_HashArray(NULL),
  m_Size(0)
{

}

StreamHash::~StreamHash()
{
  if (m_HashArray)
  {
    for (unsigned int i = 0; i < m_Size; ++i)
    {
      while (StreamQ_cnt(&m_HashArray[i]) > 0)
      {
        HashElem *elem =
          (HashElem *) StreamQ_get(&m_HashArray[i]);

        if (elem)
        {
          QTV_Delete(elem);
        }
      }
    }

    QTV_Delete_Array(m_HashArray);
    m_HashArray = NULL;
  }
}

/**
 * @brief
 *  Allocate and setup internal data structures. The hash cannot
 *  be used unless it is successfully initialized.
 *
 * @param hashSize
 *
 * @return bool
 *  true    if successful
 *  false   if failure occurs
 */
bool
StreamHash::Initialize(int hashSize)
{
  m_Size = hashSize;

  if ((int)m_Size > MAX_HASH_KEYS)
  {
    m_Size = MAX_HASH_KEYS;
  }
  else
  {
    if (0 == hashSize)
    {
      m_Size = DEFAULT_HASH_SIZE;
    }
  }

  m_HashArray = QTV_New_Array(StreamQ_type, m_Size);

  if (m_HashArray)
  {
    for (unsigned int i = 0; i < m_Size; ++i)
    {
      StreamQ_init(&m_HashArray[i]);
    }
  }
  else
  {
    QTV_MSG_PRIO1(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "StreamHash::Initialize: Failed to allocated hash array of size '%d'",
      m_Size);

    m_Size = 0;
  }

  return (m_HashArray ? true : false);
}

/**
 * @brief
 *  Insert a key-value pair for an inteher key
 *
 * @param key
 * @param value
 *
 * @return bool
 */
bool
StreamHash::Insert(int key, void* val)
{
  bool rslt = false;

  if (TYPE_INT == m_KeyType)
  {
    char keyStr[512];
    memset(keyStr, 0x0, sizeof(keyStr));
    snprintf(keyStr, sizeof(keyStr), "%d", key);
    rslt = InsertHelper((void *)keyStr, val);
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                 "StreamHash::Insert: Error Hash not defined for int");
  }

  return rslt;
}

/**
 * @brief
 *  Insert a key-value pair for an string key
 *
 * @param key
 * @param value
 *
 * @return bool
 */
bool
StreamHash::Insert(const char* key, void* value)
{
  bool rslt= false;

  if (TYPE_STRING == m_KeyType)
  {
    rslt = InsertHelper((void *)key, value);
  }
  else
  {
    QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                 "StreamHash::Insert: Error Hash not defined for string");
  }

  return rslt;
}

 /**
 * @brief
 *  Get the value for integer key.
 *
 * @param key
 *
 * @return void*
 */
void *
StreamHash::GetValue(int key) const
{
  void* value = NULL;

  if (0 == m_Size)
  {
    QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "StreamHash::GetValue: Failed. Has Initialize() been called?");
  }
  else
  {
    if (TYPE_INT == m_KeyType)
    {
      const HashElem *elem = GetElemForIntKey(key);
      value = (elem ? elem->m_Value : NULL);
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                   "StreamHash::GetValue: Error Hash not defined for int");
    }
  }

  return value;
}

/**
 * @brief
 *  Provide array access syntax for integer keys.
 *
 * @param key
 *
 * @return void*
 */
void*
StreamHash::operator[](int key)
{
  return GetValue(key);
}

 /**
 * @brief
 *  Get the value for string key.
 *
 * @param key
 *
 * @return void*
 */
void*
StreamHash::GetValue(const char* key) const
{
  void* value = NULL;

  if (0 == m_Size)
  {
    QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "StreamHash::GetValue: Failed. Has Initialize() been called?");
  }
  else
  {
    if (TYPE_STRING == m_KeyType)
    {
      const HashElem *elem = GetElemForStringKey(key);
      value = (elem ? elem->m_Value : NULL);
    }
    else
    {
      QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
                   "StreamHash::GetValue: Error Hash not defined for string");
    }
  }

  return value;
}

/**
 * @brief
 *  Provide array access syntax for string keys.
 *
 * @param key
 *
 * @return void*
 */
void*
StreamHash::operator[](const char* key)
{
  return GetValue(key);
}

/**
 * @brief
 *  Insert a key-value pair into the hash. Ensure the key-value
 *  pair inserted is not a duplicate.
 *
 * @param key
 * @param val
 *
 * @return bool
 */
bool
StreamHash::InsertHelper(void* key, void* val)
{
  bool rslt = false;
  HashElem* elem = NULL;

  if (0 == m_Size)
  {
    QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
      "StreamHash::InsertHelper: Failed. Has Initialize() been called?");
  }
  else
  {
    if (TYPE_INT == m_KeyType)
    {
      int iKey = atoi((char *)key);
      elem = GetElemForIntKey(iKey);
    }
    else // TYPE_CHAR == m_Type
    {
      elem = GetElemForStringKey((const char*)key);
    }

    if (NULL != elem)
    {
      elem->m_Value = val;
    }
    else
    {
      elem = QTV_New(HashElem);

      if (NULL == elem)
      {
        QTV_MSG_PRIO(QTVDIAG_STREAMING, QTVDIAG_PRIO_ERROR,
          "StreamHash::InsertKey: Failed to alocated hash elem");
      }
      else
      {
        unsigned int hashKey = 0;

        if (TYPE_INT == m_KeyType)
        {
          int iKey = atoi((char *)key);
          hashKey = ComputeHashIndexForInt(iKey);
          (elem->m_HashKey).keyInt = iKey;
        }
        else // TYPE_STRING == m_KeyType
        {
          hashKey = ComputeHashIndexForString((const char *)key);
          (elem->m_HashKey).keyString = (const char*)key;
        }

        (elem->m_Value) = val;

        StreamQ_link(elem, &(elem->link));
        StreamQ_put(&m_HashArray[hashKey], &(elem->link));

        rslt = true;
      }
    }
  }

  return rslt;
}

/**
 * @brief
 *  Simple algo to calcuate hashIndex from an integer key.
 *  Assumes uniform distribution of key space.
 *
 * @param key
 *
 * @return unsigned int
 */
unsigned int
StreamHash::ComputeHashIndexForInt(int key) const
{
  return (m_Size > 0 ? key % m_Size : 0);
}

/**
 * @brief
 *  Simple algo to calcualte hashIndex for a string.
 *  This probably needs to to tuned.
 *
 * @param key
 *
 * @return unsigned int
 */
unsigned int
StreamHash::ComputeHashIndexForString(const char* key) const
{
  size_t len = std_strlen(key);
  unsigned int sum = 0;
  for (size_t i = 0; i < len; ++i)
  {
    sum += (unsigned int)((key[i]) * (i + 1));

    if (sum > 100000)
    {
      sum -= 100000;
    }
  }

  return (m_Size > 0 ? sum % m_Size : 0);
}

/**
 * @brief
 *  Return pointer to the hash element if any for the
 *  integer key.
 *  To be called by non-const functions.
 *
 * @param key
 *
 * @return StreamHash::HashElem*
 */
StreamHash::HashElem*
StreamHash::GetElemForIntKey(int key)
{
  unsigned int hashKey = ComputeHashIndexForInt(key);

  char keyStr[512];
  memset(keyStr, 0x0, sizeof(keyStr));
  snprintf(keyStr, sizeof(keyStr), "%d", key);

  return (HashElem *)StreamQ_linear_search(&m_HashArray[hashKey],
                                     CompareIntKeys,
                                     (void *)keyStr);
}

/**
 * @brief
 *  Return pointer to the hash element if any for the
 *  integer key.
 *  To be called by const functions.
 *
 * @param key
 *
 * @return StreamHash::HashElem*
 */
const StreamHash::HashElem*
StreamHash::GetElemForIntKey(int key) const
{
  unsigned int hashKey = ComputeHashIndexForInt(key);

  char keyStr[512];
  memset(keyStr, 0x0, sizeof(keyStr));
  snprintf(keyStr, sizeof(keyStr), "%d", key);
  return (const HashElem *)StreamQ_linear_search(&m_HashArray[hashKey],
                                           CompareIntKeys,
                                           (void *)keyStr);
}

/**
 * @brief
 *  Return pointer to the hash element if any for the string
 *  key.
 * To be called by non-const functions.
 *
 * @param key
 *
 * @return StreamHash::HashElem*
 */
StreamHash::HashElem*
StreamHash::GetElemForStringKey(const char* key)
{
  unsigned int hashKey = ComputeHashIndexForString(key);

  return (HashElem *)StreamQ_linear_search(&m_HashArray[hashKey],
                                     CompareStringKeys,
                                     (void *)key);
}

/**
 * @brief
 *  Return pointer to the hash element if any for the
 *  string key.
 * To be called by const functions.
 *
 * @param key
 *
 * @return StreamHash::HashElem*
 */
const StreamHash::HashElem*
StreamHash::GetElemForStringKey(const char* key) const
{
  unsigned int hashKey = ComputeHashIndexForString(key);

  return (const HashElem *)StreamQ_linear_search(&m_HashArray[hashKey],
                                           CompareStringKeys,
                                           (void *)key);

}
/**
 * Comparison function needed by queue.h,cpp routines to search for element
 * in queue.
 */
int
StreamHash::CompareIntKeys(void *itemPtr, void *compareVal)
{
  HashElem *elem = (HashElem *)itemPtr;
  int key = (elem->m_HashKey).keyInt;
  int tempKey = atoi((char *)compareVal);
  return (key == tempKey) ? 1 : 0;
}

/**
 * @brief
 *  For use for as comparison function for queue routines.
 *
 * @param itemPtr
 * @param compareVal
 *
 * @return int
 */
int
StreamHash::CompareStringKeys(void *itemPtr, void *compareVal)
{
  int result = 0;

  HashElem *elem = (HashElem *)itemPtr;

  if (NULL != elem)
  {
    const char *key = (const char *)(elem->m_HashKey.keyString);

    if (elem && compareVal && key)
    {
      result = (0 == std_strnicmp(key, (char *)compareVal, std_strlen(key))
                ? 1 : 0);
    }
  }

  return result;
}
