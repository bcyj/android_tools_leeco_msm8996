/* mct_list.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MCT_LIST_H__
#define __MCT_LIST_H__

#include "mtype.h"
typedef struct _mct_list mct_list_t;

typedef boolean (* mct_list_traverse_func)(void *data, void *user_data);

typedef boolean (* mct_list_find_func)(void *data1, void *data2);

typedef void (* mct_list_operate_func)
  (void *data1, void *data2, const void *user_data);

/*
 * We define List in this form simply because
 * in certain scenario, the list can work
 * as an unbalanced tree.
 * For example, during dynamic module linking,
 * one Stream could have tree type of linked modues,
 * in which one List Node could have multiple children(next).
 *
 * For most cases, nextNum should be ONLY one.
 *
 * It is a root node when prev == NULL
 */
struct _mct_list {
  void         *data;
  mct_list_t   *prev;
  mct_list_t   **next;   /* array of next(children) */
  uint32_t next_num; /* number of next(children) */
};

#define MCT_LIST_PREV(mct_list) \
  ((mct_list) ? (((mct_list_t *)(mct_list))->prev) : NULL)
#define MCT_LIST_NEXT(mct_list) \
  ((mct_list) ? (((mct_list_t **)(mct_list))->next) : NULL)

#if defined(__cplusplus)
extern "C" {
#endif

mct_list_t *mct_list_append(mct_list_t *mct_list, void *data, void *appendto,
             mct_list_find_func list_find);

mct_list_t *mct_list_prepend(mct_list_t *mct_list, void *data);

mct_list_t *mct_list_insert(mct_list_t *mct_list, void *data, uint32_t pos);

mct_list_t *mct_list_insert_before(mct_list_t *mct_list, mct_list_t *inserted,
             const void *data);

mct_list_t *mct_list_remove(mct_list_t *mct_list, const void *data);

mct_list_t *mct_list_find_custom (mct_list_t *mct_list, void *data,
             mct_list_find_func list_find);

mct_list_t *mct_list_find_and_add_custom (mct_list_t *parent_list,
  mct_list_t *child_list, void *data, mct_list_find_func list_find);

boolean     mct_list_traverse(mct_list_t *mct_list,
              mct_list_traverse_func traverse, void *user_data);

void        mct_list_free_list(mct_list_t *mct_list);

void        mct_list_free_all(mct_list_t *mct_list,
              mct_list_traverse_func traverse);

void        mct_list_free_all_on_data(mct_list_t *mct_list,
              mct_list_traverse_func traverse, void *user_data);

void        mct_list_operate_nodes (mct_list_t *mct_list,
              mct_list_operate_func list_operate, void *user_data);

#if defined(__cplusplus)
}
#endif


#endif /* __MCT_LIST_H__ */
