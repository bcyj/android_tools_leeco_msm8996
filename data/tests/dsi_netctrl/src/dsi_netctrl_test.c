/******************************************************************************

                        D S I _ N E T C T R L _ T E S T . C

******************************************************************************/

/******************************************************************************

  @file    dsi_netctrl_test.c
  @brief   dsi_netctrl test

  DESCRIPTION
  Interactive test that allows making data calls using dsi_netctrl

  ---------------------------------------------------------------------------
  Copyright (c) 2010,2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id: //linux/pkgs/proprietary/data/main/source/test/dss/src/dss_test_1.c#1 $

when       who        what, where, why
--------   ---        -------------------------------------------------------
04/19/10   js         created

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "dsi_netctrl.h"

#define DSI_LOG_DEBUG(fmt, ...) \
  printf(fmt, __VA_ARGS__); \
  printf("\n")
#define DSI_LOG_ERROR(fmt, ...) \
  printf(fmt, __VA_ARGS__); \
  printf("\n")
#define DSI_LOG_FATAL(fmt, ...) \
  printf(fmt, __VA_ARGS__); \
  printf("\n")
#define DSI_LOG_VERBOSE(fmt, ...) \
  printf(fmt, __VA_ARGS__); \
  printf("\n")
#define DSI_LOG_INFO(fmt, ...) \
  printf(fmt, __VA_ARGS__); \
  printf("\n")

struct event_strings_s
{
  dsi_net_evt_t evt;
  char * str;
};

struct event_strings_s event_string_tbl[DSI_EVT_MAX] =
{
  { DSI_EVT_INVALID, "DSI_EVT_INVALID" },
  { DSI_EVT_NET_IS_CONN, "DSI_EVT_NET_IS_CONN" },
  { DSI_EVT_NET_NO_NET, "DSI_EVT_NET_NO_NET" },
  { DSI_EVT_PHYSLINK_DOWN_STATE, "DSI_EVT_PHYSLINK_DOWN_STATE" },
  { DSI_EVT_PHYSLINK_UP_STATE, "DSI_EVT_PHYSLINK_UP_STATE" },
  { DSI_EVT_NET_RECONFIGURED, "DSI_EVT_NET_RECONFIGURED" }
};

void test_net_ev_cb( dsi_hndl_t hndl, 
		     void * user_data, 
		     dsi_net_evt_t evt,
		     dsi_evt_payload_t *payload_ptr )
{
  int i;
  (void)hndl; (void)user_data; (void)payload_ptr;
  
  if (evt > DSI_EVT_INVALID && evt < DSI_EVT_MAX)
  {
    for (i=0; i<DSI_EVT_MAX; i++)
    {
      if (event_string_tbl[i].evt == evt)
        DSI_LOG_DEBUG("<<< callback received event [%s]", event_string_tbl[i].str);
    }
  }
}

/* demo apn */
char apn[] = "a";
/* demo apn 1 */
char apn_1[] = "b";

#define DSI_TECH_1 "UMTS"
#define DSI_MAP_TECH_1 DSI_RADIO_TECH_UMTS
#define DSI_IP_FAMILY_4 "IP"
#define DSI_TECH_2 "CDMA"
#define DSI_MAP_TECH_2 DSI_RADIO_TECH_CDMA
#define DSI_IP_FAMILY_6 "IP6"
#define DSI_TECH_3 "LTE"
#define DSI_MAP_TECH_3 DSI_RADIO_TECH_LTE
#define DSI_TECH_4 "DO"
#define DSI_MAP_TECH_4 DSI_RADIO_TECH_DO
#define DSI_TECH_5 "AUTOMATIC"
#define DSI_MAP_TECH_5 DSI_RADIO_TECH_UNKNOWN
#define DSI_TECH_5_1 "AUTOMATIC"
#define DSI_MAP_TECH_5 DSI_RADIO_TECH_UNKNOWN

