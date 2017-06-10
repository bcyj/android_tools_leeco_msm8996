/*!
  @file
  qcril_mmgsdi.c

  @brief
  Handles RIL requests for MMGSDI.

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

$Header: //linux/pkgs/proprietary/qc-ril/main/source/qcril_mmgsdi.c#16 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/28/11   at      Support for new ril.h params in RIL_UNSOL_SIM_REFRESH
11/29/10   at      Condition added for MMGSDI_INCOMPAT_PIN_STATUS
                   in qcril_mmgsdi_convert_mmgsdi_status()
09/30/10   js      Ignore MMGSDI_REFRESH_SUCCESS status for CARD PUP command
07/09/10   at      Merged branch changes to qcril_fusion folder
06/29/10   js      Put RIL card state to NOT READY on refresh reset
06/18/10   mib     Fixed GW index in case of multiple apps on the card
04/25/10   mib     PBM and CM indications on CARD_INIT_COMPLETED event
04/20/10   mib     Set FDN status in error cases
04/20/10   mib     Reset FDN status for INIT refresh
04/13/10   mib     Fixed app state when FDN is received from modem
04/12/10   mib     Added FDN not available state
04/07/10   mib     Added unsolicited response to FW after FDN confirmation
04/05/10   js      Removed support for GSDI_PERSO_PROP1 and GSDI_PERSO_PROP2
04/01/10   mib     Fixed race condition: send SIM ready after FDN status
03/11/10   mib     Moved illegal status from card to app
03/09/10   js      Support for providing number of perso retries in perso
                   deactivate response
03/01/10   fc      Re-architecture to support split modem.
02/01/10   mib     Wrong casting for MMGSDI events
12/22/09   mib     Register for refresh for call forward indicator files
12/03/09   js      Fixed illegal card processing
11/19/09   js      Added Support for illegal SIM
11/02/09   mib     Pass PIN data to framework also in case of error
11/02/09   js      Check for valid client before sending command to MMGSDI.
                   Pass null response callback for client id dereg.
10/24/09   js      QCRIL MMGSDI response function cleanup
10/22/09   js      Fixed perso event processing
10/12/09   js      Fixed pin response mapping to correct RIL type when PIN is
                   disabled
09/03/09   js      Fixed perso event callback processing
08/28/09   js      Fixed issue in processing of perso and card error event
                   Fix for sending unsolicited response for UICC files for
                   refresh event
07/20/09   js      Added support to allow PIN2 verification to be performed
                   prior to enable/disable FDN commands
07/18/09   pg      Added support for qcril_mmgsdi_process_internal_verify_pin_command_callback
                   1x non ruim build.
07/10/09   tml     Added support to handle intermediate step for PIN2
                   verification during SIM IO
06/26/09   fc      Fixed the issue of bogus RIL Request reported in call flow
                   log packet.
06/23/09   tml     Fixed perso event received too early issue
                   Fixed missing Mailbox Dialling number refresh issue
                   Fixed 6F00 invalid card notification issue
06/15/09   nd      Fixed compilation Issue.
06/01/09   sk      Changes for updating the emergency list based on
                   card events
05/18/09   fc      Changes to log debug messages to Diag directly instead
                   of through logcat.
05/14/09   pg      Mainlined FEATURE_MULTIMODE_ANDROID.
05/07/09   fc      Removed the overload system variable errno.
04/28/09   fc      Added support to perform card powerup/powerdown for
                   LPM to ONLINE or vice versa transition.
04/05/09   fc      Cleanup log macros.
03/19/09   tml     Fixed memory overwrite issue due to miscalculation of
                   pointer location
02/06/09   tml     reconstruct UICC select response into ICC select response
                   map ICC EF ID into corresponding USIM EF enum
01/19/09   pg      Corrected RIL request id in RIL response for 1x non ruim build.
01/09/09   tml     Featurize for 1x non ruim build
12/17/08   tml     Added refresh support.  Replaced client_id_is_valid with
                   client_id validity checking.  Added support to track
                   card protocol type.  Performed malloc for the
                   cnf and evt received from MMGSDI to avoid using the
                   pointer values from MMGSDI.
12/16/08   fc      Changes to support the release of AMSS MMGSDI object
                   for ONCRPC.
12/10/08   tml     Enable no card event to be sent when it is the first
                   card event received
10/08/08   tml     Added multimode support
08/04/08   tml     Populate pin info when received event init completed
07/30/08   tml     Added SIM IO Get response support
07/22/08   tml     Fixed compilation on target
07/14/08   tml     Clean up message levels
06/11/08   jod     Added support for GET IMSI
06/04/08   jod     Fixed qcril_mmgsdi_comand_callback() to pass RIL_Token.
05/28/08   jod     Add SIM IO support
05/28/08   tml     Add perso and FDN supports
05/20/08   tml     Added event support and power down/up
05/19/08   tml     initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#if !defined (FEATURE_QCRIL_UIM_QMI)

#include "ril.h"
#include "IxErrno.h"
#include "qcrili.h"
#include "qcril_log.h"
#include "qcril_reqlist.h"
#ifndef FEATURE_CDMA_NON_RUIM
#include "qcril_mmgsdii.h"
#endif /* !FEATURE_CDMA_NON_RUIM */
#include <string.h>

/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/

#ifndef FEATURE_CDMA_NON_RUIM
qcril_mmgsdi_struct_type qcril_mmgsdi;

qcril_mmgsdi_temp_perso_info_type *perso_temp_info_ptr = NULL;

#define QCRIL_MMGSDI_WORD_SIZE 4

#define QCRIL_MMGSDI_GW_UPDATE    0x1
#define QCRIL_MMGSDI_CDMA_UPDATE  0x2

#define QCRIL_MMGSDI_FDN_AVAILABLE_IN_ICC    0x10
#define QCRIL_MMGSDI_FDN_AVAILABLE_IN_UICC   0x02

/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES

===========================================================================*/


/*===========================================================================

                               INTERNAL FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_convert_mmgsdi_status

===========================================================================*/
/*!
    @brief
    Convert MMGSDI status into RIL status

    @return
    None.
*/
/*=========================================================================*/
static RIL_Errno qcril_mmgsdi_convert_mmgsdi_status
(
  mmgsdi_return_enum_type status
)
{

  switch(status)
  {
  case MMGSDI_SUCCESS:
    QCRIL_LOG_DEBUG( "mmgsdi status 0x%x -> RIL_E_SUCCESS\n", status);
    return RIL_E_SUCCESS;

  case MMGSDI_INCORRECT_CODE:
    QCRIL_LOG_DEBUG( "mmgsdi status 0x%x -> RIL_E_PASSWORD_INCORRECT\n", status);
    return RIL_E_PASSWORD_INCORRECT;

  case MMGSDI_ACCESS_DENIED:
    /* Assume that PIN1 accessed required access will be received only after
       pin1 verification. */
    if((qcril_mmgsdi.curr_pin.pin_id == MMGSDI_PIN1) &&
       (qcril_mmgsdi.curr_card_status.applications[qcril_mmgsdi.curr_pin.app_index].pin1
        == RIL_PINSTATE_DISABLED))
    {
      QCRIL_LOG_DEBUG( "mmgsdi status 0x%x -> RIL_E_REQUEST_NOT_SUPPORTED\n", status);
      return RIL_E_REQUEST_NOT_SUPPORTED;
    }
    else
    {
      QCRIL_LOG_DEBUG( "mmgsdi status 0x%x -> RIL_E_SIM_PIN2\n", status);
      return RIL_E_SIM_PIN2;
    }

  case MMGSDI_CODE_BLOCKED:
    if ((qcril_mmgsdi.curr_pin.valid) &&
        (qcril_mmgsdi.curr_pin.pin_id == MMGSDI_PIN1))
    {
      // TODO Check latest ril.h for the error code for PUK
//      QCRIL_MMGSDI_DEBUG("mmgsdi status 0x%x -> RIL_E_SIM_PUK\n", status);
//      return RIL_E_SIM_PUK;
      return RIL_E_PASSWORD_INCORRECT;
    }
    else if ((qcril_mmgsdi.curr_pin.valid) &&
             (qcril_mmgsdi.curr_pin.pin_id == MMGSDI_PIN2))
    {
      QCRIL_LOG_DEBUG( "mmgsdi status 0x%x -> RIL_E_SIM_PUK2\n", status);
      return RIL_E_SIM_PUK2;
    }
    else
    {
      QCRIL_LOG_DEBUG( "mmgsdi status 0x%x -> RIL_E_PASSWORD_INCORRECT\n", status);
      return RIL_E_PASSWORD_INCORRECT;
    }

  case MMGSDI_NOT_SUPPORTED:
    QCRIL_LOG_DEBUG( "mmgsdi status 0x%x -> RIL_E_REQUEST_NOT_SUPPORTED\n", status);
    return RIL_E_REQUEST_NOT_SUPPORTED;

  case MMGSDI_INCOMPAT_PIN_STATUS:
    /* Assume that PIN1 accessed required access will be received only after
       pin1 verification. */
    if((qcril_mmgsdi.curr_pin.pin_id == MMGSDI_PIN1) &&
       (qcril_mmgsdi.curr_card_status.applications[qcril_mmgsdi.curr_pin.app_index].pin1
        == RIL_PINSTATE_DISABLED))
    {
      QCRIL_LOG_DEBUG( "mmgsdi status 0x%x -> RIL_E_REQUEST_NOT_SUPPORTED\n", status);
      return RIL_E_REQUEST_NOT_SUPPORTED;
    }

    QCRIL_LOG_DEBUG( "mmgsdi status 0x%x -> RIL_E_GENERIC_FAILURE\n", status);
    return RIL_E_GENERIC_FAILURE;

  default:
    QCRIL_LOG_DEBUG( "mmgsdi status 0x%x -> RIL_E_GENERIC_FAILURE\n", status);
    return RIL_E_GENERIC_FAILURE;
  }
} /* qcril_mmgsdi_convert_mmgsdi_status() */


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
)
{
  switch(gsdi_status)
  {
  case GSDI_SUCCESS:
    QCRIL_LOG_DEBUG( "%s", "GSDI_SUCCESS -> MMGSDI_SUCCESS\n");
    return MMGSDI_SUCCESS;

  case GSDI_ACCESS_DENIED:
    QCRIL_LOG_DEBUG( "%s", "GSDI_ACCESS_DENIED -> MMGSDI_ACCESS_DENIED\n");
    return MMGSDI_ACCESS_DENIED;

  case GSDI_INCORRECT_CODE:
  case GSDI_PERSO_CHECK_FAILED:
  case GSDI_PERSO_INVALID_CK:
    QCRIL_LOG_DEBUG( "GSDI_INCORRECT_CODE/PERSO_CHECK_FAILEd/PERSO_INVALID_CK 0x%x -> MMGSDI_INCORRECT_CODE\n",
      gsdi_status);
    return MMGSDI_INCORRECT_CODE;

  case GSDI_CODE_BLOCKED:
  case GSDI_PERSO_CK_BLOCKED:
    QCRIL_LOG_DEBUG( "GSDI_CODE_BLOCKED/PERSO_CK_BLOCKED 0x%x -> MMGSDI_CODE_BLOCKED\n",
      gsdi_status);
    return MMGSDI_CODE_BLOCKED;

  case GSDI_NOT_SUPPORTED:
    QCRIL_LOG_DEBUG( "%s", "GSDI_NOT_SUPPORTED -> MMGSDI_NOT_SUPPORTED\n");
    return MMGSDI_NOT_SUPPORTED;

  default:
    QCRIL_LOG_DEBUG( "gsdi status 0x%x -> MMGSDI_ERROR \n", gsdi_status);
    return MMGSDI_ERROR;
  }
} /* qcril_mmgsdi_convert_from_gsdi_status() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_cal_align_size

===========================================================================*/
/*!
    @brief
    Take the input size and calculate the corresponding word aligned size.

    @return
    *aligned_size_ptr - The aligned data size.
*/
/*=========================================================================*/
void qcril_mmgsdi_cal_align_size(
  int                orig_size,
  int               *aligned_size_ptr
)
{
  int32      mod = 0;

  if (aligned_size_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "qcril_mmgsdi_cal_align_size null aligned_size_ptr");
    return;
  }

  mod = orig_size % QCRIL_MMGSDI_WORD_SIZE;
  if (mod == 0)
    *aligned_size_ptr = orig_size;
  else
    *aligned_size_ptr = orig_size + (QCRIL_MMGSDI_WORD_SIZE - mod);
} /* qcril_mmgsdi_cal_align_size*/


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_alloc_and_copy_mmgsdi_get_file_attr_cnf

===========================================================================*/
/*!
    @brief
    Allocate 1 continuous memory for a mmgsdi_cnf_type based on the
    different confirmation types.  Ensure any internal pointer in the
    structure to be word aligned.  Finally, the function will copy the data
    to the newly allocated memory space.

    @return
    qcril_mmgsdi_command_callback_params_type* - The newly allocated and
                                                 populated structure.
*/
/*=========================================================================*/
qcril_mmgsdi_command_callback_params_type* qcril_mmgsdi_alloc_and_copy_mmgsdi_get_file_attr_cnf(
  mmgsdi_return_enum_type status,
  mmgsdi_cnf_enum_type    cnf,
  const mmgsdi_cnf_type  *cnf_ptr,
  int                    *total_size_ptr)
{

  qcril_mmgsdi_command_callback_params_type *out_ptr              = NULL;
  mmgsdi_file_security_access_type          *file_access_ptr      = NULL;
  mmgsdi_file_security_access_type          *out_file_access_ptr  = NULL;
  int                                        add_size             = 0;
  int                                        aligned_add_size     = 0;

  if (total_size_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "data_ptr NULL in %s!", __FUNCTION__);
    return NULL;
  }

  if (cnf_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "cnf_ptr NULL in %s!", __FUNCTION__);
    *total_size_ptr = 0;
    return NULL;
  }

  /* mmgsdi_cnf_type and security pin enum ptr as additional sizes , ignoring access path
       ptr because we do not need this to be sent back to application */

  add_size = (int)sizeof(mmgsdi_cnf_type);
  qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
  *total_size_ptr += aligned_add_size;

  switch (cnf_ptr->get_file_attr_cnf.file_attrib.file_type)
  {
  case MMGSDI_LINEAR_FIXED_FILE:
    file_access_ptr =
      (mmgsdi_file_security_access_type*)&cnf_ptr->get_file_attr_cnf.file_attrib.file_info.linear_fixed_file.file_security;
    break;
  case MMGSDI_CYCLIC_FILE:
    file_access_ptr =
      (mmgsdi_file_security_access_type*)&cnf_ptr->get_file_attr_cnf.file_attrib.file_info.cyclic_file.file_security;
    break;
  case MMGSDI_TRANSPARENT_FILE:
    file_access_ptr =
      (mmgsdi_file_security_access_type*)&cnf_ptr->get_file_attr_cnf.file_attrib.file_info.transparent_file.file_security;
    break;
  default:
    QCRIL_LOG_ERROR( "Unhandle file_type for GET_FILE_ATTR_CNF 0x%x in qcril_mmgsdi_alloc_and_copy_mmgsdi_cnf!",
                            cnf_ptr->get_file_attr_cnf.file_attrib.file_type);
    QCRIL_LOG_ERROR( "%s", "Potential Data Abort of invalid pointer access!  Please check!!!!");
    *total_size_ptr = 0;
    return out_ptr;
  }

  /* read access pin enum */
  add_size = (int)(sizeof(mmgsdi_pin_enum_type) * file_access_ptr->read.num_protection_pin);
  qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
  *total_size_ptr += aligned_add_size;

  /* write access pin enum */
  add_size = (int)(sizeof(mmgsdi_pin_enum_type) * file_access_ptr->write.num_protection_pin);
  qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
  *total_size_ptr += aligned_add_size;

  /* increase access pin enum */
  add_size = (int)(sizeof(mmgsdi_pin_enum_type) * file_access_ptr->increase.num_protection_pin);
  qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
  *total_size_ptr += aligned_add_size;

  /* invalidate_deactivate access pin enum */
  add_size = (int)(sizeof(mmgsdi_pin_enum_type) * file_access_ptr->invalidate_deactivate.num_protection_pin);
  qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
  *total_size_ptr += aligned_add_size;

  /* rehabilitate_activate access pin enum */
  add_size = (int)(sizeof(mmgsdi_pin_enum_type) * file_access_ptr->rehabilitate_activate.num_protection_pin);
  qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
  *total_size_ptr += aligned_add_size;

  /* Allocate total size */
  out_ptr = malloc(*total_size_ptr);
  if (out_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "out_ptr alloc failed");
    *total_size_ptr = 0;
    return NULL;
  }

  /* Align pointers to word boundary */
  /* Calculate previous structure aligned size */
  add_size = (int)sizeof(qcril_mmgsdi_command_callback_params_type);
  qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
  out_ptr->cnf_ptr = (mmgsdi_cnf_type*)((uint8*)out_ptr + aligned_add_size);

  /* Move to include the size of the cnf_ptr */
  aligned_add_size += sizeof(mmgsdi_cnf_type);

  /* Copy data */
  out_ptr->cnf = cnf;
  out_ptr->status = status;
  memcpy(out_ptr->cnf_ptr, cnf_ptr, sizeof(mmgsdi_cnf_type));

  switch (cnf_ptr->get_file_attr_cnf.file_attrib.file_type)
  {
  case MMGSDI_LINEAR_FIXED_FILE:
    out_file_access_ptr = &out_ptr->cnf_ptr->get_file_attr_cnf.file_attrib.file_info.linear_fixed_file.file_security;
    break;
  case MMGSDI_CYCLIC_FILE:
    out_file_access_ptr = &out_ptr->cnf_ptr->get_file_attr_cnf.file_attrib.file_info.cyclic_file.file_security;
    break;
  case MMGSDI_TRANSPARENT_FILE:
    out_file_access_ptr = &out_ptr->cnf_ptr->get_file_attr_cnf.file_attrib.file_info.transparent_file.file_security;
    break;
  default:
    QCRIL_LOG_ERROR( "Unhandle file_type for GET_FILE_ATTR_CNF 0x%x in qcril_mmgsdi_alloc_and_copy_mmgsdi_cnf!",
                     cnf_ptr->get_file_attr_cnf.file_attrib.file_type);
    QCRIL_LOG_ERROR( "%s", "Potential Data Abort of invalid pointer access!  Please check!!!!");
    *total_size_ptr = 0;
    free(out_ptr);
    out_ptr = NULL;
    return out_ptr;
  }

  /* Align read security pin enum ptr to word boundary */
  if (file_access_ptr->read.num_protection_pin > 0)
  {
    out_file_access_ptr->read.protection_pin_ptr =
      (mmgsdi_pin_enum_type*)(((uint8*)out_ptr) + aligned_add_size);

    /* Copy data */
    memcpy(out_file_access_ptr->read.protection_pin_ptr,
           file_access_ptr->read.protection_pin_ptr,
           (sizeof(mmgsdi_pin_enum_type) *
            file_access_ptr->read.num_protection_pin));

    /* Move to include the size for the read protection pin */
    aligned_add_size += (int)(sizeof(mmgsdi_pin_enum_type) *
                    file_access_ptr->read.num_protection_pin);
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
  }
  else
  {
    out_file_access_ptr->read.protection_pin_ptr = NULL;
  }

  /* Align write security pin enum ptr to word boundary */
  if (file_access_ptr->write.num_protection_pin > 0)
  {
    out_file_access_ptr->write.protection_pin_ptr =
      (mmgsdi_pin_enum_type*)(((uint8*)out_ptr) + aligned_add_size);

    /* Copy data */
    memcpy(out_file_access_ptr->write.protection_pin_ptr,
           file_access_ptr->write.protection_pin_ptr,
           (sizeof(mmgsdi_pin_enum_type) *
            file_access_ptr->write.num_protection_pin));

    /* Move to include the size for the write protection pin */
    aligned_add_size += (int)(sizeof(mmgsdi_pin_enum_type) *
                    file_access_ptr->write.num_protection_pin);
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
  }
  else
  {
    out_file_access_ptr->write.protection_pin_ptr = NULL;
  }

  /* Align increase security pin enum ptr to word boundary */
  if (file_access_ptr->increase.num_protection_pin > 0)
  {
    out_file_access_ptr->increase.protection_pin_ptr =
      (mmgsdi_pin_enum_type*)(((uint8*)out_ptr) + aligned_add_size);

    /* Copy data */
    memcpy(out_file_access_ptr->increase.protection_pin_ptr,
           file_access_ptr->increase.protection_pin_ptr,
           (sizeof(mmgsdi_pin_enum_type) *
            file_access_ptr->increase.num_protection_pin));

    /* Move to include the size for the increase protection pin */
    aligned_add_size += (int)(sizeof(mmgsdi_pin_enum_type) *
                    file_access_ptr->increase.num_protection_pin);
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
  }
  else
  {
    out_file_access_ptr->increase.protection_pin_ptr = NULL;
  }

  /* Align invalidate_deactivate security pin enum ptr to word boundary */
  if (file_access_ptr->invalidate_deactivate.num_protection_pin > 0)
  {
    out_file_access_ptr->invalidate_deactivate.protection_pin_ptr =
      (mmgsdi_pin_enum_type*)(((uint8*)out_ptr) + aligned_add_size);

    /* Copy data */
    memcpy(out_file_access_ptr->invalidate_deactivate.protection_pin_ptr,
           file_access_ptr->invalidate_deactivate.protection_pin_ptr,
           (sizeof(mmgsdi_pin_enum_type) *
          file_access_ptr->invalidate_deactivate.num_protection_pin));

    /* Move to include the size for the invalidate_deactivate protection pin */
    aligned_add_size += (int)(sizeof(mmgsdi_pin_enum_type) *
                    file_access_ptr->invalidate_deactivate.num_protection_pin);
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
  }
  else
  {
    out_file_access_ptr->invalidate_deactivate.protection_pin_ptr = NULL;
  }

  /* Align rehabilitate_activate security pin enum ptr to word boundary */
  if (file_access_ptr->rehabilitate_activate.num_protection_pin > 0)
  {
    out_file_access_ptr->rehabilitate_activate.protection_pin_ptr =
      (mmgsdi_pin_enum_type*)(((uint8*)out_ptr) + aligned_add_size);

    /* Copy data */
    memcpy(out_file_access_ptr->rehabilitate_activate.protection_pin_ptr,
           file_access_ptr->rehabilitate_activate.protection_pin_ptr,
           (sizeof(mmgsdi_pin_enum_type) *
            file_access_ptr->rehabilitate_activate.num_protection_pin));

    /* Move to include the size for the rehabilitate_activate protection pin */
    aligned_add_size += (int)(sizeof(mmgsdi_pin_enum_type) *
                    file_access_ptr->rehabilitate_activate.num_protection_pin);
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
  }
  else
  {
    out_file_access_ptr->invalidate_deactivate.protection_pin_ptr = NULL;
  }

  return out_ptr;
} /* qcril_mmgsdi_alloc_and_copy_mmgsdi_get_file_attr_cnf */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_alloc_and_copy_mmgsdi_cnf

