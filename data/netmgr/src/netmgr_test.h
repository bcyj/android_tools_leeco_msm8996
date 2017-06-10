/******************************************************************************

                        N E T M G R _ T E S T . H
 
******************************************************************************/

/******************************************************************************

  @file    netmgr_test.h
  @brief   Network Manager tests Header File

  DESCRIPTION
  Header file for NetMgr test functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved

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
02/19/10   ar         Initial version

******************************************************************************/

#ifndef __NETMGR_TEST_H__
#define __NETMGR_TEST_H__

#include <pthread.h>

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/
#define MAX_DESCRIPTION_SIZ  80

typedef int (*netmgr_testcase_cb)(void*);

typedef struct netmgr_testcase_s {
  char                description[MAX_DESCRIPTION_SIZ];
  netmgr_testcase_cb  testcase;
} netmgr_testcase_t;


typedef struct netmgr_test_info_s {
  netmgr_testcase_t  *current;
  pthread_cond_t      cond;   /* Condition variable for signaling */
  pthread_mutex_t     mutx;   /* Mutex required by condition var */
} netmgr_test_info_t;

int netmgr_test_execute( void );

void netmgr_test_init( void );


/*===========================================================================
  FUNCTION  netmgr_client_thread_wait
===========================================================================*/
/*!
@brief
  Function to wait on client event listener thread.
  THIS IS FOR TEST PURPOSES ONLY.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None  

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_client_thread_wait( void );

#endif /* __NETMGR_TEST_H__ */
