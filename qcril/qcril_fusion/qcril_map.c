/*!
  @file qcril_map.c

  @brief Implementation of hash tables using unsigned intergers as key 
  values and using void pointers to store values. It is an assumption
  of the implementation that key values are unique within a given hash table.

  The implementation uses a hash table which is allocated as having 
  a size of 2^n bits, where n is given when the table is constructed.

  The algorithm implements chaining where there are hash key collisions,
  although this should occur relatively infrequently with a properly sized
  hash table.

  Algorithm due to Corman, Leiserson et al, from Introduction to 
  Algorithms (second edition) section 11.2.

  Regression tests are defined for qcril_map as a standalone executable
  at the end of the file. The tests supplied cover 100% of the non-test
  related code for the map implementation. Coverage obtained via:

  gcc -DQCRIL_MAP_UNIT_TEST -fprofile-arcs -ftest-coverage qcril_map.c
  a.out
  gcov qcril_map.c

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

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_map.c#2 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
05/18/09   fc      Changes to log debug messages to Diag directly instead
                   of through logcat.
04/05/09   fc      Cleanup assert macro.
26/01/08   fc      Logged assertion info.
04/06/08   jod     Fixed case where integer 0 is stored as an item.
03/06/08   jod     Added stand-alone regression tests
20i/05/08  jod     Created
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <stddef.h>
#include <malloc.h>
#include <memory.h>
#ifdef QCRIL_MMGSDI_UNIT_TEST
#include "assert.h"
#endif /* QCRIl_MMGSDI_UNIT_TEST */
#include "qcrili.h"
#include "qcril_log.h"
#include "qcril_map.h"

/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

#ifdef QCRIL_MMGSDI_UNIT_TEST
#define QCRIL_MAP_ASSERT( xx_exp ) \
  { if((xx_exp) == 0)  assert(0); }
#else
#define QCRIL_MAP_ASSERT( xx_exp ) QCRIL_ASSERT( xx_exp )
#endif /* QCRIL_MMGSDI_UNIT_TEST */

/*! Define QCRIL_MAP_UNIT_TEST to compile stand-alone for regression test */
//#define QCRIL_MAP_UNIT_TEST

/*! Hash constant approx. ((sqrt(5) - 1) / 2) * (1 << 32) (due to Knuth) */
#define QCRIL_MAP_HASH_CONST (2654435769)

/*===========================================================================

                    LOCAL FUNCTION IMPLEMENTATIONS

===========================================================================*/

/*==========================================================================*/
/*! Hash function, used to spread keys over the hash table. The value of 
    QCRIL_MAP_HASH_CONST is due to Knuth.

    @param ht Hash table pointer.
    @param k  Key value for which we want hash value.
    @return   Hash value used to select a bucket in the hash table.
 */
/*==========================================================================*/

static unsigned int qcril_map_hash(qcril_map ht, unsigned int k)
{
  unsigned int r = k * QCRIL_MAP_HASH_CONST;
  QCRIL_MAP_ASSERT(ht != NULL);

  return ((r >> (32 - ht->nbits)) & ((1 << ht->nbits) - 1));
}

/*==========================================================================*/
/*! Place a new element at the front of the chain of elements
    in hash table bucket @a bucket.

    @param ht     Hash table pointer.
    @param bucket Position in the hash table structure to place @a head.
    @param head   Hash table element to place at the front of the chain
                  of elements in the selected hash table bucket.
 */
/*==========================================================================*/

static void qcril_map_prepend(qcril_map ht,
                              const unsigned int bucket, 
                              qcril_map_elem* head)
{
  qcril_map_elem* tail = NULL;
  QCRIL_MAP_ASSERT(ht != NULL);

  if (ht->table[bucket] != NULL)
  {
    tail = ht->table[bucket];
    ht->table[bucket] = head;
    ht->table[bucket]->next = tail;
    tail->prev = head;
  }
  else
  {
    ht->table[bucket] = head;
  }
}

/*==========================================================================*/
/*! Search along the chain of elements in a hash table bucket for one
    which matches a kiven key value @a k.

    @param ht   Hash table pointer.
    @param head Pointer to the first element in the hash table bucket.
    @param k    Key value to search for
    @return     Pointer to found hash table element (NULL if not found).
 */