===========================================================================*/
/*!
    @brief
    Allocate 1 continuous memory for a mmgsdi_cnf_type based on the
    different confirmation types.  Ensure any internal pointer in the
    structure to be word aligned.  Finally, the function will copy the data
    to the newly allocated memory space.

    @return
    qcril_mmgsdi_command_callback_params_type* - The newly allocated and
                                                 populated structure.
*/
/*=========================================================================*/
qcril_mmgsdi_command_callback_params_type* qcril_mmgsdi_alloc_and_copy_mmgsdi_cnf(
  mmgsdi_return_enum_type status,
  mmgsdi_cnf_enum_type    cnf,
  const mmgsdi_cnf_type  *cnf_ptr,
  int                    *total_size_ptr)
{
  qcril_mmgsdi_command_callback_params_type *out_ptr              = NULL;
  int                                        add_size             = 0;
  int                                        aligned_add_size     = 0;

  if (total_size_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "data_ptr NULL in %s!", __FUNCTION__);
    return NULL;
  }

  /* basic size of the structure */
  /* align the size of the basic structure to word boundary */
  qcril_mmgsdi_cal_align_size(
    (int)sizeof(qcril_mmgsdi_command_callback_params_type), total_size_ptr);

  if (cnf_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "cnf_ptr NULL in %s!", __FUNCTION__);
    *total_size_ptr = 0;
    return NULL;
  }

  /* calculate additional size and alloc and copy */
  switch(cnf)
  {
  case MMGSDI_CLIENT_ID_REG_CNF:
  case MMGSDI_CLIENT_EVT_REG_CNF:
  case MMGSDI_CARD_PDOWN_CNF:
  case MMGSDI_CARD_PUP_CNF:
  case MMGSDI_REFRESH_CNF:
  case MMGSDI_PIN_OPERATION_CNF:
  case MMGSDI_WRITE_CNF:
    /* Only mmgsdi_cnf_type as additional size */
    add_size = (int)sizeof(mmgsdi_cnf_type);
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
    *total_size_ptr += aligned_add_size;

    /* Allocate total size */
    out_ptr = malloc(*total_size_ptr);
    if (out_ptr == NULL)
    {
      QCRIL_LOG_ERROR( "%s", "out_ptr alloc failed");
      *total_size_ptr = 0;
      return NULL;
    }

    /* Align pointer to word boundary */
    /* Calculate previous structure aligned size */
    add_size = (int)sizeof(qcril_mmgsdi_command_callback_params_type);
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
    out_ptr->cnf_ptr = (mmgsdi_cnf_type*)((uint8*)out_ptr + aligned_add_size);

    /* Copy data */
    out_ptr->cnf = cnf;
    out_ptr->status = status;
    memcpy(out_ptr->cnf_ptr, cnf_ptr, sizeof(mmgsdi_cnf_type));
    break;

  case MMGSDI_READ_CNF:
    /* mmgsdi_cnf_type and read_data length as additional sizes */
    add_size = (int)sizeof(mmgsdi_cnf_type);
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
    *total_size_ptr += aligned_add_size;

    add_size = (int)cnf_ptr->read_cnf.read_data.data_len;
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
    *total_size_ptr += aligned_add_size;

    /* Allocate total size */
    out_ptr = malloc(*total_size_ptr);
    if (out_ptr == NULL)
    {
      QCRIL_LOG_ERROR( "%s", "out_ptr alloc failed");
      *total_size_ptr = 0;
      return NULL;
    }

    /* Align pointers to word boundary */
    /* Calculate previous structure aligned size */
    add_size = (int)sizeof(qcril_mmgsdi_command_callback_params_type);
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
    out_ptr->cnf_ptr = (mmgsdi_cnf_type*)((uint8*)out_ptr + aligned_add_size);

    /* Copy data */
    out_ptr->cnf = cnf;
    out_ptr->status = status;
    memcpy(out_ptr->cnf_ptr, cnf_ptr, sizeof(mmgsdi_cnf_type));

    /* Align read data ptr to word boundary */
    add_size = aligned_add_size + (int)sizeof(mmgsdi_cnf_type);
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
    out_ptr->cnf_ptr->read_cnf.read_data.data_ptr = (uint8*)out_ptr + aligned_add_size;

    /* Copy data */
    memcpy(out_ptr->cnf_ptr->read_cnf.read_data.data_ptr,
           cnf_ptr->read_cnf.read_data.data_ptr,
           cnf_ptr->read_cnf.read_data.data_len);
    break;

  case MMGSDI_CARD_STATUS_CNF:
    /* mmgsdi_cnf_type and status_data length as additional sizes */
    add_size = (int)sizeof(mmgsdi_cnf_type);
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
    *total_size_ptr += aligned_add_size;

    add_size = (int)cnf_ptr->status_cnf.status_data.data_len;
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
    *total_size_ptr += aligned_add_size;

    /* Allocate total size */
    out_ptr = malloc(*total_size_ptr);
    if (out_ptr == NULL)
    {
      QCRIL_LOG_ERROR( "%s", "out_ptr alloc failed");
      *total_size_ptr = 0;
      return NULL;
    }

    /* Align pointers to word boundary */
    /* Calculate previous structure aligned size */
    add_size = (int)sizeof(qcril_mmgsdi_command_callback_params_type);
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
    out_ptr->cnf_ptr = (mmgsdi_cnf_type*)((uint8*)out_ptr + aligned_add_size);

    /* Copy data */
    out_ptr->cnf = cnf;
    out_ptr->status = status;
    memcpy(out_ptr->cnf_ptr, cnf_ptr, sizeof(mmgsdi_cnf_type));

    /* Align status data ptr to word boundary */
    add_size = aligned_add_size + (int)sizeof(mmgsdi_cnf_type);
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
    out_ptr->cnf_ptr->status_cnf.status_data.data_ptr = (uint8*)out_ptr + aligned_add_size;

    /* Copy data */
    memcpy(out_ptr->cnf_ptr->status_cnf.status_data.data_ptr,
           cnf_ptr->status_cnf.status_data.data_ptr,
           cnf_ptr->status_cnf.status_data.data_len);
    break;

  case MMGSDI_GET_FILE_ATTR_CNF:
    out_ptr = qcril_mmgsdi_alloc_and_copy_mmgsdi_get_file_attr_cnf(
      status, cnf, cnf_ptr, total_size_ptr);
    break;

  default:
    /* All CNF types that are not handled currently.
       Once a new cnf type has been identified and handled,
       it should be removed from this default case to avoid the message error below */
    QCRIL_LOG_ERROR( "Unhandle mmgsdi_cnf_type 0x%x in %s!", cnf, __FUNCTION__);
    QCRIL_LOG_ERROR( "%s", "Potential Data Abort of invalid pointer access!  Please check!!!!");
    *total_size_ptr = 0;
    break;
  }
  return out_ptr;
} /* qcril_mmgsdi_alloc_and_copy_mmgsdi_cnf*/


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_alloc_and_copy_mmgsdi_evt

===========================================================================*/
/*!
    @brief
    Allocate 1 continuous memory for a mmgsdi_event_data_type based on the
    different event types.  Ensure any internal pointer in the
    structure to be word aligned.  Finally, the function will copy the data
    to the newly allocated memory space.

    @return
    mmgsdi_event_data_type* - The newly allocated and populated structure.
*/
/*=========================================================================*/
mmgsdi_event_data_type * qcril_mmgsdi_alloc_and_copy_mmgsdi_evt(
  const mmgsdi_event_data_type  *evt_ptr,
  int                           *total_size_ptr)
{
  mmgsdi_event_data_type      *out_ptr          = NULL;
  int                          add_size         = 0;
  int                          aligned_add_size = 0;

  if (total_size_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "data_ptr NULL in %s!", __FUNCTION__);
    return NULL;
  }

  if (evt_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "evt_ptr NULL in %s!", __FUNCTION__);
    *total_size_ptr = 0;
    return NULL;
  }

  /* basic size of the structure */
  /* align the size of the basic structure to word boundary */
  qcril_mmgsdi_cal_align_size(
    (int)sizeof(mmgsdi_event_data_type), total_size_ptr);

  /* calculate additional size and alloc and copy */
  switch(evt_ptr->evt)
  {
  case MMGSDI_TERMINAL_PROFILE_DL_EVT:
  case MMGSDI_FDN_EVT:
  case MMGSDI_SWITCH_SLOT_EVT:
  case MMGSDI_CARD_INIT_COMPLETED_EVT:
  case MMGSDI_CARD_INSERTED_EVT:
  case MMGSDI_CARD_ERROR_EVT:
  case MMGSDI_CARD_REMOVED_EVT:
  case MMGSDI_ILLEGAL_CARD_EVT:
  case MMGSDI_SELECT_AID_EVT:
  case MMGSDI_PIN1_EVT:
  case MMGSDI_PIN2_EVT:
  case MMGSDI_UNIVERSAL_PIN_EVT:
    /* No additional size */
    /* Allocate total size */
    out_ptr = malloc(*total_size_ptr);
    if (out_ptr == NULL)
    {
      QCRIL_LOG_ERROR( "%s", "out_ptr alloc failed");
      *total_size_ptr = 0;
      return NULL;
    }

    /* Copy data */
    memcpy(out_ptr, evt_ptr, sizeof(mmgsdi_event_data_type));
    break;

  case MMGSDI_REFRESH_EVT:
    /* mmgsdi_refresh_file_list_type's file_list_ptr as potential additional size */
    add_size = (int)(evt_ptr->data.refresh.refresh_files.num_files *
                     sizeof(mmgsdi_file_enum_type));
    qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
    *total_size_ptr += aligned_add_size;

    /* Allocate total size */
    out_ptr = malloc(*total_size_ptr);
    if (out_ptr == NULL)
    {
      QCRIL_LOG_ERROR( "%s", "out_ptr alloc failed");
      *total_size_ptr = 0;
      return NULL;
    }

    /* Copy overall event data */
    memcpy(out_ptr, evt_ptr, sizeof(mmgsdi_event_data_type));

    /* Align pointers to word boundary */
    /* Calculate previous structure aligned size if required*/
    if (evt_ptr->data.refresh.refresh_files.num_files > 0)
    {
      add_size = (int)sizeof(mmgsdi_event_data_type);
      qcril_mmgsdi_cal_align_size(add_size, &aligned_add_size);
      out_ptr->data.refresh.refresh_files.file_list_ptr =
        (mmgsdi_file_enum_type*)((uint8*)out_ptr + aligned_add_size);

      /* Copy data */
      memcpy(out_ptr->data.refresh.refresh_files.file_list_ptr,
        evt_ptr->data.refresh.refresh_files.file_list_ptr,
        evt_ptr->data.refresh.refresh_files.num_files * sizeof(mmgsdi_file_enum_type));
    }
    break;

  default:
    /* All EVT DATA types that are not handled currently.
       Once a new evt type has been identified and handled,
       it should be removed from this default case to avoid the message error below */
    QCRIL_LOG_ERROR( "Unhandle mmgsdi_event_enum_type 0x%x in %s!", evt_ptr->evt, __FUNCTION__);
    QCRIL_LOG_ERROR( "%s", "Potential Data Abort of invalid pointer access!  Please check!!!!");
    *total_size_ptr = 0;
    break;
  }
  return out_ptr;
} /* qcril_mmgsdi_alloc_and_copy_mmgsdi_evt*/


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_alloc_and_copy_gsdi_cnf

