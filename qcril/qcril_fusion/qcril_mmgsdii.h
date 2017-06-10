/*!
  @file
  qcril_mmgsdii.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2009 - 2010 Qualcomm Technologies, Inc. All Rights Reserved

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

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
07/09/10   at      Merged branch changes to qcril_fusion folder
04/12/10   mib     Added FDN not available state
03/11/10   mib     Moved illegal status from card to app
03/09/10   js      Support for providing number of perso retries in perso
                   deactivate response
03/01/10   fc      Re-architecture to support split modem.
11/19/09   js      Added Support for illegal SIM
11/02/09   js      Check for valid client before sending command to MMGSDI.
10/24/09   js      QCRIL MMGSDI response function cleanup
08/13/09   js      Fixed FDN PIN reties not sent in QCRIL response issue
07/20/09   js      Added support to allow PIN2 verification to be performed
                   prior to enable/disable FDN commands
07/10/09   tml     Added support to handle intermediate step for PIN2 
                   verification during SIM IO
06/23/09   tml     Fixed perso event received too early issue
05/18/09   fc      Changes to log debug messages to Diag directly instead
                   of through logcat.
05/14/09   pg      Mainlined FEATURE_MULTIMODE_ANDROID.
04/05/09   fc      Cleanup log macros.
01/26/08   fc      Logged assertion info.
01/09/08   tml     Featurize for 1x non ruim build
12/17/08   tml     Externed file mapping.  Removed client_id_is_valid and
                   use client_id for the validity checking instead
12/16/08   fc      Changes to support the release of AMSS MMGSDI object
                   for ONCRPC.
12/12/08   tml     Added comments to the new cardstatus structure and typedefs
12/08/08   tml     Added unknown card state 
10/08/08   tml     Added multimode support
07/30/08   tml     Added SIM IO Get Response support
07/14/08   tml     Clean up message levels
06/11/08   jod     Added support for GET IMSI
06/02/08   jod     Added SIM IO support
05/23/08   tml     Added perso, pin enable/disable and fdn enable/disable 
                   support
05/21/08   jar     Fixed off target LTK platform compilation
05/20/08   tml     Added event support
05/19/08   tml     initial version

===========================================================================*/

#ifndef QCRIL_MMGSDII_H
#define QCRIL_MMGSDII_H

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#ifndef FEATURE_CDMA_NON_RUIM
#include "gsdi_exp.h"
#include "mmgsdilib.h"
#include "qcrili.h"
#include "qcril_log.h"

#ifdef QCRIL_MMGSDI_UNIT_TEST
#include "assert.h"
#endif /* QCRIl_MMGSDI_UNIT_TEST */


/*===========================================================================

                        DEFINITIONS AND TYPES

===========================================================================*/

/*! Maximum string length for safe_strlen */
#define MAX_STR (64)

#define RIL_CARDSTATE_UNKNOWN   0xFF
/* Perso category size in bytes */
#define QCRIL_MMGSDI_PERSO_CATEGORY_LENGTH   4
/* Minimum length of personalization control key */
#define QCRIL_MMGSDI_MIN_PERSO_KEY_LENGTH    6
/* Maximum length of personalization control key */
#define QCRIL_MMGSDI_MAX_PERSO_KEY_LENGTH    16
/* Invalid/Default client id value */
#define QCRIL_MMGSDI_INVALID_CLIENT_ID       0

typedef enum
{
  QCRIL_MMGSDI_FALSE    = 0,
  QCRIL_MMGSDI_TRUE     = 1,
  QCRIL_MMGSDI_NOT_INIT = 2
} qcril_mmgsdi_bool_enum_type;

typedef enum
{
  QCRIL_MMGSDI_FDN_NOT_INIT      = 0,
  QCRIL_MMGSDI_FDN_NOT_AVAILABLE = 1,
  QCRIL_MMGSDI_FDN_ENABLED       = 2,
  QCRIL_MMGSDI_FDN_DISABLED      = 3
} qcril_mmgsdi_fdn_status_enum_type;

