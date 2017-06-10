/******************************************************************************

                  Q C R I L _ D A T A _ S T U B S . H

******************************************************************************/

/******************************************************************************

  @file    qcril_data_stubs.h
  @brief   QCRIL Data test stubs Header File

  DESCRIPTION
  Header file for QCRIL_Data utility functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

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

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
04/22/11   ar         Initial version

******************************************************************************/

#ifndef __QCRIL_DATA_STUBS_H__
#define __QCRIL_DATA_STUBS_H__

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

typedef enum {
  QCRIL_STUB_RESPONSE_REQ,
  QCRIL_STUB_RESPONSE_UNS,
  QCRIL_STUB_RESPONSE_MAX
} qcril_stub_response_type;

typedef void (*qcril_stub_response_cb)( qcril_stub_response_type type,
                                        void *param_ptr,
                                        void *user_data );


#endif /* __QCRIL_DATA_STUBS_H__ */