===========================================================================*/
/*!
    @brief
    Allocate 1 continuous memory for a gsdi_cnf_T based on the
    different confirmation types.  Ensure any internal pointer in the
    structure to be word aligned.  Finally, the function will copy the data
    to the newly allocated memory space.

    @return
    gsdi_cnf_T* - The newly allocated and populated structure.
*/
/*=========================================================================*/
gsdi_cnf_T * qcril_mmgsdi_alloc_and_copy_gsdi_cnf(
  const gsdi_cnf_T    *cnf_ptr,
  int                 *total_size_ptr)
{
  gsdi_cnf_T        *out_ptr          = NULL;
  int                add_size         = 0;
  int                aligned_add_size = 0;

  if (total_size_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "data_ptr NULL in %s!", __FUNCTION__);
    return NULL;
  }

  if (cnf_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "cnf_ptr NULL in %s!", __FUNCTION__);
    *total_size_ptr = 0;
    return NULL;
  }

  /* basic size of the structure */
  /* align the size of the basic structure to word boundary */
  qcril_mmgsdi_cal_align_size(
    (int)sizeof(gsdi_cnf_T), total_size_ptr);

  /* calculate additional size and alloc and copy */
  switch(cnf_ptr->message_header.resp_type)
  {
  case GSDI_GET_SIM_CAPABILITIES_RSP:
  case GSDI_ENABLE_FDN_RSP:
  case GSDI_DISABLE_FDN_RSP:
  case GSDI_PERSO_REG_TASK_RSP:
  case GSDI_SELECT_RSP:
  case GSDI_PERSO_DEACT_IND_RSP:
    /* No additional size */
    /* Allocate total size */
    out_ptr = malloc(*total_size_ptr);
    if (out_ptr == NULL)
    {
      QCRIL_LOG_ERROR( "%s", "out_ptr alloc failed");
      *total_size_ptr = 0;
      return NULL;
    }

    /* Copy data */
    memcpy(out_ptr, cnf_ptr, sizeof(gsdi_cnf_T));
    break;

  default:
    /* All GSDI Cnf types that are not handled currently.
       Once a new cnf type has been identified and handled,
       it should be removed from this default case to avoid the message error below */
    QCRIL_LOG_ERROR( "Unhandle gsdi cnf message_id 0x%x in %s!", cnf_ptr->message_header.resp_type, __FUNCTION__);
    QCRIL_LOG_ERROR( "%s", "Potential Data Abort of invalid pointer access!  Please check!!!!");
    *total_size_ptr = 0;
    break;
  }
  return out_ptr;
} /* qcril_mmgsdi_alloc_and_copy_gsdi_cnf*/


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
)
{
  /*-----------------------------------------------------------------------*/

  qcril_mmgsdi_command_callback_params_type  *cmd_cb_params_ptr      = NULL;
  int                                         cmd_cb_params_tot_size = 0;
  qcril_mmgsdi_internal_sim_data_type        *internal_data_ptr      = NULL;

  QCRIL_LOG_INFO( "%s: cnf = 0x%x, status = 0x%x\n",
    __FUNCTION__, cnf, status);

  cmd_cb_params_ptr = qcril_mmgsdi_alloc_and_copy_mmgsdi_cnf(status,
                                                             cnf,
                                                             cnf_ptr,
                                                             &cmd_cb_params_tot_size);

  if ( cmd_cb_params_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s, Null cmd_cb_params_ptr\n", __FUNCTION__);
    return;
  }
  if ( cmd_cb_params_tot_size == 0 )
  {
    QCRIL_LOG_ERROR( "%s, Zero size cmd_cb_params_ptr\n", __FUNCTION__);
    free( cmd_cb_params_ptr );
    return;
  }

  if(cmd_cb_params_ptr->cnf_ptr != NULL)
  {
    internal_data_ptr =
     (qcril_mmgsdi_internal_sim_data_type*)cmd_cb_params_ptr->cnf_ptr->response_header.client_data;
  }
  if (internal_data_ptr == NULL)
  {
    free(cmd_cb_params_ptr);
    QCRIL_LOG_ERROR( "%s, Null internal_data_ptr\n", __FUNCTION__);
    return;
  }

  QCRIL_LOG_INFO( "%s qcril_event_queue\n", __FUNCTION__);
  qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, QCRIL_DATA_NOT_ON_STACK,
                     QCRIL_EVT_MMGSDI_INTERNAL_VERIFY_PIN_COMMAND_CALLBACK,
                     (void *) cmd_cb_params_ptr, cmd_cb_params_tot_size, (void*) internal_data_ptr->token );

  /* cmd_cb_params_ptr will be freed upon finish processing the event in
     qcril_mmgsdi_process_internal_verify_pin_command_callback */
} /* qcril_mmgsdi_internal_verify_pin_command_callback() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_internal_read_ust_callback

===========================================================================*/
/*!
    @brief
    Callback function for MMGSDI command for internal read of UST or SST
    file to verify if the FDN service is available

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_internal_read_ust_callback
(
  mmgsdi_return_enum_type status,
  mmgsdi_cnf_enum_type    cnf,
  const mmgsdi_cnf_type  *cnf_ptr
)
{
  qcril_mmgsdi_command_callback_params_type  *cmd_cb_params_ptr      = NULL;
  int                                         cmd_cb_params_tot_size = 0;

  QCRIL_LOG_INFO( "%s: cnf = 0x%x, status = 0x%x\n",
    __FUNCTION__, cnf, status);

  cmd_cb_params_ptr = qcril_mmgsdi_alloc_and_copy_mmgsdi_cnf(status,
                                                             cnf,
                                                             cnf_ptr,
                                                             &cmd_cb_params_tot_size);
  if ( cmd_cb_params_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s, Null cmd_cb_params_ptr\n", __FUNCTION__);
    return;
  }
  if ( cmd_cb_params_tot_size == 0 )
  {
    QCRIL_LOG_ERROR( "%s, Zero size cmd_cb_params_ptr\n", __FUNCTION__);
    free( cmd_cb_params_ptr );
    return;
  }

  QCRIL_LOG_INFO( "%s qcril_event_queue\n", __FUNCTION__);
  qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, QCRIL_DATA_NOT_ON_STACK,
                     QCRIL_EVT_INTERNAL_MMGSDI_READ_UST_VALUE,
                     (void *) cmd_cb_params_ptr, cmd_cb_params_tot_size, (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
  /* cmd_cb_params_ptr will be freed upon finish processing the event in
     qcril_mmgsdi_process_internal_read_ust_callback */
} /* qcril_mmgsdi_internal_read_ust_callback */


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
)
{
  /*-----------------------------------------------------------------------*/

  qcril_mmgsdi_command_callback_params_type *cmd_cb_params_ptr      = NULL;
  int                                        cmd_cb_params_tot_size = 0;

  QCRIL_LOG_INFO( "%s: cnf = 0x%x, status = 0x%x\n",
    __FUNCTION__, cnf, status);

  cmd_cb_params_ptr = qcril_mmgsdi_alloc_and_copy_mmgsdi_cnf(status,
                                                             cnf,
                                                             cnf_ptr,
                                                             &cmd_cb_params_tot_size);
  if ( cmd_cb_params_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s, Null cmd_cb_params_ptr\n", __FUNCTION__);
    return;
  }
  if ( cmd_cb_params_tot_size == 0 )
  {
    QCRIL_LOG_ERROR( "%s, Zero size cmd_cb_params_ptr\n", __FUNCTION__);
    free( cmd_cb_params_ptr );
    return;
  }

  QCRIL_LOG_INFO( "%s qcril_event_queue\n", __FUNCTION__);
  qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, QCRIL_DATA_NOT_ON_STACK,
                     QCRIL_EVT_MMGSDI_COMMAND_CALLBACK,
                     (void *) cmd_cb_params_ptr, cmd_cb_params_tot_size, (void*) cnf_ptr->response_header.client_data );

  /* cmd_cb_params_ptr will be freed upon finish processing the event in
     qcril_mmgsdi_process_command_callback */
} /* qcril_mmgsdi_command_callback() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_imsi_command_callback

===========================================================================*/
/*!
    @brief
    Special case callback function for RIL_REQUEST_GET_IMSI.

    The issue here is that GET_IMSI will return from MMGSDI with the same
    value in the cnf field as in the case of SIM_IO READ commands. It is
    not, therefore, possible to distinguish between the two possible
    callers if we dispatch on the same internal event.

    The solution here is to dispatch on a separate event type,
    QCRIL_EVT_MMGSDI_IMSI_COMMAND_CALLBACK, and thus to have separate
    processing for the two paths.

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_imsi_command_callback
(
  mmgsdi_return_enum_type status,
  mmgsdi_cnf_enum_type    cnf,
  const mmgsdi_cnf_type  *cnf_ptr
)
{
  /*-----------------------------------------------------------------------*/

  qcril_mmgsdi_command_callback_params_type *cmd_cb_params_ptr      = NULL;
  int                                        cmd_cb_params_tot_size = 0;

  QCRIL_LOG_INFO( "%s: cnf = 0x%x, status = 0x%x\n",
    __FUNCTION__, cnf, status);

  cmd_cb_params_ptr = qcril_mmgsdi_alloc_and_copy_mmgsdi_cnf(status,
                                                             cnf,
                                                             cnf_ptr,
                                                             &cmd_cb_params_tot_size);
  if ( cmd_cb_params_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s, Null cmd_cb_params_ptr\n", __FUNCTION__);
    return;
  }
  if ( cmd_cb_params_tot_size == 0 )
  {
    QCRIL_LOG_ERROR( "%s, Zero size cmd_cb_params_ptr\n", __FUNCTION__);
    free( cmd_cb_params_ptr );
    return;
  }

  qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, QCRIL_DATA_NOT_ON_STACK,
                     QCRIL_EVT_MMGSDI_IMSI_COMMAND_CALLBACK,
                     (void *) cmd_cb_params_ptr, cmd_cb_params_tot_size, (void*) cnf_ptr->response_header.client_data );

  /* cmd_cb_params_ptr will be freed upon finish processing the event in
     qcril_mmgsdi_process_imsi_command_callback */

} /* qcril_mmgsdi_imsi_command_callback() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_event_callback

===========================================================================*/
/*!
    @brief
    General event callback function for MMGSDI

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_event_callback
(
  const mmgsdi_event_data_type *event_ptr
)
{
  /*-----------------------------------------------------------------------*/

  mmgsdi_event_data_type        *evt_ptr           = NULL;
  int                            evt_data_tot_size = 0;

  if (event_ptr == NULL)
  {
    QCRIL_LOG_INFO( "%s, Null event_ptr\n", __FUNCTION__);
    return;
  }

  QCRIL_LOG_INFO( "%s, event:%d\n",
                 __FUNCTION__, event_ptr->evt);

  evt_ptr = qcril_mmgsdi_alloc_and_copy_mmgsdi_evt(event_ptr, &evt_data_tot_size);
  if ( evt_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s, Null evt_ptr\n", __FUNCTION__);
    return;
  }
  if ( evt_data_tot_size == 0 )
  {
    QCRIL_LOG_ERROR( "%s, Zero size evt_ptr\n", __FUNCTION__);
    free( evt_ptr );
    return;
  }

  qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, QCRIL_DATA_NOT_ON_STACK, QCRIL_EVT_MMGSDI_EVENT_CALLBACK,
                     (void*)evt_ptr, evt_data_tot_size, (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
  /* evt_ptr will be freed upon finish processing the event in
     qcril_mmgsdi_process_event_callback */

} /* qcril_mmgsdi_event_callback()*/


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
)
{
  /*-----------------------------------------------------------------------*/

  gsdi_cnf_T          *cnf_ptr           = NULL;
  int                  cnf_data_tot_size = 0;

  if (gsdi_cnf_ptr == NULL)
  {
    QCRIL_LOG_INFO( "%s, Null gsdi_cnf_ptr\n", __FUNCTION__);
    return;
  }

  QCRIL_LOG_INFO( "%s, cmd:%d\n", __FUNCTION__, gsdi_cnf_ptr->message_header.message_id);

  cnf_ptr = qcril_mmgsdi_alloc_and_copy_gsdi_cnf(gsdi_cnf_ptr, &cnf_data_tot_size);
  if ( cnf_ptr == NULL )
  {
    QCRIL_LOG_ERROR( "%s, Null cnf_ptr\n", __FUNCTION__);
    return;
  }
  if ( cnf_data_tot_size == 0 )
  {
    QCRIL_LOG_ERROR( "%s, Zero size cnf_ptr\n", __FUNCTION__);
    free( cnf_ptr );
    return;
  }

  qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, QCRIL_DATA_NOT_ON_STACK,
                     QCRIL_EVT_MMGSDI_GSDI_COMMAND_CALLBACK, (void *)cnf_ptr, cnf_data_tot_size, (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );

  /* cnf_ptr will be freed upon finish processing the cnf in
     qcril_mmgsdi_process_gsdi_command_callback */

} /* qcril_mmgsdi_gsdi_command_callback() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_perso_event_callback

===========================================================================*/
/*!
    @brief
    Perso Event Callback

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
)
{
  /*-----------------------------------------------------------------------*/

  qcril_mmgsdi_perso_event_callback_params_type  *evt_ptr  = NULL;

  QCRIL_LOG_INFO( "%s (event: 0x%x)\n", __FUNCTION__, evt);

  /* Do not support additional info at this point! */
  evt_ptr = malloc(sizeof(qcril_mmgsdi_perso_event_callback_params_type));
  if (evt_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "evt_ptr alloc failed");
    return;
  }

  evt_ptr->evt                   = evt;
  evt_ptr->slot                  = slot;
  evt_ptr->additional_info_avail = FALSE;  /* hardcoded to FALSE because we do not need additional
                                              info ptr for now */

  qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, QCRIL_DATA_NOT_ON_STACK,
                     QCRIL_EVT_MMGSDI_PERSO_EVENT_CALLBACK,
                     (void *)evt_ptr, sizeof( qcril_mmgsdi_perso_event_callback_params_type ), (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
  /* evt_ptr will be freed upon finish processing the event in
     qcril_mmgsdi_process_perso_event_callback */
} /* qcril_mmgsdi_perso_event_callback() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_response

===========================================================================*/
/*!
    @brief
    Handle MMGSDI response that requires a void pointer as response data

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_response
(
  qcril_instance_id_e_type    instance_id,
  RIL_Token                   t,
  mmgsdi_return_enum_type     status,
  void*                       rsp_data,
  size_t                      rsp_length,
  boolean                     remove_entry,
  char*                       logstr
)
{
  RIL_Errno RIL_Err = RIL_E_GENERIC_FAILURE;
  qcril_reqlist_public_type info;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  /* Convert to RIL status */
  RIL_Err = qcril_mmgsdi_convert_mmgsdi_status(status);
  if ( qcril_reqlist_query( instance_id, t, &info ) == E_SUCCESS )
  {
    qcril_default_request_resp_params( instance_id, t, info.request, RIL_Err, &resp );

    if (rsp_data != NULL)
    {
      resp.resp_pkt = rsp_data;
      resp.resp_len = rsp_length;
    }

    if ( logstr != NULL )
    {
      resp.logstr = logstr;
    }

    qcril_send_request_response( &resp );
  }

} /* qcril_mmgsdi_response() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_update_cm_card_status

===========================================================================*/
/*!
    @brief
    Update QCRIL(CM) card status per MMGSDI card state.

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_update_cm_card_status
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  qcril_card_status_e_type new_card_status
)
{
  qcril_card_info_type card_info;

  card_info.slot = 0;
  card_info.status = new_card_status;

  if ( qcril_process_event( instance_id, modem_id, QCRIL_EVT_CM_CARD_STATUS_UPDATED,
                            (void *) &card_info, sizeof( card_info ), (RIL_Token) QCRIL_TOKEN_ID_INTERNAL ) != E_SUCCESS )
  {
    QCRIL_LOG_ERROR( "%s\n", "CM_CARD_STATUS_UPDATED Failed!" )
  }

} /* qcril_mmgsdi_update_cm_card_status() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_update_pbm_card_event

===========================================================================*/
/*!
    @brief
    Update QCRIL(PBM) card event per MMGSDI card state.

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_update_pbm_card_event
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  qcril_evt_e_type pbm_card_event
)
{
  int  slot = 0;

  switch ( pbm_card_event )
  {
    case QCRIL_EVT_PBM_CARD_INSERTED:
    case QCRIL_EVT_PBM_CARD_ERROR:
    case QCRIL_EVT_PBM_CARD_INIT_COMPLETED:
      if ( qcril_process_event( instance_id, modem_id, pbm_card_event, &slot, sizeof( slot ),
                                (RIL_Token) QCRIL_TOKEN_ID_INTERNAL ) != E_SUCCESS )
      {
        QCRIL_LOG_ERROR( "PBM_HANDLE_CARD $d processing Failed!\n", pbm_card_event );
      }
      break;
    default:
      break;
  }

} /* qcril_mmgsdi_update_pbm_card_event() */


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
                              RIL_SIM_STATE_CARD_ERROR

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
)
{
  QCRIL_ASSERT( ret_ptr != NULL );

  if (!gw_card_state_update && !cdma_card_state_update)
  {
    /* no update needed */
    return;
  }

  QCRIL_LOG_DEBUG( "%s new state 0x%x: gw_update 0x%x, cdma_update 0x%x\n",
                   __FUNCTION__, new_card_state, gw_card_state_update, cdma_card_state_update );

  if (gw_card_state_update)
  {
    if (new_card_state != qcril_mmgsdi.gw_curr_card_state)
    {
      QCRIL_LOG_INFO( "New next_gw_sim_state 0x%x \n", new_card_state );
      ret_ptr->pri_gw_sim_state_changed = gw_card_state_update;
      ret_ptr->next_pri_gw_sim_state = ( qcril_sim_state_e_type ) new_card_state;
      qcril_mmgsdi.gw_curr_card_state = new_card_state;
    }
  }

  if (cdma_card_state_update)
  {
    ret_ptr->pri_cdma_sim_state_changed = cdma_card_state_update;
    ret_ptr->next_pri_cdma_sim_state = ( qcril_sim_state_e_type ) new_card_state;
  }

} /* qcril_mmgsdi_update_curr_card_state() */


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
)
{
  if (qcril_mmgsdi.curr_pin.valid)
  {
    switch (qcril_mmgsdi.curr_pin.pin_id)
    {
    case MMGSDI_PIN1:
      QCRIL_LOG_DEBUG( "%s: Orig curr pin -> PIN1\n", __FUNCTION__);
      break;
    case MMGSDI_PIN2:
      QCRIL_LOG_DEBUG( "%s: Orig curr pin -> PIN2\n", __FUNCTION__);
      break;
    default:
      QCRIL_LOG_DEBUG( "%s: Orig curr pin -> invalid 0x%x\n", __FUNCTION__, qcril_mmgsdi.curr_pin.pin_id);
      break;
    }
  }

  switch (pin_id)
  {
  case MMGSDI_PIN1:
    QCRIL_LOG_DEBUG( "%s", "     New curr pin -> PIN1\n");
    qcril_mmgsdi.curr_pin.valid = TRUE;
    qcril_mmgsdi.curr_pin.pin_id = pin_id;
    break;

  case MMGSDI_PIN2:
    QCRIL_LOG_ERROR( "%s", "     New curr pin -> PIN2\n");
    qcril_mmgsdi.curr_pin.valid = TRUE;
    qcril_mmgsdi.curr_pin.pin_id = pin_id;
    break;

  default:
    QCRIL_LOG_DEBUG( "%s", "     New curr pin -> Unchanged\n");
    break;
  }
} /* qcril_mmgsdi_set_curr_pin_ptr() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_update_fdn_status

===========================================================================*/
/*!
    @brief
    Update FDN state

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_update_fdn_status
(
  qcril_instance_id_e_type          instance_id,
  qcril_modem_id_e_type             modem_id,
  qcril_mmgsdi_fdn_status_enum_type fdn_status
)
{
  boolean cm_indication = FALSE;

  switch (qcril_mmgsdi.fdn_enable)
  {
  case QCRIL_MMGSDI_FDN_NOT_AVAILABLE:
    QCRIL_LOG_DEBUG( "%s: Orig status -> Not available\n", __FUNCTION__);
    break;
  case QCRIL_MMGSDI_FDN_ENABLED:
    QCRIL_LOG_DEBUG( "%s: Orig status -> Enabled\n", __FUNCTION__);
    break;
  case QCRIL_MMGSDI_FDN_DISABLED:
    QCRIL_LOG_DEBUG( "%s: Orig status -> Disabled\n", __FUNCTION__);
    break;
  case QCRIL_MMGSDI_FDN_NOT_INIT:
    QCRIL_LOG_DEBUG( "%s: Orig status -> Not Init\n", __FUNCTION__);
    break;
  default:
    QCRIL_LOG_DEBUG( "%s: Orig status -> Unknown 0x%x\n",
      __FUNCTION__, qcril_mmgsdi.fdn_enable);
    break;
  }

  switch (fdn_status)
  {
    case QCRIL_MMGSDI_FDN_NOT_AVAILABLE:
      cm_indication           = FALSE;
      qcril_mmgsdi.fdn_enable = QCRIL_MMGSDI_FDN_NOT_AVAILABLE;
      QCRIL_LOG_DEBUG( "%s", "    New state -> Not available\n");
      break;

    case QCRIL_MMGSDI_FDN_ENABLED:
      cm_indication           = TRUE;
      qcril_mmgsdi.fdn_enable = QCRIL_MMGSDI_FDN_ENABLED;
      QCRIL_LOG_DEBUG( "%s", "    New state -> Enabled\n");
      break;

    case QCRIL_MMGSDI_FDN_DISABLED:
      cm_indication           = FALSE;
      qcril_mmgsdi.fdn_enable = QCRIL_MMGSDI_FDN_DISABLED;
      QCRIL_LOG_DEBUG( "%s", "    New state -> Disabled\n");
      break;

    default:
      QCRIL_LOG_DEBUG( "%s", "    New state -> Invalid\n");
      break;
  }

  /* Added to maintain the FDN check status in qcril_cm. Can be removed once the modem call control feature is turned on */
  if ( qcril_process_event( instance_id, modem_id, QCRIL_EVT_CM_UPDATE_FDN_STATUS, (void *) &cm_indication, sizeof(boolean),
                            (RIL_Token) QCRIL_TOKEN_ID_INTERNAL ) != E_SUCCESS )
  {
    QCRIL_LOG_ERROR("%s","Internal QCRIL CM Event processing Failed for FDN status update!");
  }
} /* qcril_mmgsdi_update_fdn_status() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_print_evt

===========================================================================*/
/*!
    @brief
    Print the MMGSDI event

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_print_evt
(
  mmgsdi_events_enum_type evt
)
{
  switch (evt)
  {
  case MMGSDI_CARD_INSERTED_EVT:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_CARD_INSERTED_EVT\n");
    break;
  case MMGSDI_SELECT_AID_EVT:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_SELECT_AID_EVT\n");
    break;
  case MMGSDI_PIN1_EVT:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_PIN1_EVT\n");
    break;
  case MMGSDI_PIN2_EVT:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_PIN2_EVT\n");
    break;
  case MMGSDI_UNIVERSAL_PIN_EVT:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_UNIVERSAL_PIN_EVT\n");
    break;
  case MMGSDI_CARD_ERROR_EVT:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_CARD_ERROR_EVT\n");
    break;
  case MMGSDI_CARD_REMOVED_EVT:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_CARD_REMOVED_EVT\n");
    break;
  case MMGSDI_ILLEGAL_CARD_EVT:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_ILLEGAL_CARD_EVT\n");
    break;
  case MMGSDI_CARD_INIT_COMPLETED_EVT:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_CARD_INIT_COMPLETED_EVT\n");
    break;
  case MMGSDI_TERMINAL_PROFILE_DL_EVT:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_TERMINAL_PROFILE_DL_EVT\n");
    break;
  case MMGSDI_REFRESH_EVT:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_REFRESH_EVT\n");
    break;
  case MMGSDI_FDN_EVT:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_FDN_EVT\n");
    break;
  case MMGSDI_SWITCH_SLOT_EVT:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_SWITCH_SLOT_EVT\n");
    break;
  case MMGSDI_SESSION_CLOSE_EVT:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_SESSION_CLOSE_EVT\n");
    break;
  default:
    QCRIL_LOG_DEBUG( "Unknown MMGSDI Event 0x%x\n", evt);
    break;
  }

} /* qcril_mmgsdi_print_evt() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_print_perso_evt

===========================================================================*/
/*!
    @brief
    Print the MMGSDI Perso event

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_print_perso_evt
(
  gsdi_perso_event_enum_type evt
)
{
  switch (evt)
  {
  case GSDI_PERSO_NO_EVENT:              QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_NO_EVENT\n");              break;
  case GSDI_PERSO_NW_FAILURE:            QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_NW_FAILURE\n");            break;
  case GSDI_PERSO_NS_FAILURE:            QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_NS_FAILURE\n");            break;
  case GSDI_PERSO_SP_FAILURE:            QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_SP_FAILURE\n");            break;
  case GSDI_PERSO_CP_FAILURE:            QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_CP_FAILURE\n");            break;
  case GSDI_PERSO_SIM_FAILURE:           QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_SIM_FAILURE\n");           break;
  case GSDI_PERSO_NCK_BLOCKED:           QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_NCK_BLOCKED\n");           break;
  case GSDI_PERSO_NSK_BLOCKED:           QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_NSK_BLOCKED\n");           break;
  case GSDI_PERSO_SPK_BLOCKED:           QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_SPK_BLOCKED\n");           break;
  case GSDI_PERSO_CCK_BLOCKED:           QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_CCK_BLOCKED\n");           break;
  case GSDI_PERSO_PPK_BLOCKED:           QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_PPK_BLOCKED\n");           break;
  case GSDI_PERSO_NW_DEACTIVATED:        QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_NW_DEACTIVATED\n");        break;
  case GSDI_PERSO_NS_DEACTIVATED:        QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_NS_DEACTIVATED\n");        break;
  case GSDI_PERSO_SP_DEACTIVATED:        QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_SP_DEACTIVATED\n");        break;
  case GSDI_PERSO_CP_DEACTIVATED:        QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_CP_DEACTIVATED\n");        break;
  case GSDI_PERSO_SIM_DEACTIVATED:       QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_SIM_DEACTIVATED\n");       break;
  case GSDI_PERSO_NCK_UNBLOCKED:         QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_NCK_UNBLOCKED\n");         break;
  case GSDI_PERSO_NSK_UNBLOCKED:         QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_NSK_UNBLOCKED\n");         break;
  case GSDI_PERSO_SPK_UNBLOCKED:         QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_SPK_UNBLOCKED\n");         break;
  case GSDI_PERSO_CCK_UNBLOCKED:         QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_CCK_UNBLOCKED\n");         break;
  case GSDI_PERSO_PPK_UNBLOCKED:         QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_PPK_UNBLOCKED\n");         break;
  case GSDI_PERSO_RUIM_NW1_FAILURE:      QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_NW1_FAILURE\n");      break;
  case GSDI_PERSO_RUIM_NW2_FAILURE:      QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_NW2_FAILURE\n");      break;
  case GSDI_PERSO_RUIM_HRPD_FAILURE:     QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_HRPD_FAILURE\n");     break;
  case GSDI_PERSO_RUIM_SP_FAILURE:       QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_SP_FAILURE\n");       break;
  case GSDI_PERSO_RUIM_CP_FAILURE:       QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_CP_FAILURE\n");       break;
  case GSDI_PERSO_RUIM_RUIM_FAILURE:     QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_RUIM_FAILURE\n");     break;
  case GSDI_PERSO_RUIM_NW1_DEACTIVATED:  QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_NW1_DEACTIVATED\n");  break;
  case GSDI_PERSO_RUIM_NW2_DEACTIVATED:  QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_NW2_DEACTIVATED\n");  break;
  case GSDI_PERSO_RUIM_HRPD_DEACTIVATED: QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_HRPD_DEACTIVATED\n"); break;
  case GSDI_PERSO_RUIM_SP_DEACTIVATED:   QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_SP_DEACTIVATED\n");   break;
  case GSDI_PERSO_RUIM_CP_DEACTIVATED:   QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_CP_DEACTIVATED\n");   break;
  case GSDI_PERSO_RUIM_RUIM_DEACTIVATED: QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_RUIM_DEACTIVATED\n"); break;
  case GSDI_PERSO_RUIM_NCK1_BLOCKED:     QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_NCK1_BLOCKED\n");     break;
  case GSDI_PERSO_RUIM_NCK2_BLOCKED:     QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_NCK2_BLOCKED\n");     break;
  case GSDI_PERSO_RUIM_HNCK_BLOCKED:     QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_HNCK_BLOCKED\n");     break;
  case GSDI_PERSO_RUIM_SPCK_BLOCKED:     QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_SPCK_BLOCKED\n");     break;
  case GSDI_PERSO_RUIM_CCK_BLOCKED:      QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_CCK_BLOCKED\n");      break;
  case GSDI_PERSO_RUIM_PCK_BLOCKED:      QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_PCK_BLOCKED\n");      break;
  case GSDI_PERSO_RUIM_NCK1_UNBLOCKED:   QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_NCK1_UNBLOCKED\n");   break;
  case GSDI_PERSO_RUIM_NCK2_UNBLOCKED:   QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_NCK2_UNBLOCKED\n");   break;
  case GSDI_PERSO_RUIM_HNCK_UNBLOCKED:   QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_HNCK_UNBLOCKED\n");   break;
  case GSDI_PERSO_RUIM_SPCK_UNBLOCKED:   QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_SPCK_UNBLOCKED\n");   break;
  case GSDI_PERSO_RUIM_CCK_UNBLOCKED:    QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_CCK_UNBLOCKED\n");    break;
  case GSDI_PERSO_RUIM_PCK_UNBLOCKED:    QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_RUIM_PCK_UNBLOCKED\n");    break;
  case GSDI_PERSO_SANITY_ERROR:          QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_SANITY_ERROR\n");          break;
  case GSDI_PERSO_EVT_INIT_COMPLETED:    QCRIL_LOG_DEBUG( "%s", "GSDI_PERSO_EVT_INIT_COMPLETED\n");    break;
  default:                               QCRIL_LOG_DEBUG( "Unknown Perso Event 0x%x\n", evt);          break;
  }

} /* qcril_mmgsdi_print_perso_evt() */

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_print_byte_data

===========================================================================*/
/*!
    @brief
    Print the uint8 array structure

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_print_byte_data
(
  int32                   data_len,
  uint8                  *data_ptr
)
{
  int i         = 0;

  if (data_len == 0)
  {
    QCRIL_LOG_VERBOSE( "%s", "data_len = 0\n" );
    return;
  }

  QCRIL_LOG_VERBOSE( "%s", "data: \n" );

  if (data_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "Invalid data_len %d and null data_ptr\n", (int)data_len);
    return;
  }

  for (i = 1; (i <= data_len && i <= 100); i++)
  {
    QCRIL_LOG_VERBOSE("0x%x, ", data_ptr[i-1] );
    if ( (i % 10) == 0 )
    {
      QCRIL_LOG_VERBOSE("%s", "\n" );
    }
  }

  QCRIL_LOG_VERBOSE("%s", "\n" );
} /* qcril_mmgsdi_print_byte_data() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_print_appstate_string

===========================================================================*/
/*!
    @brief
    Print the RIL App State string

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_print_appstate_string
(
  RIL_AppState app_state
)
{
  switch (app_state)
  {
  case RIL_APPSTATE_UNKNOWN:
    QCRIL_LOG_DEBUG( "%s", " -App State: RIL_APPSTATE_UNKNOWN\n");
    break;
  case RIL_APPSTATE_DETECTED:
    QCRIL_LOG_DEBUG( "%s", " -App State: RIL_APPSTATE_DETECTED\n");
    break;
  case RIL_APPSTATE_PIN:
    QCRIL_LOG_DEBUG( "%s", " -App State: RIL_APPSTATE_PIN\n");
    break;
  case RIL_APPSTATE_PUK:
    QCRIL_LOG_DEBUG( "%s", " -App State: RIL_APPSTATE_PUK\n");
    break;
  case RIL_APPSTATE_SUBSCRIPTION_PERSO:
    QCRIL_LOG_DEBUG( "%s", " -App State: RIL_APPSTATE_SUBSCRIPTION_PERSO\n");
    break;
  case RIL_APPSTATE_READY:
    QCRIL_LOG_DEBUG( "%s", " -App State: RIL_APPSTATE_READY\n");
    break;
  case RIL_APPSTATE_ILLEGAL:
    QCRIL_LOG_DEBUG( "%s", " -App State: RIL_APPSTATE_ILLEGAL\n");
    break;
  default:
    QCRIL_LOG_DEBUG( "%s", " -App State: Unknown\n");
    break;
  }
} /* qcril_mmgsdi_print_appstate_string() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_print_apptype_string

===========================================================================*/
/*!
    @brief
    Print the RIL App Type String

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_print_apptype_string
(
  RIL_AppType app_type
)
{
  switch (app_type)
  {
  case RIL_APPTYPE_SIM:
    QCRIL_LOG_DEBUG( "%s", " -App Type: RIL_APPTYPE_SIM\n");
    break;
  case RIL_APPTYPE_USIM:
    QCRIL_LOG_DEBUG( "%s", " -App Type: RIL_APPTYPE_USIM\n");
    break;
  case RIL_APPTYPE_RUIM:
    QCRIL_LOG_DEBUG( "%s", " -App Type: RIL_APPTYPE_RUIM\n");
    break;
  case RIL_APPTYPE_CSIM:
    QCRIL_LOG_DEBUG( "%s", " -App Type: RIL_APPTYPE_CSIM\n");
    break;
  default:
    QCRIL_LOG_DEBUG( "%s", " -App Type: Unknown\n");
    break;
  }
} /* qcril_mmgsdi_print_apptype_string() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_print_persosubstate_string

===========================================================================*/
/*!
    @brief
    Print the RIL Perso Substate string

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_print_persosubstate_string
(
  RIL_PersoSubstate perso_substate
)
{
  switch (perso_substate)
  {
  case RIL_PERSOSUBSTATE_UNKNOWN:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_UNKNOWN\n");
    break;
  case RIL_PERSOSUBSTATE_IN_PROGRESS:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_IN_PROGRESS\n");
    break;
  case RIL_PERSOSUBSTATE_READY:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_READY\n");
    break;
  case RIL_PERSOSUBSTATE_SIM_NETWORK:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_SIM_NETWORK\n");
    break;
  case RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET\n");
    break;
  case RIL_PERSOSUBSTATE_SIM_CORPORATE:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_SIM_CORPORATE\n");
    break;
  case RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER\n");
    break;
  case RIL_PERSOSUBSTATE_SIM_SIM:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_SIM_SIM\n");
    break;
  case RIL_PERSOSUBSTATE_SIM_NETWORK_PUK:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_SIM_NETWORK_PUK\n");
    break;
  case RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET_PUK:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET_PUK\n");
    break;
  case RIL_PERSOSUBSTATE_SIM_CORPORATE_PUK:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_SIM_CORPORATE_PUK\n");
    break;
  case RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER_PUK:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER_PUK\n");
    break;
  case RIL_PERSOSUBSTATE_SIM_SIM_PUK:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_SIM_SIM_PUK\n");
    break;
  case RIL_PERSOSUBSTATE_RUIM_NETWORK1:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_RUIM_NETWORK1\n");
    break;
  case RIL_PERSOSUBSTATE_RUIM_NETWORK2:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_RUIM_NETWORK2\n");
    break;
  case RIL_PERSOSUBSTATE_RUIM_HRPD:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_RUIM_HRPD\n");
    break;
  case RIL_PERSOSUBSTATE_RUIM_CORPORATE:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_RUIM_CORPORATE\n");
    break;
  case RIL_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER\n");
    break;
  case RIL_PERSOSUBSTATE_RUIM_RUIM:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_RUIM_RUIM\n");
    break;
  case RIL_PERSOSUBSTATE_RUIM_NETWORK1_PUK:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_RUIM_NETWORK1_PUK\n");
    break;
  case RIL_PERSOSUBSTATE_RUIM_NETWORK2_PUK:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_RUIM_NETWORK2_PUK\n");
    break;
  case RIL_PERSOSUBSTATE_RUIM_HRPD_PUK:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_RUIM_HRPD_PUK\n");
    break;
  case RIL_PERSOSUBSTATE_RUIM_CORPORATE_PUK:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_RUIM_CORPORATE_PUK\n");
    break;
  case RIL_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER_PUK:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER_PUK\n");
    break;
  case RIL_PERSOSUBSTATE_RUIM_RUIM_PUK:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: RIL_PERSOSUBSTATE_RUIM_RUIM_PUK\n");
    break;
  default:
    QCRIL_LOG_DEBUG( "%s", " -Perso SubState: Unknown\n");
    break;
  }

} /* qcril_mmgsdi_print_persosubstate_string() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_print_pinstate_string

===========================================================================*/
/*!
    @brief
    Print the RIL Pin State string

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_print_pinstate_string
(
  char*        msg_tag_ptr,
  RIL_PinState pin_state
)
{

  QCRIL_ASSERT( msg_tag_ptr != NULL );
  switch (pin_state)
  {
  case RIL_PINSTATE_UNKNOWN:
    QCRIL_LOG_DEBUG( "%s: RIL_PINSTATE_UNKNOWN\n", msg_tag_ptr);
    break;
  case RIL_PINSTATE_ENABLED_NOT_VERIFIED:
    QCRIL_LOG_DEBUG( "%s: RIL_PINSTATE_ENABLED_NOT_VERIFIED\n", msg_tag_ptr);
    break;
  case RIL_PINSTATE_ENABLED_VERIFIED:
    QCRIL_LOG_DEBUG( "%s: RIL_PINSTATE_ENABLED_VERIFIED\n", msg_tag_ptr);
    break;
  case RIL_PINSTATE_DISABLED:
    QCRIL_LOG_DEBUG( "%s: RIL_PINSTATE_DISABLED\n", msg_tag_ptr);
    break;
  case RIL_PINSTATE_ENABLED_BLOCKED:
    QCRIL_LOG_DEBUG( "%s: RIL_PINSTATE_ENABLED_BLOCKED\n", msg_tag_ptr);
    break;
  case RIL_PINSTATE_ENABLED_PERM_BLOCKED:
    QCRIL_LOG_DEBUG( "%s: RIL_PINSTATE_ENABLED_PERM_BLOCKED\n", msg_tag_ptr);
    break;
  default:
    QCRIL_LOG_DEBUG( "%s: Pin State Invalid\n", msg_tag_ptr);
    break;
  }
} /* qcril_mmgsdi_print_pinstate_string() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_print_card_status

===========================================================================*/
/*!
    @brief
    Print the relevant card status info from the global structure

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_print_card_status( void )
{
  int i = 0;
  int app_index = 0;
  char universal_pin_state[22] = " -Universal Pin State\0";
  char pin1_state[13] = " -Pin1 State\0";
  char pin2_state[13] = " -Pin2 State\0";
  char pin1_replaced[17] = "Replaced by UPin\0";
  char pin1_not_replaced[21] = "Not Replaced by UPin\0";

  QCRIL_LOG_DEBUG("%s", "Card Status \n" );
  switch (qcril_mmgsdi.curr_card_status.card_state)
  {
  case RIL_CARDSTATE_ABSENT:
    QCRIL_LOG_DEBUG("%s", " -Card_State: RIL_CARDSTATE_ABSENT\n" );
    break;
  case RIL_CARDSTATE_PRESENT:
    QCRIL_LOG_DEBUG("%s", " -Card_State: RIL_CARDSTATE_PRESENT\n" );
    break;
  case RIL_CARDSTATE_ERROR:
    QCRIL_LOG_DEBUG("%s", " -Card_State: RIL_CARDSTATE_ERROR\n" );
    break;
  default:
    QCRIL_LOG_DEBUG("-Card_State: Unknown 0x%x\n",
      qcril_mmgsdi.curr_card_status.card_state);
    break;
  }

  qcril_mmgsdi_print_pinstate_string(universal_pin_state,
                                     qcril_mmgsdi.curr_card_status.universal_pin_state);

  if (qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index <= RIL_CARD_MAX_APPS)
  {
    QCRIL_LOG_DEBUG("Number of 3GPP App Indexes: 0x%x\n",
                    qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index );

    app_index = qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index;

    QCRIL_LOG_VERBOSE( "3GPP App index[0x%x]: \n", app_index);

    qcril_mmgsdi_print_apptype_string(
        qcril_mmgsdi.curr_card_status.applications[app_index].app_type);
    qcril_mmgsdi_print_appstate_string(
        qcril_mmgsdi.curr_card_status.applications[app_index].app_state);
    qcril_mmgsdi_print_persosubstate_string(
        qcril_mmgsdi.curr_card_status.applications[app_index].perso_substate);

    if (qcril_mmgsdi.curr_card_status.applications[app_index].aid_ptr != NULL)
    {
      QCRIL_LOG_DEBUG( " -Aid: %s\n",
                     qcril_mmgsdi.curr_card_status.applications[app_index].aid_ptr);
    }
    if (qcril_mmgsdi.curr_card_status.applications[app_index].app_label_ptr != NULL)
    {
      QCRIL_LOG_DEBUG( " -Label: %s\n",
                     qcril_mmgsdi.curr_card_status.applications[app_index].app_label_ptr);
    }

    QCRIL_LOG_DEBUG( " -pin1_replaced: %s\n",
      qcril_mmgsdi.curr_card_status.applications[i].pin1_replaced ? pin1_replaced : pin1_not_replaced);
    qcril_mmgsdi_print_pinstate_string(
        pin1_state, qcril_mmgsdi.curr_card_status.applications[app_index].pin1);
    qcril_mmgsdi_print_pinstate_string(
        pin2_state, qcril_mmgsdi.curr_card_status.applications[app_index].pin2);
  }
  else
  {
    QCRIL_LOG_DEBUG( "Invalid gsm_umts_subscription_app_index: %d\n",
                     qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index );
  }

  if (qcril_mmgsdi.curr_card_status.cdma_subscription_app_index <= RIL_CARD_MAX_APPS)
  {
    QCRIL_LOG_DEBUG("Number of 3GPP2 App Indexes: 0x%x\n",
                    qcril_mmgsdi.curr_card_status.cdma_subscription_app_index );
    app_index = qcril_mmgsdi.curr_card_status.cdma_subscription_app_index;

    QCRIL_LOG_VERBOSE( "3GPP2 App index[0x%x]: \n", app_index);

    qcril_mmgsdi_print_apptype_string(
        qcril_mmgsdi.curr_card_status.applications[app_index].app_type);
    qcril_mmgsdi_print_appstate_string(
        qcril_mmgsdi.curr_card_status.applications[app_index].app_state);
    qcril_mmgsdi_print_persosubstate_string(
        qcril_mmgsdi.curr_card_status.applications[app_index].perso_substate);
    if (qcril_mmgsdi.curr_card_status.applications[app_index].aid_ptr != NULL)
    {
      QCRIL_LOG_DEBUG( " -Aid: %s\n",
                     qcril_mmgsdi.curr_card_status.applications[app_index].aid_ptr);
    }
    if (qcril_mmgsdi.curr_card_status.applications[app_index].app_label_ptr != NULL)
    {
      QCRIL_LOG_DEBUG( " -Label: %s\n",
                     qcril_mmgsdi.curr_card_status.applications[app_index].app_label_ptr);
    }
    QCRIL_LOG_DEBUG( " -pin1_replaced: %s\n",
      qcril_mmgsdi.curr_card_status.applications[i].pin1_replaced ? pin1_replaced : pin1_not_replaced);
    qcril_mmgsdi_print_pinstate_string(
        pin1_state, qcril_mmgsdi.curr_card_status.applications[app_index].pin1);
    qcril_mmgsdi_print_pinstate_string(
        pin2_state, qcril_mmgsdi.curr_card_status.applications[app_index].pin2);
  }
  else
  {
    QCRIL_LOG_DEBUG( "Invalid cdma_subscription_app_index: %d\n",
                     qcril_mmgsdi.curr_card_status.cdma_subscription_app_index );
  }

  QCRIL_LOG_DEBUG( "Total number of Card Applications: 0x%x\n",
                   qcril_mmgsdi.curr_card_status.num_applications);

  for (i = 0; i < qcril_mmgsdi.curr_card_status.num_applications; i++)
  {
    QCRIL_LOG_VERBOSE( "App [0x%x]: \n", i);
    qcril_mmgsdi_print_apptype_string(
      qcril_mmgsdi.curr_card_status.applications[i].app_type);

    if (qcril_mmgsdi.curr_card_status.applications[i].aid_ptr != NULL)
    {
      QCRIL_LOG_VERBOSE( " -Aid: %s\n",
                         qcril_mmgsdi.curr_card_status.applications[i].aid_ptr);
    }
    if (qcril_mmgsdi.curr_card_status.applications[i].app_label_ptr != NULL)
    {
      QCRIL_LOG_VERBOSE( " -Label: %s\n",
                         qcril_mmgsdi.curr_card_status.applications[i].app_label_ptr);
    }

    qcril_mmgsdi_print_appstate_string(
      qcril_mmgsdi.curr_card_status.applications[i].app_state);

    qcril_mmgsdi_print_pinstate_string(
      pin1_state, qcril_mmgsdi.curr_card_status.applications[i].pin1);

    QCRIL_LOG_VERBOSE( " -Pin1 %s\n",
      qcril_mmgsdi.curr_card_status.applications[i].pin1_replaced ? pin1_replaced : pin1_not_replaced);

    qcril_mmgsdi_print_pinstate_string(
      pin2_state, qcril_mmgsdi.curr_card_status.applications[i].pin2);
  }
} /* qcril_mmgsdi_print_card_status */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_is_card_status_changed

