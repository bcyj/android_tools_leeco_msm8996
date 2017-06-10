/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdint.h>
#include "rfsa_common.h"
#include "rfsa_list.h"

static void rfsa_list_raw_insert_node_before(rfsa_list_node_t *ref_node, rfsa_list_node_t *new_node)
{
	ref_node->prev->next = new_node;
	new_node->prev = ref_node->prev;
	new_node->next = ref_node;
	ref_node->prev = new_node;
}

static void rfsa_list_raw_insert_node_after(rfsa_list_node_t *ref_node, rfsa_list_node_t *new_node)
{
	ref_node->next->prev = new_node;
	new_node->prev = ref_node;
	new_node->next = ref_node->next;
	ref_node->next = new_node;
}

static void rfsa_list_raw_delete_node(rfsa_list_node_t *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->prev = node;
	node->next = node;
}

static int32_t rfsa_list_raw_is_empty(rfsa_list_t *list)
{
	return (list->dummy.next == &list->dummy);
}

static int32_t rfsa_list_raw_is_not_empty(rfsa_list_t *list)
{
	return (list->dummy.next != &list->dummy);
}

static void rfsa_list_default_lock(void)
{
}

static void rfsa_list_default_unlock(void)
{
}

int32_t rfsa_list_init(rfsa_list_t *list, rfsa_list_lock_enter_fn_t lock_fn, rfsa_list_lock_leave_fn_t unlock_fn)
{
	if (list == NULL) {
		return RFSA_EFAILED;
	}

	if (((lock_fn != NULL) ^ (unlock_fn != NULL)) != 0) {
		return RFSA_EFAILED;
	}

	list->dummy.prev = &list->dummy;
	list->dummy.next = &list->dummy;
	list->size = 0;

	if (lock_fn != NULL) {
		list->lock_fn = lock_fn;
		list->unlock_fn = unlock_fn;
	} else {
		list->lock_fn = rfsa_list_default_lock;
		list->unlock_fn = rfsa_list_default_unlock;
	}

	return RFSA_EOK;
}

int32_t rfsa_list_destroy(rfsa_list_t *list)
{
	if (list == NULL) {
		return RFSA_EFAILED;
	}

	list->dummy.prev = &list->dummy;
	list->dummy.next = &list->dummy;
	list->size = 0;
	list->lock_fn = rfsa_list_default_lock;
	list->unlock_fn = rfsa_list_default_unlock;

	return RFSA_EOK;
}

int32_t rfsa_list_add_head(rfsa_list_t *list, rfsa_list_node_t *node)
{
	if ((list == NULL ) || ( node == NULL)) {
		return RFSA_EFAILED;
	}

	list->lock_fn();

	rfsa_list_raw_insert_node_after(&list->dummy, node);
	list->size += 1;

	list->unlock_fn();

	return RFSA_EOK;
}

int32_t rfsa_list_add_tail(rfsa_list_t *list, rfsa_list_node_t *node)
{
	if ((list == NULL) || (node == NULL)) {
		return RFSA_EFAILED;
	}

	list->lock_fn();

	rfsa_list_raw_insert_node_before(&list->dummy, node);
	list->size += 1;

	list->unlock_fn();

	return RFSA_EOK;
}

int32_t rfsa_list_peak_head(rfsa_list_t *list, rfsa_list_node_t **ret_node)
{
	if ((list == NULL) || (ret_node == NULL)) {
		return RFSA_EFAILED;
	}

	list->lock_fn();

	if (rfsa_list_raw_is_empty(list)) {
		list->unlock_fn();
		return RFSA_EFAILED;
	}

	*ret_node = list->dummy.next;

	list->unlock_fn();

	return RFSA_EOK;
}

int32_t rfsa_list_peak_tail(rfsa_list_t *list, rfsa_list_node_t **ret_node)
{
	if ((list == NULL) || (ret_node == NULL )) {
		return RFSA_EFAILED;
	}

	list->lock_fn();

	if (rfsa_list_raw_is_empty(list)) {
		list->unlock_fn();
		return RFSA_EFAILED;
	}

	*ret_node = list->dummy.prev;

	list->unlock_fn();

	return RFSA_EOK;
}

int32_t rfsa_list_remove_head(rfsa_list_t *list, rfsa_list_node_t **ret_node)
{
	rfsa_list_node_t *node;

	if (list == NULL) {
		return RFSA_EFAILED;
	}

	list->lock_fn();

	if (rfsa_list_raw_is_empty(list)) {
		list->unlock_fn();
		return RFSA_EFAILED;
	}

	node = list->dummy.next;
	rfsa_list_raw_delete_node(node);
	list->size -= 1;

	list->unlock_fn();

	if (ret_node != NULL) {
		*ret_node = node;
	}

	return RFSA_EOK;
}

int32_t rfsa_list_remove_tail(rfsa_list_t *list, rfsa_list_node_t **ret_node)
{
	rfsa_list_node_t *node;

	if (list == NULL) {
		return RFSA_EFAILED;
	}

	list->lock_fn();

	if (rfsa_list_raw_is_empty(list)) {
		list->unlock_fn();
		return RFSA_EFAILED;
	}

	node = list->dummy.prev;
	rfsa_list_raw_delete_node(node);
	list->size -= 1;

	list->unlock_fn();

	if (ret_node != NULL) {
		*ret_node = node;
	}

	return RFSA_EOK;
}

int32_t rfsa_list_get_next(rfsa_list_t *list, rfsa_list_node_t *pivot_node, rfsa_list_node_t **ret_node)
{
	rfsa_list_node_t *node;

	if ((list == NULL) || (pivot_node == NULL)) {
		return RFSA_EFAILED;
	}

	/* list->lock_fn(); */ /* Assume the caller ensures safety. */

	/* Assume the given node is in the given list. */
	node = pivot_node->next;

	/* list->unlock_fn(); */

	if (ret_node != NULL) {
		*ret_node = node;
	}

	if (node == &list->dummy)
	{  /* No more iterations. */
		return RFSA_EFAILED;
	} else {
		return RFSA_EOK;
	}
}

int32_t rfsa_list_get_prev(rfsa_list_t *list, rfsa_list_node_t *pivot_node, rfsa_list_node_t **ret_node)
{
	rfsa_list_node_t *node;

	if ((list == NULL) || (pivot_node == NULL)) {
		return RFSA_EFAILED;
	}

	/* list->lock_fn(); */ /* Assume the caller ensures safety. */

	/* Assume the given node is in the given list. */
	node = pivot_node->prev;

	/* list->unlock_fn(); */

	if (ret_node != NULL) {
		*ret_node = node;
	}

	if (node == &list->dummy)
	{  /* No more iterations. */
		return RFSA_EFAILED;
	} else {
		return RFSA_EOK;
	}
}

int32_t rfsa_list_delete(rfsa_list_t *list, rfsa_list_node_t *node)
{
	if ((list == NULL) || (node == NULL)) {
		return RFSA_EFAILED;
	}

	list->lock_fn();

	/* Assume the given node is in the given list. */
	rfsa_list_raw_delete_node(node);
	list->size -= 1;

	list->unlock_fn();

	return RFSA_EOK;
}

int32_t rfsa_list_clear(rfsa_list_t *list)
{
	rfsa_list_node_t *node;

	if (list == NULL) {
		return RFSA_EFAILED;
	}

	list->lock_fn();

	while (rfsa_list_raw_is_not_empty(list)) {
		node = list->dummy.next;
		rfsa_list_raw_delete_node( node );
	}

	list->size = 0;
	list->unlock_fn();

	return RFSA_EOK;
}