dsi_hndl_t dsi_test_v4_tech_1_hndl = 0;
dsi_hndl_t dsi_test_v4_tech_2_hndl = 0;
dsi_hndl_t dsi_test_v4_tech_3_hndl = 0;
dsi_hndl_t dsi_test_v4_tech_4_hndl = 0;
dsi_hndl_t dsi_test_v4_tech_5_hndl = 0;
dsi_hndl_t dsi_test_v4_tech_5_hndl_1 = 0;
dsi_hndl_t dsi_test_v6_tech_1_hndl = 0;
dsi_hndl_t dsi_test_v6_tech_2_hndl = 0;
dsi_hndl_t dsi_test_v6_tech_3_hndl = 0;
dsi_hndl_t dsi_test_v6_tech_4_hndl = 0;
dsi_hndl_t dsi_test_v6_tech_5_hndl = 0;
#define MAX_MPDP  3
dsi_hndl_t dsi_net_hndl[MAX_MPDP] = {0,};


void dsi_test_netup_v4_tech_1(void)
{
  dsi_call_param_value_t param_info;

  if (dsi_test_v4_tech_1_hndl)
  {
    DSI_LOG_ERROR("%s","Test App Limitation: Call already up with IPV4_TECH1");
    return;
  }

  /* obtain data service handle */
  dsi_test_v4_tech_1_hndl = dsi_get_data_srvc_hndl(test_net_ev_cb, NULL);

  /* set data call param */
  param_info.buf_val = NULL;
  param_info.num_val = DSI_MAP_TECH_1;
  DSI_LOG_DEBUG("Setting tech to %s", DSI_TECH_1);
  dsi_set_data_call_param(dsi_test_v4_tech_1_hndl, DSI_CALL_INFO_TECH_PREF, &param_info);

  param_info.buf_val = apn;
  param_info.num_val = 1;
  DSI_LOG_DEBUG("Setting APN to %s", apn);
  dsi_set_data_call_param(dsi_test_v4_tech_1_hndl, DSI_CALL_INFO_APN_NAME, &param_info);

  param_info.buf_val = NULL;
  param_info.num_val = DSI_IP_VERSION_4;
  DSI_LOG_DEBUG("Setting family to %s", DSI_IP_FAMILY_4);
  dsi_set_data_call_param(dsi_test_v4_tech_1_hndl, DSI_CALL_INFO_IP_VERSION, &param_info);

  DSI_LOG_DEBUG("Attempting took bring call up on technology %s with IP family %s",
                DSI_TECH_1, DSI_IP_FAMILY_4);
  /* start data call */
  dsi_start_data_call(dsi_test_v4_tech_1_hndl);
}

void dsi_test_netdown_v4_tech_1(void)
{
  DSI_LOG_DEBUG("Attempting to bring down call on technology %s with IP family %s", 
                DSI_TECH_1, DSI_IP_FAMILY_4);
  /* start data call */
  dsi_rel_data_srvc_hndl(dsi_test_v4_tech_1_hndl);

  dsi_test_v4_tech_1_hndl = NULL;
}

void dsi_test_netup_v4_tech_2(void)
{
  dsi_call_param_value_t param_info;

  /* obtain data service handle */
  dsi_test_v4_tech_2_hndl = dsi_get_data_srvc_hndl(test_net_ev_cb, NULL);

  /* set data call param */
  param_info.buf_val = NULL;
  param_info.num_val = DSI_MAP_TECH_2;
  DSI_LOG_DEBUG("Setting tech to %s", DSI_TECH_2);
  dsi_set_data_call_param(dsi_test_v4_tech_2_hndl, DSI_CALL_INFO_TECH_PREF, &param_info);

  param_info.buf_val = NULL;
  param_info.num_val = DSI_AUTH_PREF_PAP_CHAP_BOTH_ALLOWED;
  DSI_LOG_DEBUG("%s","Setting auth pref to both allowed");
  dsi_set_data_call_param(dsi_test_v4_tech_2_hndl, DSI_CALL_INFO_AUTH_PREF, &param_info);

  DSI_LOG_DEBUG("Attempting to bring call up on technology %s with IP family %s", 
                DSI_TECH_2, DSI_IP_FAMILY_4);
  /* start data call */
  dsi_start_data_call(dsi_test_v4_tech_2_hndl);
}