===========================================================================*/
/*!
    @brief
    Return if the card status has changed or not

    @return
    TRUE - if changed
    FALSE - if not changed
*/
/*=========================================================================*/
static boolean qcril_mmgsdi_is_card_status_changed(
  RIL_CardStatus_v6 *curr_card_status_ptr)
{
  int i = 0;


  QCRIL_ASSERT( curr_card_status_ptr != NULL );

  if (curr_card_status_ptr->card_state != qcril_mmgsdi.curr_card_status.card_state)
  {
    return TRUE;
  }
  if (curr_card_status_ptr->gsm_umts_subscription_app_index !=
      qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index)
  {
    return TRUE;
  }
  if (curr_card_status_ptr->cdma_subscription_app_index !=
      qcril_mmgsdi.curr_card_status.cdma_subscription_app_index)
  {
    return TRUE;
  }
  if (curr_card_status_ptr->num_applications !=
      qcril_mmgsdi.curr_card_status.num_applications)
  {
    return TRUE;
  }
  if (curr_card_status_ptr->universal_pin_state !=
      qcril_mmgsdi.curr_card_status.universal_pin_state)
  {
    return TRUE;
  }
  for (i = 0; i < RIL_CARD_MAX_APPS; i++)
  {
    if (curr_card_status_ptr->applications[i].app_state !=
        qcril_mmgsdi.curr_card_status.applications[i].app_state)
    {
      return TRUE;
    }
    if (curr_card_status_ptr->applications[i].perso_substate !=
        qcril_mmgsdi.curr_card_status.applications[i].perso_substate)
    {
      return TRUE;
    }
    if (curr_card_status_ptr->applications[i].app_type !=
        qcril_mmgsdi.curr_card_status.applications[i].app_type)
    {
      return TRUE;
    }
    if (curr_card_status_ptr->applications[i].pin1 !=
        qcril_mmgsdi.curr_card_status.applications[i].pin1)
    {
      return TRUE;
    }
    if (curr_card_status_ptr->applications[i].pin1_replaced !=
        qcril_mmgsdi.curr_card_status.applications[i].pin1_replaced)
    {
      return TRUE;
    }
    if (curr_card_status_ptr->applications[i].pin2 !=
        qcril_mmgsdi.curr_card_status.applications[i].pin2)
    {
      return TRUE;
    }
  }
  /* do not check for aid and label since those are unlikely to change */
  return FALSE;
} /* qcril_mmgsdi_is_card_status_changed */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_process_client_id_reg_cnf

===========================================================================*/
/*!
    @brief
    Handle Client ID reg cnf

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_process_client_id_reg_cnf
(
  mmgsdi_return_enum_type status,
  mmgsdi_cnf_enum_type    cnf,
  const mmgsdi_cnf_type  *cnf_ptr
)
{
  qcril_modem_id_e_type              modem_id = QCRIL_DEFAULT_MODEM_ID;
  mmgsdi_return_enum_type            mmgsdi_status = MMGSDI_SUCCESS;
  mmgsdi_option_type                 option;
  mmgsdi_refresh_file_list_type      refresh_files;
  mmgsdi_file_enum_type              refresh_file_enum_buf[] =
  {MMGSDI_TELECOM_ADN,    MMGSDI_USIM_ADN,
   MMGSDI_TELECOM_ADN1,   MMGSDI_USIM_ADN1,
   MMGSDI_TELECOM_ADN2,   MMGSDI_USIM_ADN2,
   MMGSDI_TELECOM_ADN3,   MMGSDI_USIM_ADN3,
   MMGSDI_TELECOM_EXT1,   MMGSDI_USIM_EXT1,
   MMGSDI_TELECOM_FDN,    MMGSDI_USIM_FDN,
   MMGSDI_TELECOM_SMS,    MMGSDI_USIM_SMS,
   MMGSDI_TELECOM_MSISDN, MMGSDI_USIM_MSISDN,
   MMGSDI_GSM_PNN,        MMGSDI_USIM_PNN,
   MMGSDI_GSM_OPL,        MMGSDI_USIM_OPL,
   MMGSDI_GSM_SPN,        MMGSDI_USIM_SPN,
   MMGSDI_GSM_CSP,        MMGSDI_USIM_CSP,
   MMGSDI_GSM_MBDN,       MMGSDI_USIM_MBDN,
   MMGSDI_GSM_CFIS,       MMGSDI_USIM_CFIS,
   MMGSDI_GSM_CFF,        MMGSDI_USIM_CFF
  };

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s (status: 0x%x)\n",
    __FUNCTION__, status);

  memset(&option, 0x00, sizeof(mmgsdi_option_type));
  memset(&refresh_files, 0x00, sizeof(mmgsdi_refresh_file_list_type));

  if (cnf_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "  Null cnf ptr\n");
    return;
  }

  if (MMGSDI_SUCCESS == status)
  {
    qcril_mmgsdi.client_id = cnf_ptr->client_id_reg_cnf.response_header.client_id;

    QCRIL_LOG_RPC( modem_id, "mmgsdi_client_evt_reg()", "client_id", (int)qcril_mmgsdi.client_id );

    if(qcril_mmgsdi.client_id == QCRIL_MMGSDI_INVALID_CLIENT_ID)
    {
      QCRIL_LOG_ERROR("%s", "Invalid client id received, aborting event and refresh reg\n");
      return;
    }

    mmgsdi_status = mmgsdi_client_evt_reg(qcril_mmgsdi.client_id,
                                          qcril_mmgsdi_event_callback,
                                          qcril_mmgsdi_command_callback,
                                          0);
    if (MMGSDI_SUCCESS != mmgsdi_status)
    {
      QCRIL_LOG_ERROR( "Unable to invoke mmgsdi_client_evt_reg: 0x%x\n", mmgsdi_status);
    }

    /* register for refresh file list */
    refresh_files.num_files = sizeof(refresh_file_enum_buf)/sizeof(mmgsdi_file_enum_type);
    refresh_files.file_list_ptr = refresh_file_enum_buf;
    mmgsdi_status = mmgsdi_register_for_refresh(qcril_mmgsdi.client_id,
                                                MMGSDI_SLOT_1,
                                                refresh_files,
                                                FALSE,
                                                option,
                                                qcril_mmgsdi_command_callback,
                                                0);
    if (MMGSDI_SUCCESS != mmgsdi_status)
    {
      QCRIL_LOG_ERROR( "Unable to invoke mmgsdi_register_for_refresh: 0x%x\n", mmgsdi_status);
    }

  }
  else
  {
    QCRIL_LOG_ERROR( "%s", " Failed to register for client ID");
  }
} /* qcril_mmgsdi_process_client_id_reg_cnf() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_process_client_evt_reg_cnf

===========================================================================*/
/*!
    @brief
    Handle Client event reg cnf

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_process_client_evt_reg_cnf
(
  mmgsdi_return_enum_type status,
  mmgsdi_cnf_enum_type    cnf,
  const mmgsdi_cnf_type  *cnf_ptr
)
{
  mmgsdi_return_enum_type mmgsdi_status = MMGSDI_SUCCESS;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_DEBUG( "%s \n", __FUNCTION__);

  if (MMGSDI_SUCCESS == status)
  {
    QCRIL_LOG_DEBUG( "%s", " qcril_mmgsdi ready to receive mmgsdi event\n");
  }
  else
  {
    QCRIL_LOG_DEBUG( " Failed to register for mmgsdi event (status: 0x%x\n", status);
  }
} /* qcril_mmgsdi_process_client_evt_reg_cnf() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_card_init_completed_end

===========================================================================*/
/*!
    @brief
    This function is executed after card init completed
    and get SIM capabilities (for FDN status) to notify
    CM, PBM and framework about the change in
    the card status.
    Update QCRIL MMGSDI global info
    Update Radio State if required

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_card_init_completed_end
(
  qcril_instance_id_e_type        instance_id,
  qcril_modem_id_e_type           modem_id,
  boolean                         gw_card_state_update,
  boolean                         cdma_card_state_update,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  int   gw_app_sub_index   = RIL_CARD_MAX_APPS;
  int   cdma_app_sub_index = RIL_CARD_MAX_APPS;
  qcril_unsol_resp_params_type unsol_resp;

  /* Make sure app state is set to READY, in case it was changed by other events */
  /* Note that for RPC-based QCRIL, this index should always be 0 */
  gw_app_sub_index = qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index;
  cdma_app_sub_index = qcril_mmgsdi.curr_card_status.cdma_subscription_app_index;

  if (gw_app_sub_index < qcril_mmgsdi.curr_card_status.num_applications)
  {
    qcril_mmgsdi.curr_card_status.applications[gw_app_sub_index].app_state = RIL_APPSTATE_READY;
    qcril_mmgsdi.curr_card_status.applications[gw_app_sub_index].perso_substate = RIL_PERSOSUBSTATE_READY;
  }

  if (cdma_app_sub_index < qcril_mmgsdi.curr_card_status.num_applications)
  {
    qcril_mmgsdi.curr_card_status.applications[cdma_app_sub_index].app_state = RIL_APPSTATE_READY;
    qcril_mmgsdi.curr_card_status.applications[cdma_app_sub_index].perso_substate = RIL_PERSOSUBSTATE_READY;
  }

  qcril_mmgsdi_update_curr_card_state(
    QCRIL_SIM_STATE_READY,
    gw_card_state_update,
    cdma_card_state_update,
    ret_ptr);

  QCRIL_LOG_INFO( "%s", "Sending SIM STATUS CHANGED\n");
  qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, &unsol_resp );
  qcril_send_unsol_response( &unsol_resp );

} /* qcril_mmgsdi_card_init_completed_end */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_process_get_sim_cap_cnf

===========================================================================*/
/*!
    @brief
    Handle Get Sim Capabilities Confirmation.
    Update fdn global

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_process_get_sim_cap_cnf
(
  qcril_instance_id_e_type     instance_id,
  qcril_modem_id_e_type        modem_id,
  gsdi_returns_T               gsdi_status,
  gsdi_get_sim_cap_cnf_T       *sim_cap_ptr,
  qcril_request_return_type    *const ret_ptr /*!< Output parameter */
)
{
  boolean              update_gw_sim_state           = FALSE;
  boolean              update_cdma_sim_state         = FALSE;

  QCRIL_ASSERT( sim_cap_ptr != NULL );

  QCRIL_LOG_DEBUG( "%s status 0x%x\n",
                    __FUNCTION__, gsdi_status);

  if (GSDI_SUCCESS == gsdi_status)
  {
    QCRIL_ASSERT( sim_cap_ptr != NULL );
    qcril_mmgsdi_update_fdn_status( instance_id, modem_id,
                                    sim_cap_ptr->sim_capabilities.fdn_enabled ? QCRIL_MMGSDI_FDN_ENABLED : QCRIL_MMGSDI_FDN_DISABLED);
  }
  else
  {
    qcril_mmgsdi_update_fdn_status(instance_id, modem_id,
                                   QCRIL_MMGSDI_FDN_NOT_AVAILABLE);
  }

  /* Userdata contains information on GW and CDMA update */
  update_gw_sim_state   = (boolean)(sim_cap_ptr->message_header.client_ref & QCRIL_MMGSDI_GW_UPDATE);
  update_cdma_sim_state = (boolean)(sim_cap_ptr->message_header.client_ref & QCRIL_MMGSDI_CDMA_UPDATE);

  /* Card init completed */
  qcril_mmgsdi_card_init_completed_end( instance_id, modem_id,
                                        update_gw_sim_state,
                                        update_cdma_sim_state,
                                        ret_ptr);

} /* qcril_mmgsdi_process_get_sim_cap_cnf() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_convert_pin_state_to_ril

===========================================================================*/
/*!
    @brief
    Convert mmgsdi pin status to RIL_PinState and also return corresponding
    sim state that may be used by caller if the pin Id is Pin1 or UPin(replaced)

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_convert_pin_state_to_ril(
  mmgsdi_pin_status_enum_type mmgsdi_pin_status,
  RIL_PinState               *ril_pin_state_ptr)
{
  QCRIL_ASSERT( ril_pin_state_ptr != NULL );

  switch (mmgsdi_pin_status)
  {
  case MMGSDI_PIN_STATUS_NOT_INITIALIZED:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_PIN_STATUS_NOT_INITIALIZED \n");
    break;

  case MMGSDI_PIN_ENABLED_NOT_VERIFIED:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_PIN_ENABLED_NOT_VERIFIED -> RIL_PINSTATE_ENABLED_NOT_VERIFIED \n");
    *ril_pin_state_ptr = RIL_PINSTATE_ENABLED_NOT_VERIFIED;
    break;

  case MMGSDI_PIN_UNBLOCKED:
  case MMGSDI_PIN_CHANGED:
  case MMGSDI_PIN_ENABLED_VERIFIED:
    /* Do not update Card state or Radio state since we want to move to READY only
       when we received INIT_COMPLETED event */
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_PIN UNBLOCKED/VERIFIED/CHANGED -> RIL_PINSTATE_ENABLED_VERIFIED \n");
    *ril_pin_state_ptr = RIL_PINSTATE_ENABLED_VERIFIED;
    break;

  case MMGSDI_PIN_DISABLED:
    /* Do not update Card state or Radio state since we want to move to READY only
       when we received INIT_COMPLETED event */
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_PIN_DISABLED -> RIL_PINSTATE_DISABLED \n");
    *ril_pin_state_ptr = RIL_PINSTATE_DISABLED;
    break;

  case MMGSDI_PIN_BLOCKED:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_PIN_BLOCKED -> RIL_PINSTATE_ENABLED_BLOCKED \n");
    *ril_pin_state_ptr = RIL_PINSTATE_ENABLED_BLOCKED;
    break;

  case MMGSDI_PIN_PERM_BLOCKED:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_PIN_PERM_BLOCKED -> RIL_PINSTATE_ENABLED_PERM_BLOCKED \n");
    *ril_pin_state_ptr = RIL_PINSTATE_ENABLED_PERM_BLOCKED;
    break;

  default:
    QCRIL_LOG_DEBUG( "MMGSDI PIN Status not supported 0x%x \n", mmgsdi_pin_status);
    break;
  }
} /* qcril_mmgsdi_convert_pin_state_to_ril */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_add_app_type_to_index

===========================================================================*/
/*!
    @brief
    Add application type to the corresponding index location in the
    qcril_mmgsdi.curr_card_status application

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_add_app_type_to_index(
  int                  index,
  mmgsdi_app_enum_type app_type,
  boolean             *gw_app_ptr,
  boolean             *cdma_app_ptr)
{

  QCRIL_ASSERT( gw_app_ptr != NULL );
  QCRIL_ASSERT( cdma_app_ptr != NULL );
  QCRIL_ASSERT( index < RIL_CARD_MAX_APPS );
  QCRIL_ASSERT( index >= 0 );

  *cdma_app_ptr = FALSE;
  *gw_app_ptr = FALSE;

  switch (app_type)
  {
  case MMGSDI_APP_RUIM:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_APP_RUIM -> RIL_APPTYPE_RUIM \n");
    qcril_mmgsdi.curr_card_status.applications[index].app_type = RIL_APPTYPE_RUIM;
    *cdma_app_ptr = TRUE;
    if (qcril_mmgsdi.icc_card == QCRIL_MMGSDI_NOT_INIT)
    {
      qcril_mmgsdi.icc_card = QCRIL_MMGSDI_TRUE;
    }
    break;
  case MMGSDI_APP_SIM:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_APP_SIM -> RIL_APPTYPE_SIM \n");
    qcril_mmgsdi.curr_card_status.applications[index].app_type = RIL_APPTYPE_SIM;
    *gw_app_ptr = TRUE;
    if (qcril_mmgsdi.icc_card == QCRIL_MMGSDI_NOT_INIT)
    {
      qcril_mmgsdi.icc_card = QCRIL_MMGSDI_TRUE;
    }
    break;
  case MMGSDI_APP_USIM:
    QCRIL_LOG_DEBUG( "%s", "MMGSDI_APP_USIM -> RIL_APPTYPE_USIM \n");
    qcril_mmgsdi.curr_card_status.applications[index].app_type = RIL_APPTYPE_USIM;
    *gw_app_ptr = TRUE;
    if (qcril_mmgsdi.icc_card == QCRIL_MMGSDI_NOT_INIT)
    {
      qcril_mmgsdi.icc_card = QCRIL_MMGSDI_FALSE;
    }
    break;
  default:
    QCRIL_LOG_DEBUG( "MMGSDI App type not mapped 0x%x \n", app_type );
    qcril_mmgsdi.curr_card_status.applications[index].app_type = RIL_APPTYPE_UNKNOWN;
    break;
  }
} /* qcril_mmgsdi_add_app_type_to_index */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_add_aid_info_to_index

===========================================================================*/
/*!
    @brief
    Add application ID to the corresponding index location in the
    qcril_mmgsdi.curr_card_status application

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_add_aid_info_to_index(
  int                            index,
  const mmgsdi_static_data_type *data_ptr)
{
  QCRIL_ASSERT( data_ptr != NULL );
  QCRIL_ASSERT( index < RIL_CARD_MAX_APPS );
  QCRIL_ASSERT( index >= 0 );

  if(qcril_mmgsdi.curr_card_status.applications[index].aid_ptr != NULL)
  {
    /* Already has the aid ptr, free it */
    free(qcril_mmgsdi.curr_card_status.applications[index].aid_ptr);
    qcril_mmgsdi.curr_card_status.applications[index].aid_ptr = NULL;
  }

  if (data_ptr->data_len <= 0)
  {
    QCRIL_LOG_DEBUG( "input aid len <= 0 %d \n", (int)data_ptr->data_len);
    return;
  }
  /* Convert the AID hex representation into char string with null termination.
     The function bin_to_hexstring will perform the malloc and data conversion.  */
  qcril_mmgsdi.curr_card_status.applications[index].aid_ptr =
    bin_to_hexstring(data_ptr->data_ptr, data_ptr->data_len);

  if (qcril_mmgsdi.curr_card_status.applications[index].aid_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Unable to allocate aid_ptr\n");
  }
} /* qcril_mmgsdi_add_aid_info_to_index */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_add_label_info_to_index

