/******************************************************************************
  @file    qcril_other.c
  @brief   qcril qmi - misc

  DESCRIPTION
    Handles RIL requests for common software functions an any other
    RIL function that doesn't fall in a different (more specific) category

  ---------------------------------------------------------------------------

  Copyright (c) 2009-2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <cutils/properties.h>

#ifdef QMI_RIL_UTF
#include <errno.h>
#endif

#include "qcrili.h"
#include "qcril_arb.h"
#include "qcril_reqlist.h"
#include "qcril_otheri.h"
#include "qcril_other.h"
#include "qcrilhook_oem.h"
#include "qcril_qmi_voice.h"
#include "voice_service_v02.h"
#include "network_access_service_v01.h"
#include "qcril_qmi_client.h"
#include "ip_multimedia_subsystem_presence_v01.h"
#include "radio_frequency_radiated_performance_enhancement_v01.h"
/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

/* the following enum and struct defines are copying from nv.h file */

typedef enum {
  NV_AUTO_ANSWER_I                               = 74,
  NV_PREF_VOICE_SO_I                             = 285,
  NV_ROAM_CNT_I                                  = 169,
  NV_AIR_CNT_I                                   = 168,
  NV_MIN1_I                                      = 32,
  NV_MIN2_I                                      = 33,
  NV_IMSI_MCC_I                                  = 176,
  NV_IMSI_11_12_I                                = 177,
  NV_IMSI_T_S1_I                                 = 262,
  NV_IMSI_T_S2_I                                 = 263,
  NV_IMSI_T_MCC_I                                = 264,
  NV_IMSI_T_11_12_I                              = 265,
  NV_IMSI_T_ADDR_NUM_I                           = 266,
  NV_PCDMACH_I                                   = 20,
  NV_SCDMACH_I                                   = 21,
  NV_HOME_SID_NID_I                              = 259,
  NV_DIR_NUMBER_I                                = 178,
  NV_SID_NID_I                                   = 38,
  NV_MOB_CAI_REV_I                               = 6,
  NV_NAME_NAM_I                                  = 43
} nv_items_enum_type;

/* Up to 2 MINs per NAM allowed */
#define  NV_MAX_MINS                                             2
/* Up to 20 home SID/NID pairs */
#define  NV_MAX_HOME_SID_NID                                    20
/* num digits in dir_number */
#define  NV_DIR_NUMB_SIZ                                        10
/* Max SID+NID */
#define  NV_MAX_SID_NID                                          1
/* With up to 12-letter names */
#define  NV_MAX_LTRS                                            12

#define QMI_RIL_SPC_TLV_NVWRITE                   "persist.radio.spc_tlv_nvwrite"

/* Type to specify auto answer rings and enable/disable. */
typedef PACKED struct PACKED_POST{
  /* TRUE if auto answer enabled */
  boolean                                          enable;
  /* Number of rings when to answer call */
  byte                                             rings;
} nv_auto_answer_type;

typedef PACKED struct PACKED_POST{
  /* NAM id 0-N */
  byte                                             nam;
  /* evrc_capability_enabled */
  boolean                                          evrc_capability_enabled;
  /* home_page_voice_so */
  word                                             home_page_voice_so;
  /* home_orig_voice_so */
  word                                             home_orig_voice_so;
  /* roam_orig_voice_so */
  word                                             roam_orig_voice_so;
} nv_pref_voice_so_type;

/* Air time counter */
typedef PACKED struct PACKED_POST{
  /* NAM id 0-N */
  byte                                             nam;
  /* count */
  dword                                            cnt;
} nv_call_cnt_type;

/* Type to hold MIN1p for 2 MINs along with the associated NAM id. */
/*
 * The number is 24 bits, per CAI section 2.3.1.
 */
typedef PACKED struct PACKED_POST{
  /* NAM id 0-N */
  byte                                             nam;
  /* MIN1 */
  dword                                            min1[NV_MAX_MINS];
} nv_min1_type;

/* Type to hold MIN2p for 2 MINs along with the associated NAM id. */
/*
 * The number is 10 bits, per CAI section 2.3.1.
 */
typedef PACKED struct PACKED_POST{
  /* NAM id 0-N */
  byte                                             nam;
  /* MIN2 */
  word                                             min2[NV_MAX_MINS];
} nv_min2_type;

/* Type to hold IMSI MCC , along with the associated NAM id. */
/*
 * The number is 24 bits
 */
typedef PACKED struct PACKED_POST{
  /* NAM id 0-N */
  byte                                             nam;
  /* imsi_mcc */
  word                                             imsi_mcc;
} nv_imsi_mcc_type;

/* Type to hold IMSI_11_12 for 4 MINs along with the associated NAM id */
/*
 * The number is 8 bits.
 */
typedef PACKED struct PACKED_POST{
  /* NAM id 0-N */
  byte                                             nam;
  /* imsi_11_12 */
  byte                                             imsi_11_12;
} nv_imsi_11_12_type;

/* Type to hold IMSI length along with associated NAM id */
/*
 * The number is 3 bits, per J-STD-008 section 2.3.1.
 */
typedef PACKED struct PACKED_POST{
  /* NAM id 0-N */
  byte                                             nam;
  /* Length of the IMSI for this NAM */
  byte                                             num;
} nv_imsi_addr_num_type;

/* Type to hold CDMA channel and associated NAM. */
/*
 * Value is 11 bits for Primary and Secondary channels,
 * per CAI section 6.1.1.
 */
typedef PACKED struct PACKED_POST{
  /* NAM id 0-N */
  byte                                             nam;
  /* A carrier channel number */
  word                                             channel_a;
  /* B carrier channel number */
  word                                             channel_b;
} nv_cdmach_type;

/* Type to hold SID+NID pairs. */
/*
 * The SID is 15 bits, per CAI 2.3.8, and the NID is 16 bits,
 * per CAI section 2.3.10.3.
 */
typedef PACKED struct PACKED_POST{
  /* 15 bits, per CAI 2.3.8 */
  word                                             sid;
  /* 16 bits, per CAI section 2.3.10.3 */
  word                                             nid;
} nv_sid_nid_pair_type;

/* Type to hold 'home' SID+NID pairs for CDMA acquisition */
/*
 * The type also holds NAM id. Note that this item is NOT
 * 'per-MIN'
 */
typedef PACKED struct PACKED_POST{
  /* NAM id 0-N */
  byte                                             nam;
  /* SID+NID pair */
  nv_sid_nid_pair_type                             pair[NV_MAX_HOME_SID_NID];
} nv_home_sid_nid_type;

/* Type to hold DIR_NUMBER with associated NAM id */
typedef PACKED struct PACKED_POST{
  /* NAM id 0-N */
  byte                                             nam;
  /* Directory Number */
  byte                                             dir_number[NV_DIR_NUMB_SIZ];
} nv_dir_number_type;

/* Type to hold SID+NID pairs for CDMA acquisition along with NAM id. */
/*
 * NID is 16 bits, per CAI section 2.3.10.3.  There are up to 4 SID+NID
 * pairs, in descending preferrence (0=first, 3=last).
 */
typedef PACKED struct PACKED_POST{
  /* NAM id 0-N */
  byte                                             nam;
  /* SID+NID Pair */
  nv_sid_nid_pair_type                             pair[NV_MAX_MINS][NV_MAX_SID_NID];
} nv_sid_nid_type;

/* Type to hold the name of each NAM, along with the associated NAM id */
typedef PACKED struct PACKED_POST{
  /* NAM id 0-N */
  byte                                             nam;
  /* NAM name string */
  byte                                             name[NV_MAX_LTRS];
} nv_name_nam_type;

typedef PACKED union PACKED_POST{
  nv_auto_answer_type                              auto_answer;
  nv_pref_voice_so_type                            pref_voice_so;
  nv_call_cnt_type                                 air_cnt;
  nv_call_cnt_type                                 roam_cnt;
  nv_min1_type                                     min1;
  nv_min2_type                                     min2;
  nv_imsi_mcc_type                                 imsi_mcc;
  nv_imsi_11_12_type                               imsi_11_12;
  nv_min1_type                                     imsi_t_s1;
  nv_min2_type                                     imsi_t_s2;
  nv_imsi_mcc_type                                 imsi_t_mcc;
  nv_imsi_11_12_type                               imsi_t_11_12;
  nv_imsi_addr_num_type                            imsi_t_addr_num;
  nv_cdmach_type                                   pcdmach;
  nv_cdmach_type                                   scdmach;
  nv_home_sid_nid_type                             home_sid_nid;
  nv_dir_number_type                               dir_number;
  nv_sid_nid_type                                  sid_nid;
  byte                                             mob_cai_rev;
  nv_name_nam_type                                 name_nam;
} nv_item_type;

/* finished copying enum and struct defines from nv.h file */






#define QCRIL_OTHER_NV_SO( item )       FSIZ( nv_item_type, item ), FPOS( nv_item_type, item )
#define QCRIL_OTHER_NUM_OF_NV_ITEMS     (sizeof( qcril_other_nv_table ) / sizeof( qcril_other_nv_table_entry_type ))

#define DEFAULT_NAM_ID 0xFF

/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

static qcril_other_struct_type *qcril_other;

static const qcril_other_nv_table_entry_type qcril_other_nv_table[] =
{
  { NV_AUTO_ANSWER_I,           QCRIL_OTHER_NV_SO( auto_answer ),         "NV_AUTO_ANSWER_I" },
  { NV_PREF_VOICE_SO_I,         QCRIL_OTHER_NV_SO( pref_voice_so ),       "NV_PREF_VOICE_SO_I" },
  { NV_ROAM_CNT_I,              QCRIL_OTHER_NV_SO( roam_cnt ),            "NV_ROAM_CNT_I" },
  { NV_AIR_CNT_I,               QCRIL_OTHER_NV_SO( air_cnt ),             "NV_AIR_CNT_I" },
  { NV_MIN1_I,                  QCRIL_OTHER_NV_SO( min1 ),                "NV_MIN1_I" },
  { NV_MIN2_I,                  QCRIL_OTHER_NV_SO( min2 ),                "NV_MIN2_I" },
  { NV_IMSI_11_12_I,            QCRIL_OTHER_NV_SO( imsi_11_12 ),          "NV_IMSI_11_12_I" },
  { NV_IMSI_MCC_I,              QCRIL_OTHER_NV_SO( imsi_mcc ),            "NV_IMSI_MCC_I" },
  { NV_IMSI_T_MCC_I,            QCRIL_OTHER_NV_SO( imsi_t_mcc ),          "NV_IMSI_T_MCC_I" },
  { NV_IMSI_T_11_12_I,          QCRIL_OTHER_NV_SO( imsi_t_11_12 ),        "NV_IMSI_T_11_12_I" },
  { NV_IMSI_T_S1_I,             QCRIL_OTHER_NV_SO( imsi_t_s1 ),           "NV_IMSI_T_S1_I" },
  { NV_IMSI_T_S2_I,             QCRIL_OTHER_NV_SO( imsi_t_s2 ),           "NV_IMSI_T_S2_I" },
  { NV_IMSI_T_ADDR_NUM_I,       QCRIL_OTHER_NV_SO( imsi_t_addr_num ),     "NV_IMSI_T_ADDR_NUM_I" },
  { NV_PCDMACH_I,               QCRIL_OTHER_NV_SO( pcdmach ),             "NV_PCDMACH_I" },
  { NV_SCDMACH_I,               QCRIL_OTHER_NV_SO( scdmach ),             "NV_SCDMACH_I" },
  { NV_HOME_SID_NID_I,          QCRIL_OTHER_NV_SO( home_sid_nid ),        "NV_HOME_SID_NID_I" },
  { NV_DIR_NUMBER_I,            QCRIL_OTHER_NV_SO( dir_number ),          "NV_DIR_NUMBER_I" },
  { NV_SID_NID_I,               QCRIL_OTHER_NV_SO( sid_nid ),             "NV_SID_NID_I" },
  { NV_MOB_CAI_REV_I,           QCRIL_OTHER_NV_SO( mob_cai_rev ),         "NV_MOB_CAI_REV_I" },
};


