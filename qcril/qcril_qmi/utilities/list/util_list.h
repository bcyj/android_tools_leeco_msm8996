/***************************************************************************************************
    @file
    util_list.h

    @brief
    Facilitates list related operations by providing list related utilities.

    Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
    Qualcomm Technologies Proprietary and Confidential.
***************************************************************************************************/

#ifndef UTIL_LIST
#define UTIL_LIST

#include "utils_standard.h"
#include "util_synchronization.h"
#include "util_bit_field.h"

#define UTIL_LIST_BIT_FIELD_NONE                      ((uint64_t) 0)
#define UTIL_LIST_BIT_FIELD_CREATED_ON_HEAP           (((uint64_t) 1) << 0)
#define UTIL_LIST_BIT_FIELD_SORTED                    (((uint64_t) 1) << 1)
#define UTIL_LIST_BIT_FIELD_AUTO_LOCK                 (((uint64_t) 1) << 2)
#define UTIL_LIST_BIT_FIELD_USE_COND_VAR              (((uint64_t) 1) << 3)

#define UTIL_LIST_NODE_BIT_FIELD_NONE                   ((uint64_t) 0)
#define UTIL_LIST_NODE_BIT_FIELD_USE_SYNC_VAR           (((uint64_t) 1) << 0)
#define UTIL_LIST_NODE_BIT_FIELD_IS_USERDATA_MODIFIED   (((uint64_t) 1) << 1)

typedef struct util_list_node_data_type
{
    util_sync_data_type node_sync_data;
    util_bit_field_type node_bit_field;
    void *user_data;
}util_list_node_data_type;

typedef int (*add_evaluator_type)(util_list_node_data_type *to_be_added_data,
                                  util_list_node_data_type *to_be_evaluated_data);

typedef void (*delete_evaluator_type)(util_list_node_data_type *to_be_deleted_data);

typedef int (*find_evaluator_type)(util_list_node_data_type *to_be_found_data);

typedef int (*enumerate_evaluator_type)(util_list_node_data_type *to_be_evaluated_data);

typedef struct util_list_node_type
{
    util_list_node_data_type node_data;
    struct util_list_node_type *prev;
    struct util_list_node_type *next;
}util_list_node_type;

typedef struct util_list_info_type
{
    util_sync_data_type list_sync_data;
    util_list_node_type *list_head;
    util_list_node_type *list_tail;
    add_evaluator_type default_add_evaluator;
    delete_evaluator_type default_delete_evaluator;
    util_bit_field_type list_bit_field;
    unsigned int num_of_node;
}util_list_info_type;

void util_list_default_delete_evaluator(util_list_node_data_type *to_be_deleted_data);
void util_list_delete_data_from_list_by_user_data(util_list_info_type *list_info,
                                                  void *to_be_deleted_data,
                                                  delete_evaluator_type delete_evaluator);
util_list_node_data_type* util_list_find_data_in_list_with_param(
    const util_list_info_type *list_info,
    int (*find_evaluator)(const util_list_node_data_type *to_be_found_data, void *compare_data),
    const void* compare_data
);



/***************************************************************************************************
    @function
    util_list_create

    @brief
    Creates a list.

    @param[in]
        list_info
           pointer to the info object that is used to hold the meta data of the list
           can be NULL, if the list's meta data is to be stored on heap
        default_add_evaluator
           pointer to function that is used by default to evaluate at which position of the list a
           newly created node needs to be inserted
           need to return TRUE (or FALSE) when the to be added node needs to be inserted
           before (or not) the currently compared node present in the list
        default_delete_evaluator
           pointer to function that is used by default to free the memory of the user data stored
           in a node when it is being removed from the list
        list_bit_field
           list attributes which need to be considered while creating the list
           Supported attributes:
               UTIL_LIST_BIT_FIELD_CREATED_ON_HEAP
                    list's meta data is to be stored on the heap
               UTIL_LIST_BIT_FIELD_SORTED
                    list would be maintained in a sorted order by the user
               UTIL_LIST_BIT_FIELD_AUTO_LOCK
                    locks the list on add/delete/find node operation automatically
               UTIL_LIST_BIT_FIELD_USE_COND_VAR
                    creates conditional variable on which a thread adding a node to the list can
                    wait on until the node has been processed

    @param[out]
        none

    @retval
    pointer to the info object that holds the meta data of the list If list has been created
    successfully, NULL otherwise
***************************************************************************************************/
util_list_info_type* util_list_create(util_list_info_type *list_info,
                                      add_evaluator_type default_add_evaluator,
                                      delete_evaluator_type default_delete_evaluator,
                                      util_bit_field_type list_bit_field);