void dsi_test_netdown_v4_tech_2(void)
{
  DSI_LOG_DEBUG("Attempting to bring down call on technology %s with IP family %s", 
                DSI_TECH_2, DSI_IP_FAMILY_4);
  /* stop data call */
  dsi_rel_data_srvc_hndl(dsi_test_v4_tech_2_hndl);

  dsi_test_v4_tech_2_hndl = NULL;
}

void dsi_test_netup_v4_tech_3(void)
{
  dsi_call_param_value_t param_info;

  /* obtain data service handle */
  dsi_test_v4_tech_3_hndl = dsi_get_data_srvc_hndl(test_net_ev_cb, NULL);

  param_info.buf_val = NULL;
  param_info.num_val = DSI_MAP_TECH_3;
  DSI_LOG_DEBUG("Setting tech to %s", DSI_TECH_3);
  dsi_set_data_call_param(dsi_test_v4_tech_3_hndl, DSI_CALL_INFO_TECH_PREF, &param_info);

  param_info.buf_val = apn;
  param_info.num_val = 1;
  DSI_LOG_DEBUG("Setting APN to %s", apn);
  dsi_set_data_call_param(dsi_test_v4_tech_3_hndl, DSI_CALL_INFO_APN_NAME, &param_info);

  /*
  param_info.buf_val = NULL;
  param_info.num_val = DSI_AUTH_PREF_PAP_CHAP_BOTH_ALLOWED;
  DSI_LOG_DEBUG("%s","Setting auth pref to both allowed");
  dsi_set_data_call_param(dsi_test_v4_tech_3_hndl, DSI_CALL_INFO_AUTH_PREF, &param_info);
  */

  DSI_LOG_DEBUG("Attempting to bring call up on technology %s with IP family %s", 
                DSI_TECH_3, DSI_IP_FAMILY_4);
  /* start data call */
  dsi_start_data_call(dsi_test_v4_tech_3_hndl);
}

void dsi_test_netdown_v4_tech_3(void)
{
  DSI_LOG_DEBUG("Attempting to bring down call on technology %s with IP family %s", 
                DSI_TECH_3, DSI_IP_FAMILY_4);
  /* stop data call */
  dsi_rel_data_srvc_hndl(dsi_test_v4_tech_3_hndl);

  dsi_test_v4_tech_3_hndl = NULL;
}

void dsi_test_netup_v4_tech_4(void)
{
  dsi_call_param_value_t param_info;

  /* obtain data service handle */
  dsi_test_v4_tech_4_hndl = dsi_get_data_srvc_hndl(test_net_ev_cb, NULL);


  param_info.buf_val = NULL;
  param_info.num_val = DSI_MAP_TECH_4;
  DSI_LOG_DEBUG("Setting tech to %s", DSI_TECH_4);
  dsi_set_data_call_param(dsi_test_v4_tech_4_hndl, DSI_CALL_INFO_TECH_PREF, &param_info);

  DSI_LOG_DEBUG("Attempting to bring call up on technology %s with IP family %s", 
                DSI_TECH_4, DSI_IP_FAMILY_4);
  /* start data call */
  dsi_start_data_call(dsi_test_v4_tech_4_hndl);
}

void dsi_test_netdown_v4_tech_4(void)
{
  DSI_LOG_DEBUG("Attempting to bring down call on technology %s with IP family %s", 
                DSI_TECH_4, DSI_IP_FAMILY_4);
  /* stop data call */
  dsi_rel_data_srvc_hndl(dsi_test_v4_tech_4_hndl);

  dsi_test_v4_tech_4_hndl = NULL;
}