/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/

void qcril_other_request_oem_hook_neighboring_cells_info_cb
(
  qmi_client_type              user_handle,
  unsigned long                msg_id,
  void                         *resp_c_struct,
  int                          resp_c_struct_len,
  void                         *resp_cb_data,
  qmi_client_error_type        transp_err
);

void qmi_ril_get_property_value_helper(const char *property_name,
                                       char *property_value,
                                       const char *default_property_value);

errno_enum_type qmi_ril_set_property_value_helper(const char *property_name,
                                                  const char *property_value);

/*===========================================================================

  FUNCTION:  qcril_other_send_request_reponse

===========================================================================*/
/*!
    @brief
    internel use only. Wrap the response information and send it.

    @return
    None.
*/
/*=========================================================================*/
static void qcril_other_send_request_reponse
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  int request_id,
  RIL_Errno err_no,
  void *data,
  int data_size
)
{
   qcril_request_resp_params_type resp;
   qcril_default_request_resp_params(instance_id, t, request_id, err_no, &resp);
   if (NULL != data && data_size > 0 && RIL_E_SUCCESS == err_no)
   {
      resp.resp_pkt = data;
      resp.resp_len = data_size;
   }
   qcril_send_request_response(&resp);
}

/*===========================================================================

  FUNCTION:  qcril_other_get_3gpp2_subscription_info

===========================================================================*/
/*!
    @brief
    internel use only. Get 3gpp2 subscription info.

    @return
    error code.
    nas_get_3gpp2_subscription_info_resp_msg_v01 is returned back in get_config_resp_msg parameter if successful.
*/
/*=========================================================================*/
static int qcril_other_get_3gpp2_subscription_info
(
   nas_get_3gpp2_subscription_info_resp_msg_v01 *get_config_resp_msg,
   int nam_id
)
{
   nas_get_3gpp2_subscription_info_req_msg_v01 get_config_req_msg;
   int ret = RIL_E_SUCCESS;

   /* To start with, set all the optional fields to be invalid */
   memset(&get_config_req_msg, 0, sizeof(get_config_req_msg));
   memset(get_config_resp_msg, 0, sizeof(*get_config_resp_msg));
   get_config_req_msg.nam_id = nam_id;

   if (E_SUCCESS != qcril_qmi_client_send_msg_sync(QCRIL_QMI_CLIENT_NAS,
                                                   QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01,
                                                   &get_config_req_msg,
                                                   sizeof(get_config_req_msg),
                                                   get_config_resp_msg,
                                                   sizeof(*get_config_resp_msg)
                                                  )
      )
   {
      QCRIL_LOG_INFO("qcril_qmi_client_send_msg_sync returned error: %d", get_config_resp_msg->resp.error);
      ret = RIL_E_GENERIC_FAILURE;
   }
   return ret;
}

/*===========================================================================

  FUNCTION:  qcril_other_preset_min_based_info

===========================================================================*/
/*!
    @brief
    internel use only. set min_based_info to the current value.

    @return
    error code.
    current min value is set in min_based_info if the function call is successful.
*/
/*=========================================================================*/
static int qcril_other_preset_min_based_info
(
   nas_3gpp2_min_based_info_type_v01 *min_based_info
)
{
   nas_get_3gpp2_subscription_info_resp_msg_v01 get_config_resp_msg;
   int ret = qcril_other_get_3gpp2_subscription_info(&get_config_resp_msg, DEFAULT_NAM_ID);
   do
   {
      if (RIL_E_SUCCESS != ret)
      {
         break;
      }

      if (!get_config_resp_msg.min_based_info_valid)
      {
         QCRIL_LOG_INFO("min_based_info not available");
         ret = RIL_E_GENERIC_FAILURE;
      }

      memcpy(min_based_info, &get_config_resp_msg.min_based_info, sizeof(*min_based_info));
   }while(0);
   return ret;
}

/*===========================================================================

  FUNCTION:  qcril_other_preset_min_based_info

===========================================================================*/
/*!
    @brief
    internel use only. set true_imsi to the current value.

    @return
    error code.
    current true imsi value is set in true_imsi if the function call is successful.
*/
/*=========================================================================*/
static int qcril_other_preset_true_imsi_info
(
  nas_3gpp2_true_imsi_info_type_v01 *true_imsi
)
{
   nas_get_3gpp2_subscription_info_resp_msg_v01 get_config_resp_msg;
   int ret = qcril_other_get_3gpp2_subscription_info(&get_config_resp_msg, DEFAULT_NAM_ID);
   do
   {
      if (RIL_E_SUCCESS != ret)
      {
         break;
      }

      if (!get_config_resp_msg.true_imsi_valid)
      {
         QCRIL_LOG_INFO("true_imsi not available");
         ret = RIL_E_GENERIC_FAILURE;
      }

      memcpy(true_imsi, &get_config_resp_msg.true_imsi, sizeof(*true_imsi));
   }while(0);
   return ret;
}

/*===========================================================================

  FUNCTION:  qcril_other_preset_cdma_channel_info

===========================================================================*/
/*!
    @brief
    internel use only. set cdmach to the current value.

    @return
    error code.
    current cdma channel value is set in cdmach if the function call is successful.
*/
/*=========================================================================*/
static int qcril_other_preset_cdma_channel_info
(
  nas_cdma_channel_info_type_v01 *cdmach
)
{
   nas_get_3gpp2_subscription_info_resp_msg_v01 get_config_resp_msg;
   int ret = qcril_other_get_3gpp2_subscription_info(&get_config_resp_msg, DEFAULT_NAM_ID);
   do
   {
      if (RIL_E_SUCCESS != ret)
      {
         break;
      }

      if (!get_config_resp_msg.cdma_channel_info_valid)
      {
         QCRIL_LOG_INFO("cdma channel info not available");
         ret = RIL_E_GENERIC_FAILURE;
      }

      memcpy(cdmach, &get_config_resp_msg.cdma_channel_info, sizeof(*cdmach));
   }while(0);
   return ret;
}

/*===========================================================================

                                FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_other_ascii_to_int

===========================================================================*/
/*!
    @brief
    Convert a non NULL terminated string to an integer

    @return
    the integer value of the string.
*/
/*=========================================================================*/
int qcril_other_ascii_to_int(const char* str, int size)
{
  int ret=0;
  char *tmp = qcril_malloc(size+1);
  if(tmp != NULL)
  {
  memcpy(tmp, str, size);
  tmp[size] = 0;
  ret = atoi(tmp);
  qcril_free(tmp);
  }
  else
  {
      ret = 0;
      QCRIL_LOG_FATAL("CHECK FAILED");
  }
  return ret;
}

/*===========================================================================

  FUNCTION:  qcril_other_int_to_ascii

===========================================================================*/
/*!
    @brief
    Convert an integer value to a non NULL terminated string

    @return
    None.
*/
/*=========================================================================*/
void qcril_other_int_to_ascii(char* str, int size, int value)
{
  int i;
  for (i=size-1; i>=0; i--)
  {
    str[i] = value % 10 + '0';
    value /= 10;
  }
}

/*===========================================================================

  FUNCTION:  qcril_other_hex_to_int

===========================================================================*/
/*!
    @brief
    Convert a hexadecimal string to integer

    @return
    None.
*/
/*=========================================================================*/
int qcril_other_hex_to_int(char *hex_string,int *number)
{
    int iter_i=0;
    int temp_digit=0;
    int err=0;
    int len=0;

    if(hex_string && number)
    {
        len = strlen(hex_string);
        *number = 0;
        for(iter_i = 0; iter_i < len; iter_i++)
        {
            if( isdigit(hex_string[iter_i]) )
            {
                temp_digit = hex_string[iter_i]-'0';
            }
            else
            {
                switch( hex_string[iter_i] )
                {
                case 'a':
                case 'A':
                    temp_digit = 10;
                    break;
                case 'b':
                case 'B':
                    temp_digit = 11;
                    break;
                case 'c':
                case 'C':
                    temp_digit = 12;
                    break;
                case 'd':
                case 'D':
                    temp_digit = 13;
                    break;
                case 'e':
                case 'E':
                    temp_digit = 14;
                    break;
                case 'f':
                case 'F':
                    temp_digit = 15;
                    break;
                default:
                    QCRIL_LOG_INFO("Invalid hex character %d", hex_string[iter_i]);
                    err = -1;
                    break;
                }
            }
            if(0 != err)
            {
                *number = 0;
                break;
            }
            *number <<= 4;
            *number += temp_digit;
        }
    }
    else
    {
        QCRIL_LOG_INFO("Null Pointer");
        err = -1;
    }

    return err;
}

/*===========================================================================

  FUNCTION:  qcril_other_is_number_found

===========================================================================*/
/*!
    @brief
    Checks If number is found in the patterns. ',' is the delimiter of the patterns.
    '\0' and ':' can be used to terminate the patterns.


    @return
    Returns TRUE if number is found in the patterns
*/
/*=========================================================================*/
int qcril_other_is_number_found(char * number, char *patterns)
{
  int res = FALSE;

  char single_num[ PROPERTY_VALUE_MAX ];

  char * cur_p;
  char * cur_n;
  char cur_c;

  int prop_req_res;

  cur_p = patterns;
  cur_n = single_num;
  do
  {
    cur_c = *cur_p;

    switch ( cur_c )
    {
      case ',':
        cur_p++;    // fallthrourg

      case '\0':
      case ':' :
        *cur_n = 0;
        if ( 0 == strcmp( single_num, number ) )
        {
          res = TRUE;
        }
        else
        {
          cur_n = single_num;
        }
        break;

      default:
        *cur_n = cur_c;
        cur_n++;
        cur_p++;
        break;
    }

  } while ( !res && cur_c != '\0' && cur_c != ':' );

  QCRIL_LOG_INFO("qcril_other_is_number_found for %s completed with %d", number, (int) res);

  return res;
} // qcril_other_is_number_found

/*===========================================================================

  FUNCTION:  qcril_other_init

===========================================================================*/
/*!
    @brief
    Initialize the Other subsystem of the RIL.

    @return
    None.
*/
/*=========================================================================*/
void qcril_other_init( void )
{
  uint8 i;
  qcril_other_struct_type *i_ptr;

  /*-----------------------------------------------------------------------*/

  /* Allow cache */
  qcril_other = (qcril_other_struct_type *) qcril_arb_allocate_cache( QCRIL_ARB_CACHE_OTHER );
  QCRIL_ASSERT( qcril_other != NULL );

  /* Initialize internal data */
  for ( i = 0; i < QCRIL_MAX_INSTANCE_ID; i++ )
  {
    i_ptr = &qcril_other[ i ];

    /* Initialize uplink mute setting */
    i_ptr->uplink_mute_setting = QCRIL_OTHER_MUTE_ENABLED;

    /* Initialize status of NV items */
    i_ptr->curr_nam = 0;
  }

} /* qcril_other_init() */