/***************************************************************************************************
    @function
    util_list_cleanup

    @brief
    Cleansup a created list after removing and freeing the nodes.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list
        delete_evaluator
           pointer to function that is used to free the memory of the user data stored
           in a node when it is being removed from the list
           can be NULL if a default delete evaluator was provided while creating the list

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void util_list_cleanup(util_list_info_type *list_info,
                       delete_evaluator_type delete_evaluator);






/***************************************************************************************************
    @function
    util_list_enumerate

    @brief
    Enumerates the nodes of the list.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list
        enumerate_evaluator
           pointer to function that is used to perform operations on the node being enumerated
           need to return TRUE (or FALSE) when the enumerated node needs to be removed
           (or not) from the list
           user data is not freed even If the enumerated node is removed

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void util_list_enumerate(util_list_info_type *list_info,
                         enumerate_evaluator_type enumerate_evaluator);






/***************************************************************************************************
    @function
    util_list_add

    @brief
    Adds user data to the list by creating a new node.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list
        to_be_added_data
           pointer to the user data that needs to be added to the list
        add_evaluator
           pointer to function that is used by to evaluate at which position of the list a
           newly created node needs to be inserted
           need to return TRUE (or FALSE) when the to be added node needs to be inserted
           before (or not) the currently compared node present in the list
           can be NULL if a default add evaluator was provided while creating the list
        node_bit_field
           node attributes which need to be considered while creating the node
           Supported attributes:
               UTIL_LIST_NODE_BIT_FIELD_USE_SYNC_VAR
                    creates synchronization object using which a thread adding a node to the list
                    can wait on until the node has been processed
               UTIL_LIST_NODE_BIT_FIELD_IS_USERDATA_MODIFIED
                    indicates if the user data of the node has been modified since the last
                    enumeration

    @param[out]
        none

    @retval
    ESUCCESS If the user data has been added successfully to the list, appropriate error code
    otherwise
***************************************************************************************************/
int util_list_add(util_list_info_type *list_info,
                  void *to_be_added_data,
                  add_evaluator_type add_evaluator,
                  util_bit_field_type node_bit_field);





/***************************************************************************************************
    @function
    util_list_delete

    @brief
    Deletes user data from the the list by removing the corresponding node.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list
         to_be_deleted_data
           pointer to the data that needs to be deleted from the list
        delete_evaluator
           pointer to function that is used to free the memory of the user data stored
           in a node when it is being removed from the list
           can be NULL if a default delete evaluator was provided while creating the list

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void util_list_delete(util_list_info_type *list_info,
                      util_list_node_data_type *to_be_deleted_data,
                      delete_evaluator_type delete_evaluator);





/***************************************************************************************************
    @function
    util_list_enque

    @brief
    Adds user data to the start of the list by creating a new node.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list
        to_be_added_data
           pointer to the user data that needs to be added to the list
        node_bit_field
           node attributes which need to be considered while creating the node
           Supported attributes:
               UTIL_LIST_NODE_BIT_FIELD_USE_SYNC_VAR
                    creates synchronization object using which a thread adding a node to the list
                    can wait on until the node has been processed
               UTIL_LIST_NODE_BIT_FIELD_IS_USERDATA_MODIFIED
                    indicates if the user data of the node has been modified since the last
                    enumeration

    @param[out]
        none

    @retval
    ESUCCESS If the user data has been added successfully to the list, appropriate error code
    otherwise
***************************************************************************************************/
int util_list_enque(util_list_info_type *list_info,
                    void *to_be_added_data,
                    util_bit_field_type node_bit_field);






