#ifndef _AUDCAL_DIAG_H_
#define _AUDCAL_DIAG_H_

/*============================================================================

FILE:       audcal_diag.h

DESCRIPTION: Contains device driver's diag API.

PUBLIC CLASSES:  Not Applicable

INITIALIZATION AND SEQUENCING REQUIREMENTS:  N/A

Copyright (c) 2010-2012 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#include "acdb_includes.h"

typedef int32_t (*audcal_diag_cmd)(uint32_t cmd, void* params, uint32_t size);

#define AUDCAL_DIAG_CMD_ACTIVE_DEV_QUERY         1
#define AUDCAL_DIAG_CMD_ACTIVE_DEV_INFO_QUERY    2
#define AUDCAL_DIAG_CMD_AC_QUERY                 3
#define AUDCAL_DIAG_CMD_AC_INFO_QUERY            4
#define AUDCAL_DIAG_CMD_AC_AS_ASSOCIATION_QUERY  5
#define AUDCAL_DIAG_CMD_AS_INFO_QUERY            6

#define AUDCAL_DIAG_AUD_CB_TYPE                  1
#define AUDCAL_DIAG_VOC_CB_TYPE                  2

/* AUDCAL_DIAG_CMD_ACTIVE_DEV_QUERY */
struct audcal_diag_active_dev_query
{
   uint32_t num_devs;  /* out param */
};

/* AUDCAL_DIAG_CMD_ACTIVE_DEV_INFO_QUERY */
struct audcal_diag_dev_info
{
   uint32_t dev_id;   				
   uint32_t sample_rate;
   uint32_t bits_per_sample;
   uint32_t port_id;
   bool_t is_virtual_dev;		/* is it a virtual device */
   uint16_t timing_mode;	    /* For virtual device only: FTRT/AV Timer */
};
struct audcal_diag_active_dev_info_query
{
   uint32_t num_entries;        /* in/out param */
   struct audcal_diag_dev_info* entries; /* out params. caller malloc */
};

/* AUDCAL_DIAG_CMD_AC_QUERY */
struct audcal_diag_ac_query
{
   uint32_t num_acs;  /* out param */
};

/* AUDCAL_DIAG_CMD_AC_INFO_QUERY */
struct audcal_diag_ac_info
{
   uint32_t dev_id;   
   uint32_t ac_handle;   				
   uint32_t copp_id;
   uint32_t ac_category;  /* the audio context category: 0 default context category */
   uint32_t ac_mode;      /* This flag determine if ac should drop sample or not */
   uint32_t num_as_handles;
};
struct audcal_diag_active_ac_info_query
{
   uint32_t num_entries;               /* in/out param */
   struct audcal_diag_ac_info* entries;   /* out parm. caller malloc */
};

/* AUDCAL_DIAG_CMD_AC_AS_ASSOCIATION_QUERY */
struct audcal_diag_as_ac_association_query
{
   uint32_t ac_handle;              /* in param */
   uint32_t num_as_handles;         /* in/out param */
   uint32_t* as_handles;            /* as handle */
};

/* AUDCAL_DIAG_CMD_AS_INFO_QUERY */
struct audcal_as_info {
   uint32_t    session_id;
   uint32_t    stream_id;
   uint32_t    op_code;
   uint32_t    app_type;
   uint32_t    rx_type;
   uint32_t    tx_type;
};
struct audcal_diag_as_query
{
   uint32_t as_handle;            /* input parameter */
   struct audcal_as_info as_info; /* out params */
};

#endif /* _AUDCAL_DIAG_H_ */