===========================================================================*/
/*!
    @brief
    Add application Label to the corresponding index location in the
    qcril_mmgsdi.curr_card_status application

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_add_label_info_to_index(
  int                            index,
  const mmgsdi_static_data_type *data_ptr)
{
  QCRIL_ASSERT( data_ptr != NULL );
  QCRIL_ASSERT( index < RIL_CARD_MAX_APPS );
  QCRIL_ASSERT( index >= 0 );

  if(qcril_mmgsdi.curr_card_status.applications[index].app_label_ptr != NULL)
  {
    /* Already has the aid ptr, free it */
    free(qcril_mmgsdi.curr_card_status.applications[index].app_label_ptr);
    qcril_mmgsdi.curr_card_status.applications[index].app_label_ptr = NULL;
  }

  if (data_ptr->data_len <= 0)
  {
    QCRIL_LOG_DEBUG( "input label len <= 0 %d \n", (int)data_ptr->data_len);
    return;
  }
  /* Convert the AID hex representation into char string with null termination.
     The function bin_to_hexstring will perform the malloc and data conversion.  */
  qcril_mmgsdi.curr_card_status.applications[index].app_label_ptr =
    bin_to_hexstring(data_ptr->data_ptr, data_ptr->data_len);

  if (qcril_mmgsdi.curr_card_status.applications[index].app_label_ptr == NULL)
  {
    QCRIL_LOG_ERROR( "%s", "Unable to allocate app_label_ptr\n");
  }
} /* qcril_mmgsdi_add_label_info_to_index */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_find_aid_index

===========================================================================*/
/*!
    @brief
    Check if the aid passed in matches any of the aid_ptr presented in the
    qcril mmgsdi global structure

    @return
    None.
*/
/*=========================================================================*/
static boolean qcril_mmgsdi_find_aid_index (
  mmgsdi_app_enum_type           app_type,
  const mmgsdi_static_data_type *aid_ptr,
  int                           *index_ptr
)
{
  int         i                 = 0;
  char       *converted_aid_ptr = NULL;
  boolean     found             = FALSE;
  size_t      converted_aid_len = 0;
  RIL_AppType ril_app_type      = RIL_APPTYPE_UNKNOWN;

  QCRIL_ASSERT( aid_ptr != NULL );
  QCRIL_ASSERT( index_ptr != NULL );

  switch(app_type)
  {
  case MMGSDI_APP_NONE:
    QCRIL_LOG_DEBUG( "%s", "Invalid App Type None \n");
    return found;
  case MMGSDI_APP_SIM:
    ril_app_type = RIL_APPTYPE_SIM;
    break;
  case MMGSDI_APP_USIM:
    ril_app_type = RIL_APPTYPE_USIM;
    break;
  case MMGSDI_APP_RUIM:
    ril_app_type = RIL_APPTYPE_RUIM;
    break;
  default:
    ril_app_type = RIL_APPTYPE_UNKNOWN;
    break;
  }

  if (ril_app_type != RIL_APPTYPE_SIM && ril_app_type != RIL_APPTYPE_RUIM)
  {
    if (aid_ptr->data_len <= 0)
    {
      QCRIL_LOG_DEBUG( "input aid len <= 0 0x%x \n", (unsigned int)aid_ptr->data_len);
      return found;
    }

    converted_aid_ptr = bin_to_hexstring(aid_ptr->data_ptr, aid_ptr->data_len);
    QCRIL_ASSERT( converted_aid_ptr != NULL );

    converted_aid_len = safe_strlen(converted_aid_ptr, MAX_STR);

    if (converted_aid_len == 0)
    {
      free(converted_aid_ptr);
      QCRIL_LOG_DEBUG( "%s", "string len check failed because converted_aid_ptr exceeds MAX_STR \n");
      return found;
    }
  }

  for (i = 0; (i < RIL_CARD_MAX_APPS && i < qcril_mmgsdi.curr_card_status.num_applications); i++)
  {
    if (qcril_mmgsdi.curr_card_status.applications[i].app_type != ril_app_type)
    {
      continue;
    }
    if ((ril_app_type == RIL_APPTYPE_SIM) || (ril_app_type == RIL_APPTYPE_RUIM))
    {
      /* found a match */
      QCRIL_LOG_DEBUG( "App SIM or RUIM match 0x%x \n", (unsigned int)i);
      *index_ptr = (uint32)i;
      found = TRUE;
      break;
    }
    if (qcril_mmgsdi.curr_card_status.applications[i].aid_ptr != NULL)
    {
      if (converted_aid_len !=
           safe_strlen(qcril_mmgsdi.curr_card_status.applications[i].aid_ptr, MAX_STR))
      {
        continue;
      }
      if (!memcmp(converted_aid_ptr,
                  qcril_mmgsdi.curr_card_status.applications[i].aid_ptr,
                  converted_aid_len))
      {
        /* found a match */
        QCRIL_LOG_DEBUG( "aid matches index 0x%x in qcril mmgsdi global \n", i);
        *index_ptr = (uint32)i;
        found = TRUE;
        break;
      }
    }
  }
  free(converted_aid_ptr);
  return found;
} /* qcril_mmgsdi_find_aid_index*/


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_cleanup_curr_card_status

===========================================================================*/
/*!
    @brief
    Clean up the qcril mmgsdi global structure for the curr_card_status

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_cleanup_curr_card_status()
{
  int i = 0;

  for (i = 0; i < RIL_CARD_MAX_APPS; i++)
  {
    if (qcril_mmgsdi.curr_card_status.applications[i].aid_ptr != NULL)
    {
      free(qcril_mmgsdi.curr_card_status.applications[i].aid_ptr);
    }
    if (qcril_mmgsdi.curr_card_status.applications[i].app_label_ptr != NULL)
    {
      free(qcril_mmgsdi.curr_card_status.applications[i].app_label_ptr);
    }
    memset(&qcril_mmgsdi.curr_card_status.applications[i],
           0x00,
           sizeof(RIL_AppStatus));
  }

  qcril_mmgsdi.curr_card_status.card_state          = RIL_CARDSTATE_ABSENT;
  qcril_mmgsdi.curr_card_status.num_applications    = 0;
  qcril_mmgsdi.curr_card_status.universal_pin_state = RIL_PINSTATE_UNKNOWN;
  qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index = RIL_CARD_MAX_APPS;
  qcril_mmgsdi.curr_card_status.cdma_subscription_app_index = RIL_CARD_MAX_APPS;
} /* qcril_mmgsdi_cleanup_curr_card_status */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_process_card_inserted_evt

===========================================================================*/
/*!
    @brief
    Process Card Inserted event:
    Update internal Card State data structure
    Update Radio State if required

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_process_card_inserted_evt
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  mmgsdi_card_inserted_evt_info_type *card_inserted_evt_ptr,
  qcril_request_return_type          *const ret_ptr /*!< Output parameter */
)
{
  boolean              update_gw_sim_state           = FALSE;
  boolean              update_cdma_sim_state         = FALSE;
  boolean              gw_app                        = FALSE;
  boolean              cdma_app                      = FALSE;
  uint32               i                             = 0;

  QCRIL_LOG_INFO( "%s", "qcril_mmgsdi_process_card_inserted_evt \n");

  QCRIL_ASSERT( ret_ptr != NULL );
  QCRIL_ASSERT( card_inserted_evt_ptr != NULL );

  /* update card state and num of applications */
  qcril_mmgsdi.curr_card_status.card_state = RIL_CARDSTATE_PRESENT;

  /* Copy additional info into RIL_CardStatus_v6 structure */
  for (i = 0; (i < card_inserted_evt_ptr->num_aids_avail && i < RIL_CARD_MAX_APPS); i++)
  {
    /* This is the first event received, need to populate everything */
    qcril_mmgsdi_add_aid_info_to_index(i, &card_inserted_evt_ptr->aid_info[i].aid);
    qcril_mmgsdi_add_label_info_to_index(i, &card_inserted_evt_ptr->aid_info[i].label);
    qcril_mmgsdi_add_app_type_to_index(
      i, card_inserted_evt_ptr->aid_info[i].app_type, &gw_app, &cdma_app);
    update_cdma_sim_state |= cdma_app;
    update_gw_sim_state |= gw_app;

    qcril_mmgsdi.curr_card_status.applications[i].app_state = RIL_APPSTATE_DETECTED;
    qcril_mmgsdi.curr_card_status.applications[i].perso_substate =
      RIL_PERSOSUBSTATE_UNKNOWN;
    qcril_mmgsdi.curr_card_status.num_applications++;
  }

  /* Card Inserted can be the first received MMGSDI event that indicates Card status after UE power on or Card power up, notify
     QCRIL(CM) that Card Mode is UP */
  qcril_mmgsdi_update_cm_card_status( instance_id, modem_id, QCRIL_CARD_STATUS_UP );

  qcril_mmgsdi_update_curr_card_state(
    QCRIL_SIM_STATE_NOT_READY,
    update_gw_sim_state,
    update_cdma_sim_state,
    ret_ptr);

  /* Notify QCRIL(PBM) card event */
  qcril_mmgsdi_update_pbm_card_event( instance_id, modem_id, QCRIL_EVT_PBM_CARD_INSERTED );

} /* qcril_mmgsdi_process_card_inserted_evt() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_process_select_aid_evt

===========================================================================*/
/*!
    @brief
    Process Select AID event:
    Update internal Card State data structure
    Update Radio State if required

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_process_select_aid_evt
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  mmgsdi_select_aid_evt_info_type    *select_aid_evt_ptr,
  qcril_request_return_type          *const ret_ptr /*!< Output parameter */
)
{
  boolean              update_gw_sim_state           = FALSE;
  boolean              update_cdma_sim_state         = FALSE;
  boolean              gw_app                        = FALSE;
  boolean              cdma_app                      = FALSE;
  int                  index                         = 0;
  boolean              increment_num_apps            = FALSE;

  QCRIL_LOG_INFO( "%s \n", __FUNCTION__);

  QCRIL_ASSERT( ret_ptr != NULL );
  QCRIL_ASSERT( select_aid_evt_ptr != NULL );

  qcril_mmgsdi.curr_card_status.card_state = RIL_CARDSTATE_PRESENT;

  /* Copy info into RIL_CardStatus_v6 structure */
  if (!qcril_mmgsdi_find_aid_index(
        select_aid_evt_ptr->app_data.app_type,
        &select_aid_evt_ptr->app_data.aid,
        &index))
  {
    /* the AID is not previously listed in the qcril_mmgsdi data structure */
    index = qcril_mmgsdi.curr_card_status.num_applications;
    increment_num_apps = TRUE;
  }

  if ((index >= RIL_CARD_MAX_APPS) || (index < 0))
  {
    QCRIL_LOG_ERROR( "invalid index 0x%x \n", index );
    return;
  }

  qcril_mmgsdi_add_aid_info_to_index(index, &select_aid_evt_ptr->app_data.aid);
  qcril_mmgsdi_add_label_info_to_index(index, &select_aid_evt_ptr->app_data.label);

  qcril_mmgsdi_add_app_type_to_index(index, select_aid_evt_ptr->app_data.app_type,
    &gw_app, &cdma_app);
  update_cdma_sim_state |= cdma_app;
  update_gw_sim_state |= gw_app;

  /* select AID event reflects the subscription application, update the
     subscription app index */
  /* For RPC-based QCRIL, subscription indexes are always stored in [0] */
  if (gw_app)
  {
    qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index = index;
  }
  if (cdma_app)
  {
    qcril_mmgsdi.curr_card_status.cdma_subscription_app_index = index;
  }

  qcril_mmgsdi.curr_card_status.applications[index].app_state = RIL_APPSTATE_DETECTED;
  qcril_mmgsdi.curr_card_status.applications[index].perso_substate =
    RIL_PERSOSUBSTATE_UNKNOWN;

  qcril_mmgsdi.curr_pin.app_index = index;
  qcril_mmgsdi.curr_pin.pin_id = MMGSDI_PIN1;
  qcril_mmgsdi.curr_pin.valid = TRUE;

  if (increment_num_apps)
  {
    qcril_mmgsdi.curr_card_status.num_applications++;
  }

  /* Select AID can be the first received MMGSDI event that indicates Card status after UE power on, notify QCRIL(CM) that
     Card Mode is UP */
  qcril_mmgsdi_update_cm_card_status( instance_id, modem_id, QCRIL_CARD_STATUS_UP );

  qcril_mmgsdi_update_curr_card_state(
    QCRIL_SIM_STATE_NOT_READY,
    update_gw_sim_state,
    update_cdma_sim_state,
    ret_ptr);

} /* qcril_mmgsdi_process_select_aid_evt() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_update_app_state_from_pin_evt

===========================================================================*/
/*!
    @brief
    Update the App State based on the PIN state from the PIN event notification

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_update_app_state_from_pin_evt(
  int                  index,
  RIL_PinState         pin_state,
  boolean             *update_gw_sim_state_ptr,
  boolean             *update_cdma_sim_state_ptr,
  int                 *sim_state_ptr
)
{
  boolean     *init_completed_ptr            = NULL;


  QCRIL_ASSERT( index < RIL_CARD_MAX_APPS );
  QCRIL_ASSERT( index >= 0);
  QCRIL_ASSERT( update_gw_sim_state_ptr != NULL );
  QCRIL_ASSERT( update_cdma_sim_state_ptr != NULL );
  QCRIL_ASSERT( sim_state_ptr != NULL );

  switch (qcril_mmgsdi.curr_card_status.applications[index].app_type)
  {
  case RIL_APPTYPE_RUIM:
  case RIL_APPTYPE_CSIM:
    init_completed_ptr = &qcril_mmgsdi.cdma_init_completed_recv;
    *update_cdma_sim_state_ptr = TRUE;
    break;
  case RIL_APPTYPE_SIM:
  case RIL_APPTYPE_USIM:
    init_completed_ptr = &qcril_mmgsdi.gw_init_completed_recv;
    *update_gw_sim_state_ptr = TRUE;
    break;
  default:
    break;
  }

  switch (pin_state)
  {
  case RIL_PINSTATE_ENABLED_NOT_VERIFIED:
    qcril_mmgsdi.curr_card_status.applications[index].app_state = RIL_APPSTATE_PIN;
    *sim_state_ptr = QCRIL_SIM_STATE_PIN;
    break;

  case RIL_PINSTATE_ENABLED_BLOCKED:
    qcril_mmgsdi.curr_card_status.applications[index].app_state = RIL_APPSTATE_PUK;
    *sim_state_ptr = QCRIL_SIM_STATE_PUK;
    break;

  case RIL_PINSTATE_ENABLED_PERM_BLOCKED:
    qcril_mmgsdi.curr_card_status.applications[index].app_state = RIL_APPSTATE_PUK;
    *sim_state_ptr = QCRIL_SIM_STATE_ABSENT;
    break;

  case RIL_PINSTATE_DISABLED:
  case RIL_PINSTATE_ENABLED_VERIFIED:
    qcril_mmgsdi.curr_card_status.applications[index].app_state = RIL_APPSTATE_READY;
    /* If it is the subscription app, move to the perso state as needed */
    if ((qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index == (int32)index) ||
        (qcril_mmgsdi.curr_card_status.cdma_subscription_app_index == (int32)index))
    {
      if (init_completed_ptr)
      {
        if (*init_completed_ptr)
        {
          *sim_state_ptr = QCRIL_SIM_STATE_READY;
        }
        else
        {
          qcril_mmgsdi.curr_card_status.applications[index].app_state =
            RIL_APPSTATE_SUBSCRIPTION_PERSO;
          if (perso_temp_info_ptr)
          {
            /* Already received the perso event! */
            qcril_mmgsdi.curr_card_status.applications[index].perso_substate =
              perso_temp_info_ptr->perso_state;
            *sim_state_ptr = perso_temp_info_ptr->sim_state;
            free(perso_temp_info_ptr);
            perso_temp_info_ptr = NULL;
            if (*sim_state_ptr == QCRIL_SIM_STATE_ABSENT)
            {
              qcril_mmgsdi.curr_card_status.card_state = RIL_CARDSTATE_ERROR;
            }
          }
        }
      }
    }
    break;

  default:
    break;
  }
} /* qcril_mmgsdi_update_app_state_from_pin_evt */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_process_pin_evts