typedef enum
{ 
  QCRIL_MMGSDI_ORIG_SIM_UNKNOWN,
  QCRIL_MMGSDI_ORIG_SIM_IO_UPDATE_RECORD,
  QCRIL_MMGSDI_ORIG_SIM_IO_UPDATE_BINARY,
  QCRIL_MMGSDI_ORIG_SIM_IO_READ_RECORD,
  QCRIL_MMGSDI_ORIG_SIM_IO_READ_BINARY,
  QCRIL_MMGSDI_ORIG_ENABLE_FDN,
  QCRIL_MMGSDI_ORIG_DISABLE_FDN
} qcril_mmgsdi_orig_cmd_enum_type;

typedef enum
{
  QCRIL_MMGSDI_PERSO_UNKNOWN,
  QCRIL_MMGSDI_PERSO_NETWORK,
  QCRIL_MMGSDI_PERSO_NETWORK_SUBSET,
  QCRIL_MMGSDI_PERSO_SERVICE_PROVIDER,
  QCRIL_MMGSDI_PERSO_CORPORATE,
  QCRIL_MMGSDI_PERSO_SIM_USIM,
  QCRIL_MMGSDI_PERSO_CDMA_NETWORK1,
  QCRIL_MMGSDI_PERSO_CDMA_NETWORK2,
  QCRIL_MMGSDI_PERSO_CDMA_HRPD,
  QCRIL_MMGSDI_PERSO_CDMA_SERVICE_PROVIDER,
  QCRIL_MMGSDI_PERSO_CDMA_CORPORATE,
  QCRIL_MMGSDI_PERSO_CDMA_RUIM
} qcril_mmgsdi_perso_category_enum_type;

typedef struct
{
  RIL_Token                         token;
  qcril_mmgsdi_orig_cmd_enum_type   cmd;
  mmgsdi_access_type                access;
  mmgsdi_data_type                  data;   /* data to be written */
  mmgsdi_rec_num_type               record; /* record number for linear/cyclic file*/
  mmgsdi_offset_type                offset; /* offset for transparent file */
  mmgsdi_len_type                   length; /* data length to be read */
  uint8                             aid_data[GSDI_MAX_APP_ID_LEN]; /* Aid data buffer */
  uint8                             aid_len; /* aid data length */
  mmgsdi_pin_info_type              pin_info; /* Pin Information */
  boolean                           do_pin_verify; /* if we must verify PIN */
} qcril_mmgsdi_internal_sim_data_type;

typedef struct
{
  boolean              valid;
  int                  app_index;
  mmgsdi_pin_enum_type pin_id;
} qcril_mmgsdi_curr_pin_type;

/*! @brief MMGSDI command callback parameters 
*/
typedef struct
{
  mmgsdi_cnf_enum_type    cnf;
  mmgsdi_return_enum_type status;
  mmgsdi_cnf_type        *cnf_ptr;
} qcril_mmgsdi_command_callback_params_type;

/*! @brief GSDI Perso event callback parameters 
*/
typedef struct
{
  gsdi_perso_event_enum_type        evt;             
  gsdi_slot_id_type                 slot;              
  boolean                           additional_info_avail;                
  gsdi_perso_additional_info_type   info; 
} qcril_mmgsdi_perso_event_callback_params_type;

/*! @brief Typedef variables internal to module qcril_mmgsdi.c
*/
typedef struct
{
  mmgsdi_client_id_type                         client_id;
  qcril_mmgsdi_curr_pin_type                    curr_pin;
  qcril_mmgsdi_bool_enum_type                   icc_card;
  int                                           gw_curr_card_state;
  RIL_CardStatus_v6                             curr_card_status;
  boolean                                       gw_init_completed_recv;
  boolean                                       cdma_init_completed_recv;
  qcril_mmgsdi_fdn_status_enum_type             fdn_enable;
} qcril_mmgsdi_struct_type;

extern qcril_mmgsdi_struct_type qcril_mmgsdi;

/* to hold the perso state if received prior to other card events */
typedef struct
{
  int               sim_state;
  RIL_PersoSubstate perso_state;
} qcril_mmgsdi_temp_perso_info_type;

extern qcril_mmgsdi_temp_perso_info_type *perso_temp_info_ptr;

/*! @brief Type used for packing MMGSDI and UIM file enums */
typedef struct FileEnums
{
  mmgsdi_file_enum_type gsdi_enum;
  uim_items_enum_type   uim_enum;
} FileEnums;

/*! @brief Type used for the data tables defining contents of the EF mappings */
typedef struct EFMap
{
  unsigned int EF;              /*!< EF number */
  FileEnums    file;            /*!< Corresponding value in MMGSDI and UIM*/
} EFMap;