/*===========================================================================

  FUNCTION:  qcril_other_mute

===========================================================================*/
/*!
    @brief
    Wrapper function for SND RPC call to set mute levels.

    @return
    None.
*/
/*=========================================================================*/
void qcril_other_mute
(
  qcril_instance_id_e_type instance_id,
  boolean mic_mute,
  boolean ear_mute
)
{
  qcril_other_struct_type *i_ptr;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  i_ptr = &qcril_other[ instance_id ];

  /*-----------------------------------------------------------------------*/

  ear_mute = ear_mute;
  /* Update uplink mute state */
  if ( mic_mute )
  {
    i_ptr->uplink_mute_setting = QCRIL_OTHER_MUTE_ENABLED;
  }
  else
  {
    i_ptr->uplink_mute_setting = QCRIL_OTHER_MUTE_DISABLED;
  }

} /* qcril_other_mute() */


/*===========================================================================

  FUNCTION:  qcril_other_request_set_mute

===========================================================================*/
/*!
    @brief

    Handles RIL_REQUEST_SET_MUTE. If not in call, we mute all channels.
    If in call, we leave earpiece unmuted, and mute or unmute mic as
    requested.

    @return
    None.
*/

/*=========================================================================*/
void qcril_other_request_set_mute
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id = QCRIL_DEFAULT_MODEM_ID;
  int mute_cmd;
  char *mute_state_str = NULL;
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry;

  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_NOTUSED( ret_ptr );

  do
  {
    if( params_ptr->datalen == 0 || params_ptr->data == NULL )
    {
      qcril_default_request_resp_params( instance_id, params_ptr->t,
                          params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
      qcril_send_request_response( &resp );
      break;
    }

    mute_cmd = * ( (int *) params_ptr->data );

    QCRIL_LOG_INFO("Handling %s (%d) Token ID (%d) - Mute value %d",
                    qcril_log_lookup_event_name( params_ptr->event_id ),
                    params_ptr->event_id,
                    qcril_log_get_token_id( params_ptr->t ),
                    mute_cmd );

    /* Add entry to ReqList */
    qcril_reqlist_default_entry(params_ptr->t, params_ptr->event_id,
                                modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                                QCRIL_EVT_NONE, NULL, &reqlist_entry );
    if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
    {
      /* Fail to add to ReqList */
      break;
    }

    if ( mute_cmd )
    {
      QCRIL_LOG_INFO( "In call - Muting mic" );
      mute_state_str = "Mic (Off), Ear (On)";
      qcril_other_mute( instance_id, TRUE, FALSE );
    }
    else
    {
      QCRIL_LOG_INFO( "In call - Unmuting mic" );
      mute_state_str = "Mic (On), Ear (On)";
      qcril_other_mute( instance_id, FALSE, FALSE );
    }

    qcril_default_request_resp_params( instance_id, params_ptr->t,
                                  params_ptr->event_id, RIL_E_SUCCESS, &resp );
    qcril_send_request_response( &resp );

  } while(0);

} /* qcril_other_request_set_mute() */


/*===========================================================================

  FUNCTION:  qcril_other_request_get_mute

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_GET_MUTE.

    @return
    None.
*/
/*=========================================================================*/
void qcril_other_request_get_mute
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id = QCRIL_DEFAULT_MODEM_ID;
  qcril_other_struct_type *i_ptr;
  int uplink_mute_setting;
  qcril_request_resp_params_type resp;
  qcril_reqlist_public_type reqlist_entry;

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  i_ptr = &qcril_other[ instance_id ];
  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  do
  {
  QCRIL_LOG_INFO( "Handling %s (%d) Token ID (%d)",
                  qcril_log_lookup_event_name( params_ptr->event_id ), params_ptr->event_id,
                  qcril_log_get_token_id( params_ptr->t ) );

  /* Add entry to ReqList */
  qcril_reqlist_default_entry( params_ptr->t, params_ptr->event_id, modem_id, QCRIL_REQ_AWAITING_CALLBACK,
                               QCRIL_EVT_NONE, NULL, &reqlist_entry );
  if ( qcril_reqlist_new( instance_id, &reqlist_entry ) != E_SUCCESS )
  {
    /* Fail to add to ReqList */
        break;
  }

  uplink_mute_setting = i_ptr->uplink_mute_setting;

  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, &resp );
  resp.resp_pkt = (void *) &uplink_mute_setting;
  resp.resp_len = sizeof( uplink_mute_setting );
  qcril_send_request_response( &resp );
  }while(0);

} /* qcril_other_request_get_mute() */


/*===========================================================================

  FUNCTION:  qcril_other_request_oem_hook_strings

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_OEM_HOOK_STRINGS.

    @return
    None.
*/
/*=========================================================================*/
void qcril_other_request_oem_hook_strings
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/


  instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  QCRIL_NOTUSED( ret_ptr );

  /*-----------------------------------------------------------------------*/

  qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_REQUEST_NOT_SUPPORTED, &resp );
  qcril_send_request_response( & resp );

} /* qcril_other_request_oem_hook_strings() */