===========================================================================*/
/*!
    @brief
    Process PIN events:
    Update global variable for PIN info
    Update Radio State if required

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_process_pin_evts
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  mmgsdi_events_enum_type    evt_enum,
  mmgsdi_pin_evt_info_type  *pin_evt_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  RIL_PinState pin_state                     = RIL_PINSTATE_UNKNOWN;
  int          sim_state                     = QCRIL_SIM_STATE_NOT_READY;
  uint32       i                             = 0;
  int          j                             = 0;
  int          index                         = 0;
  boolean      update_gw_sim_state           = FALSE;
  boolean      update_cdma_sim_state         = FALSE;

  QCRIL_LOG_INFO( "%s \n", __FUNCTION__);

  QCRIL_ASSERT( ret_ptr != NULL );
  QCRIL_ASSERT( pin_evt_ptr != NULL );

  qcril_mmgsdi.curr_card_status.card_state = RIL_CARDSTATE_PRESENT;

  qcril_mmgsdi_convert_pin_state_to_ril(pin_evt_ptr->pin_info.status,
                                        &pin_state);

  switch (evt_enum)
  {
  case MMGSDI_PIN1_EVT:
    for (i = 0; (i < pin_evt_ptr->num_aids && i < MMGSDI_MAX_APP_INFO); i++)
    {
      /* Update all the apps that are using this PIN1 */
      if (qcril_mmgsdi_find_aid_index(pin_evt_ptr->aid_type[i].app_type,
                                      &pin_evt_ptr->aid_type[i].aid,
                                      &index))
      {
        qcril_mmgsdi.curr_card_status.applications[index].pin1 = pin_state;

        if (pin_evt_ptr->pin_info.pin_replacement == MMGSDI_PIN_REPLACED_BY_UNIVERSAL)
        {
          qcril_mmgsdi.curr_card_status.applications[index].pin1_replaced = TRUE;
        }
        else
        {
          qcril_mmgsdi.curr_card_status.applications[index].pin1_replaced = FALSE;
          qcril_mmgsdi_update_app_state_from_pin_evt(
            index, pin_state, &update_gw_sim_state, &update_cdma_sim_state, &sim_state);
        }
      }
    }

    if (pin_evt_ptr->pin_info.status == MMGSDI_PIN_PERM_BLOCKED)
    {
      /* Notify QCRIL(PBM) card event */
      QCRIL_LOG_DEBUG("%s","PIN1 Evt received and MMGSDI_PIN_PERM_BLOCKED");
      qcril_mmgsdi_update_pbm_card_event( instance_id, modem_id, QCRIL_EVT_PBM_CARD_ERROR );
    }
    else
    {
      /* If PBM on modem misses card inserted event, it gets the SIM-ECC upon PIN1 evt.
      So if the SIM ECC is not present in property, get the ECC from PBM now. */
      QCRIL_LOG_DEBUG("%s","PIN1 Evt recieved. Check and get the SIM ECC from PBM ");
      qcril_mmgsdi_update_pbm_card_event( instance_id, modem_id, QCRIL_EVT_PBM_CARD_INSERTED );
    }
    break;

  case MMGSDI_PIN2_EVT:
    /* Each PIN2 can be associated with 1 app only */
    if (pin_evt_ptr->num_aids == 1)
    {
      if (qcril_mmgsdi_find_aid_index(pin_evt_ptr->aid_type[0].app_type,
                                      &pin_evt_ptr->aid_type[0].aid,
                                      &index))
      {
        qcril_mmgsdi.curr_card_status.applications[index].pin2 = pin_state;
      }
    }
    else
    {
      QCRIL_LOG_ERROR( "%s", "PIN2_EVT with no App associated with \n");
    }
    break;

  case MMGSDI_UNIVERSAL_PIN_EVT:
    qcril_mmgsdi.curr_card_status.universal_pin_state = pin_state;
    for (j = 0; j < qcril_mmgsdi.curr_card_status.num_applications; j++)
    {
      if (qcril_mmgsdi.curr_card_status.applications[j].pin1_replaced)
      {
        QCRIL_LOG_DEBUG( "App[0x%x] has pin1_replaced \n", j);
        /* the Upin is used to replace Pin1 for this app */
        qcril_mmgsdi_update_app_state_from_pin_evt(
          (uint32)j,
          pin_state,
          &update_gw_sim_state,
          &update_cdma_sim_state,
          &sim_state);
      }
    }
    break;;

  default:
    QCRIL_LOG_ERROR( "Incorrect event 0x%x \n", evt_enum);
    return;
  }

  if (sim_state != QCRIL_SIM_STATE_NOT_READY)
  {
    qcril_mmgsdi_update_curr_card_state(
      sim_state,
      update_gw_sim_state,
      update_cdma_sim_state,
      ret_ptr);
  }

} /* qcril_mmgsdi_process_pin_evts() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_process_card_init_completed_evt

===========================================================================*/
/*!
    @brief
    Process Card Init Completed events:
    Get Sim Capabilities to determine FDN info
    Update QCRIL MMGSDI global info
    Update Radio State if required

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_process_card_init_completed_evt
(
  qcril_instance_id_e_type                 instance_id,
  qcril_modem_id_e_type                    modem_id,
  mmgsdi_card_init_completed_evt_info_type *card_init_completed_evt_ptr,
  qcril_request_return_type                *const ret_ptr /*!< Output parameter */
)
{
  boolean                 update_gw_sim_state           = FALSE;
  boolean                 update_cdma_sim_state         = FALSE;
  boolean                 gw_app                        = FALSE;
  boolean                 cdma_app                      = FALSE;
  uint32                  i                             = 0;
  boolean                 increment_num_apps            = FALSE;
  int                     index                         = 0;
  int                     sim_state                     = QCRIL_SIM_STATE_READY;
  mmgsdi_return_enum_type mmgsdi_status                 = MMGSDI_SUCCESS;
  mmgsdi_access_type      access;

  QCRIL_LOG_INFO( "%s \n", __FUNCTION__);

  QCRIL_ASSERT( ret_ptr != NULL );
  QCRIL_ASSERT( card_init_completed_evt_ptr != NULL );

  qcril_mmgsdi.curr_card_status.card_state = RIL_CARDSTATE_PRESENT;

  /* Copy all the info into RIL_CardStatus_v6 structure */

  // ???????????
  qcril_mmgsdi_convert_pin_state_to_ril(
    card_init_completed_evt_ptr->app_info[0].universal_pin.status,
    &qcril_mmgsdi.curr_card_status.universal_pin_state);

  for (i = 0;
       (i < card_init_completed_evt_ptr->num_avail_apps && i < RIL_CARD_MAX_APPS);
       i++)
  {
    increment_num_apps = FALSE;
    if (!qcril_mmgsdi_find_aid_index(
          card_init_completed_evt_ptr->app_info[i].app_data.app_type,
          &card_init_completed_evt_ptr->app_info[i].app_data.aid,
          &index))
    {
      /* CARD_INIT_COMPELTED event could be the first event received, add the application
         to the right index location */
      index = qcril_mmgsdi.curr_card_status.num_applications;
      increment_num_apps = TRUE;
    }

    if ((index >= RIL_CARD_MAX_APPS) || (index < 0))
    {
      QCRIL_LOG_ERROR( "Invalid index %d \n", index );
      break;
    }

    /* This is the first event received, need to populate everything */
    qcril_mmgsdi_add_aid_info_to_index(
      index, &card_init_completed_evt_ptr->app_info[i].app_data.aid);
    qcril_mmgsdi_add_label_info_to_index(
      index, &card_init_completed_evt_ptr->app_info[i].app_data.label);
    qcril_mmgsdi_convert_pin_state_to_ril(
      card_init_completed_evt_ptr->app_info[i].pin1.status,
      &qcril_mmgsdi.curr_card_status.applications[index].pin1);
    qcril_mmgsdi_convert_pin_state_to_ril(
      card_init_completed_evt_ptr->app_info[i].pin2.status,
      &qcril_mmgsdi.curr_card_status.applications[index].pin2);
    if (card_init_completed_evt_ptr->app_info[i].pin1.pin_replacement ==
        MMGSDI_PIN_REPLACED_BY_UNIVERSAL)
    {
      qcril_mmgsdi.curr_card_status.applications[index].pin1_replaced = TRUE;
    }

    qcril_mmgsdi_add_app_type_to_index(
      index,
      card_init_completed_evt_ptr->app_info[i].app_data.app_type,
      &gw_app, &cdma_app);
    update_cdma_sim_state |= cdma_app;
    update_gw_sim_state |= gw_app;

    /* Card init completed carries application that has the provisioning info.
       Assume 1 app for each technology */
    /* For RPC-based QCRIL, subscription indexes are always stored in [0] */
    if (gw_app)
    {
      qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index = index;
      qcril_mmgsdi.gw_init_completed_recv = TRUE;
    }
    if (cdma_app)
    {
      qcril_mmgsdi.curr_card_status.cdma_subscription_app_index = index;
      qcril_mmgsdi.cdma_init_completed_recv = TRUE;
    }

    qcril_mmgsdi.curr_card_status.applications[index].app_state = RIL_APPSTATE_READY;
    qcril_mmgsdi.curr_card_status.applications[index].perso_substate =
      RIL_PERSOSUBSTATE_READY;
    if(increment_num_apps)
    {
      qcril_mmgsdi.curr_card_status.num_applications++;
    }

    /* Assign the pin as pin1 for the "first" found subscription app */
    if (!qcril_mmgsdi.curr_pin.valid)
    {
      qcril_mmgsdi.curr_pin.app_index = index;
      qcril_mmgsdi.curr_pin.pin_id = MMGSDI_PIN1;
      qcril_mmgsdi.curr_pin.valid = TRUE;
    }
  }

  if (perso_temp_info_ptr)
  {
    /* Received init completed, the perso info must not be valid anymore */
    free(perso_temp_info_ptr);
    perso_temp_info_ptr = NULL;
  }

  /* Card Initialization Completed can be the first received MMGSDI event that
     indicates Card status after UE power on, notify QCRIL(CM) that Card
     Mode is UP */
  qcril_mmgsdi_update_cm_card_status( instance_id, modem_id, QCRIL_CARD_STATUS_UP );

  /* Card is initialized. Get the emergency numbers from pbm */
  qcril_mmgsdi_update_pbm_card_event( instance_id, modem_id, QCRIL_EVT_PBM_CARD_INIT_COMPLETED );

  /* Initialize FDN to NOT_INIT (should be already in that state)
     Read SST or UST to check if FDN is available. Use the userdata
     to carry over the information on update_gw and update_cdma. In
     both cases, I can read only the first byte, as that contains
     the required information. */
  qcril_mmgsdi.fdn_enable = QCRIL_MMGSDI_FDN_NOT_INIT;

  memset(&access, 0, sizeof(mmgsdi_access_type));
  access.access_method = MMGSDI_EF_ENUM_ACCESS;
  if (qcril_mmgsdi.icc_card == QCRIL_MMGSDI_TRUE)
  {
    if (update_cdma_sim_state)
    {
      access.file.file_enum = MMGSDI_CDMA_SVC_TABLE;
    }
    else
    {
      access.file.file_enum = MMGSDI_GSM_SST;
    }
  }
  else
  {
    access.file.file_enum = MMGSDI_USIM_UST;
  }

  QCRIL_LOG_DEBUG( "Reading the service table: 0x%x \n", access.file.file_enum );
  mmgsdi_status = mmgsdi_read_transparent_ext(qcril_mmgsdi.client_id,
                                              MMGSDI_SLOT_1,
                                              access,
                                              0,        /* Offset */
                                              1,        /* Length */
                                              qcril_mmgsdi_internal_read_ust_callback,
                                              (update_gw_sim_state ? QCRIL_MMGSDI_GW_UPDATE : 0) |
                                              (update_cdma_sim_state ? QCRIL_MMGSDI_CDMA_UPDATE : 0));

  if (mmgsdi_status != MMGSDI_SUCCESS)
  {
    QCRIL_LOG_ERROR( "Failed to read SST or UST: 0x%x \n", mmgsdi_status);

    qcril_mmgsdi_update_fdn_status( instance_id, modem_id,
                                    QCRIL_MMGSDI_FDN_NOT_AVAILABLE);
    /* Do continue processing since failure to perform this only means that
       we won't get FDN info */
    qcril_mmgsdi_card_init_completed_end( instance_id, modem_id,
                                          update_gw_sim_state,
                                          update_cdma_sim_state,
                                          ret_ptr );
  }
} /* qcril_mmgsdi_process_card_init_completed_evt() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_process_card_not_accessible_evts

===========================================================================*/
/*!
    @brief
    Process Card Error, Illegal Card events:
    Update Internal Card State global
    Update Radio State if required

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_process_card_not_accessible_evts
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  mmgsdi_events_enum_type             err_evt_type,
  mmgsdi_card_err_info_enum_type      card_err_info,
  qcril_request_return_type          *const ret_ptr /*!< Output parameter */
)
{
  boolean              update_gw_sim_state           = FALSE;
  boolean              update_cdma_sim_state         = FALSE;
  int                  i                             = 0;
  int                  new_sim_state                 = QCRIL_SIM_STATE_ABSENT;

  QCRIL_LOG_INFO( "%s \n", __FUNCTION__);

  QCRIL_ASSERT( ret_ptr != NULL );

  if (qcril_mmgsdi.curr_card_status.card_state == RIL_CARDSTATE_UNKNOWN)
  {
    update_gw_sim_state = TRUE;
    update_cdma_sim_state = TRUE;
  }
  else
  {
    /* find out if there is any application associated with gw or cdma */
    for (i = 0;
         (i < RIL_CARD_MAX_APPS && i < qcril_mmgsdi.curr_card_status.num_applications);
         i++)
    {
      if ((qcril_mmgsdi.curr_card_status.applications[i].app_type == RIL_APPTYPE_SIM) ||
          (qcril_mmgsdi.curr_card_status.applications[i].app_type == RIL_APPTYPE_USIM))
      {
        update_gw_sim_state = TRUE;
      }
      else if ((qcril_mmgsdi.curr_card_status.applications[i].app_type == RIL_APPTYPE_RUIM) ||
          (qcril_mmgsdi.curr_card_status.applications[i].app_type == RIL_APPTYPE_CSIM))
      {
        update_cdma_sim_state = TRUE;
      }
    }
  }

  switch(err_evt_type)
  {
  case MMGSDI_CARD_ERROR_EVT:
  case MMGSDI_CARD_REMOVED_EVT:
    /* Card error, wipe out all the global content pretaining to apps */
    qcril_mmgsdi.curr_card_status.card_state = RIL_CARDSTATE_ABSENT;
    qcril_mmgsdi_cleanup_curr_card_status();
    qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index = RIL_CARD_MAX_APPS;
    qcril_mmgsdi.curr_card_status.cdma_subscription_app_index = RIL_CARD_MAX_APPS;

    qcril_mmgsdi.curr_pin.valid = FALSE;
    qcril_mmgsdi.curr_pin.pin_id = MMGSDI_MAX_PIN_ENUM;
    qcril_mmgsdi.curr_pin.app_index = RIL_CARD_MAX_APPS;
    qcril_mmgsdi.fdn_enable = QCRIL_MMGSDI_FDN_NOT_INIT;
    qcril_mmgsdi.icc_card = QCRIL_MMGSDI_NOT_INIT;
    if ((card_err_info != MMGSDI_CARD_ERR_NO_ATR_RCVD_AT_MAX_VOLT) &&
        (card_err_info != MMGSDI_CARD_ERR_NO_ATR_RCVD_AFTER_RESET))
    {
      qcril_mmgsdi.curr_card_status.card_state = RIL_CARDSTATE_ERROR;
      new_sim_state = QCRIL_SIM_STATE_CARD_ERROR;
    }
    qcril_mmgsdi.cdma_init_completed_recv = FALSE;
    qcril_mmgsdi.gw_init_completed_recv = FALSE;

    /* Notify QCRIL(PBM) card event */
    qcril_mmgsdi_update_pbm_card_event( instance_id, modem_id, QCRIL_EVT_PBM_CARD_ERROR );

    /* Notify QCRIL that Card Mode is DOWN */
    if ( card_err_info == MMGSDI_CARD_ERR_PWR_DN_CMD_NOTIFY )
    {
      qcril_mmgsdi_update_cm_card_status( instance_id, modem_id, QCRIL_CARD_STATUS_DOWN );
    }
    /* Notify QCRIL that Card Mode is ILLEGAL */
    else
    {
      qcril_mmgsdi_update_cm_card_status( instance_id, modem_id, QCRIL_CARD_STATUS_NOT_ACCESSIBLE );
    }
    break;

  case MMGSDI_ILLEGAL_CARD_EVT: /* e.g., stolen SIM */
    /* keep the card as present since access to application is still allowed */
    qcril_mmgsdi.curr_card_status.card_state = RIL_CARDSTATE_PRESENT;
    new_sim_state = QCRIL_SIM_STATE_ILLEGAL;

      /* mark the applications as illegal */
      if (qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index >= 0 &&
          qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index < RIL_CARD_MAX_APPS)
      {
        i = qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index;
        qcril_mmgsdi.curr_card_status.applications[i].app_state = RIL_APPSTATE_ILLEGAL;

        update_gw_sim_state = TRUE;
      }
      if (qcril_mmgsdi.curr_card_status.cdma_subscription_app_index >= 0 &&
          qcril_mmgsdi.curr_card_status.cdma_subscription_app_index < RIL_CARD_MAX_APPS)
      {
        i = qcril_mmgsdi.curr_card_status.cdma_subscription_app_index;
        qcril_mmgsdi.curr_card_status.applications[i].app_state = RIL_APPSTATE_ILLEGAL;

      update_cdma_sim_state = TRUE;
    }

    /* Notify QCRIL that Card Mode is ILLEGAL */
    qcril_mmgsdi_update_cm_card_status( instance_id, modem_id, QCRIL_CARD_STATUS_ILLEGAL );
    break;

  default:
    QCRIL_LOG_ERROR( "invalid mmgsdi evt for card_err processing 0x%x\n", err_evt_type);
    return;
  }

  qcril_mmgsdi_update_curr_card_state(
    new_sim_state,
    update_gw_sim_state,
    update_cdma_sim_state,
    ret_ptr);

} /* qcril_mmgsdi_process_card_not_accessible_evts() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_process_refresh_evt

===========================================================================*/
/*!
    @brief
    Process Refresh Card and Refresh events:
    Update Internal Card State global
    Update Radio State if required

    @return
    None.
*/
/*=========================================================================*/
static void qcril_mmgsdi_process_refresh_evt
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  mmgsdi_refresh_evt_info_type      *refresh_evt_ptr,
  qcril_request_return_type         *const ret_ptr /*!< Output parameter */
)
{
  boolean                           gw_app                        = FALSE;
  boolean                           cdma_app                      = FALSE;
  int                               i                             = 0;
  mmgsdi_option_type                option;
  mmgsdi_return_enum_type           mmgsdi_status                 = MMGSDI_SUCCESS;
  boolean                           send_refresh_unsol            = FALSE;
  RIL_SimRefreshResponse_v6         ril_refresh_data;
  boolean                           aid_allocated                 = FALSE;
  RIL_SimRefreshResult              ril_refresh_mode              = SIM_FILE_UPDATE;
  int                               ril_sim_state                 = 0;
  int                               index                         = RIL_CARD_MAX_APPS;
  qcril_unsol_resp_params_type unsol_resp;

  QCRIL_LOG_INFO( "%s \n", __FUNCTION__);

  QCRIL_ASSERT( ret_ptr != NULL );
  QCRIL_ASSERT( refresh_evt_ptr != NULL );

  memset(&option, 0x00, sizeof(mmgsdi_option_type));

  switch (refresh_evt_ptr->stage)
  {
  case MMGSDI_REFRESH_STAGE_WAIT_FOR_OK_TO_FCN:
    QCRIL_LOG_RPC2( modem_id, "mmgsdi_ok_to_refresh()", "" );
    if(qcril_mmgsdi.client_id == QCRIL_MMGSDI_INVALID_CLIENT_ID)
    {
      QCRIL_LOG_ERROR("%s", "Invalid client id, can not call mmgsdi_ok_to_refresh() \n");
      return;
    }
    mmgsdi_status = mmgsdi_ok_to_refresh (
      qcril_mmgsdi.client_id,
      MMGSDI_SLOT_1,
      TRUE, /* ok_to_refresh */
      option,
      qcril_mmgsdi_command_callback,
      0);
    if (mmgsdi_status != MMGSDI_SUCCESS)
    {
      QCRIL_LOG_ERROR( "%s: failed with %d\n", __FUNCTION__, mmgsdi_status);
    }
    return;

  case MMGSDI_REFRESH_STAGE_START:
    switch (refresh_evt_ptr->mode)
    {
      case MMGSDI_REFRESH_NAA_FCN:
        /* Perform refresh_complete, the Unsol notification will be sent
           upon STAGE_END_SUCCESS */
        if(qcril_mmgsdi.client_id == QCRIL_MMGSDI_INVALID_CLIENT_ID)
        {
          QCRIL_LOG_ERROR("%s", "Invalid client id, can not call mmgsdi_refresh_complete() \n");
          return;
        }
        QCRIL_LOG_RPC2( modem_id, "mmgsdi_refresh_complete()" , "" );
        mmgsdi_status = mmgsdi_refresh_complete (
          qcril_mmgsdi.client_id,
          MMGSDI_SLOT_1,
          TRUE, /* ok_to_refresh */
          option,
          qcril_mmgsdi_command_callback,
          0);
        if (mmgsdi_status != MMGSDI_SUCCESS)
        {
          QCRIL_LOG_ERROR( "%s: failed with %d\n", __FUNCTION__, mmgsdi_status);
        }
        return;

      case MMGSDI_REFRESH_NAA_INIT_FULL_FCN:
      case MMGSDI_REFRESH_NAA_INIT_FCN:
      case MMGSDI_REFRESH_NAA_INIT:
        if (refresh_evt_ptr->aid.aid.data_len == 0)
        {
          /* default app */
          gw_app = TRUE;
          cdma_app = TRUE;
        }
        else
        {
          if (!qcril_mmgsdi_find_aid_index(refresh_evt_ptr->aid.app_type,
                                           &refresh_evt_ptr->aid.aid,
                                           &index))
          {
            /* the AID is not previously listed in the qcril_mmgsdi data structure */
            index = RIL_CARD_MAX_APPS;
          }
          if ((index >= RIL_CARD_MAX_APPS) || (index < 0))
          {
            QCRIL_LOG_ERROR( "invalid index 0x%x \n", (unsigned int)index);
            return;
          }

          /* For RPC-based QCRIL, subscription indexes are always stored in [0] */
          if (index == qcril_mmgsdi.curr_card_status.cdma_subscription_app_index)
          {
            cdma_app = TRUE;
          }
          else if (index ==
                   qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index)
          {
            gw_app = TRUE;
          }
          else
          {
            QCRIL_LOG_ERROR( "Non Provisioning App App Reset Refresh [0x%x], not supported \n", (unsigned int)index);
            return;
          }
        }

        if (gw_app)
        {
          index = qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index;
          if ((index < RIL_CARD_MAX_APPS) && (index >= 0))
          {
            qcril_mmgsdi.curr_card_status.applications[index].app_state =
              RIL_APPSTATE_SUBSCRIPTION_PERSO;
            qcril_mmgsdi.curr_card_status.applications[index].perso_substate =
              RIL_PERSOSUBSTATE_UNKNOWN;
          }
          qcril_mmgsdi.gw_init_completed_recv = FALSE;
        }
        if (cdma_app)
        {
          index = qcril_mmgsdi.curr_card_status.cdma_subscription_app_index;
          if ((index < RIL_CARD_MAX_APPS) && (index >= 0))
          {
            qcril_mmgsdi.curr_card_status.applications[index].app_state =
              RIL_APPSTATE_SUBSCRIPTION_PERSO;
            qcril_mmgsdi.curr_card_status.applications[index].perso_substate =
              RIL_PERSOSUBSTATE_UNKNOWN;
          }
          qcril_mmgsdi.cdma_init_completed_recv = FALSE;
        }
        qcril_mmgsdi.fdn_enable = QCRIL_MMGSDI_FDN_NOT_INIT;
        send_refresh_unsol = TRUE;
        ril_refresh_mode = SIM_INIT;
        ril_sim_state = QCRIL_SIM_STATE_NOT_READY;
        break;

      case MMGSDI_REFRESH_RESET:
      case MMGSDI_REFRESH_RESET_AUTO:
        if (qcril_mmgsdi.curr_card_status.card_state == RIL_CARDSTATE_UNKNOWN)
        {
          gw_app = TRUE;
          cdma_app = TRUE;
        }
        else
        {
          /* find out if there is any application associated with gw or cdma */
          for (i = 0;
               (i < RIL_CARD_MAX_APPS && i < qcril_mmgsdi.curr_card_status.num_applications);
               i++)
          {
            if ((qcril_mmgsdi.curr_card_status.applications[i].app_type == RIL_APPTYPE_SIM) ||
                (qcril_mmgsdi.curr_card_status.applications[i].app_type == RIL_APPTYPE_USIM))
            {
              gw_app = TRUE;
            }
            else if ((qcril_mmgsdi.curr_card_status.applications[i].app_type == RIL_APPTYPE_RUIM) ||
                (qcril_mmgsdi.curr_card_status.applications[i].app_type == RIL_APPTYPE_CSIM))
            {
              cdma_app = TRUE;
            }
          }
        }

        /* Update as if there is no card because we expect to receive subsequent
           power up like events.
           We expect to receive events starting from Card Inserted event again */
        qcril_mmgsdi.curr_card_status.card_state = RIL_CARDSTATE_ABSENT;
        qcril_mmgsdi_cleanup_curr_card_status();
        qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index = RIL_CARD_MAX_APPS;
        qcril_mmgsdi.curr_card_status.cdma_subscription_app_index = RIL_CARD_MAX_APPS;

        qcril_mmgsdi.curr_pin.valid = FALSE;
        qcril_mmgsdi.curr_pin.pin_id = MMGSDI_MAX_PIN_ENUM;
        qcril_mmgsdi.curr_pin.app_index = RIL_CARD_MAX_APPS;
        qcril_mmgsdi.fdn_enable = QCRIL_MMGSDI_FDN_NOT_INIT;
        qcril_mmgsdi.icc_card = QCRIL_MMGSDI_NOT_INIT;
        qcril_mmgsdi.gw_init_completed_recv = FALSE;
        qcril_mmgsdi.cdma_init_completed_recv = FALSE;
        /* Don't send unsol refresh for MMGSDI_REFRESH_AUTO that inappropiately triggers Android to ask RIL to go into LPM */
        if ( refresh_evt_ptr->mode == MMGSDI_REFRESH_RESET )
        {
          send_refresh_unsol = TRUE;
          ril_refresh_mode = SIM_RESET;
        }
        ril_sim_state = QCRIL_SIM_STATE_NOT_READY;

        /* Card Refresh, notify QCRIL(CM) that Card Mode is REFRESH */
        qcril_mmgsdi_update_cm_card_status( instance_id, modem_id, QCRIL_CARD_STATUS_REFRESH );

        /* Notify QCRIL(PBM) card event */
        qcril_mmgsdi_update_pbm_card_event( instance_id, modem_id, QCRIL_EVT_PBM_CARD_ERROR );
        break;

      case MMGSDI_REFRESH_NAA_APP_RESET:
      case MMGSDI_REFRESH_3G_SESSION_RESET:
#ifdef NAA_REFRESH_SUPPORTED
        /* We expect to receive events starting from Selected AID event again */
        if (refresh_evt_ptr->aid.aid.data_len == 0)
        {
          /* default app */
          gw_app = TRUE;
          cdma_app = TRUE;
        }
        else
        {
          if (!qcril_mmgsdi_find_aid_index(refresh_evt_ptr->aid.app_type,
                                           &refresh_evt_ptr->aid.aid,
                                           &index))
          {
            /* the AID is not previously listed in the qcril_mmgsdi data structure */
            index = RIL_CARD_MAX_APPS;
          }
          if ((index >= RIL_CARD_MAX_APPS) || (index < 0))
          {
            QCRIL_LOG_ERROR( "invalid index 0x%x \n", (unsigned int)index);
            return;
          }

          /* For RPC-based QCRIL, subscription indexes are always stored in [0] */
          if (index == qcril_mmgsdi.curr_card_status.cdma_subscription_app_index)
          {
            cdma_app = TRUE;
          }
          else if (index == qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index)
          {
            gw_app = TRUE;
          }
          else
          {
            QCRIL_LOG_ERROR( "Non Provisioning App App Reset Refresh [0x%x], not supported \n", (unsigned int)index);
            return;
          }
        }

        /* Reset the app_index for the corresponding technology */
        if (gw_app)
        {
          index = qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index;
          if ((index < RIL_CARD_MAX_APPS) && (index >= 0))
          {
            qcril_mmgsdi.curr_card_status.applications[index].app_state =
              RIL_APPSTATE_DETECTED;
            qcril_mmgsdi.curr_card_status.applications[index].perso_substate =
              RIL_PERSOSUBSTATE_UNKNOWN;
            qcril_mmgsdi.curr_card_status.applications[index].pin1_replaced = FALSE;
            qcril_mmgsdi.curr_card_status.applications[index].pin1 =
              RIL_PINSTATE_UNKNOWN;
            qcril_mmgsdi.curr_card_status.applications[index].pin2 =
              RIL_PINSTATE_UNKNOWN;
          }
          qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index = RIL_CARD_MAX_APPS;
          qcril_mmgsdi.gw_init_completed_recv = FALSE;
        }
        if (cdma_app)
        {
          index = qcril_mmgsdi.curr_card_status.cdma_subscription_app_index;
          if ((index < RIL_CARD_MAX_APPS) && (index >= 0))
          {
            qcril_mmgsdi.curr_card_status.applications[index].app_state =
              RIL_APPSTATE_DETECTED;
            qcril_mmgsdi.curr_card_status.applications[index].perso_substate =
              RIL_PERSOSUBSTATE_UNKNOWN;
            qcril_mmgsdi.curr_card_status.applications[index].pin1_replaced = FALSE;
            qcril_mmgsdi.curr_card_status.applications[index].pin1 =
              RIL_PINSTATE_UNKNOWN;
            qcril_mmgsdi.curr_card_status.applications[index].pin2 =
              RIL_PINSTATE_UNKNOWN;
          }
          qcril_mmgsdi.curr_card_status.cdma_subscription_app_index = RIL_CARD_MAX_APPS;
          qcril_mmgsdi.cdma_init_completed_recv = FALSE;
        }
        qcril_mmgsdi.fdn_enable = QCRIL_MMGSDI_FDN_NOT_INIT;
        send_refresh_unsol = TRUE;
        ril_refresh_mode = //;
#endif /* NAA_REFRESH_SUPPORTED */
        break;

      case MMGSDI_REFRESH_MODE_ENUM_MAX:
        QCRIL_LOG_ERROR( "%s", "invalid mode: MODE_ENUM_MAX");
        return;

      default:
        QCRIL_LOG_ERROR( "%s 0x%x\n", "Refresh mode not supported:", refresh_evt_ptr->mode );
        return;
    }
    break;

  case MMGSDI_REFRESH_STAGE_END_SUCCESS:
    if (refresh_evt_ptr->mode == MMGSDI_REFRESH_NAA_FCN)
    {
      send_refresh_unsol = TRUE;
    }
    break;

  default:
    break;
  }

  /* Update all QCRIL modules on card state change before sending RIL_UNSOL_SIM_REFRESH notification */
  qcril_mmgsdi_update_curr_card_state(
    ril_sim_state,
    gw_app,
    cdma_app,
    ret_ptr);

  if (send_refresh_unsol)
  {
    memset(&ril_refresh_data, 0, sizeof(RIL_SimRefreshResponse_v6));

    /* Update refreshResult */
    ril_refresh_data.result = ril_refresh_mode;

    /* Update AID, initialize to an empty string */
    ril_refresh_data.aid = "";
    if ((refresh_evt_ptr->aid.aid.data_len >= 0) &&
        (refresh_evt_ptr->aid.aid.data_ptr != NULL))
    {
      char *   converted_aid_ptr = NULL;

      converted_aid_ptr = bin_to_hexstring(refresh_evt_ptr->aid.aid.data_ptr,
                                           refresh_evt_ptr->aid.aid.data_len);
      if(converted_aid_ptr != NULL)
      {
        ril_refresh_data.aid = converted_aid_ptr;
        aid_allocated = TRUE;
        converted_aid_ptr = NULL;
      }
    }

    QCRIL_LOG_DEBUG( "Sending RIL_UNSOL_SIM_REFRESH, RIL_SimRefreshResult: 0x%x, AID: %s",
                     ril_refresh_data.result, ril_refresh_data.aid );

    if (ril_refresh_mode == SIM_FILE_UPDATE)
    {
      uint32      i, j = 0;
      uint32      app_map_size = 0;
      EFMap       *app_map  = NULL;

      if (qcril_mmgsdi.icc_card == QCRIL_MMGSDI_TRUE)
      {
        app_map = gsm_ef_mapping;
        app_map_size = gsm_ef_map_size;
      }
      else
      {
        app_map = umts_ef_mapping;
        app_map_size = umts_ef_map_size;
      }

      for (i = 0; i < refresh_evt_ptr->refresh_files.num_files; i++)
      {
        for (j = 0; j < app_map_size; j++)
        {
          if (refresh_evt_ptr->refresh_files.file_list_ptr[i] ==
              app_map[j].file.gsdi_enum)
          {
            ril_refresh_data.ef_id = app_map[j].EF;

            QCRIL_LOG_INFO( "Sending RIL_UNSOL_SIM_REFRESH - SIM_FILE_UPDATE for 0x%x \n",
                            ril_refresh_data.ef_id);
            qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_SIM_REFRESH, &unsol_resp );
            unsol_resp.resp_pkt = ( void * ) &ril_refresh_data;
            unsol_resp.resp_len = sizeof( RIL_SimRefreshResponse_v6 );
            qcril_send_unsol_response( &unsol_resp );
            break;
          }
        }
      }
    }
    else
    {
      qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_SIM_REFRESH, &unsol_resp );
      unsol_resp.resp_pkt = ( void * ) &ril_refresh_data;
      unsol_resp.resp_len = sizeof( RIL_SimRefreshResponse_v6 );
      qcril_send_unsol_response( &unsol_resp );
    }

    /* Free up any allocated memory for AID */
    if(aid_allocated)
    {
      free(ril_refresh_data.aid);
      ril_refresh_data.aid = NULL;
      QCRIL_LOG_DEBUG("%s", "Freed ril_refresh_data.aidPtr \n");
    }
  }

} /* qcril_mmgsdi_process_refresh_evt() */


