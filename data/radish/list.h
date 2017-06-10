/*
 * Copyright (c) 2011-2012,2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef RADISH_LIST_H_INCLUDED
#define RADISH_LIST_H_INCLUDED

#include "radish.h"

#define OFFSET(type, member) ({ (size_t)(&(((type *)NULL)->member)); })
#define OWNER(type, member, object) ({ (type *)(((size_t)(object)) - OFFSET(type, member)); })
#define LIST_OWNER(list, type, member) ({ List *__l = (list); rlist_is_valid(__l) ? OWNER(type, member, __l) : NULL; })

#define rlist_foreach(var, list) for((var) = rlist_first(list); (var) && (var) != (var)->head; (var) = (var)->next)

struct list;
typedef struct list List;

struct list {
    List *next;
    List *prev;
    List *head;
};

static inline int rlist_is_valid(List *list)
{
    return list && list != list->head;
}

static inline int rlist_init(List *list)
{
    if (!list) return -1;

    list->next = list->prev = list->head = list;

    return 0;
}

static inline int rlist_insert(List *list, List *item)
{
    if (!list || !item) return -1;

    item->head = list->head;

    list->prev->next = item;
    item->next = list;

    item->prev = list->prev;
    list->prev = item;

    return 0;
}

static inline int rlist_append(List *where, List *item)
{
    if (!where || !(where->head) || !item) return -1;

    where = where->head;

    rlist_insert(where, item);
    return 0;
}

static inline int rlist_prepend(List *where, List *item)
{
    if (!where || !(where->head) || !item) return -1;

    where = where->head;
    item->head = where->head;

    where->next->prev = item;
    item->prev = where;

    item->next = where->next;
    where->next = item;

    return 0;
}

static inline int rlist_remove(List *list, List *item)
{
    if (!list || !item || !(item->head) || item->head != list || !item->prev || !item->next) return -1;

    item->prev->next = item->next;
    item->next->prev = item->prev;

    item->next = item->prev = item->head = NULL;

    return 0;
}

static inline int rlist_is_empty(List *list)
{
    if (!list) {
        LOGE("%s: List is null\n", __func__);
        return 1;
    }
    RAD_LOGI("%s: list: %p list->head: %p list->next: %p list->prev: %p\n",
             __func__,
             (unsigned*)list,
             (unsigned*)list->head,
             (unsigned*)list->next,
             (unsigned*)list->prev);
    if (list->head == list->next && !(list->head == list && list->next == list)) {
      RAD_LOGI("%s: list->head %p, list: %p, list->next: %p\n",
               __func__,
               (unsigned*) list->head,
               (unsigned*)list,
               (unsigned*)list->next);
    }
    return list->head == list->next;
}

static inline List *rlist_first(List *list)
{
    return rlist_is_empty(list) ? NULL : list->head->next;
}

static inline List *rlist_pop_first(List *list)
{
    List *ret = rlist_first(list);
    rlist_remove(list, ret);
    return ret;
}

static inline List *rlist_last(List *list)
{
    List *__l = (list);
    return rlist_is_empty(__l) ? NULL : __l->head->prev;
}

static inline List *rlist_pop_last(List *list)
{
    List *__l = (list);
    List *__ret = rlist_last(list);
    rlist_remove(__l, __ret);
    return __ret;
}

static inline void rlist_move(List *dst, List *src)
{
    List *cur;

    if (!dst || !src) return;

    for (cur = rlist_pop_first(src); cur; cur = rlist_pop_first(src)) {
        rlist_append(dst, cur);
    }
}

#endif /* RADISH_LIST_H_INCLUDED */