void dsi_test_netup_v4_tech_5(void)
{
  /* obtain data service handle */
  dsi_test_v4_tech_5_hndl = dsi_get_data_srvc_hndl(test_net_ev_cb, NULL);

  DSI_LOG_DEBUG("Attempting to bring call up on technology %s with IP family %s", 
                DSI_TECH_5, DSI_IP_FAMILY_4);
  /* start data call */
  dsi_start_data_call(dsi_test_v4_tech_5_hndl);
}

void dsi_test_netdown_v4_tech_5(void)
{
  DSI_LOG_DEBUG("Attempting to bring down call on technology %s with IP family %s", 
                DSI_TECH_5, DSI_IP_FAMILY_4);
  /* stop data call */
  dsi_rel_data_srvc_hndl(dsi_test_v4_tech_5_hndl);

  dsi_test_v4_tech_5_hndl = NULL;
}

void dsi_test_netup_v4_tech_5_1(void)
{
  /* obtain data service handle */
  dsi_test_v4_tech_5_hndl_1 = dsi_get_data_srvc_hndl(test_net_ev_cb, NULL);

  DSI_LOG_DEBUG("Attempting to bring call up on technology %s with IP family %s", 
                DSI_TECH_5, DSI_IP_FAMILY_4);
  /* start data call */
  dsi_start_data_call(dsi_test_v4_tech_5_hndl_1);
}

void dsi_test_netdown_v4_tech_5_1(void)
{
  DSI_LOG_DEBUG("Attempting to bring down call on technology %s with IP family %s", 
                DSI_TECH_5, DSI_IP_FAMILY_4);
  /* stop data call */
  dsi_rel_data_srvc_hndl(dsi_test_v4_tech_5_hndl_1);

  dsi_test_v4_tech_5_hndl_1 = NULL;
}

void dsi_test_netup_v6_tech_1(void)
{
  dsi_call_param_value_t param_info;

  /* obtain data service handle */
  dsi_test_v6_tech_1_hndl = dsi_get_data_srvc_hndl(test_net_ev_cb, NULL);

  /* set data call param */
  param_info.buf_val = NULL;
  param_info.num_val = DSI_MAP_TECH_1;
  DSI_LOG_DEBUG("Setting tech to %s", DSI_TECH_1);
  dsi_set_data_call_param(dsi_test_v6_tech_1_hndl, DSI_CALL_INFO_TECH_PREF, &param_info);

  param_info.buf_val = NULL;
  param_info.num_val = DSI_IP_VERSION_6;
  DSI_LOG_DEBUG("Setting family to %s", DSI_IP_FAMILY_6);
  dsi_set_data_call_param(dsi_test_v6_tech_1_hndl, DSI_CALL_INFO_IP_VERSION, &param_info);

  /* start data call */
  DSI_LOG_DEBUG("Attempting to bring call up on technology %s with IP family %s", 
                DSI_TECH_1, DSI_IP_FAMILY_6);
  dsi_start_data_call(dsi_test_v6_tech_1_hndl);
}

void dsi_test_netdown_v6_tech_1(void)
{
  DSI_LOG_DEBUG("Attempting to bring down call on technology %s with IP family %s", 
                DSI_TECH_1, DSI_IP_FAMILY_6);
  /* stop data call */
  dsi_rel_data_srvc_hndl(dsi_test_v6_tech_1_hndl);

  dsi_test_v6_tech_1_hndl = NULL;
}

void dsi_test_netup_v6_tech_2(void)
{
  dsi_call_param_value_t param_info;

  /* obtain data service handle */
  dsi_test_v6_tech_2_hndl = dsi_get_data_srvc_hndl(test_net_ev_cb, NULL);

  /* set data call param */
  param_info.buf_val = NULL;
  param_info.num_val = DSI_MAP_TECH_2;
  DSI_LOG_DEBUG("Setting tech to %s", DSI_TECH_2);
  dsi_set_data_call_param(dsi_test_v6_tech_2_hndl, DSI_CALL_INFO_TECH_PREF, &param_info);

  param_info.buf_val = NULL;
  param_info.num_val = DSI_IP_VERSION_6;
  DSI_LOG_DEBUG("Setting family to %s", DSI_IP_FAMILY_6);
  dsi_set_data_call_param(dsi_test_v6_tech_2_hndl, DSI_CALL_INFO_IP_VERSION, &param_info);

  /* start data call */
  DSI_LOG_DEBUG("Attempting to bring call up on technology %s with IP family %s", 
                DSI_TECH_2, DSI_IP_FAMILY_6);
  dsi_start_data_call(dsi_test_v6_tech_2_hndl);
}