/*=========================================================================

  FUNCTION:  qcril_other_request_oem_hook_nv_read

===========================================================================*/
/*!
    @brief
    Reads the request NAM parameter from NV.

    @return
    If NV read is success then the corresponding NV item value is returned
    void
*/
/*=========================================================================*/
void qcril_other_request_oem_hook_nv_read
(
 const qcril_request_params_type *const params_ptr,
 qcril_request_return_type *const ret_ptr /*!< Output parameter */
 )
{
   qcril_instance_id_e_type instance_id;
   char *data_ptr;
   qcril_request_resp_params_type resp;

   nv_item_type nv_item;
   uint32 nv_item_id = 0, nv_item_len = 0, index, nvIdx;

/*-----------------------------------------------------------------------*/


   instance_id = QCRIL_DEFAULT_INSTANCE_ID;

   data_ptr = ( char *)params_ptr->data;


/*-----------------------------------------------------------------------*/

   QCRIL_LOG_FUNC_ENTRY();
   do
   {
      if( params_ptr->datalen == 0 || params_ptr->data == NULL )
      {
         qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
         qcril_send_request_response( &resp );
         break;
      }

      /* decode the NV item from the raw stream, data[0--3], 4 bytes */
      memcpy(&nv_item_id, data_ptr, QCRIL_OTHER_OEM_ITEMID_LEN);

      QCRIL_LOG_INFO("Received request for Reading nv_item_id = %lu", nv_item_id);

      /* Get the index of NV item data item */
      for (index = 0; index < QCRIL_OTHER_NUM_OF_NV_ITEMS; index++)
      {
         if(qcril_other_nv_table[index].nv_item == nv_item_id)
         {
            break;
         }
      }

      /* Check if the requested NV item was found in the nv table */
      if (QCRIL_OTHER_NUM_OF_NV_ITEMS == index)
      {
         QCRIL_LOG_INFO("Requested NV item not found = %lu", nv_item_id);
         qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                       RIL_E_GENERIC_FAILURE, NULL, 0);
         break;
      }

      /* dispatch nv read request processing */
      switch(nv_item_id)
      {
      case NV_AUTO_ANSWER_I:
      case NV_PREF_VOICE_SO_I:
      case NV_ROAM_CNT_I:
      case NV_AIR_CNT_I:
         {
            voice_get_config_req_msg_v02 get_config_req_msg;
            voice_get_config_resp_msg_v02 get_config_resp_msg;

            /* To start with, set all the optional fields to be invalid */
            memset(&get_config_req_msg, 0, sizeof(get_config_req_msg));
            memset(&get_config_resp_msg, 0, sizeof(get_config_resp_msg));

            if (NV_AUTO_ANSWER_I == nv_item_id)
            {
               get_config_req_msg.auto_answer_valid = TRUE;
               get_config_req_msg.auto_answer = 1;
            }
            else if (NV_PREF_VOICE_SO_I == nv_item_id)
            {
               get_config_req_msg.pref_voice_so_valid = TRUE;
               get_config_req_msg.pref_voice_so = 1;
            }
            else if (NV_ROAM_CNT_I == nv_item_id)
            {
               get_config_req_msg.roam_timer_valid = TRUE;
               get_config_req_msg.roam_timer = 1;
            }
            else
            {
               /* case NV_AIR_CNT_I */
               get_config_req_msg.air_timer_valid = TRUE;
               get_config_req_msg.air_timer = 1;
            }

            if (E_SUCCESS !=
                qcril_qmi_client_send_msg_sync(QCRIL_QMI_CLIENT_VOICE,
                                               QMI_VOICE_GET_CONFIG_REQ_V02,
                                               &get_config_req_msg,
                                               sizeof(get_config_req_msg),
                                               &get_config_resp_msg,
                                               sizeof(get_config_resp_msg)
                                               )
                )
            {
               QCRIL_LOG_INFO("qcril_qmi_client_send_msg_sync returned error: %d", get_config_resp_msg.resp.error);
               qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                RIL_E_GENERIC_FAILURE, NULL, 0);
               break;
            }

            /* check response status */
            if(QMI_RESULT_SUCCESS_V01 != get_config_resp_msg.resp.result ||
               ( (NV_AUTO_ANSWER_I == nv_item_id) && !(get_config_resp_msg.auto_answer_status_valid)) ||
               ( (NV_PREF_VOICE_SO_I == nv_item_id) && !(get_config_resp_msg.current_preferred_voice_so_valid)) ||
               ( (NV_ROAM_CNT_I == nv_item_id) && !(get_config_resp_msg.roam_timer_count_valid)) ||
               ( (NV_AIR_CNT_I == nv_item_id) && !(get_config_resp_msg.air_timer_count_valid))
              )
            {
               QCRIL_LOG_INFO("Response is invalid");
               qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                RIL_E_GENERIC_FAILURE, NULL, 0);
               break;
            }

            if (NV_AUTO_ANSWER_I == nv_item_id)
            {
               /* fill the data with response */
               nv_item.auto_answer.enable = get_config_resp_msg.auto_answer_status;
               /* TBD qmi voice does not support this field, we hard code 3 here as tmp workaround */
               nv_item.auto_answer.rings = 3;
               QCRIL_LOG_INFO("nv_item.auto_answer.enable: %d", nv_item.auto_answer.enable);
            }
            else if (NV_PREF_VOICE_SO_I == nv_item_id)
            {
               nv_item.pref_voice_so.nam = get_config_resp_msg.current_preferred_voice_so.nam_id;
               nv_item.pref_voice_so.evrc_capability_enabled = get_config_resp_msg.current_preferred_voice_so.evrc_capability;
               nv_item.pref_voice_so.home_page_voice_so = get_config_resp_msg.current_preferred_voice_so.home_page_voice_so;
               nv_item.pref_voice_so.home_orig_voice_so = get_config_resp_msg.current_preferred_voice_so.home_orig_voice_so;
               nv_item.pref_voice_so.roam_orig_voice_so = get_config_resp_msg.current_preferred_voice_so.roam_orig_voice_so;
               QCRIL_LOG_INFO("nv_item.pref_voice_so.nam: %d", nv_item.pref_voice_so.nam);
               QCRIL_LOG_INFO("nv_item.pref_voice_so.evrc_capability_enabled: %d", nv_item.pref_voice_so.evrc_capability_enabled);
               QCRIL_LOG_INFO("nv_item.pref_voice_so.home_page_voice_so: %d", nv_item.pref_voice_so.home_page_voice_so);
               QCRIL_LOG_INFO("nv_item.pref_voice_so.home_orig_voice_so: %d", nv_item.pref_voice_so.home_orig_voice_so);
               QCRIL_LOG_INFO("nv_item.pref_voice_so.roam_orig_voice_so: %d", nv_item.pref_voice_so.roam_orig_voice_so);
            }
            else if (NV_ROAM_CNT_I == nv_item_id)
            {
               nv_item.roam_cnt.nam = get_config_resp_msg.current_preferred_voice_so.nam_id;
               nv_item.roam_cnt.cnt = get_config_resp_msg.roam_timer_count.roam_timer;
               QCRIL_LOG_INFO("nv_item.roam_cnt.cnt: %d", nv_item.roam_cnt.cnt);
            }
            else
            {
               /* case NV_AIR_CNT_I */
               nv_item.air_cnt.nam = get_config_resp_msg.current_preferred_voice_so.nam_id;
               nv_item.air_cnt.cnt = get_config_resp_msg.air_timer_count.air_timer;
               QCRIL_LOG_INFO("nv_item.air_cnt.cnt: %d", nv_item.air_cnt.cnt);
            }
            qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                             RIL_E_SUCCESS, (void *) &nv_item, qcril_other_nv_table[index].nv_item_size);
         }
         break;
      case NV_MIN1_I:
      case NV_MIN2_I:
      case NV_IMSI_11_12_I:
      case NV_IMSI_MCC_I:          /* NV_MIN1_I to NV_IMSI_MCC_I are retrieved from nas_3gpp2_min_based_info_type_v01*/
      case NV_IMSI_T_MCC_I:
      case NV_IMSI_T_11_12_I:
      case NV_IMSI_T_S1_I:
      case NV_IMSI_T_S2_I:
      case NV_IMSI_T_ADDR_NUM_I:   /* NV_IMSI_T_MCC_I to NV_IMSI_T_ADDR_NUM_I are retrieved from nas_3gpp2_true_imsi_info_type_v01*/
      case NV_PCDMACH_I:
      case NV_SCDMACH_I:           /* NV_PCDMACH_I and NV_SCDMACH_I are retrieved from nas_cdma_channel_info_type_v01*/
      case NV_HOME_SID_NID_I:      /* retrieved from nas_3gpp2_home_sid_nid_info_type_v01*/
      case NV_DIR_NUMBER_I:        /* retrieved from dir_num*/
      case NV_NAME_NAM_I:          /* retrieved from nam_name*/
         {
            nas_get_3gpp2_subscription_info_req_msg_v01 get_config_req_msg;
            nas_get_3gpp2_subscription_info_resp_msg_v01 get_config_resp_msg;

            /* To start with, set all the optional fields to be invalid */
            memset(&get_config_req_msg, 0, sizeof(get_config_req_msg));
            memset(&get_config_resp_msg, 0, sizeof(get_config_resp_msg));
            get_config_req_msg.nam_id = DEFAULT_NAM_ID; // current NAM

            if (E_SUCCESS !=
               qcril_qmi_client_send_msg_sync(QCRIL_QMI_CLIENT_NAS,
                                              QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01,
                                              &get_config_req_msg,
                                              sizeof(get_config_req_msg),
                                              &get_config_resp_msg,
                                              sizeof(get_config_resp_msg)
                                              )
               )
            {
               QCRIL_LOG_INFO("qcril_qmi_client_send_msg_sync returned error: %d", get_config_resp_msg.resp.error);
               qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                 RIL_E_GENERIC_FAILURE, NULL, 0);
               break;
            }

            /* check response status */
            if(QMI_RESULT_SUCCESS_V01 != get_config_resp_msg.resp.result ||
              ( ((NV_MIN1_I == nv_item_id) ||
                 (NV_MIN2_I == nv_item_id) ||
                 (NV_IMSI_11_12_I == nv_item_id) ||
                 (NV_IMSI_MCC_I == nv_item_id)) && !(get_config_resp_msg.min_based_info_valid)) ||
              ( ((NV_IMSI_T_MCC_I == nv_item_id) ||
                 (NV_IMSI_T_11_12_I == nv_item_id) ||
                 (NV_IMSI_T_S1_I == nv_item_id) ||
                 (NV_IMSI_T_S2_I == nv_item_id) ||
                 (NV_IMSI_T_ADDR_NUM_I == nv_item_id)) && !(get_config_resp_msg.true_imsi_valid)) ||
              ( ((NV_PCDMACH_I == nv_item_id) ||
                 (NV_SCDMACH_I == nv_item_id)) && !(get_config_resp_msg.cdma_channel_info_valid)) ||
              ( (NV_HOME_SID_NID_I == nv_item_id) && !(get_config_resp_msg.cdma_sys_id_valid)) ||
              ( (NV_DIR_NUMBER_I == nv_item_id) && !(get_config_resp_msg.dir_num_valid)) ||
              ( (NV_NAME_NAM_I == nv_item_id) && !(get_config_resp_msg.nam_name_valid))
              )
            {
               QCRIL_LOG_INFO("Response is invalid");
               qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                RIL_E_GENERIC_FAILURE, NULL, 0);
               break;
            }

            if (NV_MIN1_I == nv_item_id)
            {
               /* fill the data with response */
               nv_item.min1.nam = get_config_req_msg.nam_id;
               nv_item.min1.min1[1] = qcril_other_ascii_to_int(get_config_resp_msg.min_based_info.imsi_m_s1, NAS_IMSI_MIN1_LEN_V01);
               QCRIL_LOG_INFO("min1_val: %d", nv_item.min1.min1[1]);
            }
            else if (NV_MIN2_I == nv_item_id)
            {
               /* fill the data with response */
               nv_item.min1.nam = get_config_req_msg.nam_id;
               nv_item.min2.min2[1] = qcril_other_ascii_to_int(get_config_resp_msg.min_based_info.imsi_m_s2, NAS_IMSI_MIN2_LEN_V01);
               QCRIL_LOG_INFO("min2_val: %d", nv_item.min2.min2[1]);
            }
            else if (NV_IMSI_11_12_I == nv_item_id)
            {
               /* fill the data with response */
               nv_item.imsi_11_12.nam = get_config_req_msg.nam_id;
               nv_item.imsi_11_12.imsi_11_12 = qcril_other_ascii_to_int(get_config_resp_msg.min_based_info.imsi_m_11_12, NAS_IMSI_11_12_LEN_V01);
               QCRIL_LOG_INFO("imsi_11_12 value: %d", nv_item.imsi_11_12.imsi_11_12);
            }
            else if (NV_IMSI_MCC_I == nv_item_id)
            {
               /* fill the data with response */
               nv_item.imsi_mcc.nam = get_config_req_msg.nam_id;
               nv_item.imsi_mcc.imsi_mcc = qcril_other_ascii_to_int(get_config_resp_msg.min_based_info.mcc_m, NAS_MCC_LEN_V01);
               QCRIL_LOG_INFO("imsi_mcc value: %d", nv_item.imsi_mcc.imsi_mcc);
            }
            else if (NV_IMSI_T_MCC_I == nv_item_id)
            {
               /* fill the data with response */
               nv_item.imsi_t_mcc.nam = get_config_req_msg.nam_id;
               nv_item.imsi_t_mcc.imsi_mcc = qcril_other_ascii_to_int(get_config_resp_msg.true_imsi.mcc_t, NAS_MCC_LEN_V01);
               QCRIL_LOG_INFO("imsi_t_mcc value: %d", nv_item.imsi_t_mcc.imsi_mcc);
            }
            else if (NV_IMSI_T_11_12_I == nv_item_id)
            {
               /* fill the data with response */
               nv_item.imsi_t_11_12.nam = get_config_req_msg.nam_id;
               nv_item.imsi_t_11_12.imsi_11_12 = qcril_other_ascii_to_int(get_config_resp_msg.true_imsi.imsi_t_11_12, NAS_IMSI_11_12_LEN_V01);
               QCRIL_LOG_INFO("imsi_t_11_12 value: %d", nv_item.imsi_t_11_12.imsi_11_12);
            }
            else if (NV_IMSI_T_S1_I == nv_item_id)
            {
               /* fill the data with response */
               nv_item.imsi_t_s1.nam = get_config_req_msg.nam_id;
               nv_item.imsi_t_s1.min1[1] = qcril_other_ascii_to_int(get_config_resp_msg.true_imsi.imsi_t_s1, NAS_IMSI_MIN1_LEN_V01);
               QCRIL_LOG_INFO("imsi_t_s1: %d", nv_item.imsi_t_s1.min1[1]);
            }
            else if (NV_IMSI_T_S2_I == nv_item_id)
            {
               /* fill the data with response */
               nv_item.imsi_t_s2.nam = get_config_req_msg.nam_id;
               nv_item.imsi_t_s2.min2[1] = qcril_other_ascii_to_int(get_config_resp_msg.true_imsi.imsi_t_s2, NAS_IMSI_MIN2_LEN_V01);
               QCRIL_LOG_INFO("imsi_t_s2: %d", nv_item.imsi_t_s2.min2[1]);
            }
            else if (NV_IMSI_T_ADDR_NUM_I == nv_item_id)
            {
                /* fill the data with response */
               nv_item.imsi_t_addr_num.nam = get_config_req_msg.nam_id;
               nv_item.imsi_t_addr_num.num = get_config_resp_msg.true_imsi.imsi_t_addr_num;
               QCRIL_LOG_INFO("imsi_t_addr_num value: %d", nv_item.imsi_t_addr_num.num);
            }
            else if (NV_PCDMACH_I == nv_item_id)
            {
               nv_item.pcdmach.nam = get_config_req_msg.nam_id;
               nv_item.pcdmach.channel_a = get_config_resp_msg.cdma_channel_info.pri_ch_a;
               nv_item.pcdmach.channel_b = get_config_resp_msg.cdma_channel_info.pri_ch_b;
               QCRIL_LOG_INFO("pcdmach value: ch_a: %d, ch_b: %d", nv_item.pcdmach.channel_a, nv_item.pcdmach.channel_b);
            }
            else if (NV_SCDMACH_I == nv_item_id)
            {
               nv_item.scdmach.nam = get_config_req_msg.nam_id;
               nv_item.scdmach.channel_a = get_config_resp_msg.cdma_channel_info.sec_ch_a;
               nv_item.scdmach.channel_b = get_config_resp_msg.cdma_channel_info.sec_ch_b;
               QCRIL_LOG_INFO("scdmach value: ch_a: %d, ch_b: %d", nv_item.scdmach.channel_a, nv_item.scdmach.channel_b);
            }
            else if (NV_HOME_SID_NID_I == nv_item_id)
            {
               nv_item.home_sid_nid.nam = get_config_req_msg.nam_id;
               uint32_t i;
               for (i=0; i<get_config_resp_msg.cdma_sys_id_len; i++)
               {
                  nv_item.home_sid_nid.pair[i].sid = get_config_resp_msg.cdma_sys_id[i].sid;
                  nv_item.home_sid_nid.pair[i].nid = get_config_resp_msg.cdma_sys_id[i].nid;
               }
            }
            else if (NV_DIR_NUMBER_I == nv_item_id)
            {
               nv_item.dir_number.nam = get_config_req_msg.nam_id;
               memcpy(nv_item.dir_number.dir_number, get_config_resp_msg.dir_num, get_config_resp_msg.dir_num_len);
            }
            else
            {
               /* case NV_NAME_NAM_I */
               nv_item.name_nam.nam = get_config_req_msg.nam_id;
               memcpy(nv_item.name_nam.name, get_config_resp_msg.nam_name, get_config_resp_msg.nam_name_len);
            }

            qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                             RIL_E_SUCCESS, (void *) &nv_item, qcril_other_nv_table[index].nv_item_size);
         }
         break;
      case NV_SID_NID_I:
         {
            nas_get_home_network_resp_msg_v01 get_config_resp_msg;

            /* To start with, set all the optional fields to be invalid */
            memset(&get_config_resp_msg, 0, sizeof(get_config_resp_msg));

            if (E_SUCCESS !=
                qcril_qmi_client_send_msg_sync(QCRIL_QMI_CLIENT_NAS,
                                               QMI_NAS_GET_HOME_NETWORK_REQ_MSG_V01,
                                               NULL,
                                               0,
                                               &get_config_resp_msg,
                                               sizeof(get_config_resp_msg)
                                               )
                )
            {
               QCRIL_LOG_INFO("qcril_qmi_client_send_msg_sync returned error: %d", get_config_resp_msg.resp.error);
               qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                RIL_E_GENERIC_FAILURE,NULL, 0);
               break;
            }

            /* check response status */
            if(QMI_RESULT_SUCCESS_V01 != get_config_resp_msg.resp.result ||
               !(get_config_resp_msg.home_system_id_valid)
              )
            {
               QCRIL_LOG_INFO("Response is invalid");
               qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                RIL_E_GENERIC_FAILURE, NULL, 0);
               break;
            }

            /* fill the data with response */
            nv_item.sid_nid.pair[1][0].sid = get_config_resp_msg.home_system_id.sid;
            nv_item.sid_nid.pair[1][0].nid = get_config_resp_msg.home_system_id.nid;
            QCRIL_LOG_INFO("sid val: %d; nid val: %d", nv_item.sid_nid.pair[1][0].sid, nv_item.sid_nid.pair[1][0].nid);

            qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                             RIL_E_SUCCESS, (void *) &nv_item, qcril_other_nv_table[index].nv_item_size);
         }
         break;
      case NV_MOB_CAI_REV_I:
         {
            nas_get_mob_cai_rev_resp_v01 get_config_resp_msg;

            /* To start with, set all the optional fields to be invalid */
            memset(&get_config_resp_msg, 0, sizeof(get_config_resp_msg));

            if (E_SUCCESS !=
                qcril_qmi_client_send_msg_sync(QCRIL_QMI_CLIENT_NAS,
                                               QMI_NAS_GET_MOB_CAI_REV_REQ_MSG_V01,
                                               NULL,
                                               0,
                                               &get_config_resp_msg,
                                               sizeof(get_config_resp_msg)
                                               )
                )
            {
               QCRIL_LOG_INFO("qcril_qmi_client_send_msg_sync returned error: %d", get_config_resp_msg.resp.error);
               qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                RIL_E_GENERIC_FAILURE, NULL, 0);
               break;
            }

            /* check response status */
            if(QMI_RESULT_SUCCESS_V01 != get_config_resp_msg.resp.result ||
               !(get_config_resp_msg.cai_rev_valid))
            {
               QCRIL_LOG_INFO("Response is invalid");
               qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                RIL_E_GENERIC_FAILURE, NULL, 0);
               break;
            }

            /* fill the data with response */
            nv_item.mob_cai_rev = get_config_resp_msg.cai_rev;
            QCRIL_LOG_INFO("nv_item.mob_cai_rev val: %d", nv_item.mob_cai_rev);

            qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                             RIL_E_SUCCESS, (void *) &nv_item, qcril_other_nv_table[index].nv_item_size);
         }
         break;
      default:
         QCRIL_LOG_INFO("Requested NV item not supported = %lu", nv_item_id);
         qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                          RIL_E_GENERIC_FAILURE, NULL, 0);
         break;
      }
   }while(0);

} /* qcril_other_request_oem_hook_nv_read */