/*===========================================================================

                               QCRIL INTERFACE FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_mmgsdi_init

===========================================================================*/
/*!
    @brief
    Initialize the MMGSDI subsystem of the RIL.

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_init
(
 void
)
{
  qcril_modem_id_e_type   modem_id      = QCRIL_DEFAULT_MODEM_ID;
  mmgsdi_return_enum_type mmgsdi_status = MMGSDI_SUCCESS;
  gsdi_returns_T          gsdi_status   = GSDI_SUCCESS;
  int                     i             = 0;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s", "qcril_mmgsdi_init\n");

  memset(&qcril_mmgsdi, 0x00, sizeof(qcril_mmgsdi_struct_type));

  qcril_mmgsdi.gw_curr_card_state = QCRIL_SIM_STATE_NOT_READY;

  qcril_mmgsdi.curr_card_status.card_state = RIL_CARDSTATE_UNKNOWN;
  for (i = 0; i < RIL_CARD_MAX_APPS; i++)
  {
    qcril_mmgsdi.curr_card_status.applications[i].app_state = RIL_APPSTATE_UNKNOWN;
    qcril_mmgsdi.curr_card_status.applications[i].perso_substate =
      RIL_PERSOSUBSTATE_UNKNOWN;
  }
  qcril_mmgsdi.curr_pin.pin_id = MMGSDI_MAX_PIN_ENUM;
  qcril_mmgsdi.curr_pin.app_index = RIL_CARD_MAX_APPS;
  qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index = RIL_CARD_MAX_APPS;
  qcril_mmgsdi.curr_card_status.cdma_subscription_app_index = RIL_CARD_MAX_APPS;

  qcril_mmgsdi.icc_card = QCRIL_MMGSDI_NOT_INIT;
  qcril_mmgsdi.fdn_enable = QCRIL_MMGSDI_FDN_NOT_INIT;
  qcril_mmgsdi.client_id = QCRIL_MMGSDI_INVALID_CLIENT_ID;

  /* Initialize EF maps used for path translation */
  qcril_mmgsdi_init_maps();

  /* mmgsdi client ID registration */
  QCRIL_LOG_RPC2( modem_id, "mmgsdi_client_id_reg()", "" );
  mmgsdi_status = mmgsdi_client_id_reg( qcril_mmgsdi_command_callback,
                                        0);
  if (MMGSDI_SUCCESS != mmgsdi_status)
  {
    QCRIL_LOG_ERROR( "mmgsdi_client_id_reg failed mmgsdi_status: 0x%x\n", mmgsdi_status);
    return;
  }

  QCRIL_LOG_RPC2( modem_id, "gsdi_perso_register_task()", "" );
  gsdi_status = gsdi_perso_register_task( qcril_mmgsdi_perso_event_callback,
                                          0,
                                          qcril_mmgsdi_gsdi_command_callback);

  if (GSDI_SUCCESS != gsdi_status)
  {
    QCRIL_LOG_ERROR( "gsdi_perso_register_task failed gsdi_status: 0x%x\n", gsdi_status);
  }

} /* qcril_mmgsdi_init() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_release

===========================================================================*/
/*!
    @brief
    Release the MMGSDI object.

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_release
(
 void
)
{
  qcril_modem_id_e_type   modem_id      = QCRIL_DEFAULT_MODEM_ID;
  mmgsdi_return_enum_type mmgsdi_status = MMGSDI_SUCCESS;
  /* 0 is not a valid mmgsdi client_id and qcril_mmgsdi initialize to
     0 client_id value */
  if ( qcril_mmgsdi.client_id  != QCRIL_MMGSDI_INVALID_CLIENT_ID )
  {
    QCRIL_LOG_RPC( modem_id, "mmgsdi_client_id_dereg()",
                  "client_id", (int)qcril_mmgsdi.client_id );
    mmgsdi_status = mmgsdi_client_id_dereg( qcril_mmgsdi.client_id,
                                            NULL,
                                            0 );
    if (mmgsdi_status != MMGSDI_SUCCESS)
    {
      QCRIL_LOG_ERROR( "  Failed to deregister with MMGSDI 0x%x\n", mmgsdi_status);
    }
    qcril_mmgsdi.client_id = QCRIL_MMGSDI_INVALID_CLIENT_ID;
  }

} /* qcril_mmgsdi_release */


/*=========================================================================

  FUNCTION:  qcril_mmgsdi_process_internal_read_ust_callback

===========================================================================*/
/*!
    @brief
    Handle MMGSDI command callback.

    @return
    None
*/
/*=========================================================================*/
void qcril_mmgsdi_process_internal_read_ust_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type                   instance_id;
  qcril_modem_id_e_type                      modem_id;
  mmgsdi_client_data_type                    client_data           = 0;
  boolean                                    update_gw_sim_state   = FALSE;
  boolean                                    update_cdma_sim_state = FALSE;
  boolean                                    fdn_available         = FALSE;
  qcril_mmgsdi_command_callback_params_type *mmgsdi_cmd_cnf_ptr    = NULL;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: QCRIL_EVT_INTERNAL_MMGSDI_READ_UST_VALUE\n", __FUNCTION__);

  QCRIL_ASSERT( params_ptr != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  mmgsdi_cmd_cnf_ptr = (qcril_mmgsdi_command_callback_params_type *) params_ptr->data;
  QCRIL_ASSERT( mmgsdi_cmd_cnf_ptr != NULL );
  QCRIL_ASSERT( mmgsdi_cmd_cnf_ptr->cnf_ptr != NULL );

  /* Userdata contains information on GW and CDMA update */
  client_data = mmgsdi_cmd_cnf_ptr->cnf_ptr->read_cnf.response_header.client_data;

  update_gw_sim_state   = (boolean)(client_data & QCRIL_MMGSDI_GW_UPDATE);
  update_cdma_sim_state = (boolean)(client_data & QCRIL_MMGSDI_CDMA_UPDATE);

  if (mmgsdi_cmd_cnf_ptr->status == MMGSDI_SUCCESS)
  {
    /* For 2G SIM card, we read the EF-SST or EF-CST... in any case
       the service 3 contains the FDN status */
    if (qcril_mmgsdi.icc_card == QCRIL_MMGSDI_TRUE &&
        mmgsdi_cmd_cnf_ptr->cnf_ptr->read_cnf.read_data.data_len > 0 &&
        mmgsdi_cmd_cnf_ptr->cnf_ptr->read_cnf.read_data.data_ptr != NULL)
    {
      if (mmgsdi_cmd_cnf_ptr->cnf_ptr->read_cnf.read_data.data_ptr[0] &
          QCRIL_MMGSDI_FDN_AVAILABLE_IN_ICC)
      {
        fdn_available = TRUE;
      }
    }

    /* For 3G USIM card, we read the EF-UST */
    if (qcril_mmgsdi.icc_card == QCRIL_MMGSDI_FALSE &&
        mmgsdi_cmd_cnf_ptr->cnf_ptr->read_cnf.read_data.data_len > 0 &&
        mmgsdi_cmd_cnf_ptr->cnf_ptr->read_cnf.read_data.data_ptr != NULL)
    {
      if (mmgsdi_cmd_cnf_ptr->cnf_ptr->read_cnf.read_data.data_ptr[0] &
          QCRIL_MMGSDI_FDN_AVAILABLE_IN_UICC)
      {
        fdn_available = TRUE;
      }
    }
  }

  QCRIL_LOG_INFO( "FDN available: 0x%x \n", fdn_available);

  /* free cnf ptr, allocated in the qcril_mmgsdi_command_callback */
  if (mmgsdi_cmd_cnf_ptr != NULL)
    free(mmgsdi_cmd_cnf_ptr);

  if (fdn_available)
  {
    sim_capabilities_T   dummy_sim_cap;
    gsdi_returns_T       gsdi_status  = GSDI_SUCCESS;

    memset(&dummy_sim_cap, 0, sizeof(sim_capabilities_T));

    /* FDN is available. Continue checking if it's activated */
    QCRIL_LOG_RPC2( modem_id, "gsdi_get_sim_capabilities()", "" );
    gsdi_status = gsdi_get_sim_capabilities(&dummy_sim_cap,
                                            (update_gw_sim_state ? QCRIL_MMGSDI_GW_UPDATE : 0) |
                                            (update_cdma_sim_state ? QCRIL_MMGSDI_CDMA_UPDATE : 0),
                                            qcril_mmgsdi_gsdi_command_callback);
    if (GSDI_SUCCESS == gsdi_status)
    {
      return;
    }
    QCRIL_LOG_ERROR( "Failed to call gsdi_get_sim_capabilities 0x%x \n", gsdi_status);
  }

  /* FDN is not available */
  qcril_mmgsdi_update_fdn_status( instance_id, modem_id, QCRIL_MMGSDI_FDN_NOT_AVAILABLE );

  qcril_mmgsdi_card_init_completed_end( instance_id, modem_id,
                                        update_gw_sim_state,
                                        update_cdma_sim_state,
                                        ret_ptr );

} /* qcril_mmgsdi_process_internal_read_ust_callback */


/*=========================================================================

  FUNCTION:  qcril_mmgsdi_process_command_callback

===========================================================================*/
/*!
    @brief
    Handle MMGSDI command callback.

    @return
    None
*/
/*=========================================================================*/
void qcril_mmgsdi_process_command_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  qcril_mmgsdi_command_callback_params_type *mmgsdi_cmd_cnf_ptr = NULL;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: QCRIL_EVT_MMGSDI_COMMAND_CALLBACK\n", __FUNCTION__);

  QCRIL_ASSERT( params_ptr != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  mmgsdi_cmd_cnf_ptr = (qcril_mmgsdi_command_callback_params_type *) params_ptr->data;
  QCRIL_ASSERT( mmgsdi_cmd_cnf_ptr != NULL );

  switch (mmgsdi_cmd_cnf_ptr->cnf)
  {
  case MMGSDI_CLIENT_ID_REG_CNF:
    qcril_mmgsdi_process_client_id_reg_cnf(mmgsdi_cmd_cnf_ptr->status,
                                           mmgsdi_cmd_cnf_ptr->cnf,
                                           mmgsdi_cmd_cnf_ptr->cnf_ptr);
    break;
  case MMGSDI_CLIENT_EVT_REG_CNF:
    qcril_mmgsdi_process_client_evt_reg_cnf(mmgsdi_cmd_cnf_ptr->status,
                                            mmgsdi_cmd_cnf_ptr->cnf,
                                            mmgsdi_cmd_cnf_ptr->cnf_ptr);
    break;
  case MMGSDI_PIN_OPERATION_CNF:
    qcril_mmgsdi_sec_process_cnf( instance_id,
                                  mmgsdi_cmd_cnf_ptr->status,
                                  mmgsdi_cmd_cnf_ptr->cnf,
                                  mmgsdi_cmd_cnf_ptr->cnf_ptr );
    break;
  case MMGSDI_WRITE_CNF:
    qcril_mmgsdi_common_simio_update_cnf(params_ptr, ret_ptr);
    break;
  case MMGSDI_READ_CNF:
    qcril_mmgsdi_common_simio_read_cnf(params_ptr, ret_ptr);
    break;
  case MMGSDI_CARD_STATUS_CNF:
    qcril_mmgsdi_common_simio_get_status_cnf(params_ptr, ret_ptr);
    break;
  case MMGSDI_GET_FILE_ATTR_CNF:
    qcril_mmgsdi_common_process_cnf( instance_id,
                                     mmgsdi_cmd_cnf_ptr->status,
                                     mmgsdi_cmd_cnf_ptr->cnf,
                                     mmgsdi_cmd_cnf_ptr->cnf_ptr );
    break;

  case MMGSDI_CARD_PDOWN_CNF:
    QCRIL_LOG_DEBUG( "Cnf 0x%x status 0x%x \n", mmgsdi_cmd_cnf_ptr->cnf, mmgsdi_cmd_cnf_ptr->status);
    if ( mmgsdi_cmd_cnf_ptr->status != MMGSDI_SUCCESS )
    {
      /* Card PowerDown Failure, notify QCRIL that Card Mode PowerDown failure */
      qcril_mmgsdi_update_cm_card_status( instance_id, modem_id, QCRIL_CARD_STATUS_POWERDOWN_FAILED );
    }
    break;

  case MMGSDI_CARD_PUP_CNF:
    QCRIL_LOG_DEBUG( "Cnf 0x%x status 0x%x \n", mmgsdi_cmd_cnf_ptr->cnf, mmgsdi_cmd_cnf_ptr->status);
    if (( mmgsdi_cmd_cnf_ptr->status != MMGSDI_SUCCESS ) &&
        ( mmgsdi_cmd_cnf_ptr->status != MMGSDI_REFRESH_SUCCESS))
    {
      /* Card PowerUp Failure, notify QCRIL that Card Mode PowerUp failure */
      qcril_mmgsdi_update_cm_card_status( instance_id, modem_id, QCRIL_CARD_STATUS_POWERUP_FAILED );
    }
    break;

  case MMGSDI_REFRESH_CNF:
    if (mmgsdi_cmd_cnf_ptr->cnf_ptr != NULL)
    {
      QCRIL_LOG_DEBUG( "MMGSDI_REFRESH_CNF type 0x%x status 0x%x \n",
                       mmgsdi_cmd_cnf_ptr->cnf_ptr->refresh_cnf.orig_refresh_req,
                       mmgsdi_cmd_cnf_ptr->status);
    }
    else
    {
      QCRIL_LOG_DEBUG( "MMGSDI_REFRESH_CNF cnf_ptr == NULL, status 0x%x \n",  mmgsdi_cmd_cnf_ptr->status);
    }
    break;

  default:
    /* Cleanup pending request */
    QCRIL_LOG_ERROR( "Unhandled Cnf 0x%x status 0x%x \n", mmgsdi_cmd_cnf_ptr->cnf, mmgsdi_cmd_cnf_ptr->status);
    qcril_mmgsdi_response( instance_id, params_ptr->t,MMGSDI_NOT_SUPPORTED, NULL,0,TRUE,NULL );

    break;
  }

  /*-----------------------------------------------------------------------*/

  /* free cnf ptr, allocated in the qcril_mmgsdi_command_callback */
  if (mmgsdi_cmd_cnf_ptr != NULL)
    free(mmgsdi_cmd_cnf_ptr);

} /* qcril_mmgsdi_process_command_callback() */


/*=========================================================================
  FUNCTION:  qcril_mmgsdi_process_imsi_command_callback

===========================================================================*/
/*!
    @brief
    Handle MMGSDI command callback for the RIL_REQUEST_GET_IMSI special case.

    @return
    None
*/
/*=========================================================================*/
void qcril_mmgsdi_process_imsi_command_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_mmgsdi_command_callback_params_type *mmgsdi_cmd_cnf_ptr = NULL;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: QCRIL_EVT_MMGSDI_IMSI_COMMAND_CALLBACK\n", __FUNCTION__);

  QCRIL_ASSERT( params_ptr != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  mmgsdi_cmd_cnf_ptr = (qcril_mmgsdi_command_callback_params_type *) params_ptr->data;
  QCRIL_ASSERT( mmgsdi_cmd_cnf_ptr != NULL );
  QCRIL_ASSERT( mmgsdi_cmd_cnf_ptr->cnf == MMGSDI_READ_CNF );

  qcril_mmgsdi_common_get_imsi_cnf(params_ptr, ret_ptr);

  /*-----------------------------------------------------------------------*/

  /* free cnf ptr, allocated in the qcril_mmgsdi_imsi_command_callback */
  if (mmgsdi_cmd_cnf_ptr != NULL)
    free(mmgsdi_cmd_cnf_ptr);

} /* qcril_mmgsdi_process_imsi_command_callback() */


/*=========================================================================
  FUNCTION:  qcril_mmgsdi_process_internal_verify_pin_command_callback

===========================================================================*/
/*!
    @brief
    Handle MMGSDI command callback for the internal PIN verification during
    SIM IO request.

    @return
    None
*/
/*=========================================================================*/
void qcril_mmgsdi_process_internal_verify_pin_command_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_mmgsdi_command_callback_params_type *mmgsdi_cmd_cnf_ptr = NULL;

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: QCRIL_EVT_MMGSDI_INTERNAL_VERIFY_PIN_COMMAND_CALLBACK\n",
                         __FUNCTION__);

  QCRIL_ASSERT( params_ptr != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  mmgsdi_cmd_cnf_ptr =
    (qcril_mmgsdi_command_callback_params_type *) params_ptr->data;
  QCRIL_ASSERT( mmgsdi_cmd_cnf_ptr != NULL );
  QCRIL_ASSERT( mmgsdi_cmd_cnf_ptr->cnf == MMGSDI_PIN_OPERATION_CNF );

  qcril_mmgsdi_common_proceed_internal_cmd_processing(params_ptr, ret_ptr);

  /*-----------------------------------------------------------------------*/

  /* free cnf ptr, allocated in the
     qcril_mmgsdi_internal_verify_pin_command_callback */
  if (mmgsdi_cmd_cnf_ptr != NULL)
    free(mmgsdi_cmd_cnf_ptr);

} /* qcril_mmgsdi_process_internal_verify_pin_command_callback() */