/*==========================================================================*/

static qcril_map_elem* qcril_map_search(qcril_map_elem* head, 
                                        const unsigned int k)
{
  while ((head != NULL) && (head->key != k))
    head = head->next;

  return head;
}

/*==========================================================================*/
/*! Remove a single element from a hash table bucket. The element
    is freed on removal.

    It is expected that this function will normally be called from 
    qcril_map_delete().

    @param ht     Hash table pointer.
    @param bucket Position in the hash table structure containing @a elem
    @param elem   Hash table entry to remove.
    @see qcril_map_delete().
 */
/*==========================================================================*/

static void qcril_map_remove(qcril_map ht, 
                             unsigned int bucket, 
                             qcril_map_elem* elem)
{
  qcril_map_elem* back;
  qcril_map_elem* fwd;

  QCRIL_MAP_ASSERT(ht != NULL);
  QCRIL_MAP_ASSERT(elem != NULL);

  back = elem->prev;
  fwd  = elem->next;

  if (back != NULL) back->next = fwd;
  if (fwd  != NULL) fwd->prev  = back;

  /* If there is no back and forward, bucket is empty */
  if ((back == NULL) && (fwd == NULL))
    ht->table[bucket] = NULL;
  else if ((back == NULL) && (fwd != NULL))
    ht->table[bucket] = fwd;

  free(elem);
}

/*==========================================================================*/
/*! Constuct and return a new hash table element.

    @param key   Key value used to access the item.
    @param item  Void pointer to the 'value' of the item.
    @return Hash table element with NULL previous and next pointers.
 */
/*==========================================================================*/

static qcril_map_elem* qcril_map_elem_constructor(const unsigned int key, 
                                                 void* item,
                                                 const int is_ptr)
{
  qcril_map_elem* new_elem = malloc(sizeof(qcril_map_elem));

  QCRIL_MAP_ASSERT(new_elem != NULL);
  if (is_ptr)
    QCRIL_MAP_ASSERT(item != NULL);

  new_elem->key    = key;
  new_elem->item   = item;
  new_elem->next   = NULL;
  new_elem->prev   = NULL;
  new_elem->is_ptr = is_ptr;

  return new_elem;
}

/*==========================================================================*/
/*! Destroy a hash table entry, its contents <b>and all subsequent
    entries and their contents.</b>.

    This function is intended to be called only from qcril_map_destructor().
    The implementation is recursive, but the stack frame is unlikely to 
    grow excessively on a suitable sized hash table as chain length should
    be short (usually only one item, in fact).

    @param elem First element in the chain to destroy.
 */
/*==========================================================================*/

static void qcril_map_elem_destructor(qcril_map_elem* elem)
{
  QCRIL_MAP_ASSERT(elem != NULL);
  QCRIL_MAP_ASSERT(elem->item != NULL);

  if (elem->is_ptr)
    free(elem->item);

  if (elem->next != NULL)
    qcril_map_elem_destructor(elem->next);

  free(elem);
}

/*===========================================================================

                    EXTERNAL FUNCTION IMPLEMENTATIONS

===========================================================================*/

/*==========================================================================*/
/*! The hashtable is constrained to have a number of buckets which is a
    power of two. To ensure that this is guaranteed, the power of two to
    be used is supplied (@a nbits) rather than the size.

    @param nbits Power of two used for the number of buckets.
    @return Empty (NULL buckets) hashtable pointer.
 */
/*==========================================================================*/

qcril_map qcril_map_constructor(const int nbits)
{
  qcril_map ht = malloc(sizeof(qcril_map_ht));
  QCRIL_MAP_ASSERT(ht != NULL);

  ht->nbits = nbits;
  ht->table = malloc(sizeof(qcril_map_elem*) * (1 << nbits));
  QCRIL_MAP_ASSERT(ht->table != NULL);
  memset(ht->table, 0, sizeof(qcril_map_elem*) * (1 << nbits));

  return ht;
}

/*==========================================================================*/
/*! @param ht Hash table pointer. 
 */
/*==========================================================================*/