void dsi_test_netdown_v6_tech_2(void)
{
  DSI_LOG_DEBUG("Attempting to bring down call on technology %s with IP family %s", 
                DSI_TECH_2, DSI_IP_FAMILY_6);
  /* stop data call */
  dsi_rel_data_srvc_hndl(dsi_test_v6_tech_2_hndl);

  dsi_test_v6_tech_2_hndl = NULL;
}

void dsi_test_netup_v6_tech_3(void)
{
  dsi_call_param_value_t param_info;

  /* obtain data service handle */
  dsi_test_v6_tech_3_hndl = dsi_get_data_srvc_hndl(test_net_ev_cb, NULL);


  param_info.buf_val = NULL;
  param_info.num_val = DSI_MAP_TECH_3;
  DSI_LOG_DEBUG("Setting tech to %s", DSI_TECH_3);
  dsi_set_data_call_param(dsi_test_v6_tech_3_hndl, DSI_CALL_INFO_TECH_PREF, &param_info);

  param_info.buf_val = NULL;
  param_info.num_val = DSI_IP_VERSION_6;
  DSI_LOG_DEBUG("Setting family to %s", DSI_IP_FAMILY_6);
  dsi_set_data_call_param(dsi_test_v6_tech_3_hndl, DSI_CALL_INFO_IP_VERSION, &param_info);

  /* start data call */
  DSI_LOG_DEBUG("Attempting to bring call up on technology %s with IP family %s", 
                DSI_TECH_3, DSI_IP_FAMILY_6);
  dsi_start_data_call(dsi_test_v6_tech_3_hndl);
}

void dsi_test_netdown_v6_tech_3(void)
{
  DSI_LOG_DEBUG("Attempting to bring down call on technology %s with IP family %s", 
                DSI_TECH_3, DSI_IP_FAMILY_6);
  /* stop data call */
  dsi_rel_data_srvc_hndl(dsi_test_v6_tech_3_hndl);

  dsi_test_v6_tech_3_hndl = NULL;
}

void dsi_test_netup_v6_tech_4(void)
{
  dsi_call_param_value_t param_info;

  /* obtain data service handle */
  dsi_test_v6_tech_4_hndl = dsi_get_data_srvc_hndl(test_net_ev_cb, NULL);


  param_info.buf_val = NULL;
  param_info.num_val = DSI_MAP_TECH_4;
  DSI_LOG_DEBUG("Setting tech to %s", DSI_TECH_4);
  dsi_set_data_call_param(dsi_test_v6_tech_4_hndl, DSI_CALL_INFO_TECH_PREF, &param_info);

  param_info.buf_val = NULL;
  param_info.num_val = DSI_IP_VERSION_6;
  DSI_LOG_DEBUG("Setting family to %s", DSI_IP_FAMILY_6);
  dsi_set_data_call_param(dsi_test_v6_tech_4_hndl, DSI_CALL_INFO_IP_VERSION, &param_info);

  /* start data call */
  DSI_LOG_DEBUG("Attempting to bring call up on technology %s with IP family %s", 
                DSI_TECH_4, DSI_IP_FAMILY_6);
  dsi_start_data_call(dsi_test_v6_tech_4_hndl);
}

void dsi_test_netdown_v6_tech_4(void)
{
  DSI_LOG_DEBUG("Attempting to bring down call on technology %s with IP family %s", 
                DSI_TECH_4, DSI_IP_FAMILY_6);
  /* stop data call */
  dsi_rel_data_srvc_hndl(dsi_test_v6_tech_4_hndl);

  dsi_test_v6_tech_4_hndl = NULL;
}

