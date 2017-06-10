/******************************************************************************
  @file    ds_util.h
  @brief    

  DESCRIPTION
  This file contains:
  1. Utility functions for DS PROFILE

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  N/A

  ---------------------------------------------------------------------------
  Copyright (C) 2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/modem/data/common/dsprofile/main/latest/inc/ds_util.h#2 $ $DateTime: 2009/10/23 12:09:31 $ $Author: mghotge $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/30/09   mg      Created the module. First version of the file.
===========================================================================*/

#ifndef DS_UTIL_H
#define DS_UTIL_H

#include "comdef.h"
#include "customer.h"
#include "ds_sl_list.h"

#define DS_SUCCESS 0
#define DS_FAILURE 1


#define DS_INVALID     -1
#define DS_FALSE        0
#define DS_TRUE         1

/*---------------------------------------------------------------------------
                     Iterator Public Data Structures
---------------------------------------------------------------------------*/
typedef void * ds_util_itr_hndl_type;
typedef void   (*frst)( void * );
typedef int   (*next)( void * );
typedef void * (*data)( void * );
typedef int    (*size)( void * );
typedef void   (*dstr)( void * );

typedef struct
{
  frst  f;
  next  n;
  data  i;
  size  s;
  dstr  d;
  void *curr;
} ds_util_iterable_type;

/*---------------------------------------------------------------------------
                          List Public Data Structures
---------------------------------------------------------------------------*/
typedef void *  ds_util_list_hndl_type;

typedef struct
{
  list_link_type  link;
  void           *data;
  int             size;
  void           *self;
} ds_util_list_node_type;

typedef struct
{
  ds_util_iterable_type   itr;
  list_type               lst;
  void                   *priv;
  void                   *self;
} ds_util_list_type;

/*---------------------------------------------------------------------------
                          List Public Fn declaration
---------------------------------------------------------------------------*/
int ds_util_list_get_hndl(
  ds_util_list_hndl_type *hndl
);

int ds_util_list_add(
  ds_util_list_hndl_type  hndl,
  void                   *info,
  uint32                  info_size
);

uint32 ds_util_list_get_size(
  ds_util_list_hndl_type  hndl
);

int ds_util_list_rel_hndl(
  ds_util_list_hndl_type hndl
);



/*---------------------------------------------------------------------------
                      Iterator Public Fn
---------------------------------------------------------------------------*/
int ds_util_itr_get_hndl(
  ds_util_iterable_type *obj, /* -> */
  ds_util_itr_hndl_type *hndl /* <- */
);

int ds_util_itr_get_data(
  ds_util_itr_hndl_type   hndl,
  void                   *obj,  /* <- */
  uint32                 *size  /* <-> */
);

int ds_util_itr_first(
  ds_util_itr_hndl_type   hndl
);

int ds_util_itr_next(
  ds_util_itr_hndl_type  hndl
);

int ds_util_itr_rel_hndl(
   ds_util_itr_hndl_type  hndl
);

#endif /* DS_UTIL_H */