void qcril_map_destructor(qcril_map ht)
{
  int i;

  QCRIL_MAP_ASSERT(ht != NULL);

  for (i = 0; i < (1 << ht->nbits); i++)
  {
    if (ht->table[i] != NULL)
    {
      qcril_map_elem_destructor(ht->table[i]);
    }
  }
  free(ht->table);
  free(ht);
}

/*==========================================================================*/
/*! If find operation fails, NULL is returned.

    @param ht Hash table pointer.
    @param k  Key to search for.
    @return   Void pointer to the value mapped by @a k.
 */
/*==========================================================================*/

void* qcril_map_find(const qcril_map ht, const unsigned int k)
{
  qcril_map_elem* elem;
  QCRIL_MAP_ASSERT(ht != NULL);

  elem = qcril_map_search(ht->table[qcril_map_hash(ht, k)], k);
  return elem ? elem->item : NULL;
}

/*==========================================================================*/
/*! @param ht   Hash table pointer
    @param k    Key value to use in mapping.
    @para. item Void pointer to the item to store in the map.
 */
/*==========================================================================*/

void qcril_map_add(qcril_map ht, const unsigned int k, void* item, const int is_ptr)
{
  unsigned int bucket ;
  QCRIL_MAP_ASSERT(ht != NULL);
  
  bucket = qcril_map_hash(ht, k);
  qcril_map_prepend(ht, bucket, qcril_map_elem_constructor(k, item, is_ptr));
}

/*==========================================================================*/
/*! If the key does not find an item in the map, the function 
    does nothing.

    @param ht Hash table pointer
    @param k  Key value to delete from the map.
 */
/*==========================================================================*/

void qcril_map_delete(qcril_map ht, const unsigned int k)
{
  unsigned int bucket; 
  qcril_map_elem* elem;

  QCRIL_MAP_ASSERT(ht != NULL);

  bucket = qcril_map_hash(ht, k);
  elem = qcril_map_search(ht->table[bucket], k);

  if (elem)
    qcril_map_remove(ht, bucket, elem);
}

/*===========================================================================

                     STANDALONE REGRESSION TESTS

===========================================================================*/
#if defined(QCRIL_MAP_UNIT_TEST)

#include <stdio.h>

#define TRUE (1 == 1)
#define FALSE (1 == 0)

typedef struct map_test_struct
{
  int a;
  int b;
  int c;
} map_test_struct;

typedef struct map_int
{
  int key;
  int val;
} map_int;

typedef struct map_struct
{
  int             key;
  map_test_struct val;
} map_struct;

static map_int int_data[] =
{
  { 1, 100 }, { 2, 200 }, { 4, 400 }, { 7, 700 },
  { 3, 300 }, { 8, 800 }, { 6, 600 }, { 5, 500 },
  { 9, 900 }, { 0,  50 }, { 0,   0 }
};

static map_struct struct_data[] =
{
  { 23, { 2300, 2200, 2100 } },
  { 71, { 7100, 7000, 6900 } },
  { 11, { 1100, 1000,  900 } },
  { 47, { 4700, 4600, 4500 } },
  { 53, { 5300, 5200, 5100 } },
  { 77, { 7700, 7600, 7500 } },
  { 86, { 8600, 8500, 8400 } },
  { 16, { 1600, 1500, 1400 } },
  { 28, { 2800, 2700, 2600 } },
  { 33, { 3300, 3200, 3100 } },
  { 42, { 4200, 4100, 4000 } },
  { 38, { 3800, 3700, 3600 } },
  { 17, { 1700, 1600, 1500 } },
  { 66, { 6600, 6500, 6400 } },
  { 14, { 1400, 1300, 1200 } },
  { 91, { 9100, 9000, 8900 } },
  { 67, { 6700, 6600, 6500 } },
  { 0,  {    0,    0,    0 } }
};

static qcril_map map_a;  /* Map of ints */
static qcril_map map_b;  /* Map of structs - chaining improbable */
static qcril_map map_c;  /* Map of structs - chaining certain */
static qcril_map map_d;  /* Map of structs - used to test destructor */