void dsi_test_netup_v6_tech_5(void)
{
  dsi_call_param_value_t param_info;

  /* obtain data service handle */
  dsi_test_v6_tech_5_hndl = dsi_get_data_srvc_hndl(test_net_ev_cb, NULL);

  param_info.buf_val = NULL;
  param_info.num_val = DSI_IP_VERSION_6;
  DSI_LOG_DEBUG("Setting family to %s", DSI_IP_FAMILY_6);
  dsi_set_data_call_param(dsi_test_v6_tech_5_hndl, DSI_CALL_INFO_IP_VERSION, &param_info);

  /* start data call */
  DSI_LOG_DEBUG("Attempting to bring call up on technology %s with IP family %s", 
                DSI_TECH_5, DSI_IP_FAMILY_6);
  dsi_start_data_call(dsi_test_v6_tech_5_hndl);
}

void dsi_test_netdown_v6_tech_5(void)
{
  DSI_LOG_DEBUG("Attempting to bring down call on technology %s with IP family %s", 
                DSI_TECH_5, DSI_IP_FAMILY_6);
  /* stop data call */
  dsi_rel_data_srvc_hndl(dsi_test_v6_tech_5_hndl);

  dsi_test_v6_tech_5_hndl = NULL;
}

void dsi_test_netup_v4_tech_1_profile( unsigned int  uprofid )
{
  dsi_call_param_value_t param_info;
  unsigned int index = uprofid-1;
  
  if(index >= MAX_MPDP)
    return;

  if (dsi_net_hndl[index])
  {
    DSI_LOG_ERROR("%s","Test App Limitation: Call already up with IPV4_TECH1");
    return;
  }

  /* obtain data service handle */
  dsi_net_hndl[index] = dsi_get_data_srvc_hndl(test_net_ev_cb, NULL);

  /* set data call param */
  param_info.buf_val = NULL;
  param_info.num_val = DSI_MAP_TECH_5;
  DSI_LOG_DEBUG("Setting tech to %s", DSI_TECH_1);
  dsi_set_data_call_param(dsi_net_hndl[index], DSI_CALL_INFO_TECH_PREF, &param_info);

  param_info.num_val = (int)uprofid;
  DSI_LOG_DEBUG("Setting UMTS PROFILE to %d", param_info.num_val);
  dsi_set_data_call_param(dsi_net_hndl[index], DSI_CALL_INFO_UMTS_PROFILE_IDX, &param_info);
  
  DSI_LOG_DEBUG("Attempting to bring call up on technology %s with IP family %s using profile %d", 
                DSI_TECH_1, DSI_IP_FAMILY_4, param_info.num_val );
  /* start data call */
  dsi_start_data_call(dsi_net_hndl[index]);
}

void dsi_test_netdown_v4_tech_1_profile( unsigned int  uprofid )
{
  unsigned int index = uprofid-1;

  if(index >= MAX_MPDP)
    return;  

  DSI_LOG_DEBUG("Attempting to bring down call on technology %s with IP family %s using profile %d", 
                DSI_TECH_1, DSI_IP_FAMILY_4, uprofid );
  /* stop data call */
  dsi_rel_data_srvc_hndl( dsi_net_hndl[index] );

  dsi_net_hndl[index] = NULL;
}