/*=========================================================================

  FUNCTION:  qcril_other_request_oem_hook_nv_write

===========================================================================*/
/*!
    @brief
    Writes the requested NAM parameter to NV item after validating the data.

    @return
    Void
    NV Write status is returned back in nv_write_status parameter.
*/
/*=========================================================================*/
void qcril_other_request_oem_hook_nv_write
(
 const qcril_request_params_type *const params_ptr,
 qcril_request_return_type *const ret_ptr /*!< Output parameter */
 )
{
   qcril_instance_id_e_type instance_id;
   char *pcData;
   qcril_request_resp_params_type resp;
   qmi_client_error_type qmi_client_error;
   RIL_Errno   ril_req_res = RIL_E_SUCCESS;

   nv_item_type nv_item;
   uint32 nv_item_id = 0, nv_item_len = 0, index, nvIdx;

   int spc_tlv_valid = TRUE;

/*-----------------------------------------------------------------------*/


   instance_id = QCRIL_DEFAULT_INSTANCE_ID;

   pcData = (char *) params_ptr->data;


/*-----------------------------------------------------------------------*/

   do
   {
      if( params_ptr->datalen == 0 || params_ptr->data == NULL )
      {
         qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, &resp );
         qcril_send_request_response( &resp );
         break;
      }

      QCRIL_LOG_FUNC_ENTRY();
      /* Initializing the data which is not yet initialized */
      memset(&nv_item, 0, sizeof(nv_item));

      /* decode the NV item id from the raw stream, data[0--3], 4 bytes */
      index = 0;
      memcpy(&nv_item_id, &(pcData[index]), QCRIL_OTHER_OEM_ITEMID_LEN);
      index += QCRIL_OTHER_OEM_ITEMID_LEN;

      /* Decode the NV item size from the raw stream, data[4--7], 4 bytes */
      memcpy(&nv_item_len, &(pcData[index]), QCRIL_OTHER_OEM_ITEMID_DATA_LEN);
      index += QCRIL_OTHER_OEM_ITEMID_DATA_LEN;

      /* Get the index of nv_item in nv table */
      for (nvIdx = 0; nvIdx < QCRIL_OTHER_NUM_OF_NV_ITEMS; nvIdx++)
      {
         if (qcril_other_nv_table[nvIdx].nv_item == nv_item_id)
         {
            break;
         }
      }

      /* Check if the requested NV item was found in the nv table */
      if (QCRIL_OTHER_NUM_OF_NV_ITEMS == nvIdx)
      {
         QCRIL_LOG_INFO("Requested NV item not found = %lu", nv_item_id);
         qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                          RIL_E_GENERIC_FAILURE, NULL, 0);
             break;
      }

      /* Validate the NV item length */
      if (qcril_other_nv_table[nvIdx].nv_item_size != nv_item_len)
      {
         QCRIL_LOG_INFO("Mismatch in Recieved Length = %lu and Expected Length = %lu for nv item = %s",
                         nv_item_len, qcril_other_nv_table[nvIdx].nv_item_size, qcril_other_nv_table[nvIdx].name);
         qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                          RIL_E_GENERIC_FAILURE, NULL, 0);
             break;
      }

      memcpy( (void *) (((char*) (&nv_item)) + qcril_other_nv_table[nvIdx].nv_item_offset),
              &(pcData[index]), nv_item_len);

      switch(nv_item_id)
      {
      case NV_AUTO_ANSWER_I:
      case NV_PREF_VOICE_SO_I:
      case NV_ROAM_CNT_I:
      case NV_AIR_CNT_I:
         {
            voice_set_config_req_msg_v02 set_config_req_msg;
            voice_set_config_resp_msg_v02 set_config_resp_msg;

            /* To start with, set all the optional fields to be invalid */
            memset(&set_config_req_msg, 0, sizeof(set_config_req_msg));
            memset(&set_config_resp_msg, 0, sizeof(set_config_resp_msg));

            if (NV_AUTO_ANSWER_I == nv_item_id)
            {
               set_config_req_msg.auto_answer_valid = TRUE;
               set_config_req_msg.auto_answer = nv_item.auto_answer.enable;
               /* TBD nv_item.auto_answer.rings
                *     qmi voice does not support this field, we ignore this field for now
                */
            }
            else if (NV_PREF_VOICE_SO_I == nv_item_id)
            {
               /* NV_PREF_VOICE_SO_I case */
               set_config_req_msg.preferred_voice_so_valid = TRUE;
               set_config_req_msg.preferred_voice_so.nam_id = nv_item.pref_voice_so.nam;
               set_config_req_msg.preferred_voice_so.evrc_capability = nv_item.pref_voice_so.evrc_capability_enabled;
               set_config_req_msg.preferred_voice_so.home_page_voice_so = nv_item.pref_voice_so.home_page_voice_so;
               set_config_req_msg.preferred_voice_so.home_orig_voice_so = nv_item.pref_voice_so.home_orig_voice_so;
               set_config_req_msg.preferred_voice_so.roam_orig_voice_so = nv_item.pref_voice_so.roam_orig_voice_so;
            }
            else if (NV_ROAM_CNT_I == nv_item_id)
            {
               set_config_req_msg.roam_timer_valid = TRUE;
               set_config_req_msg.roam_timer.roam_timer = nv_item.roam_cnt.cnt;
            }
            else
            {
               /* case NV_AIR_CNT_I */
               set_config_req_msg.air_timer_valid = TRUE;
               set_config_req_msg.air_timer.air_timer = nv_item.air_cnt.cnt;
            }

            qmi_client_error = qcril_qmi_client_send_msg_sync(QCRIL_QMI_CLIENT_VOICE,
                                                              QMI_VOICE_SET_CONFIG_REQ_V02,
                                                              &set_config_req_msg,
                                                              sizeof(set_config_req_msg),
                                                              &set_config_resp_msg,
                                                              sizeof(set_config_resp_msg)
                                                              );

            /* check response status */
            ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_client_error, &set_config_resp_msg.resp);
            QCRIL_LOG_INFO("ril_req_res %d, qmi_client_error %d", (int) ril_req_res, qmi_client_error);

            if ( RIL_E_SUCCESS == ril_req_res )
            {
               qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, NULL, 0);
            }
            else
            {
               QCRIL_LOG_INFO("Response is invalid");
               qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, NULL, 0);
            }
         }
         break;
      case NV_MIN1_I:
      case NV_MIN2_I:
      case NV_IMSI_11_12_I:
      case NV_IMSI_MCC_I:          /* retrieved from nas_3gpp2_min_based_info_type_v01*/
      case NV_IMSI_T_MCC_I:
      case NV_IMSI_T_11_12_I:
      case NV_IMSI_T_S1_I:
      case NV_IMSI_T_S2_I:
      case NV_IMSI_T_ADDR_NUM_I:   /* retrieved from nas_3gpp2_true_imsi_info_type_v01*/
      case NV_PCDMACH_I:
      case NV_SCDMACH_I:           /* retrieved from nas_cdma_channel_info_type_v01*/
      case NV_HOME_SID_NID_I:           /* retrieved from NV_HOME_SID_NID_I*/
      case NV_DIR_NUMBER_I:           /* retrieved from NV_HOME_SID_NID_I*/
      case NV_NAME_NAM_I:          /* retrieved from nam_name*/
         {
            nas_set_3gpp2_subscription_info_req_msg_v01 set_config_req_msg;
            nas_set_3gpp2_subscription_info_resp_msg_v01 set_config_resp_msg;

            /* To start with, set all the optional fields to be invalid */
            memset(&set_config_req_msg, 0, sizeof(set_config_req_msg));
            memset(&set_config_resp_msg, 0, sizeof(set_config_resp_msg));

            set_config_req_msg.nam_id = DEFAULT_NAM_ID;

            if (NV_MIN1_I == nv_item_id)
            {
               /* first need to fill the whole min_based info struct*/
               int ret = qcril_other_preset_min_based_info(&set_config_req_msg.min_based_info);
               if (RIL_E_SUCCESS != ret)
               {
                  qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                ret, NULL, 0);
                  break;
               }
               set_config_req_msg.min_based_info_valid = 1;

               /* fill min1 */
               qcril_other_int_to_ascii(set_config_req_msg.min_based_info.imsi_m_s1, NAS_IMSI_MIN1_LEN_V01, nv_item.min1.min1[1]);
            }
            else if (NV_MIN2_I == nv_item_id)
            {
               /* first need to fill the whole min_based info struct*/
               int ret = qcril_other_preset_min_based_info(&set_config_req_msg.min_based_info);
               if (RIL_E_SUCCESS != ret)
               {
                  qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                ret, NULL, 0);
                  break;
               }
               set_config_req_msg.min_based_info_valid = 1;

               /* fill min2 */
               qcril_other_int_to_ascii(set_config_req_msg.min_based_info.imsi_m_s2, NAS_IMSI_MIN2_LEN_V01, nv_item.min2.min2[1]);
            }
            else if (NV_IMSI_11_12_I == nv_item_id)
            {
               /* first need to fill the whole min_based info struct*/
               int ret = qcril_other_preset_min_based_info(&set_config_req_msg.min_based_info);
               if (RIL_E_SUCCESS != ret)
               {
                  qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                ret, NULL, 0);
                  break;
               }
               set_config_req_msg.min_based_info_valid = 1;

               /* fill imsi_11_22 */
               qcril_other_int_to_ascii(set_config_req_msg.min_based_info.imsi_m_11_12, NAS_IMSI_11_12_LEN_V01, nv_item.imsi_11_12.imsi_11_12);
            }
            else if (NV_IMSI_MCC_I == nv_item_id)
            {
               /* first need to fill the whole min_based info struct*/
               int ret = qcril_other_preset_min_based_info(&set_config_req_msg.min_based_info);
               if (RIL_E_SUCCESS != ret)
               {
                  qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                ret, NULL, 0);
                  break;
               }
               set_config_req_msg.min_based_info_valid = 1;

               /* fill imsi_11_22 */
               qcril_other_int_to_ascii(set_config_req_msg.min_based_info.mcc_m, NAS_MCC_LEN_V01, nv_item.imsi_11_12.imsi_11_12);
            }
            else if (NV_IMSI_T_MCC_I == nv_item_id)
            {
               /* first need to fill the whole true_imsi struct*/
               int ret = qcril_other_preset_true_imsi_info(&set_config_req_msg.true_imsi);
               if (RIL_E_SUCCESS != ret)
               {
                  qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                ret, NULL, 0);
                  break;
               }
               set_config_req_msg.true_imsi_valid = 1;

               /* fill imsi_11_22 */
               qcril_other_int_to_ascii(set_config_req_msg.true_imsi.mcc_t, NAS_MCC_LEN_V01, nv_item.imsi_t_mcc.imsi_mcc);
            }
            else if (NV_IMSI_T_11_12_I == nv_item_id)
            {
               /* first need to fill the whole true_imsi struct*/
               int ret = qcril_other_preset_true_imsi_info(&set_config_req_msg.true_imsi);
               if (RIL_E_SUCCESS != ret)
               {
                  qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                ret, NULL, 0);
                  break;
               }
               set_config_req_msg.true_imsi_valid = 1;

               /* fill imsi_11_22 */
               qcril_other_int_to_ascii(set_config_req_msg.true_imsi.imsi_t_11_12, NAS_IMSI_11_12_LEN_V01, nv_item.imsi_t_11_12.imsi_11_12);
            }
            else if (NV_IMSI_T_S1_I == nv_item_id)
            {
               /* first need to fill the whole true_imsi struct*/
               int ret = qcril_other_preset_true_imsi_info(&set_config_req_msg.true_imsi);
               if (RIL_E_SUCCESS != ret)
               {
                  qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                ret, NULL, 0);
                  break;
               }
               set_config_req_msg.true_imsi_valid = 1;

               /* fill imsi_t_s1 */
               qcril_other_int_to_ascii(set_config_req_msg.true_imsi.imsi_t_s1, NAS_IMSI_MIN1_LEN_V01, nv_item.imsi_t_s1.min1[1]);
            }
            else if (NV_IMSI_T_S2_I == nv_item_id)
            {
               /* first need to fill the whole true_imsi struct*/
               int ret = qcril_other_preset_true_imsi_info(&set_config_req_msg.true_imsi);
               if (RIL_E_SUCCESS != ret)
               {
                  qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                ret, NULL, 0);
                  break;
               }
               set_config_req_msg.true_imsi_valid = 1;

               /* fill imsi_t_s2 */
               qcril_other_int_to_ascii(set_config_req_msg.true_imsi.imsi_t_s2, NAS_IMSI_MIN2_LEN_V01, nv_item.imsi_t_s2.min2[1]);
            }
            else if (NV_IMSI_T_ADDR_NUM_I == nv_item_id)
            {
               /* first need to fill the whole true_imsi struct*/
               int ret = qcril_other_preset_true_imsi_info(&set_config_req_msg.true_imsi);
               if (RIL_E_SUCCESS != ret)
               {
                  qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                ret, NULL, 0);
                  break;
               }
               set_config_req_msg.true_imsi_valid = 1;

               /* fill imsi_t_addr_num */
               set_config_req_msg.true_imsi.imsi_t_addr_num = nv_item.imsi_t_addr_num.num;
            }
            else if (NV_PCDMACH_I == nv_item_id)
            {
               /* first need to fill the whole cdma_channel_info struct*/
               int ret = qcril_other_preset_cdma_channel_info(&set_config_req_msg.cdma_channel_info);
               if (RIL_E_SUCCESS != ret)
               {
                  qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                ret, NULL, 0);
                  break;
               }
               set_config_req_msg.cdma_channel_info_valid = 1;

               set_config_req_msg.cdma_channel_info.pri_ch_a = nv_item.pcdmach.channel_a;
               set_config_req_msg.cdma_channel_info.pri_ch_b = nv_item.pcdmach.channel_b;
            }
            else if (NV_SCDMACH_I == nv_item_id)
            {
               /* first need to fill the whole cdma_channel_info struct*/
               int ret = qcril_other_preset_cdma_channel_info(&set_config_req_msg.cdma_channel_info);
               if (RIL_E_SUCCESS != ret)
               {
                  qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                ret, NULL, 0);
                  break;
               }
               set_config_req_msg.cdma_channel_info_valid = 1;

               set_config_req_msg.cdma_channel_info.sec_ch_a = nv_item.scdmach.channel_a;
               set_config_req_msg.cdma_channel_info.sec_ch_b = nv_item.scdmach.channel_b;
            }
            else if (NV_HOME_SID_NID_I == nv_item_id)
            {
               set_config_req_msg.cdma_sys_id_valid = 1;
               set_config_req_msg.cdma_sys_id_len = NV_MAX_HOME_SID_NID;
               uint32_t i;
               for (i=0; i<set_config_req_msg.cdma_sys_id_len; i++)
               {
                  set_config_req_msg.cdma_sys_id[i].sid = nv_item.home_sid_nid.pair[i].sid;
                  set_config_req_msg.cdma_sys_id[i].nid = nv_item.home_sid_nid.pair[i].nid;
               }
            }
            else if (NV_DIR_NUMBER_I == nv_item_id)
            {
               set_config_req_msg.dir_num_valid = 1;
               set_config_req_msg.nam_id = nv_item.dir_number.nam;
               set_config_req_msg.dir_num_len = strlen((char*)nv_item.dir_number.dir_number);
               memcpy(set_config_req_msg.dir_num, nv_item.dir_number.dir_number, set_config_req_msg.dir_num_len);
            }
            else
            {
               /* case NV_NAME_NAM_I */
               set_config_req_msg.nam_name_valid = 1;
               set_config_req_msg.nam_id = nv_item.name_nam.nam;
               set_config_req_msg.nam_name_len = strlen((char*)nv_item.name_nam.name);
               memcpy(set_config_req_msg.nam_name, nv_item.name_nam.name, set_config_req_msg.nam_name_len);
            }

            qmi_ril_get_property_value_from_integer(QMI_RIL_SPC_TLV_NVWRITE, &spc_tlv_valid, FALSE);

            if( TRUE == spc_tlv_valid )
            {
                if ( ( ( params_ptr->datalen - ( index + nv_item_len) ) > NAS_SPC_MAX_V01 ) ||
                     ( ( params_ptr->datalen - ( index + nv_item_len) ) < 1 )
                    )
                {
                    QCRIL_LOG_ERROR("Invalid SPC len %d", ( params_ptr->datalen - ( index + nv_item_len) ) );
                    ril_req_res = RIL_E_GENERIC_FAILURE;
                }
                else
                {
                    set_config_req_msg.spc_valid = TRUE;
                    memcpy(set_config_req_msg.spc, &pcData[index + nv_item_len], ( params_ptr->datalen - ( index + nv_item_len) ) );
                }
            }

            if ( RIL_E_SUCCESS == ril_req_res )
            {
                qmi_client_error = qcril_qmi_client_send_msg_sync(QCRIL_QMI_CLIENT_NAS,
                                                              QMI_NAS_SET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01,
                                                              &set_config_req_msg,
                                                              sizeof(set_config_req_msg),
                                                              &set_config_resp_msg,
                                                              sizeof(set_config_resp_msg) );

                /* check response status */
                ril_req_res = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_client_error, &set_config_resp_msg.resp);
                QCRIL_LOG_INFO("ril_req_res %d, qmi_client_error %d", (int) ril_req_res, qmi_client_error);
            }

            if ( RIL_E_SUCCESS == ril_req_res )
            {
               qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id, RIL_E_SUCCESS, NULL, 0);
            }
            else
            {
               QCRIL_LOG_INFO("Response is invalid");
               qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id, RIL_E_GENERIC_FAILURE, NULL, 0);
            }
         }
         break;
      default:
         QCRIL_LOG_INFO("Requested NV item not supported = %lu", nv_item_id);
         qcril_other_send_request_reponse(instance_id, params_ptr->t, params_ptr->event_id,
                                                RIL_E_GENERIC_FAILURE, NULL, 0);
         break;
      }
   }while(0);
} /* qcril_other_request_oem_hook_nv_write */