extern EFMap umts_ef_mapping[];
extern EFMap gsm_ef_mapping[];

extern uint32 umts_ef_map_size;
extern uint32 gsm_ef_map_size;


/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/
/*===========================================================================

  FUNCTION:  qcril_mmgsdi_convert_from_gsdi_status

===========================================================================*/
/*!
    @brief
    Convert gsdi into mmgsdi status.  Currently being used in Perso and FDN
    operations only.

    @return
    None.
*/
/*=========================================================================*/
mmgsdi_return_enum_type qcril_mmgsdi_convert_from_gsdi_status
(
  gsdi_returns_T gsdi_status
);


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_internal_verify_pin_command_callback

===========================================================================*/
/*!
    @brief
    Callback function for MMGSDI command for internal pin verification before
    subsequent command can be proceeded

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_internal_verify_pin_command_callback 
(
  mmgsdi_return_enum_type status,
  mmgsdi_cnf_enum_type    cnf,
  const mmgsdi_cnf_type  *cnf_ptr
);


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_command_callback

===========================================================================*/
/*!
    @brief
    General callback function for MMGSDI 

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_command_callback 
(
  mmgsdi_return_enum_type status,
  mmgsdi_cnf_enum_type    cnf,
  const mmgsdi_cnf_type  *cnf_ptr
);


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_perso_event_callback

===========================================================================*/
/*!
    @brief
    Convert gsdi into mmgsdi status.  Currently being used in Perso and FDN
    operations only.

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_perso_event_callback
(
  gsdi_perso_event_enum_type        evt,             
  gsdi_slot_id_type                 slot,              
  boolean                           additional_info_avail,                
  gsdi_perso_additional_info_type * info_ptr        
);


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_gsdi_command_callback

===========================================================================*/
/*!
    @brief
    General callback function for GSDI 

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_gsdi_command_callback 
(
  gsdi_cnf_T* gsdi_cnf_ptr
);


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_update_curr_card_state

===========================================================================*/
/*!
    @brief
    Update Curr Card State.
    Update Radio State if required
      Valid input parameters: QCRIL_SIM_STATE_ABSENT      
                              QCRIL_SIM_STATE_NOT_READY   
                              QCRIL_SIM_STATE_READY       
                              QCRIL_SIM_STATE_PIN         
                              QCRIL_SIM_STATE_PUK         
                              QCRIL_SIM_STATE_NETWORK_PERSONALIZATION 

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_update_curr_card_state
(
  int                              new_card_state,
  boolean                          gw_card_state_update,
  boolean                          cdma_card_state_update,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */  
);


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_set_curr_pin_ptr

===========================================================================*/
/*!
    @brief
    Set the Current Pin pointer 

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_set_curr_pin_ptr 
(
  mmgsdi_pin_enum_type pin_id
);


/*=========================================================================*/
/*! @brief Convert an array of binary bytes into a hex string.

    @note If memory allocation fails, will return NULL. Caller must be 
          able to deal with this case.

    @param data   Pointer to allocated area of bytes. Must not be NULL.
    @param length Length (in bytes) of the data pointed to by @a data
    @return       A NULL perminated string containing a hex ASCII
                  representation of the bytes in @a data.
 */
/*=========================================================================*/
char* bin_to_hexstring
(
 const uint8* const data, 
 const mmgsdi_len_type length
);


/*=========================================================================*/
/*! @brief Determine the length of a string up to a maximum, with no
    side effects.

    @pre @a str is not NULL, and should point to a NULL terminated string.
    @param str Pointer to the string to scan.
    @param max_len Maximum permissible length of string.
    @return Number of characters in string excluding NULL character, 
            or 0 if the string exceeds the maximum permitted length.
 */
/*=========================================================================*/
size_t safe_strlen(const char* const str, size_t max_len);

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_response

===========================================================================*/
void qcril_mmgsdi_response
(
  qcril_instance_id_e_type    instance_id,
  RIL_Token                   t,
  mmgsdi_return_enum_type     status,
  void*                       rsp_data,
  size_t                      rsp_length,
  boolean                     remove_entry,
  char*                       logstr
);