/***************************************************************************************************
    @function
    util_list_deque

    @brief
    Removes user data stored inside the first node of the list by removing the corresponding node.
    user data is not freed.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list

    @param[out]
        none

    @retval
    pointer to the user data whose node has been been dequed from the list
***************************************************************************************************/
void* util_list_deque(util_list_info_type *list_info);





/***************************************************************************************************
    @function
    util_list_retrieve_successor

    @brief
    Retrieves the successor of a node from the list.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list
        predecessor_data
           pointer to the predecessor node data

    @param[out]
        none

    @retval
    pointer to the successor node data
***************************************************************************************************/
util_list_node_data_type* util_list_retrieve_successor(util_list_info_type *list_info,
                                                       util_list_node_data_type *predecessor_data);





/***************************************************************************************************
    @function
    util_list_retrieve_predecessor

    @brief
    Retrieves the predecessor of a node from the list.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list
        successor_data
           pointer to the successor node data

    @param[out]
        none

    @retval
    pointer to the predecessor node data
***************************************************************************************************/
util_list_node_data_type* util_list_retrieve_predecessor(util_list_info_type *list_info,
                                                         util_list_node_data_type *successor_data);





/***************************************************************************************************
    @function
    util_list_retrieve_head

    @brief
    Retrieves the head node of the list.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list

    @param[out]
        none

    @retval
    pointer to the head node data
***************************************************************************************************/
util_list_node_data_type* util_list_retrieve_head(util_list_info_type *list_info);





/***************************************************************************************************
    @function
    util_list_retrieve_tail

    @brief
    Retrieves the tail node of the list.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list

    @param[out]
        none

    @retval
    pointer to the tail node data
***************************************************************************************************/
util_list_node_data_type* util_list_retrieve_tail(util_list_info_type *list_info);






/***************************************************************************************************
    @function
    util_list_find

    @brief
    Finds the node of a list which satisfies user specifications.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list
        find_evaluator
           pointer to function that is used to check if particular user data satisfies
           user specifications
           need to return TRUE (or FALSE) when the being checked node satisfies
           user specifications (or not)

    @param[out]
        none

    @retval
    pointer to the data of the node that satisfies user specifications
***************************************************************************************************/
util_list_node_data_type* util_list_find(util_list_info_type *list_info,
                                         find_evaluator_type find_evaluator);






/***************************************************************************************************
    @function
    util_list_lock_list

    @brief
    Locks the list.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list

    @param[out]
        none

    @retval
    ESUCCESS If the list has been locked successfully, appropriate error code otherwise
***************************************************************************************************/
int util_list_lock_list(util_list_info_type *list_info);






/***************************************************************************************************
    @function
    util_list_unlock_list

    @brief
    Unlocks the list.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list

    @param[out]
        none

    @retval
    ESUCCESS If the list has been unlocked successfully, appropriate error code otherwise
***************************************************************************************************/
int util_list_unlock_list(util_list_info_type *list_info);






/***************************************************************************************************
    @function
    util_list_wait_on_list

    @brief
    Waits on the list.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list
        wait_for_time_seconds
            time (in seconds) to wait on the conditional variable, NIL If wait needs
            to indefinite

    @param[out]
        none

    @retval
    ESUCCESS If wait been performed and the thread has been wokenup,
    appropriate error code otherwise
***************************************************************************************************/
int util_list_wait_on_list(util_list_info_type *list_info,
                           int wait_for_time_seconds);






/***************************************************************************************************
    @function
    util_list_signal_for_list

    @brief
    Signals the thread waiting on the list.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list

    @param[out]
        none

    @retval
    ESUCCESS If signalling is successful, appropriate error code otherwise
***************************************************************************************************/
int util_list_signal_for_list(util_list_info_type *list_info);