/*=========================================================================

  FUNCTION:  qcril_other_request_oem_hook_neighboring_cells_info

===========================================================================*/
/*!
    @brief
    Retrieves the neighboring cells information

    @return
    If success then the neighboring cells information is returned
*/
/*=========================================================================*/
void qcril_other_request_oem_hook_neighboring_cells_info
(
 const qcril_request_params_type *const params_ptr,
 qcril_request_return_type *const ret_ptr /*!< Output parameter */
 )
{
    qcril_instance_id_e_type instance_id;
    qcril_request_resp_params_type resp;

    RIL_Errno   ril_req_res = RIL_E_GENERIC_FAILURE;
    qmi_txn_handle txn_handle;
    qcril_reqlist_public_type qcril_req_info_ptr;
    qmi_client_error_type qmi_client_error;

    nas_get_cell_location_info_resp_msg_v01 *qmi_response;
    qcril_other_get_cell_location_info_resp_msg_v01 *ril_resp_helper_msg = NULL;
    QCRIL_NOTUSED( ret_ptr );



    instance_id = QCRIL_DEFAULT_INSTANCE_ID;


    QCRIL_LOG_FUNC_ENTRY();

    qcril_reqlist_default_entry( params_ptr->t,
                                 params_ptr->event_id,
                                 QCRIL_DEFAULT_MODEM_ID,
                                 QCRIL_REQ_AWAITING_MORE_AMSS_EVENTS,
                                 QCRIL_EVT_HOOK_NEIGHBOR_CELL_INFO_RCVD,
                                 NULL,
                                 &qcril_req_info_ptr );

    if ( qcril_reqlist_new( instance_id, &qcril_req_info_ptr ) == E_SUCCESS )
    {
        qmi_response = qcril_malloc( sizeof( *qmi_response ) );
        if ( qmi_response )
        {
            memset(qmi_response,0,sizeof(*qmi_response));
            ril_resp_helper_msg = qcril_malloc( sizeof( *ril_resp_helper_msg ) );

            if ( ril_resp_helper_msg )
            {
                qmi_client_error =  qmi_client_send_msg_async( qcril_qmi_client_get_user_handle( QCRIL_QMI_CLIENT_NAS ),
                                                                   QMI_NAS_GET_CELL_LOCATION_INFO_REQ_MSG_V01,
                                                                   NULL,
                                                                   QMI_RIL_ZERO, // empty payload
                                                                   (void*) qmi_response,
                                                                   sizeof( *qmi_response ),
                                                                   qcril_other_request_oem_hook_neighboring_cells_info_cb,
                                                                   (void *)ril_resp_helper_msg,
                                                                   &txn_handle
                                                     );
                ril_req_res = ( QMI_NO_ERR == qmi_client_error ) ? RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE;
            }
        }

        QCRIL_LOG_INFO( "result=%d",ril_req_res);
        if ( RIL_E_SUCCESS != ril_req_res )
        {
            qcril_default_request_resp_params( instance_id, params_ptr->t, params_ptr->event_id, ril_req_res, &resp );
            qcril_send_request_response( &resp );

            if ( ril_resp_helper_msg )
            {
                qcril_free( ril_resp_helper_msg );
            }
            if ( qmi_response )
            {
                qcril_free( qmi_response );
            }
        }
    }
} /* qcril_other_request_oem_hook_neighboring_cells_info */