#define QCRIL_MMGSDI_RETURN_ERR_RSP_IF_INVALID_CLIENT(instance_id, client_id, token, remove_entry)  \
if(client_id == QCRIL_MMGSDI_INVALID_CLIENT_ID)                                                     \
{                                                                                                   \
  QCRIL_LOG_ERROR("%s", "Invalid QCRIL MMGSDI client id error\n");                                  \
  qcril_mmgsdi_response( instance_id, (RIL_Token)token, MMGSDI_ERROR, NULL,0, remove_entry, NULL);  \
  return;                                                                                           \
}

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_sec_process_cnf

===========================================================================*/
/*!
    @brief
    General callback function for MMGSDI security related operations

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_sec_process_cnf 
(
  qcril_instance_id_e_type    instance_id,
  mmgsdi_return_enum_type     status,                              
  mmgsdi_cnf_enum_type        cnf,                                    
  const mmgsdi_cnf_type       *cnf_ptr
);


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_sec_process_perso_deact_cnf

===========================================================================*/
/*!
    @brief
    Handle Perso Deactivation confirmation 

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_sec_process_perso_deact_cnf 
(
  qcril_instance_id_e_type    instance_id,
  gsdi_returns_T              status,
  uint32                      ril_token,
  gsdi_perso_enum_type        perso_feature,
  uint32                      num_retries
);


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_process_cnf

===========================================================================*/
/*!
    @brief
    General callback function for MMGSDI general operations

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_common_process_cnf 
(
  qcril_instance_id_e_type    instance_id,
  mmgsdi_return_enum_type     status,                              
  mmgsdi_cnf_enum_type        cnf,                                    
  const mmgsdi_cnf_type       *cnf_ptr
);


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_process_fdn_status_cnf

===========================================================================*/
/*!
    @brief
    Handle FDN enabling/disabling confirmation 

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_common_process_fdn_status_cnf 
(
  qcril_instance_id_e_type    instance_id,
  gsdi_returns_T              status,
  uint32                      ril_token
);

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_process_select_cnf

===========================================================================*/
/*!
    @brief
    Handle Select confirmation (for SIM IO Get Response Solicited command)

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_common_process_select_cnf 
(
  qcril_instance_id_e_type    instance_id,
  const gsdi_select_cnf_T*    select_rsp_ptr
);

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_init_maps

===========================================================================*/

void qcril_mmgsdi_init_maps();

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_simio_read_cnf

===========================================================================*/

void qcril_mmgsdi_common_simio_read_cnf
(
 const qcril_request_params_type* const req,
       qcril_request_return_type* const ret
);

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_simio_update_cnf

===========================================================================*/

void qcril_mmgsdi_common_simio_update_cnf
(
 const qcril_request_params_type* const req,
       qcril_request_return_type* const ret
);

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_simio_get_status_cnf

===========================================================================*/

void qcril_mmgsdi_common_simio_get_status_cnf
(
 const qcril_request_params_type* const req,
       qcril_request_return_type* const ret
);

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_simio_get_response_cnf

===========================================================================*/

void qcril_mmgsdi_common_simio_get_response_cnf
(
 const qcril_request_params_type* const req,
       qcril_request_return_type* const ret
);

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_proceed_sim_io_processing

===========================================================================*/
/*!
    @brief Callback processing for RIL_REQUEST_SIM_IO's internal 
    PIN2 verification

    @param req_ptr [in]  Request object containing raw data returned by MMGSDI.
    @param ret_ptr [out] Updated radio state.
*/
/*=========================================================================*/
void qcril_mmgsdi_common_proceed_internal_cmd_processing
(
 const qcril_request_params_type* const req_ptr,
       qcril_request_return_type* const ret_ptr
);

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_common_get_imsi_cnf

===========================================================================*/

void qcril_mmgsdi_common_get_imsi_cnf
(
 const qcril_request_params_type* const req,
       qcril_request_return_type* const ret
);

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_imsi_command_callback

===========================================================================*/

void qcril_mmgsdi_imsi_command_callback 
(
  mmgsdi_return_enum_type status,
  mmgsdi_cnf_enum_type    cnf,
  const mmgsdi_cnf_type  *cnf_ptr
);

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_print_byte_data

===========================================================================*/
/*!
    @brief
    Print the uint8 pointer structure 

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_print_byte_data 
(
  int32                   data_len,
  uint8                  *data_ptr
);
#endif /* FEATURE_CDMA_NON_RUIM*/

#endif /* QCRIL_MMGSDII_H */