void dsi_test_exit(void)
{
  int i;
  
  /* release data service */
  if(dsi_test_v4_tech_1_hndl)
    dsi_rel_data_srvc_hndl(dsi_test_v4_tech_1_hndl);
  if(dsi_test_v4_tech_2_hndl)
    dsi_rel_data_srvc_hndl(dsi_test_v4_tech_2_hndl);
  if(dsi_test_v6_tech_1_hndl)
    dsi_rel_data_srvc_hndl(dsi_test_v6_tech_1_hndl);
  if(dsi_test_v6_tech_2_hndl)
    dsi_rel_data_srvc_hndl(dsi_test_v6_tech_2_hndl);
  if(dsi_test_v4_tech_3_hndl)
    dsi_rel_data_srvc_hndl(dsi_test_v4_tech_3_hndl);
  if(dsi_test_v6_tech_3_hndl)
    dsi_rel_data_srvc_hndl(dsi_test_v6_tech_3_hndl);

  for( i=0; i<MAX_MPDP; i++ ) {
    if( dsi_net_hndl[i] ) {
      dsi_rel_data_srvc_hndl( dsi_net_hndl[i] );
    }
  }
}


void print_usage()
{
  DSI_LOG_VERBOSE("%s","==========================");
  DSI_LOG_VERBOSE("%s","MAIN MENU");
  DSI_LOG_VERBOSE("%s","0. EXIT");
  DSI_LOG_VERBOSE("a. NET UP (%s %s)", DSI_TECH_1, DSI_IP_FAMILY_4);
  DSI_LOG_VERBOSE("b. NET UP (%s %s)", DSI_TECH_2, DSI_IP_FAMILY_4);
  DSI_LOG_VERBOSE("c. NET UP (%s %s)", DSI_TECH_3, DSI_IP_FAMILY_4);
  DSI_LOG_VERBOSE("d. NET UP (%s %s)", DSI_TECH_4, DSI_IP_FAMILY_4);
  DSI_LOG_VERBOSE("e. NET UP (%s %s)", DSI_TECH_5, DSI_IP_FAMILY_4);
  DSI_LOG_VERBOSE("f. NET UP (%s %s)", DSI_TECH_5_1, DSI_IP_FAMILY_4);

  DSI_LOG_VERBOSE("g. NET UP (%s %s)", DSI_TECH_1, DSI_IP_FAMILY_6);
  DSI_LOG_VERBOSE("h. NET UP (%s %s)", DSI_TECH_2, DSI_IP_FAMILY_6);
  DSI_LOG_VERBOSE("i. NET UP (%s %s)", DSI_TECH_3, DSI_IP_FAMILY_6);
  DSI_LOG_VERBOSE("j. NET UP (%s %s)", DSI_TECH_4, DSI_IP_FAMILY_6);
  DSI_LOG_VERBOSE("k. NET UP (%s %s)", DSI_TECH_5, DSI_IP_FAMILY_6);

  DSI_LOG_VERBOSE("l. NET DOWN (%s %s)", DSI_TECH_1, DSI_IP_FAMILY_4);
  DSI_LOG_VERBOSE("m. NET DOWN (%s %s)", DSI_TECH_2, DSI_IP_FAMILY_4);
  DSI_LOG_VERBOSE("n. NET DOWN (%s %s)", DSI_TECH_3, DSI_IP_FAMILY_4);
  DSI_LOG_VERBOSE("o. NET DOWN (%s %s)", DSI_TECH_4, DSI_IP_FAMILY_4);
  DSI_LOG_VERBOSE("p. NET DOWN (%s %s)", DSI_TECH_5, DSI_IP_FAMILY_4);
  DSI_LOG_VERBOSE("q. NET DOWN (%s %s)", DSI_TECH_5_1, DSI_IP_FAMILY_4);

  DSI_LOG_VERBOSE("r. NET DOWN (%s %s)", DSI_TECH_1, DSI_IP_FAMILY_6);
  DSI_LOG_VERBOSE("s. NET DOWN (%s %s)", DSI_TECH_2, DSI_IP_FAMILY_6);
  DSI_LOG_VERBOSE("t. NET DOWN (%s %s)", DSI_TECH_3, DSI_IP_FAMILY_6);
  DSI_LOG_VERBOSE("u. NET DOWN (%s %s)", DSI_TECH_4, DSI_IP_FAMILY_6);
  DSI_LOG_VERBOSE("v. NET DOWN (%s %s)", DSI_TECH_5, DSI_IP_FAMILY_6);

  DSI_LOG_VERBOSE("1. NET UP (%s %s %d)", DSI_TECH_5, DSI_IP_FAMILY_4, 1);
  DSI_LOG_VERBOSE("2. NET UP (%s %s %d)", DSI_TECH_5, DSI_IP_FAMILY_4, 2);
  DSI_LOG_VERBOSE("3. NET UP (%s %s %d)", DSI_TECH_5, DSI_IP_FAMILY_4, 3);
  DSI_LOG_VERBOSE("5. NET DOWN (%s %s %d)", DSI_TECH_5, DSI_IP_FAMILY_4, 1);
  DSI_LOG_VERBOSE("6. NET DOWN (%s %s %d)", DSI_TECH_5, DSI_IP_FAMILY_4, 2);
  DSI_LOG_VERBOSE("7. NET DOWN (%s %s %d)", DSI_TECH_5, DSI_IP_FAMILY_4, 3);
  DSI_LOG_VERBOSE("%c. FOR THIS MESSAGE",'?');

  DSI_LOG_VERBOSE("%s","==========================");
}