//===========================================================================
// qcril_other_request_oem_hook_neighboring_cells_info_cb
//===========================================================================
void qcril_other_request_oem_hook_neighboring_cells_info_cb
(
  qmi_client_type              user_handle,
  unsigned long                msg_id,
  void                         *resp_c_struct,
  int                          resp_c_struct_len,
  void                         *resp_cb_data,
  qmi_client_error_type        transp_err
)
{
    nas_get_cell_location_info_resp_msg_v01 * qmi_response   = (nas_get_cell_location_info_resp_msg_v01 *) resp_c_struct;
    qcril_other_get_cell_location_info_resp_msg_v01 *    ril_resp_helper = (qcril_other_get_cell_location_info_resp_msg_v01 *) resp_cb_data;
    RIL_Errno   ril_req_res                                = RIL_E_GENERIC_FAILURE;
    qcril_reqlist_public_type qcril_req_info;
    errno_enum_type           found_qcril_request;
    qcril_request_resp_params_type resp;

    unsigned int iter=0,iter_j=0,iter_k=0;

    user_handle = user_handle;
    msg_id = msg_id;
    resp_c_struct_len = resp_c_struct_len;

    found_qcril_request = qcril_reqlist_query_by_event( QCRIL_DEFAULT_INSTANCE_ID,
                                  QCRIL_DEFAULT_MODEM_ID,
                                  QCRIL_EVT_HOOK_NEIGHBOR_CELL_INFO_RCVD,
                                  &qcril_req_info );

    if ( qmi_response && (E_SUCCESS == found_qcril_request) && ril_resp_helper  )
    {
        if ( QMI_NO_ERR == transp_err )
        {
            memset( ril_resp_helper, 0, sizeof(*ril_resp_helper) );
            memcpy( ril_resp_helper, qmi_response, sizeof(*ril_resp_helper));
            iter=0;
            if( ril_resp_helper->geran_info_valid && ril_resp_helper->geran_info.nmr_cell_info_len <= QCRIL_OTHER_NMR_MAX_NUM_V01)
            {
                QCRIL_LOG_INFO("GSM cells");
                for(iter_j=0; iter_j < ril_resp_helper->geran_info.nmr_cell_info_len; iter_j++,iter++)
                {
                    QCRIL_LOG_INFO("cell id %d lac %d",ril_resp_helper->geran_info.nmr_cell_info[iter_j].nmr_cell_id,ril_resp_helper->geran_info.nmr_cell_info[iter_j].nmr_lac);
                }
            }
            if( ril_resp_helper->umts_info_valid && ril_resp_helper->umts_info.umts_monitored_cell_len <= QCRIL_OTHER_UMTS_MAX_MONITORED_CELL_SET_NUM_V01)
            {
                QCRIL_LOG_INFO("UMTS cells");
                for(iter_j=0; iter_j < ril_resp_helper->umts_info.umts_monitored_cell_len; iter_j++,iter++)
                {
                    QCRIL_LOG_INFO("psc %d",ril_resp_helper->umts_info.umts_monitored_cell[iter_j].umts_psc);
                }
            }
            if( ril_resp_helper->lte_intra_valid && ril_resp_helper->lte_intra.cells_len <= QCRIL_OTHER_LTE_NGBR_NUM_CELLS_V01)
            {
                QCRIL_LOG_INFO("INTRA LTE cells");
                QCRIL_LOG_INFO("tac %d",ril_resp_helper->lte_intra.tac);
                for(iter_j=0; iter_j < ril_resp_helper->lte_intra.cells_len; iter_j++,iter++)
                {
                    QCRIL_LOG_INFO("pci %d",ril_resp_helper->lte_intra.cells[iter_j].pci);
                }
            }
            if( ril_resp_helper->lte_inter_valid && ril_resp_helper->lte_inter.freqs_len <= QCRIL_OTHER_LTE_NGBR_NUM_FREQS_V01)
            {
                QCRIL_LOG_INFO("INTER LTE cells");
                for(iter_j=0; iter_j < ril_resp_helper->lte_inter.freqs_len; iter_j++)
                {
                    if( ril_resp_helper->lte_inter.freqs[iter_j].cells_len <= QCRIL_OTHER_LTE_NGBR_NUM_CELLS_V01)
                    {
                        QCRIL_LOG_INFO("earfcn %d",ril_resp_helper->lte_inter.freqs[iter_j].earfcn);
                        for( iter_k=0; iter_k < ril_resp_helper->lte_inter.freqs[iter_j].cells_len; iter_k++,iter++ )
                        {
                            QCRIL_LOG_INFO("pci %d",ril_resp_helper->lte_inter.freqs[iter_j].cells[iter_k].pci);
                        }
                    }
                }
            }
            if( ril_resp_helper->lte_gsm_valid && ril_resp_helper->lte_gsm.freqs_len <= QCRIL_OTHER_LTE_NGBR_GSM_NUM_FREQS_V01)
            {
                QCRIL_LOG_INFO("GSM LTE cells");
                for(iter_j=0; iter_j < ril_resp_helper->lte_gsm.freqs_len; iter_j++)
                {
                    if( ril_resp_helper->lte_gsm.freqs[iter_j].cells_len <= QCRIL_OTHER_LTE_NGBR_GSM_NUM_CELLS_V01)
                    {
                        QCRIL_LOG_INFO("cell res priority %d",ril_resp_helper->lte_gsm.freqs[iter_j].cell_resel_priority);
                        for( iter_k=0; iter_k < ril_resp_helper->lte_gsm.freqs[iter_j].cells_len; iter_k++,iter++ )
                        {
                            QCRIL_LOG_INFO("bsc id %d",ril_resp_helper->lte_gsm.freqs[iter_j].cells[iter_k].bsic_id);
                        }
                    }
                }
            }
            if( ril_resp_helper->lte_wcdma_valid && ril_resp_helper->lte_wcdma.freqs_len <= QCRIL_OTHER_LTE_NGBR_WCDMA_NUM_FREQS_V01)
            {
                QCRIL_LOG_INFO("UMTS LTE cells");
                for(iter_j=0; iter_j < ril_resp_helper->lte_wcdma.freqs_len; iter_j++)
                {
                    if( ril_resp_helper->lte_wcdma.freqs[iter_j].cells_len <= QCRIL_OTHER_LTE_NGBR_WCDMA_NUM_CELLS_V01)
                    {
                        QCRIL_LOG_INFO("uarfcn %d",ril_resp_helper->lte_wcdma.freqs[iter_j].uarfcn);
                        for( iter_k=0; iter_k < ril_resp_helper->lte_wcdma.freqs[iter_j].cells_len; iter_k++,iter++ )
                        {
                            QCRIL_LOG_INFO("psc %d",ril_resp_helper->lte_wcdma.freqs[iter_j].cells[iter_k].psc);
                        }
                    }
                }
            }

            if( iter > 0 )
            {
                QCRIL_LOG_INFO("number of cells %d",iter);
                ril_req_res = RIL_E_SUCCESS;
            }
        }
        qcril_default_request_resp_params( QCRIL_DEFAULT_INSTANCE_ID, qcril_req_info.t, qcril_req_info.request, ril_req_res , &resp );
        if ( RIL_E_SUCCESS == ril_req_res )
        {
            resp.resp_pkt = (void *) ril_resp_helper;
            resp.resp_len = sizeof( *ril_resp_helper );
        }
        qcril_send_request_response( &resp );
    }
    if ( qmi_response )
    {
        qcril_free( qmi_response );
    }
    if ( ril_resp_helper )
    {
        qcril_free( ril_resp_helper );
    }

} // qcril_other_request_oem_hook_neighboring_cells_info_cb

