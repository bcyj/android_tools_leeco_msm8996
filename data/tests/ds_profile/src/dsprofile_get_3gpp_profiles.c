/******************************************************************************

           D S P R O F I L E _ G E T _ 3 G P P _ P R O F I L E S

******************************************************************************/

/******************************************************************************

  @file    dsprofile_get_3gpp_profiles.c
  @brief   ds_profiles API test

  DESCRIPTION
  Test to query Modem for 3GPP profile list

  ---------------------------------------------------------------------------
  Copyright (c) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
11/10/10   ar         created

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "ds_util.h"
#include "ds_profile.h"

#define SUCCESS      (0)
#define FAILURE      (-1)
#define MAX_NAME_SIZ (128)

int test_setup()
{
  ds_profile_status_etype ret;

  /* Initialize library */
  ds_log_low("INFO: Initializing profile library\n" );
  ret = ds_profile_init_lib();
  if( DS_PROFILE_REG_RESULT_SUCCESS !=  ret ) {
    ds_log_err( "Failed on ds_profile_init_lib, status=%d\n", ret );
    return FAILURE;
  }

  return SUCCESS;
}

int test_teardown()
{
  return SUCCESS;
}


int test_body()
{
  ds_profile_status_etype ret;
  ds_profile_list_type lst;
  ds_profile_itr_type  itr;
  ds_profile_list_info_type  info;
  ds_profile_info_type profile_info;
  char name[MAX_NAME_SIZ];

  memset( &lst, 0x0, sizeof(lst) );
  memset( &itr, 0x0, sizeof(itr) );

  ds_log_high("OBJECTIVE: Query 3GPP profiles from Modem\n" );

  /* Get list of 3GPP profiles */
  ds_log_low("INFO: Creating 3GPP list iterator\n" );
  lst.dfn = DS_PROFILE_LIST_ALL_PROFILES;
  ret = ds_profile_get_list_itr( DS_PROFILE_TECH_3GPP, &lst, &itr );
  if( DS_PROFILE_REG_RESULT_SUCCESS != ret ) {
    ds_log_err( "Failed on ds_profile_get_list_itr, status=%d\n", ret );
    ret = ds_profile_itr_destroy(itr);
    if( DS_PROFILE_REG_RESULT_SUCCESS != ret ) {
      ds_log_err( "Failed on ds_profile_itr_destroy, status=%d\n", ret);
      free( (void *)itr );
    }
    goto failure;
  }

  /* Report profiles */
  while( 1 ) {

    memset( &info, 0x0, sizeof(info) );
    memset( &profile_info, 0x0, sizeof(profile_info) );
    memset( name, 0x0, sizeof(name) );

    info.name = &profile_info;
    info.name->len  = sizeof(name);
    info.name->buf  = (void*)name;

    /* Retrieve the list item info */
    ds_log_low("INFO: Getting list item info\n" );
    ret = ds_profile_get_info_by_itr( itr, &info );
    if( DS_PROFILE_REG_RESULT_SUCCESS != ret ) {
      ds_log_err( "Failed on ds_profile_get_list_itr, status=%d\n", ret );
      ret = ds_profile_itr_destroy(itr);
      if( DS_PROFILE_REG_RESULT_SUCCESS != ret ) {
        ds_log_err( "Failed on ds_profile_itr_destroy, status=%d\n", ret);
        free( (void *)itr );
      }
      goto failure;
    }

    /* Summarize the info */
    ds_log_low("DATA: profile num=%d name.len=%d name.buf=%s\n",
               info.num, info.name->len, (char*)info.name->buf );

    /* Move to next item */
    ds_log_low("INFO: Advancing iterator\n" );
    ret = ds_profile_itr_next( itr );
    if( DS_PROFILE_REG_RESULT_SUCCESS != ret ) {
      /* Check for end of list */
      if( DS_PROFILE_REG_RESULT_LIST_END == ret ) {
        ds_log_low("INFO: End of list reached\n" );
        break;
      } else {
        ds_log_err( "Failed on ds_profile_get_list_itr, status=%d\n", ret );
        ret = ds_profile_itr_destroy(itr);
        if( DS_PROFILE_REG_RESULT_SUCCESS != ret ) {
          ds_log_err( "Failed on ds_profile_itr_destroy, status=%d\n", ret);
          free( (void *)itr );
        }
        goto failure;
      }
    }
  }

  /* Clean up list iterator */
  ds_log_low("INFO: Destroying list iterator\n" );
  ret = ds_profile_itr_destroy( itr );
  if( DS_PROFILE_REG_RESULT_SUCCESS != ret ) {
    ds_log_err( "Failed on ds_profile_itr_destroy, status=%d\n", ret );
    free( (void *)itr );
    goto failure;
  }

  ds_log_low("RESULT: Test successful\n" );
  return SUCCESS;

failure:
  ds_log_low("RESULT: Test failed\n" );
  return FAILURE;
}

int main()
{
  /* Test initialization */
  if( SUCCESS != test_setup() ) {
    return FAILURE;
  }

  /* Test body */
  if( SUCCESS != test_body() ) {
    return FAILURE;
  }

  /* Test wrapup */
  if( SUCCESS != test_teardown() ) {
    return FAILURE;
  }

  return SUCCESS;
}