/*=========================================================================
  FUNCTION:  qcril_mmgsdi_process_event_callback

===========================================================================*/
/*!
    @brief
    Handle MMGSDI event callback.

    @return
    None
*/
/*=========================================================================*/
void qcril_mmgsdi_process_event_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  mmgsdi_event_data_type *evt_ptr = NULL;
  RIL_CardStatus_v6          curr_card_status;
  qcril_unsol_resp_params_type unsol_resp;

  /*-----------------------------------------------------------------------*/

  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: QCRIL_EVT_MMGSDI_EVENT_CALLBACK\n", __FUNCTION__);

  QCRIL_ASSERT( params_ptr != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  evt_ptr = params_ptr->data;
  QCRIL_ASSERT( evt_ptr != NULL );

  qcril_mmgsdi_print_evt(evt_ptr->evt);

  memcpy(&curr_card_status, &qcril_mmgsdi.curr_card_status, sizeof(RIL_CardStatus_v6));

  QCRIL_LOG_INFO( "curr modem state 0x%x,  curr GW sim state 0x%x,  curr CDMA sim state 0x%x \n",
                  ret_ptr->next_modem_state,
                  ret_ptr->next_pri_gw_sim_state,
                  ret_ptr->next_pri_cdma_sim_state);

  switch  (evt_ptr->evt)
  {
  case MMGSDI_CARD_INSERTED_EVT:
    qcril_mmgsdi_process_card_inserted_evt( instance_id,
                                            modem_id,
                                            &evt_ptr->data.card_inserted,
                                            ret_ptr );
    break;

  case MMGSDI_SELECT_AID_EVT:
    qcril_mmgsdi_process_select_aid_evt( instance_id,
                                         modem_id,
                                         &evt_ptr->data.select_aid,
                                         ret_ptr );
    break;

  case MMGSDI_PIN1_EVT:
  case MMGSDI_PIN2_EVT:
  case MMGSDI_UNIVERSAL_PIN_EVT:
    qcril_mmgsdi_process_pin_evts( instance_id, modem_id,
                                   evt_ptr->evt, &evt_ptr->data.pin, ret_ptr );
    break;

  case MMGSDI_CARD_ERROR_EVT:
    qcril_mmgsdi_process_card_not_accessible_evts( instance_id, modem_id,
                                                   evt_ptr->evt,
                                                   evt_ptr->data.card_error.info,
                                                   ret_ptr );
    break;

  case MMGSDI_CARD_REMOVED_EVT:
  case MMGSDI_ILLEGAL_CARD_EVT: /* e.g., stolen SIM */
    qcril_mmgsdi_process_card_not_accessible_evts( instance_id, modem_id,
                                                   evt_ptr->evt,
                                                   MMGSDI_CARD_ERR_UNKNOWN_ERROR,
                                                   ret_ptr );
    break;

  case MMGSDI_CARD_INIT_COMPLETED_EVT:
    qcril_mmgsdi_process_card_init_completed_evt( instance_id, modem_id,
                                                  &evt_ptr->data.card_init_completed,
                                                  ret_ptr );
    break;

  case MMGSDI_TERMINAL_PROFILE_DL_EVT:
    break;

  case MMGSDI_REFRESH_EVT:
    qcril_mmgsdi_process_refresh_evt( instance_id, modem_id,
                                      &evt_ptr->data.refresh, ret_ptr );
    break;

  case MMGSDI_FDN_EVT:
    qcril_mmgsdi_update_fdn_status( instance_id, modem_id,
                                    evt_ptr->data.fdn.enabled ? QCRIL_MMGSDI_FDN_ENABLED : QCRIL_MMGSDI_FDN_DISABLED );
    break;

  /* future use */
  case MMGSDI_SWITCH_SLOT_EVT:
  case MMGSDI_SESSION_CLOSE_EVT:
    QCRIL_LOG_DEBUG( "Event to be handed in the future 0x%x\n", evt_ptr->evt);
    break;

  default:
    QCRIL_LOG_ERROR( "Unhandled Event 0x%x\n", evt_ptr->evt);
    break;
  }

  qcril_mmgsdi_print_card_status();

  /* If status changed, send unsolicited response to framework. In case
     of CARD_INIT_COMPLETED, this will be sent when FDN is available by
     function qcril_mmgsdi_card_init_completed_end() */
  if (qcril_mmgsdi_is_card_status_changed(&curr_card_status) &&
      evt_ptr->evt != MMGSDI_CARD_INIT_COMPLETED_EVT)
  {
    qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, &unsol_resp );
    qcril_send_unsol_response( &unsol_resp );
  }

  QCRIL_LOG_INFO( " new modem state 0x%x,  new GW sim state 0x%x,  new CDMA sim state 0x%x \n",
                  ret_ptr->next_modem_state,
                  ret_ptr->next_pri_gw_sim_state,
                  ret_ptr->next_pri_cdma_sim_state);

  /* free evt ptr, allocated in the qcril_mmgsdi_event_callback */
  if (evt_ptr != NULL)
    free(evt_ptr);
} /* qcril_mmgsdi_process_event_callback() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_process_perso_event_callback

===========================================================================*/
/*!
    @brief
    Handles GSDI Perso Events

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_process_perso_event_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type     instance_id;
  int                          gw_app_sub_index         = RIL_CARD_MAX_APPS;
  int                          cdma_app_sub_index       = RIL_CARD_MAX_APPS;
  int                          new_sim_state            = QCRIL_SIM_STATE_NOT_READY;
  int                          new_perso_substate       = RIL_PERSOSUBSTATE_UNKNOWN;
  boolean                      gw_perso_evt             = FALSE;
  boolean                      cdma_perso_evt           = FALSE;
  RIL_CardStatus_v6               curr_card_status;
  boolean                      is_app_selected          = FALSE;
  qcril_unsol_resp_params_type unsol_resp;
  qcril_mmgsdi_perso_event_callback_params_type * perso_evt_ptr = NULL;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: QCRIL_EVT_MMGSDI_PERSO_EVENT_CALLBACK\n", __FUNCTION__);

  QCRIL_ASSERT( params_ptr != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  perso_evt_ptr = (qcril_mmgsdi_perso_event_callback_params_type*)params_ptr->data;
  QCRIL_ASSERT( perso_evt_ptr != NULL );

  qcril_mmgsdi_print_perso_evt(perso_evt_ptr->evt);

  memcpy(&curr_card_status, &qcril_mmgsdi.curr_card_status, sizeof(RIL_CardStatus_v6));

  QCRIL_LOG_INFO( " curr modem state 0x%x,  curr GW sim state 0x%x,  curr CDMA sim state 0x%x \n",
                  ret_ptr->next_modem_state,
                  ret_ptr->next_pri_gw_sim_state,
                  ret_ptr->next_pri_cdma_sim_state);

  gw_app_sub_index = qcril_mmgsdi.curr_card_status.gsm_umts_subscription_app_index;
  cdma_app_sub_index = qcril_mmgsdi.curr_card_status.cdma_subscription_app_index;

  switch(perso_evt_ptr->evt)
  {
  case GSDI_PERSO_NO_EVENT:
    QCRIL_LOG_DEBUG( "%s", "No Special Handling for GSDI_PERSO_NO_EVENT\n");
    break;

  case GSDI_PERSO_NW_FAILURE:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_SIM_NETWORK \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_NETWORK_PERSONALIZATION \n");
    new_perso_substate = RIL_PERSOSUBSTATE_SIM_NETWORK;
    new_sim_state = QCRIL_SIM_STATE_NETWORK_PERSONALIZATION;
    gw_perso_evt = TRUE;
    break;

  case GSDI_PERSO_NS_FAILURE:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    gw_perso_evt = TRUE;
    break;

  case GSDI_PERSO_SP_FAILURE:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    gw_perso_evt = TRUE;
    break;

  case GSDI_PERSO_CP_FAILURE:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_SIM_CORPORATE \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_SIM_CORPORATE;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    gw_perso_evt = TRUE;
    break;

  case GSDI_PERSO_SIM_FAILURE:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_SIM_SIM \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_SIM_SIM;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    gw_perso_evt = TRUE;
    break;

  case GSDI_PERSO_NCK_BLOCKED:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_SIM_NETWORK_PUK \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_SIM_NETWORK_PUK;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    gw_perso_evt = TRUE;
    break;

  case GSDI_PERSO_NSK_BLOCKED:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET_PUK \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_SIM_NETWORK_SUBSET_PUK;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    gw_perso_evt = TRUE;
    break;

  case GSDI_PERSO_SPK_BLOCKED:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER_PUK \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_SIM_SERVICE_PROVIDER_PUK;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    gw_perso_evt = TRUE;
    break;

  case GSDI_PERSO_CCK_BLOCKED:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_SIM_CORPORATE_PUK \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_SIM_CORPORATE_PUK;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    gw_perso_evt = TRUE;
    break;

  case GSDI_PERSO_PPK_BLOCKED:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_SIM_SIM_PUK \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_SIM_SIM_PUK;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    gw_perso_evt = TRUE;
    break;

  case GSDI_PERSO_NW_DEACTIVATED:
  case GSDI_PERSO_NS_DEACTIVATED:
  case GSDI_PERSO_SP_DEACTIVATED:
  case GSDI_PERSO_CP_DEACTIVATED:
  case GSDI_PERSO_SIM_DEACTIVATED:
  case GSDI_PERSO_NCK_UNBLOCKED:
  case GSDI_PERSO_NSK_UNBLOCKED:
  case GSDI_PERSO_SPK_UNBLOCKED:
  case GSDI_PERSO_CCK_UNBLOCKED:
  case GSDI_PERSO_PPK_UNBLOCKED:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_IN_PROGRESS \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: No change \n");
    new_perso_substate = RIL_PERSOSUBSTATE_IN_PROGRESS;
    // no change to sim state
    gw_perso_evt = TRUE;
    break;

  case GSDI_PERSO_RUIM_NW1_FAILURE:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_RUIM_NETWORK1 \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_RUIM_NETWORK1;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    cdma_perso_evt = TRUE;
    break;

  case GSDI_PERSO_RUIM_NW2_FAILURE:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_RUIM_NETWORK2 \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_RUIM_NETWORK2;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    cdma_perso_evt = TRUE;
    break;

  case GSDI_PERSO_RUIM_HRPD_FAILURE:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_RUIM_HRPD \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_RUIM_HRPD;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    cdma_perso_evt = TRUE;
    break;

  case GSDI_PERSO_RUIM_SP_FAILURE:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    cdma_perso_evt = TRUE;
    break;

  case GSDI_PERSO_RUIM_CP_FAILURE:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_RUIM_CORPORATE \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_RUIM_CORPORATE;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    cdma_perso_evt = TRUE;
    break;

  case GSDI_PERSO_RUIM_RUIM_FAILURE:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_RUIM_RUIM \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_RUIM_RUIM;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    cdma_perso_evt = TRUE;
    break;

  case GSDI_PERSO_RUIM_NCK1_BLOCKED:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_RUIM_NETWORK1_PUK \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_RUIM_NETWORK1_PUK;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    cdma_perso_evt = TRUE;
    break;

  case GSDI_PERSO_RUIM_NCK2_BLOCKED:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_RUIM_NETWORK2_PUK \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_RUIM_NETWORK2_PUK;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    cdma_perso_evt = TRUE;
    break;

  case GSDI_PERSO_RUIM_HNCK_BLOCKED:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_RUIM_HRPD_PUK \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_RUIM_HRPD_PUK;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    cdma_perso_evt = TRUE;
    break;

  case GSDI_PERSO_RUIM_SPCK_BLOCKED:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER_PUK \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER_PUK;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    cdma_perso_evt = TRUE;
    break;

  case GSDI_PERSO_RUIM_CCK_BLOCKED:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_RUIM_CORPORATE_PUK \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_RUIM_CORPORATE_PUK;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    cdma_perso_evt = TRUE;
    break;

  case GSDI_PERSO_RUIM_PCK_BLOCKED:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_RUIM_RUIM_PUK \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_perso_substate = RIL_PERSOSUBSTATE_RUIM_RUIM_PUK;
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    cdma_perso_evt = TRUE;
    break;

  case GSDI_PERSO_RUIM_NW1_DEACTIVATED:
  case GSDI_PERSO_RUIM_NW2_DEACTIVATED:
  case GSDI_PERSO_RUIM_HRPD_DEACTIVATED:
  case GSDI_PERSO_RUIM_SP_DEACTIVATED:
  case GSDI_PERSO_RUIM_CP_DEACTIVATED:
  case GSDI_PERSO_RUIM_RUIM_DEACTIVATED:
  case GSDI_PERSO_RUIM_NCK1_UNBLOCKED:
  case GSDI_PERSO_RUIM_NCK2_UNBLOCKED:
  case GSDI_PERSO_RUIM_HNCK_UNBLOCKED:
  case GSDI_PERSO_RUIM_SPCK_UNBLOCKED:
  case GSDI_PERSO_RUIM_CCK_UNBLOCKED:
  case GSDI_PERSO_RUIM_PCK_UNBLOCKED:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_IN_PROGRESS \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: No change \n");
    new_perso_substate = RIL_PERSOSUBSTATE_IN_PROGRESS;
    // no change to sim state
    cdma_perso_evt = TRUE;
    break;

  case GSDI_PERSO_SANITY_ERROR:
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: QCRIL_SIM_STATE_ABSENT \n");
    new_sim_state = QCRIL_SIM_STATE_ABSENT;
    gw_perso_evt = TRUE;
    cdma_perso_evt = TRUE;
    qcril_mmgsdi.curr_card_status.card_state = RIL_CARDSTATE_ERROR;
    break;

  case GSDI_PERSO_EVT_INIT_COMPLETED:
    QCRIL_LOG_VERBOSE( "%s", "new_perso_substate: RIL_PERSOSUBSTATE_READY \n");
    QCRIL_LOG_VERBOSE( "%s", "new_sim_state: No update \n");
    new_perso_substate = RIL_PERSOSUBSTATE_READY;
    gw_perso_evt = TRUE;
    cdma_perso_evt = TRUE;
    qcril_mmgsdi.curr_card_status.card_state = RIL_CARDSTATE_PRESENT;
    break;

  default:
    QCRIL_LOG_ERROR( "Unhandled perso event 0x%x\n", perso_evt_ptr->evt);
    break;

  }

  if((gw_perso_evt) &&
    (gw_app_sub_index < qcril_mmgsdi.curr_card_status.num_applications) &&
    (new_perso_substate != RIL_PERSOSUBSTATE_UNKNOWN))
  {
    qcril_mmgsdi.curr_card_status.applications[gw_app_sub_index].perso_substate = new_perso_substate;
    qcril_mmgsdi.curr_card_status.applications[gw_app_sub_index].app_state = RIL_APPSTATE_SUBSCRIPTION_PERSO;
    if ( qcril_mmgsdi.gw_init_completed_recv &&
         ( qcril_mmgsdi.fdn_enable != QCRIL_MMGSDI_FDN_NOT_INIT ) )
    {
      if ((new_perso_substate == RIL_PERSOSUBSTATE_IN_PROGRESS)
           || (new_perso_substate == RIL_PERSOSUBSTATE_READY))
      {
        /* move back to READY state */
        qcril_mmgsdi.curr_card_status.applications[gw_app_sub_index].app_state =
          RIL_APPSTATE_READY;
        qcril_mmgsdi.curr_card_status.applications[gw_app_sub_index].perso_substate =
          RIL_PERSOSUBSTATE_READY;
        new_sim_state = QCRIL_SIM_STATE_READY;
        qcril_mmgsdi.gw_curr_card_state = QCRIL_SIM_STATE_READY;
      }
    }
    is_app_selected = TRUE;
  }
  if((cdma_perso_evt) &&
     (cdma_app_sub_index < qcril_mmgsdi.curr_card_status.num_applications) &&
     (new_perso_substate != RIL_PERSOSUBSTATE_UNKNOWN))
  {
    qcril_mmgsdi.curr_card_status.applications[cdma_app_sub_index].perso_substate = new_perso_substate;
    qcril_mmgsdi.curr_card_status.applications[cdma_app_sub_index].app_state = RIL_APPSTATE_SUBSCRIPTION_PERSO;
    if (qcril_mmgsdi.cdma_init_completed_recv &&
        qcril_mmgsdi.fdn_enable != QCRIL_MMGSDI_FDN_NOT_INIT)
    {
      if ((new_perso_substate == RIL_PERSOSUBSTATE_IN_PROGRESS) || (new_perso_substate == RIL_PERSOSUBSTATE_READY))
      {
        /* move back to READY state */
        qcril_mmgsdi.curr_card_status.applications[cdma_app_sub_index].app_state = RIL_APPSTATE_READY;
        qcril_mmgsdi.curr_card_status.applications[cdma_app_sub_index].perso_substate = RIL_PERSOSUBSTATE_READY;
      }
    }
    is_app_selected = TRUE;
  }
  if(is_app_selected == FALSE)
  {
    if (perso_temp_info_ptr == NULL)
    {
      perso_temp_info_ptr = (qcril_mmgsdi_temp_perso_info_type*)malloc(sizeof(qcril_mmgsdi_temp_perso_info_type));
    }
    if (perso_temp_info_ptr == NULL)
    {
      QCRIL_LOG_INFO( "%s: perso_temp_info_ptr NULL after alloc\n", __FUNCTION__);
    }
    else
    {
      perso_temp_info_ptr->perso_state = new_perso_substate;
      perso_temp_info_ptr->sim_state = new_sim_state;
    }
  }

  if (perso_temp_info_ptr == NULL)
  {
    /* Update only if we have already identified a prov app.
       If prov app has not been identified, the population and notification
       will be done after we have received PIN events */
    if (new_sim_state != QCRIL_SIM_STATE_NOT_READY)
    {
      qcril_mmgsdi_update_curr_card_state(new_sim_state,
                                          gw_perso_evt,
                                          cdma_perso_evt,
                                          ret_ptr);
    }

    if (new_sim_state == QCRIL_SIM_STATE_ABSENT)
    {
      qcril_mmgsdi.curr_card_status.card_state = RIL_CARDSTATE_ERROR;
    }

    qcril_mmgsdi_print_card_status();

    if (qcril_mmgsdi_is_card_status_changed(&curr_card_status))
    {
      qcril_default_unsol_resp_params( instance_id, (int) RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED, &unsol_resp );
      qcril_send_unsol_response( &unsol_resp );
    }

    QCRIL_LOG_INFO( " new modem state 0x%x,  new GW sim state 0x%x,  new CDMA sim state 0x%x \n",
                    ret_ptr->next_modem_state,
                    ret_ptr->next_pri_gw_sim_state,
                    ret_ptr->next_pri_cdma_sim_state);
  }

  /* free perso evt ptr, allocated in the qcril_mmgsdi_perso_event_callback */
  if (perso_evt_ptr != NULL)
    free(perso_evt_ptr);

} /* qcril_mmgsdi_process_perso_event_callback() */


/*=========================================================================
  FUNCTION:  qcril_mmgsdi_process_internal_command

===========================================================================*/
/*!
    @brief
    Handle MMGSDI command sent by internal qcril modules.

    @return
    None
*/
/*=========================================================================*/
void qcril_mmgsdi_process_internal_command
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type    modem_id;
  mmgsdi_return_enum_type  mmgsdi_status = MMGSDI_ERROR;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  QCRIL_ASSERT( params_ptr != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  switch (params_ptr->event_id)
  {
  case QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_DOWN:
    if(qcril_mmgsdi.client_id == QCRIL_MMGSDI_INVALID_CLIENT_ID)
    {
      QCRIL_LOG_ERROR("%s", "Invalid client id, can not call mmgsdi_card_pdown() \n");
      return;
    }
    QCRIL_LOG_RPC2( modem_id, "mmgsdi_card_pdown()", "MMGSDI_CARD_POWER_DOWN_NOTIFY_GSDI" );
    mmgsdi_status = mmgsdi_card_pdown(qcril_mmgsdi.client_id,
                                      MMGSDI_SLOT_1,
                                      qcril_mmgsdi_command_callback,
                                      MMGSDI_CARD_POWER_DOWN_NOTIFY_GSDI,
                                      0);
    if ( mmgsdi_status != MMGSDI_SUCCESS )
    {
      /* Card PowerDown Failure, tell QCRIL that Card Mode PowerDown failure */
      qcril_mmgsdi_update_cm_card_status( instance_id, modem_id,
                                          QCRIL_CARD_STATUS_POWERDOWN_FAILED );
    }
    break;

  case QCRIL_EVT_INTERNAL_MMGSDI_CARD_POWER_UP:
    if(qcril_mmgsdi.client_id == QCRIL_MMGSDI_INVALID_CLIENT_ID)
    {
      QCRIL_LOG_ERROR("%s", "Invalid client id, can not call mmgsdi_card_pup() \n");
      return;
    }
    QCRIL_LOG_RPC2( modem_id, "mmgsdi_card_pup()", "MMGSDI_CARD_POWER_UP_INITIAL_PUP" );
    mmgsdi_status = mmgsdi_card_pup(qcril_mmgsdi.client_id,
                                    MMGSDI_SLOT_1,
                                    qcril_mmgsdi_command_callback,
                                    MMGSDI_CARD_POWER_UP_INITIAL_PUP,
                                    0);
    if ( mmgsdi_status != MMGSDI_SUCCESS )
    {
      /* Card PowerUp Failure, tell QCRIL that Card Mode PowerUp failure */
      qcril_mmgsdi_update_cm_card_status( instance_id, modem_id,
                                          QCRIL_CARD_STATUS_POWERUP_FAILED );
    }
    break;

  default:
    QCRIL_LOG_ERROR( "Invalid event id 0x%x\n", params_ptr->event_id);
    mmgsdi_status = MMGSDI_ERROR;
    break;
  }

  if ( MMGSDI_SUCCESS!= mmgsdi_status )
  {
    QCRIL_LOG_ERROR( "Processing failed 0x%x\n", mmgsdi_status);
  }

}  /* qcril_mmgsdi_process_internal_command() */


/*===========================================================================

  FUNCTION:  qcril_mmgsdi_process_gsdi_command_callback

===========================================================================*/
/*!
    @brief
    Handles GSDI command responses

    @return
    None.
*/
/*=========================================================================*/
void qcril_mmgsdi_process_gsdi_command_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  qcril_instance_id_e_type instance_id;
  qcril_modem_id_e_type modem_id;
  mmgsdi_return_enum_type mmgsdi_status = MMGSDI_SUCCESS;
  gsdi_cnf_T             *cnf_ptr       = NULL;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );
  modem_id = params_ptr->modem_id;
  QCRIL_ASSERT( modem_id < QCRIL_MAX_MODEM_ID );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: QCRIL_EVT_MMGSDI_GSDI_COMMAND_CALLBACK\n", __FUNCTION__);

  QCRIL_ASSERT( params_ptr != NULL );
  QCRIL_ASSERT( ret_ptr != NULL );

  cnf_ptr = (gsdi_cnf_T*)params_ptr->data;
  QCRIL_ASSERT( cnf_ptr != NULL );

  if (cnf_ptr->message_header.resp_type == GSDI_GET_SIM_CAPABILITIES_RSP)
  {
    QCRIL_LOG_INFO( "curr modem state 0x%x,  curr GW sim state 0x%x,  curr CDMA sim state 0x%x \n",
                    ret_ptr->next_modem_state,
                    ret_ptr->next_pri_gw_sim_state,
                    ret_ptr->next_pri_cdma_sim_state);
  }

  switch (cnf_ptr->message_header.resp_type)
  {
  case GSDI_PERSO_REG_TASK_RSP:
    QCRIL_LOG_INFO( "GSDI_PERSO_REG_TASK_RSP status 0x%x\n",
                    cnf_ptr->message_header.gsdi_status);
    break;

  case GSDI_PERSO_DEACT_IND_RSP:
    QCRIL_LOG_INFO( "%s", "GSDI_PERSO_DEACT_IND_RSP\n");
    qcril_mmgsdi_sec_process_perso_deact_cnf( instance_id,
                                              cnf_ptr->message_header.gsdi_status,
                                              cnf_ptr->message_header.client_ref,
                                              cnf_ptr->dact_ind_cnf.perso_feature,
                                              cnf_ptr->dact_ind_cnf.num_retries );
    break;

  case GSDI_PERSO_GET_DCK_NUM_RETRIES_RSP:
    QCRIL_LOG_INFO( "%s", "GSDI_PERSO_GET_DCK_NUM_RETRIES_RSP\n");
    /* We force the gsdi status to GSDI ERROR since -
       We need a way for framework to know number of perso NW retries. There is no RIL support
       for retrieving number of retries from modem. We queue RIL NW DEPERSO command with
       empty perso key. For this special case we queue gsdi get dck retries  to modem.
       To keep response consistent as modem would send for an empty perso key string for
       deactivate perso request we force response status to error. */
    qcril_mmgsdi_sec_process_perso_deact_cnf( instance_id,
                                              GSDI_ERROR,
                                              cnf_ptr->message_header.client_ref,
                                              GSDI_PERSO_NW,
                                              cnf_ptr->dck_num_retries_cnf.nw_num_retries );
    break;

  case GSDI_ENABLE_FDN_RSP:
    QCRIL_LOG_INFO( "%s", "GSDI_ENABLE_FDN_RSP\n");
    qcril_mmgsdi_common_process_fdn_status_cnf( instance_id, cnf_ptr->message_header.gsdi_status,
                                                cnf_ptr->message_header.client_ref );
    break;

  case GSDI_DISABLE_FDN_RSP:
    QCRIL_LOG_INFO( "%s", "GSDI_DISABLE_FDN_RSP\n");
    qcril_mmgsdi_common_process_fdn_status_cnf( instance_id, cnf_ptr->message_header.gsdi_status,
                                                cnf_ptr->message_header.client_ref );
    break;

  case GSDI_GET_SIM_CAPABILITIES_RSP:
    QCRIL_LOG_INFO( "%s", "GSDI_GET_SIM_CAPABILITIES_RSP\n");
    qcril_mmgsdi_process_get_sim_cap_cnf( instance_id, modem_id, cnf_ptr->message_header.gsdi_status,
                                          &cnf_ptr->get_sim_cap_cnf, ret_ptr );
    break;

  case GSDI_SELECT_RSP:
    QCRIL_LOG_INFO( "%s", "GSDI_SELECT_RSP\n");
    qcril_mmgsdi_common_process_select_cnf( instance_id, &cnf_ptr->select_cnf);
    break;

  default:
    QCRIL_LOG_ERROR( "Invalid gsdi resp type 0x%x\n", cnf_ptr->message_header.resp_type);
    break;
  }

  /* free cnf ptr, allocated in the qcril_mmgsdi_gsdi_command_callback */
  if (cnf_ptr != NULL)
    free(cnf_ptr);

  /* Get SIM capabilities is invoked after initialization and at this point
     we need to notify the framework. The QCRIL status does not change, so
     we cannot rely on RIL_CardStatus_v6 to send the event. */
  if (cnf_ptr->message_header.resp_type == GSDI_GET_SIM_CAPABILITIES_RSP)
  {
    QCRIL_LOG_INFO( " new modem state 0x%x, new GW sim state 0x%x, new CDMA sim state 0x%x \n",
                    ret_ptr->next_modem_state,
                    ret_ptr->next_pri_gw_sim_state,
                    ret_ptr->next_pri_cdma_sim_state);
  }
} /* qcril_mmgsdi_process_gsdi_command_callback() */

#else

void qcril_mmgsdi_init
(
 void
)
{
  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);
}

void qcril_mmgsdi_release
(
 void
)
{
  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);
}

void qcril_mmgsdi_process_command_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);
}

void qcril_mmgsdi_process_imsi_command_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);
}

void qcril_mmgsdi_process_internal_verify_pin_command_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);
}

void qcril_mmgsdi_process_event_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);
}

void qcril_mmgsdi_process_gsdi_command_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);
}

void qcril_mmgsdi_process_perso_event_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);
}

void qcril_mmgsdi_process_internal_command
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);
}

void qcril_mmgsdi_process_internal_read_ust_callback
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);
}

void qcril_mmgsdi_response_not_supported
(
  const qcril_request_params_type *const params_ptr
)
{
  qcril_instance_enum_type instance_id;
  qcril_request_resp_params_type resp;

  /*-----------------------------------------------------------------------*/

  QCRIL_ASSERT( params_ptr != NULL );
  instance_id = params_ptr->instance_id;
  QCRIL_ASSERT( instance_id < QCRIL_MAX_INSTANCE_ID );

  /*-----------------------------------------------------------------------*/

  qcril_default_request_resp_params( instance_id, params_event_id, RIL_E_REQUEST_NOT_SUPPORTED, &resp );
  qcril_send_request_response( &resp );

} /* qcril_mmgsdi_response_not_supported */


void qcril_mmgsdi_request_get_fdn_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_ASSERT( params_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);

  qcril_mmgsdi_response_not_supported( params_ptr );

} /* qcril_mmgsdi_request_get_fdn_status */


void qcril_mmgsdi_request_set_fdn_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_ASSERT( params_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);

  qcril_mmgsdi_response_not_supported( params_ptr );

} /* qcril_mmgsdi_request_set_fdn_status */


void qcril_mmgsdi_request_get_pin_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_ASSERT( params_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);

  qcril_mmgsdi_response_not_supported( params_ptr );

} /* qcril_mmgsdi_request_get_pin_status */


void qcril_mmgsdi_request_set_pin_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_ASSERT( params_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);

  qcril_mmgsdi_response_not_supported( params_ptr );

} /* qcril_mmgsdi_request_set_pin_status */


void qcril_mmgsdi_request_get_sim_status
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_ASSERT( params_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);

  qcril_mmgsdi_response_not_supported( params_ptr );

} /* qcril_mmgsdi_request_get_sim_status */


void qcril_mmgsdi_request_enter_pin
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_ASSERT( params_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);

  qcril_mmgsdi_response_not_supported( params_ptr );

} /* qcril_mmgsdi_request_enter_pin */


void qcril_mmgsdi_request_enter_puk
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_ASSERT( params_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);

  qcril_mmgsdi_response_not_supported( params_ptr );

} /* qcril_mmgsdi_request_enter_puk */


void qcril_mmgsdi_request_change_pin
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_ASSERT( params_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);

  qcril_mmgsdi_response_not_supported( params_ptr );

} /* qcril_mmgsdi_request_change_pin */


void qcril_mmgsdi_request_enter_perso_key
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_ASSERT( params_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);

  qcril_mmgsdi_response_not_supported( params_ptr );

} /* qcril_mmgsdi_request_enter_perso_key */


void qcril_mmgsdi_request_oem_hook_me_depersonalization
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_ASSERT( params_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);

  qcril_mmgsdi_response_not_supported( params_ptr );

} /* qcril_mmgsdi_request_oem_hook_me_depersonalization */


void qcril_mmgsdi_request_get_imsi
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_ASSERT( params_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);

  qcril_mmgsdi_response_not_supported( params_ptr );

} /* qcril_mmgsdi_request_get_imsi */


void qcril_mmgsdi_request_sim_io
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type       *const ret_ptr /*!< Output parameter */
)
{
  QCRIL_ASSERT( params_ptr != NULL );

  /*-----------------------------------------------------------------------*/

  QCRIL_LOG_INFO( "%s: Not Supported\n", __FUNCTION__);

  qcril_mmgsdi_response_not_supported( params_ptr );

} /* qcril_mmgsdi_request_sim_io */
#endif /* !FEATURE_CDMA_NON_RUIM */

#endif /* !defined (FEATURE_QCRIL_UIM_QMI) */