//===========================================================================
//qmi_ril_oem_hook_qmi_idl_tunneling_get_service_object
//===========================================================================
qmi_idl_service_object_type qmi_ril_oem_hook_qmi_idl_tunneling_get_service_object( qmi_ril_oem_hook_qmi_tunneling_service_id_type service_id )
{
    qmi_idl_service_object_type res = NULL;

    switch ( service_id )
    {
    case QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_VT:
        res = ims_qmi_get_service_object_v01();
        break;

    case QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_PRESENCE:
        res = imsp_get_service_object_v01();
        break;

    case QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_EMBMS:
        res = embms_get_service_object_v01();
        break;

    case QMI_RIL_OEM_HOOK_QMI_TUNNELING_SERVICE_QTUNER:
        res = Qtuner_get_service_object_v01();
        break;

    default:
        res = NULL;
        break;
    }

    return res;
} // qmi_ril_oem_hook_qmi_idl_tunneling_get_service_object

//===========================================================================
//qmi_ril_get_property_value_helper
//===========================================================================
void qmi_ril_get_property_value_helper(const char *property_name,
                                       char *property_value,
                                       const char *default_property_value)
{
    if(property_name &&
       property_value &&
       default_property_value)
    {
        property_get(property_name,
                     property_value,
                     default_property_value);
    }
    else
    {
        QCRIL_LOG_ERROR("invalid property name/value/default value");
    }
} //qmi_ril_get_property_value_helper


//===========================================================================
//qmi_ril_set_property_value_helper
//===========================================================================
errno_enum_type qmi_ril_set_property_value_helper(const char *property_name,
                                                  const char *property_value)
{
    errno_enum_type res;

    res = E_FAILURE;

    if(property_name && property_value)
    {
        if(strlen(property_value) <= PROPERTY_VALUE_MAX)
        {
            res = property_set(property_name,
                               property_value);

            if (E_SUCCESS != res)
            {
                QCRIL_LOG_ERROR("failed to set %s to %s",
                                property_name,
                                property_value);
            }
            else
            {
                QCRIL_LOG_INFO("%s set to %s successfully",
                               property_name,
                               property_value);
            }
        }
        else
        {
            QCRIL_LOG_ERROR("property value can not have more than %d characters",
                            PROPERTY_VALUE_MAX);
        }
    }
    else
    {
        QCRIL_LOG_ERROR("invalid property name/value");
    }

    return res;
} //qmi_ril_set_property_value_helper


//===========================================================================
//qmi_ril_get_property_value_from_string
//===========================================================================
void qmi_ril_get_property_value_from_string(const char *property_name,
                                            char *property_value,
                                            const char *default_property_value)
{
    if(property_name && property_value)
    {
        qmi_ril_get_property_value_helper(property_name,
                                          property_value,
                                          default_property_value);

        QCRIL_LOG_INFO("retrieved %s from %s",
                       property_value,
                       property_name);
    }
    else
    {
        QCRIL_LOG_ERROR("invalid property name/value");
    }
} //qmi_ril_get_property_value_from_string

//===========================================================================
//qmi_ril_get_property_value_from_integer
//===========================================================================
void qmi_ril_get_property_value_from_integer(const char *property_name,
                                             int *property_value,
                                             int default_property_value)
{
    unsigned long res;
    char *end_ptr;
    char read_value[ PROPERTY_VALUE_MAX ];
    char temp_default_property_value[ PROPERTY_VALUE_MAX ];

    res = QMI_RIL_ZERO;
    end_ptr = NULL;
    memset(read_value,
           QMI_RIL_ZERO,
           sizeof(read_value));
    memset(temp_default_property_value,
           QMI_RIL_ZERO,
           sizeof(temp_default_property_value));

    if(property_name && property_value)
    {
        QCRIL_SNPRINTF(temp_default_property_value,
                       sizeof( temp_default_property_value ),
                       "%d",
                       default_property_value);

        qmi_ril_get_property_value_helper(property_name,
                                          read_value,
                                          temp_default_property_value);

        if ((strlen(read_value) > 0))
        {
            res = strtoul(read_value,
                          &end_ptr,
                          QMI_RIL_ZERO);

            if (((errno == ERANGE) &&
                 (res == ULONG_MAX)) ||
                *end_ptr)
            {
                QCRIL_LOG_ERROR("failed to convert %s, read value %s",
                                property_name,
                                read_value);
                *(property_value) = QMI_RIL_ZERO;
            }
            else
            {
                *(property_value) = (int) res;
                QCRIL_LOG_INFO("retrieved %d from %s",
                               *(property_value),
                               property_name);
            }
        }
    }
    else
    {
        QCRIL_LOG_ERROR("invalid property name/value/default value");
    }
} //qmi_ril_get_property_value_from_integer

//===========================================================================
//qmi_ril_get_property_value_from_boolean
//===========================================================================
void qmi_ril_get_property_value_from_boolean(const char *property_name,
                                             boolean *property_value,
                                             boolean default_property_value)
{
    unsigned long res;
    char *end_ptr;
    char read_value[ PROPERTY_VALUE_MAX ];
    char temp_default_property_value[ PROPERTY_VALUE_MAX ];

#define QCRIL_TRUE_STR "true"

    res = QMI_RIL_ZERO;
    end_ptr = NULL;
    memset(read_value,
           QMI_RIL_ZERO,
           sizeof(read_value));
    memset(temp_default_property_value,
           QMI_RIL_ZERO,
           sizeof(temp_default_property_value));

    if(property_name && property_value)
    {
        QCRIL_SNPRINTF(temp_default_property_value,
                       sizeof( temp_default_property_value ),
                       "%d",
                       !!default_property_value);

        qmi_ril_get_property_value_helper(property_name,
                                          read_value,
                                          temp_default_property_value);

        res = strtoul(read_value,
                      &end_ptr,
                      QMI_RIL_ZERO);

        if (end_ptr == read_value)
        {
            if ( !strncmp( read_value, QCRIL_TRUE_STR, strlen(QCRIL_TRUE_STR) ) )
            {
                *(property_value) = TRUE;
            }
            else
            {
                *(property_value) = FALSE;
            }
        }
        else if (((errno == ERANGE) &&
                 (res == ULONG_MAX)) ||
                 *end_ptr)
        {
            QCRIL_LOG_ERROR("failed to convert %s, read value %s",
                             property_name,
                             read_value);
            *(property_value) = !!default_property_value;
        }
        else
        {
            *(property_value) = (boolean) !!res;
            QCRIL_LOG_INFO("retrieved %d from %s(%s)",
                           *(property_value),
                           property_name,
                           read_value);
        }
    }
    else
    {
        QCRIL_LOG_ERROR("invalid property name/value/default value");
    }
} //qmi_ril_get_property_value_from_boolean


//===========================================================================
//qmi_ril_set_property_value_to_string
//===========================================================================
errno_enum_type qmi_ril_set_property_value_to_string(const char *property_name,
                                                     const char *property_value)
{
    errno_enum_type res;

    res = E_FAILURE;

    if(property_name && property_value)
    {
        res = qmi_ril_set_property_value_helper(property_name,
                                                property_value);

    }
    else
    {
        QCRIL_LOG_ERROR("invalid property name/value");
    }

    return res;
} //qmi_ril_set_property_value_to_string

//===========================================================================
//qmi_ril_set_property_value_to_integer
//===========================================================================
errno_enum_type qmi_ril_set_property_value_to_integer(const char *property_name,
                                                      int property_value)
{
    errno_enum_type res;
    char write_value[ PROPERTY_VALUE_MAX ];

    res = E_FAILURE;
    memset(write_value,
           QMI_RIL_ZERO,
           sizeof(write_value));

    if(property_name)
    {
        QCRIL_SNPRINTF(write_value,
                       sizeof(write_value),
                       "%d",
                       (int) property_value);

        res = qmi_ril_set_property_value_helper(property_name,
                                                write_value);

    }
    else
    {
        QCRIL_LOG_ERROR("invalid property name");
    }

    return res;
} //qmi_ril_set_property_value_to_integer

//===========================================================================
//qmi_ril_retrieve_system_time_in_ms
//===========================================================================
uint64_t qmi_ril_retrieve_system_time_in_ms()
{
    uint64_t abs_time;
    struct timespec ts;
    struct timeval tv;

    abs_time = 0;
    memset(&ts,
           0,
           sizeof(ts));
    memset(&tv,
           0,
           sizeof(tv));

    if(qmi_ril_is_feature_supported(QMI_RIL_FEATURE_POSIX_CLOCKS))
    {
        clock_gettime(CLOCK_MONOTONIC,
                      &ts);
        tv.tv_sec = ts.tv_sec;
        tv.tv_usec = ts.tv_nsec/1000;
    }
    else
    {
        gettimeofday(&tv,
                     NULL);
    }
    abs_time = (tv.tv_sec * 1000) + (tv.tv_usec / 1000); //converting system time to ms

    return abs_time;
} //qmi_ril_retrieve_system_time_in_ms

//===========================================================================
//qmi_ril_retrieve_directory_list
//===========================================================================
void qmi_ril_retrieve_directory_list(char *path, char *subStr, qcril_other_dirlist **dir_list)
{
  DIR                  *base_dir = NULL;
  int                  itr       = 0;
  int                  tmp_len   = 0;
  qcril_other_dirlist  *dir_trav = NULL;
  struct dirent        *dir;

  if(path == NULL)
  {
    return;
  }

  base_dir = opendir(path);
  if (base_dir)
  {
    while ((dir = readdir(base_dir)) != NULL)
    {
      QCRIL_LOG_INFO("Dir: %s\n", dir->d_name);
      if(((dir->d_type == DT_DIR) || (dir->d_type == DT_LNK)) &&
         ((NULL == subStr) || ((NULL != subStr) && (!strncmp(dir->d_name, subStr, strlen(subStr))))))
      {
          if(dir_trav)
          {
            dir_trav->next = qcril_malloc(sizeof(*(dir_trav->next)));
            dir_trav = dir_trav->next;
          }
          else
          {
            *dir_list = qcril_malloc(sizeof(**dir_list));
            dir_trav = *dir_list;
          }

          if( dir_trav != NULL )
          {
            tmp_len = strlen(dir->d_name) + 1;
            dir_trav->dir_name = qcril_malloc(tmp_len);
            if(dir_trav->dir_name != NULL)
            {
              strlcpy(dir_trav->dir_name, dir->d_name, tmp_len);
              QCRIL_LOG_INFO("Matched dir %s\n", dir_trav->dir_name);
            }
            else
            {
              QCRIL_LOG_ERROR("Failed to allocate memory");
              break;
            }
          }
          else
          {
            QCRIL_LOG_ERROR("Failed to allocate memory");
            break;
          }
      }
    }

    closedir(base_dir);
  }
  else
  {
    QCRIL_LOG_ERROR("Failed to open directory %s", path);
  }

  return;
}

//===========================================================================
//qmi_ril_free_directory_list
//===========================================================================
void qmi_ril_free_directory_list(qcril_other_dirlist *dir_list)
{
  qcril_other_dirlist *dir_trav = dir_list;

  while(dir_list != NULL)
  {
    if(dir_list->dir_name != NULL) qcril_free(dir_list->dir_name);
    dir_trav = dir_list->next;
    qcril_free(dir_list);
    dir_list = dir_trav;
  }

  return;
}

