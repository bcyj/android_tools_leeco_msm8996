#ifndef _STREAMHASH_H_
#define _STREAMHASH_H_

/************************************************************************* */
/**
 * StreamHash.h
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

$Header: //source/qcom/qct/multimedia2/Video/Source/IPStream/Common/StreamUtils/main/latest/inc/StreamHash.h#5 $
$DateTime: 2012/03/20 07:46:30 $
$Change: 2284651 $

========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "StreamQueue.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
/* Forward declarations for class referenced */

/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                        Macro Definitions
** ======================================================================= */

/* =======================================================================
**                        Class & Function Declarations
** ======================================================================= */

class StreamHash
{
public:

  /**
   * Keep an upper limit on the array size
   */
  static const int MAX_HASH_KEYS = 1024;

  /**
   * Default hash size of size zero passed to Initialize()
   */
  static const int DEFAULT_HASH_SIZE = 512;

  /**
   * Supported key types
   */
  enum KeyType
  {
    TYPE_INT,
    TYPE_STRING
  };

  /**
   * c'tor
   *
   * @param keyType
   */
  StreamHash(KeyType keyType);

  /**
   * d/tor
   */
  ~StreamHash();

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
  bool Initialize(int hashSize);

  /**
   * @brief
   *  Insert a key-value pair for an inteher key
   *
   * @param key
   * @param value
   *
   * @return bool
   */
  bool Insert(int key, void* value);

  /**
   * @brief
   *  Insert a key-value pair for an string key
   *
   * @param key
   * @param value
   *
   * @return bool
   */
  bool Insert(const char* key, void* value);

  /**
   * @brief
   *  Get the value for integer key.
   *
   * @param key
   *
   * @return void*
   */
  void* GetValue(int key) const;

  /**
   * @brief
   *  Provide array access syntax for integer keys.
   *
   * @param key
   *
   * @return void*
   */
  void* operator[](int key);

   /**
   * @brief
   *  Get the value for string key.
   *
   * @param key
   *
   * @return void*
   */
  void* GetValue(const char* key) const;

  /**
   * @brief
   *  Provide array access syntax for string keys.
   *
   * @param key
   *
   * @return void*
   */
  void* operator[](const char* key);

  /**
   * @brief
   *  For use for as comparison function for queue routines.
   *
   * @param itemPtr
   * @param compareVal
   *
   * @return int
   */
  static int CompareIntKeys(void *itemPtr, void *compareVal);

  /**
   * @brief
   *  For use for as comparison function for queue routines.
   *
   * @param itemPtr
   * @param compareVal
   *
   * @return int
   */
  static int CompareStringKeys(void *itemPtr, void *compareVal);

private:
  StreamHash();
  StreamHash(const StreamHash&);
  StreamHash& operator=(const StreamHash&);

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
  bool InsertHelper(void* key, void* val);

  /**
   * @brief
   *  Simple algo to calcuate hashIndex from an integer key.
   *  Assumes uniform distribution of key space.
   *
   * @param key
   *
   * @return unsigned int
   */
  unsigned int ComputeHashIndexForInt(int key) const;

  /**
   * @brief
   *  Simple algo to calcualte hashIndex for a string.
   *  This probably needs to to tuned.
   *
   * @param key
   *
   * @return unsigned int
   */
  unsigned int ComputeHashIndexForString(const char* key) const;

  typedef struct
  {
    StreamQ_link_type link;

    union
    {
      int         keyInt;
      const char* keyString;

    } m_HashKey;

    void* m_Value;

  } HashElem;

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
  HashElem* GetElemForIntKey(int key);

  /**
   * @brief
   *  Return pointer to the hash element if any for the
   *  integer key.
   * To be called by const functions.
   *
   * @param key
   *
   * @return StreamHash::HashElem*
   */
  const HashElem* GetElemForIntKey(int key) const;

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
  HashElem* GetElemForStringKey(const char* key);

  /**
   * @brief
   *  Return pointer to the hash element if any for the string
   *  key.
    *To be called by const functions.
   *
   * @param key
   *
   * @return StreamHash::HashElem*
   */
  const HashElem* GetElemForStringKey(const char* key) const;

  KeyType m_KeyType;
  StreamQ_type* m_HashArray;
  unsigned int m_Size;
};

#endif
