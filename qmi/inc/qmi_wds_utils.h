/*===========================================================================

  Copyright (c) 2007, 2014 Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.

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

#ifndef QMI_PROFILE_LL_SRVC_H
#define QMI_PROFILE_LL_SRVC_H

#include "qmi_wds_srvc.h"
#include "ds_sl_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
* Definitions associated with linked list based profile APIS
************************************************************************/

typedef   list_type             qmi_wds_profile_node_list_type;
typedef   list_link_type        qmi_wds_list_link_type;

typedef struct
{
  unsigned char           type;
  unsigned short          len;
  void                    *data;
} qmi_wds_profile_element_type;

typedef struct
{
  qmi_wds_list_link_type          *link;
  qmi_wds_profile_element_type    profile_element;
} qmi_wds_profile_node_type;


/*===========================================================================
  List version of the above Profile API's
===========================================================================*/

/*===========================================================================
  FUNCTION  qmi_wds_utils_create_profile
===========================================================================*/
/*!
@brief
  The List version of the qmi_wds_create_profile API.
  Takes as input a user client handle, profile params in the form of a linked list
  and a WDS profile ID structure and creates a QMI profile (on modem processor)
  based on these parameters.  The input profile ID MUST have the 'technology'
  field set to a valid value.

@return
  QMI_NO_ERR if the operation was sucessful, < 0 if not. If return code is
  QMI_SERVICE_ERR,then the qmi_err_code will be valid and will indicate which
  QMI error occurred.  Upon successful  return, the profile ID parameter will
  have the 'profile_index' field set to the profile index of the newly created
  profile.

  If return is QMI_EXTENDED_ERR, then qmi_err_code does not contain
  QMI_SERVICE_ERR_* error codes, but contains subsystem(module which modem QMI
  talks to, ex: ds_profile) specific error codes.
@note

  - This function executes synchronously.
  - Dependencies
    - None.

  - Side Effects
    - Creates a new profile on the modem processor.
*/
/*=========================================================================*/
EXTERN int
qmi_wds_utils_create_profile
(
  int                               user_handle,
  qmi_wds_profile_id_type           *profile_id,
  qmi_wds_profile_node_list_type    *profile_list,
  int                               *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_wds_utils_modify_profile
===========================================================================*/
/*!
@brief
  The List version of the qmi_wds_modify_profile API
  Takes as input a user client handle, WDS profile parameters as a linked list
  and a WDS profile ID structure and modifies parameters of an existing profile
  on the modem processor. The input profile ID MUST have the 'technology'
  field and 'profile_index' set to an existing valid profile index value.


@return
  QMI_NO_ERR if the operation was sucessful, < 0 if not. If return code is
  QMI_SERVICE_ERR,then the qmi_err_code will be valid and will indicate which
  QMI error occurred.

  If return is QMI_EXTENDED_ERR, then qmi_err_code does not contain
  QMI_SERVICE_ERR_* error codes, but contains subsystem(module which modem QMI
  talks to, ex: ds_profile) specific error codes.

@note

  - This function executes synchronously.

  - Dependencies
    - None.

  - Side Effects
    - Modifies an existing profile on the modem processor.
*/
/*=========================================================================*/
EXTERN int
qmi_wds_utils_modify_profile
(
  int                               user_handle,
  qmi_wds_profile_id_type           *profile_id,
  qmi_wds_profile_node_list_type    *profile_list,
  int                               *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_wds_utils_query_profile
===========================================================================*/
/*!
@brief
  The list version of the qmi_wds_query_profile
  Takes as input a user client handle, and a WDS  profile ID structure and
  queries the parameters of an existing profileon the modem processor. The
  input profile ID MUST have the 'technology' field and 'profile_index' set
  to an existing valid profile index value. The current values of the parameters
  will be returned in the profile_params structure.

  On a successful return the profile_list will point to a linked list of
  profile params for the given profile Id.

@return
  QMI_NO_ERR if the operation was sucessful, < 0 if not. If return code is
  QMI_SERVICE_ERR,then the qmi_err_code will be valid and will indicate which
  QMI error occurred.

  If return is QMI_EXTENDED_ERR, then qmi_err_code does not contain
  QMI_SERVICE_ERR_* error codes, but contains subsystem(module which modem QMI
  talks to, ex: ds_profile) specific error codes.
@note

  - This function executes synchronously.
  - The library generates a linked list of profile params on successful query.
  - The client should free up the linked list i.e. the data of each node,
    the node itself and list's head node.

  - Dependencies
    - None.

  - Side Effects
    - Queries an existing profile on the modem processor.
*/
/*=========================================================================*/
EXTERN int
qmi_wds_utils_query_profile
(
  int                               user_handle,
  qmi_wds_profile_id_type           *profile_id,
  qmi_wds_profile_node_list_type    *profile_list,/*Should point to a VALID list head node*/
                                                  /*Sucessful return(QMI_NO_ERR) will have this point
                                                   to a linked list of profile params*/
  int                               *qmi_err_code
);

/*===========================================================================
  FUNCTION qmi_wds_utils_write_optional_profile_tlvs
===========================================================================*/
/*!
@brief
  Takes the input profile data in the form of a linked list, and writes
  it in TLV form to the tx buffer.  Buffer pointers and length indicators
  are adjusted to reflect new TLV.

@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note
  - This function frees the list elements, list node and the list head.

  - Dependencies
    - None.

  - Side Effects
    - Writes TLV to input buffer and updates pointers and byte count
*/
/*=========================================================================*/
EXTERN int
qmi_wds_utils_write_optional_profile_tlvs
(
  unsigned char                   **tx_buf,
  int                             *tx_buf_len,
  qmi_wds_profile_node_list_type  *profile_list
);

/*===========================================================================
  FUNCTION  qmi_wds_cleanup_list
===========================================================================*/
/*!
@brief
  Takes as input a linked list head node and frees up the entire list.
  i.e. all the list nodes elements, the nodes itself and the list head node.


@return
  QMI_INTERNAL_ERR if an error occurred, QMI_NO_ERR if not

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
EXTERN void
qmi_wds_utils_cleanup_list
(
  qmi_wds_profile_node_list_type      *profile_list
);

/*===========================================================================
  FUNCTION  qmi_wds_utils_get_profile_list
===========================================================================*/
/*!
@brief
  Takes as input a user client handle, and an array of profile query
  structures in which to place response, and the number of elements in that
  array. Also optionally the technology and key-value search pairs(formatted as
  tlvs linked list) can be given as input, these will be used to further filter
  the search on the modem.

  Note that the num_profile_list_elements is both an input and output
  parameter.  It should be used to indicated the number of elements in
  the profile_list array when calling the function, and will contain the
  number of elements returned when the function returns.

@return
  Returned is the array filled in with return data, and the number of
  elements of the array that have been returned.
  QMI_INTERNAL_ERR or QMI_SERVICE_ERR if an error occurred, QMI_NO_ERR if not.
  If QMI_SERVICE_ERR is returned, the qmi_err_code return parameter
  will be valid, otherwise it won't
  If return is QMI_EXTENDED_ERR, then qmi_err_code does not contain
  QMI_SERVICE_ERR_* error codes, but contains subsystem(module which modem QMI
  talks to, ex: ds_profile) specific error codes.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
EXTERN int
qmi_wds_utils_get_profile_list
(
  int                             user_handle,
  qmi_wds_profile_tech_type       *profile_tech,
  qmi_wds_profile_node_list_type  *profile_search_list,
  qmi_wds_profile_list_type       *profile_list,
  int                             *num_profile_list_elements,
  int                             *qmi_err_code
);


/*===========================================================================
  FUNCTION  qmi_wds_utils_get_profile_list2
===========================================================================*/
/*!
@brief
  Takes as input a user client handle, and an array of profile query
  structures in which to place response, and the number of elements in
  that array. Also optionally the technology and single key-value search
  pair (formatted as parameter mask and value) can be given as input,
  these will be used to further filter the search on the modem.

  Note that the num_profile_list_elements is both an input and output
  parameter.  It should be used to indicated the number of elements in
  the profile_list array when calling the function, and will contain the
  number of elements returned when the function returns.

@return
  Returned is the array filled in with return data, and the number of
  elements of the array that have been returned.
  QMI_INTERNAL_ERR or QMI_SERVICE_ERR if an error occurred, QMI_NO_ERR if not.
  If QMI_SERVICE_ERR is returned, the qmi_err_code return parameter
  will be valid, otherwise it won't
  If return is QMI_EXTENDED_ERR, then qmi_err_code does not contain
  QMI_SERVICE_ERR_* error codes, but contains subsystem(module which modem QMI
  talks to, ex: ds_profile) specific error codes.

@note

  - This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
EXTERN int
qmi_wds_utils_get_profile_list2
(
  int                              user_handle,
  qmi_wds_profile_tech_type        profile_tech,
  uint64_t                         param_mask,
  void                            *param_value,
  qmi_wds_profile_list_type       *profile_list,
  int                             *num_profile_list_elements,
  int                             *qmi_err_code
);

#ifdef __cplusplus
}
#endif

#endif

