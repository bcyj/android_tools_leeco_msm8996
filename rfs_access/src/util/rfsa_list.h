#ifndef __RFSA_LIST_H__
#define __RFSA_LIST_H__

/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

typedef struct rfsa_list_node_t rfsa_list_node_t;
typedef struct rfsa_list_t rfsa_list_t;

typedef void (*rfsa_list_lock_enter_fn_t) (void);
typedef void (*rfsa_list_lock_leave_fn_t) (void);

struct rfsa_list_node_t {
	rfsa_list_node_t	*prev;
	rfsa_list_node_t	*next;
};

struct rfsa_list_t {
	rfsa_list_node_t		dummy;
	uint32_t			size;

	rfsa_list_lock_enter_fn_t	lock_fn;
	rfsa_list_lock_leave_fn_t	unlock_fn;
};

int32_t rfsa_list_init(rfsa_list_t *list, rfsa_list_lock_enter_fn_t lock_fn, rfsa_list_lock_leave_fn_t unlock_fn);
int32_t rfsa_list_destroy(rfsa_list_t *list);
int32_t rfsa_list_add_head(rfsa_list_t *list, rfsa_list_node_t *node);
int32_t rfsa_list_add_tail(rfsa_list_t *list, rfsa_list_node_t *node);
int32_t rfsa_list_peak_head(rfsa_list_t *list, rfsa_list_node_t **ret_node);
int32_t rfsa_list_peak_tail(rfsa_list_t *list, rfsa_list_node_t **ret_node);
int32_t rfsa_list_remove_head(rfsa_list_t *list, rfsa_list_node_t **ret_node);
int32_t rfsa_list_remove_tail(rfsa_list_t *list, rfsa_list_node_t **ret_node);
int32_t rfsa_list_get_next(rfsa_list_t *list, rfsa_list_node_t *pivot_node, rfsa_list_node_t **ret_node);
int32_t rfsa_list_get_prev(rfsa_list_t *list, rfsa_list_node_t *pivot_node, rfsa_list_node_t **ret_node);
int32_t rfsa_list_delete(rfsa_list_t *list, rfsa_list_node_t *node);
int32_t rfsa_list_clear(rfsa_list_t *list);

#endif /* __RFSA_LIST_H__ */