int main()
{
  char ch;
  char _exit = 0;
  
  //DSI_LOG_DEBUG("%s","qcril_data_test: dsi_init()");
  /* initilize qcril library  */
  if(DSI_SUCCESS != dsi_init(DSI_MODE_GENERAL))
  {
    DSI_LOG_ERROR("%s","dsi_init failed !!");
    return -1;
  }

  print_usage();
  
  atexit(dsi_test_exit);

  do
  {
    ch = (char)getchar();

    switch(ch)
    {
    case 'a':
      dsi_test_netup_v4_tech_1();
      break;
    case 'b':
      dsi_test_netup_v4_tech_2();
      break;
    case 'c':
      dsi_test_netup_v4_tech_3();
      break;
    case 'd':
      dsi_test_netup_v4_tech_4();
      break;
    case 'e':
      dsi_test_netup_v4_tech_5();
      break;
    case 'f':
      dsi_test_netup_v4_tech_5_1();
      break;

    case 'g':
      dsi_test_netup_v6_tech_1();
      break;
    case 'h':
      dsi_test_netup_v6_tech_2();
      break;
    case 'i':
      dsi_test_netup_v6_tech_3();
      break;
    case 'j':
      dsi_test_netup_v6_tech_4();
      break;
    case 'k':
      dsi_test_netup_v6_tech_5();
      break;

    case 'l':
      dsi_test_netdown_v4_tech_1();
      break;
    case 'm':
      dsi_test_netdown_v4_tech_2();
      break;
    case 'n':
      dsi_test_netdown_v4_tech_3();
      break;
    case 'o':
      dsi_test_netdown_v4_tech_4();
      break;
    case 'p':
      dsi_test_netdown_v4_tech_5();
      break;
    case 'q':
      dsi_test_netdown_v4_tech_5_1();
      break;

    case 'r':
      dsi_test_netdown_v6_tech_1();
      break;
    case 's':
      dsi_test_netdown_v6_tech_2();
      break;
    case 't':
      dsi_test_netdown_v6_tech_3();
      break;
    case 'u':
      dsi_test_netdown_v6_tech_4();
      break;
    case 'v':
      dsi_test_netdown_v6_tech_5();
      break;

    case '1':
      dsi_test_netup_v4_tech_1_profile( 1 );
      break;
    case '2':
      dsi_test_netup_v4_tech_1_profile( 2 );
      break;
    case '3':
      dsi_test_netup_v4_tech_1_profile( 3 );
      break;
    case '5':
      dsi_test_netdown_v4_tech_1_profile( 1 );
      break;
    case '6':
      dsi_test_netdown_v4_tech_1_profile( 2 );
      break;
    case '7':
      dsi_test_netdown_v4_tech_1_profile( 3 );
      break;

    case '?':
      print_usage();
      break;

    case '0':
      _exit = 1;
      break;

    default:
      break;
    }
  } while (!_exit);

  return 0;
}