/***************************************************************************************************
    @function
    util_list_set_bits_in_list_bit_field

    @brief
    Sets the list attributes.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list
        bits_to_be_set
           list attributes to be set

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void util_list_set_bits_in_list_bit_field(util_list_info_type *list_info,
                                          util_bit_field_type bits_to_be_set);






/***************************************************************************************************
    @function
    util_list_remove_bits_from_list_bit_field

    @brief
    Removes specific attributes of the list.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list
        bits_to_be_removed
           attributes of the list to be removed

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void util_list_remove_bits_from_list_bit_field(util_list_info_type *list_info,
                                               util_bit_field_type bits_to_be_removed);






/***************************************************************************************************
    @function
    util_list_is_bits_set_in_list_bit_field

    @brief
    Checks if the list contains specific attributes.

    @param[in]
        list_info
           pointer to the info object that holds the meta data of the list
        bits_to_be_checked
           to be checked attributes
        is_partial_match_accepted
           TRUE If a subset of contained attributes is good enough for a successful match,
           FALSE otherwise

    @param[out]
        none

    @retval
    TRUE If the match has been successful, FALSE otherwise
***************************************************************************************************/
int util_list_is_bits_set_in_list_bit_field(util_list_info_type *list_info,
                                            util_bit_field_type bits_to_be_checked,
                                            int is_partial_match_accepted);





/***************************************************************************************************
    @function
    util_list_lock_node

    @brief
    Locks the node.

    @param[in]
        node_data
           pointer to the node data object that needs to be locked

    @param[out]
        none

    @retval
    ESUCCESS If the node has been locked successfully, appropriate error code otherwise
***************************************************************************************************/
int util_list_lock_node(util_list_node_data_type *node_data);





/***************************************************************************************************
    @function
    util_list_unlock_node

    @brief
    Unlocks the node.

    @param[in]
        node_data
           pointer to the node data object that needs to be unlocked

    @param[out]
        none

    @retval
    ESUCCESS If the node has been unlocked successfully, appropriate error code otherwise
***************************************************************************************************/
int util_list_unlock_node(util_list_node_data_type *node_data);





/***************************************************************************************************
    @function
    util_list_wait_on_node

    @brief
    Waits on the node.

    @param[in]
        node_data
           pointer to the node data object that needs to be used for waiting
        wait_for_time_seconds
            time (in seconds) to wait on the conditional variable, NIL If wait needs
            to indefinite

    @param[out]
        none

    @retval
    ESUCCESS If wait been performed and the thread has been wokenup,
    appropriate error code otherwise
***************************************************************************************************/
int util_list_wait_on_node(util_list_node_data_type *node_data,
                           int wait_for_time_seconds);





/***************************************************************************************************
    @function
    util_list_signal_for_node

    @brief
    Signals the thread waiting on the node.

    @param[in]
        node_data
           pointer to the node data object that needs to be used for signalling

    @param[out]
        none

    @retval
    ESUCCESS If signalling is successful, appropriate error code otherwise
***************************************************************************************************/
int util_list_signal_for_node(util_list_node_data_type *node_data);






/***************************************************************************************************
    @function
    util_list_set_bits_in_node_bit_field

    @brief
    Sets the node attributes.

    @param[in]
        node_data
           pointer to the node data object whose attributes need to be set
        bits_to_be_set
           node attributes to be set

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void util_list_set_bits_in_node_bit_field(util_list_node_data_type *node_data,
                                          util_bit_field_type bits_to_be_set);






/***************************************************************************************************
    @function
    util_list_remove_bits_from_node_bit_field

    @brief
    Removes specific attributes of the node.

    @param[in]
        node_data
           pointer to the node data object whose specific attributes need to be removed
        bits_to_be_removed
           attributes of the node to be removed

    @param[out]
        none

    @retval
    none
***************************************************************************************************/
void util_list_remove_bits_from_node_bit_field(util_list_node_data_type *node_data,
                                               util_bit_field_type bits_to_be_removed);






/***************************************************************************************************
    @function
    util_list_is_bits_set_in_node_bit_field

    @brief
    Checks if the node contains specific attributes.

    @param[in]
        node_data
           pointer to the node data object whose specific attributes need to be checked
        bits_to_be_checked
           to be checked attributes
        is_partial_match_accepted
           TRUE If a subset of contained attributes is good enough for a successful match,
           FALSE otherwise

    @param[out]
        none

    @retval
    TRUE If the match has been successful, FALSE otherwise
***************************************************************************************************/
int util_list_is_bits_set_in_node_bit_field(util_list_node_data_type *node_data,
                                            util_bit_field_type bits_to_be_checked,
                                            int is_partial_match_accepted);

#endif