void init_maps()
{
  int i;
  int k = 2;
  int v1 = 3;
  int v2 = 4;
  int v3 = 5;

  map_test_struct* elem;
  
  map_a = qcril_map_constructor(5);  /* 32 buckets - chain improbable */
  for (i = 0; int_data[i].val != 0; i++)
  {
    qcril_map_add(map_a, int_data[i].key, (void*) int_data[i].val, FALSE);
  }

  map_b = qcril_map_constructor(5);  /* 32 buckets - chain improbable */
  for (i = 0; struct_data[i].key != 0; i++)
  {
    qcril_map_add(map_b, struct_data[i].key, (void*) &struct_data[i].val, TRUE);
  }

  map_c = qcril_map_constructor(3);  /* 8 buckets - chain certain */
  for (i = 0; struct_data[i].key != 0; i++)
  {
    qcril_map_add(map_c, struct_data[i].key, (void*) &struct_data[i].val, TRUE);
  }
  
  map_d = qcril_map_constructor(6);

  for (i = 0; i < 1000; i++)
  {
    elem = malloc(sizeof(map_test_struct));
    QCRIL_MAP_ASSERT(elem != NULL);
    elem->a = v1++;
    elem->b = v2++;
    elem->c = v3++;
    qcril_map_add(map_d, k++, (void*) elem, TRUE);
  }
}

int test_lookup_int_map(const int key, const int expect)
{
  int val = (int) qcril_map_find(map_a, key);

  if (val != expect)
  {
    printf("ERR: key: %d, val: %d, expect: %d\n", key, val, expect);
    return FALSE;
  }
  else
    return TRUE;
}

int test_lookup_struct_map(const qcril_map ht, const int key, const int e2)
{
  map_test_struct* val = (map_test_struct*) qcril_map_find(ht, key);

  if ((val == NULL) || (val->b != e2))
  {
    printf("ERR: key: %d, val: %d, expect: %d\n", key, val->b, e2);
    return FALSE;
  }
  else
    return TRUE;
}

int test_add_struct_map(const qcril_map ht, const int key,
                        const int e1, const int e2, const int e3)
{
  map_test_struct* ins = malloc(sizeof(map_test_struct));

  QCRIL_MAP_ASSERT(ins != NULL);
  ins->a = e1;
  ins->b = e2;
  ins->c = e3;

  qcril_map_add(ht, key, (void*) ins, TRUE);
  
  /* check if value added */
  return test_lookup_struct_map(ht, key, e2);
}

int test_delete_struct_map(const qcril_map ht, const int key)
{
  qcril_map_delete(ht, key);
  
  /* Check if really deleted */
  if (qcril_map_find(ht, key) != NULL)
  {
    printf("ERR: key: %d not deleted\n", key);
    return FALSE;
  }
  else 
    return TRUE;
}

int test_ints()
{
  return ( test_lookup_int_map(3, 300) &&
           test_lookup_int_map(8, 800) &&
           test_lookup_int_map(0, 50) );
}

int test_structs(const qcril_map ht)
{
  return ( test_lookup_struct_map(ht, 77, 7600) &&
           test_lookup_struct_map(ht, 42, 4100) &&
           test_lookup_struct_map(ht, 11, 1000) &&
           test_lookup_struct_map(ht, 23, 2200) &&
           test_add_struct_map(ht,  3,  300, 200, 100) &&
           test_lookup_struct_map(ht, 38, 3700) &&
           test_add_struct_map(ht, 89, 8900, 8800, 8700) &&
           test_delete_struct_map(ht, 3) &&
           test_lookup_struct_map(ht, 89, 8800) );
}

int main(int argc, char** argv)
{
  init_maps();

  if (test_ints())
    printf("qcril_map.c: int map: PASSED\n");
  else
    printf("qcril_map.c: int map: FAILED\n");

  if (test_structs(map_b))
    printf("qcril_map.c: struct map (no chain): PASSED\n");
  else
    printf("qcril_map.c: struct map (no chain): FAILED\n");

  if (test_structs(map_c))
    printf("qcril_map.c: struct map (chain): PASSED\n");
  else
    printf("qcril_map.c: struct map (chain): FAILED\n");

  /* This will give memory leak / access violation on error. Check output */
  qcril_map_destructor(map_d);
}

#endif /* QCRIL_MAP_UNIT_TEST */
