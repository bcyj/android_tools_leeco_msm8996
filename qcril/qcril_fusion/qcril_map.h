/*!
  @file qcril_map.h

  @brief External declarations for an implementation of hash tables using
  unsigned intergers as key values and using void pointers to store values.

  Algorithm due to Corman, Leiserson et al, from Introduction to 
  Algorithms (second edition) section 11.2.

  Copyright (C) Qualcomm Technologies, Inc., All Rights Reserved.
*/

/*===========================================================================

  Copyright (c) 2008 - 2009 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

#if !defined(QCRIL_MAP_H)
#define QCRIL_MAP_H

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_map.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
20May2008  jod     Created
===========================================================================*/

/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

/*! @brief We return NULL value when not present. */
#define QCRIL_MAP_NOT_PRESENT 0

/*! @brief Hash table element */
typedef struct qcril_map_elem
{
  int                    is_ptr;  /*!< TRUE if item is a pointer type */
  unsigned int           key;     /*!< Key value for item */
  void*                  item;    /*!< Item pointer */
  struct qcril_map_elem* next;    /*!< Pointer to next chained value */
  struct qcril_map_elem* prev;    /*!< Pointer to previous chained value */
} qcril_map_elem;

/*! @brief Hash table container structure */
typedef struct qcril_map_ht
{
  struct qcril_map_elem** table;  /*!< Hash table pointer structure */
  int                     nbits;  /*!< Number of bits in the hash table */
} qcril_map_ht;

/*! @brief Hash table pointer ADT */
typedef qcril_map_ht* qcril_map;

/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/

/*! @brief Construct and return a new hash table */
extern qcril_map qcril_map_constructor(const int nbits);

/*! @brief Destroy a hash table and all of its contents. O(n) */
extern void qcril_map_destructor(qcril_map ht);

/*! @brief Find a value in a hash table given a key. O(1) */
extern void* qcril_map_find(const qcril_map ht, const unsigned int k);

/*! @brief Add a new value to a hash table. O(1) */
extern void qcril_map_add(qcril_map ht, const unsigned int k, 
                          void* item, const int is_ptr);

/*! @brief Delete hashtable entry corresponding to a given key. O(1) */
extern void qcril_map_delete(qcril_map ht, const unsigned int k);

#endif /* QRIL_MAP_H */
