/*===========================================================================

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
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

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //depot/asic/sandbox/users/micheleb/ril/qcril_uim_util.c#2 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/12/14   at      Change the fci property name to persist
06/17/14   tl      Added logic to better determine FCI value from AID
06/16/14   at      Compile error in qcril_uim_remove_non_provisioning_session
05/02/14   tkl     Added support for RIL_REQUEST_SIM_AUTHENTICATION
03/25/14   tl      Added utility to check app type is provisioning
12/19/13   yt      Keep ISIM session open after ISIM access
10/14/13   yt      Allow ICCID read from Slot 3
07/16/13   yt      Remove closing of non-prov session to ISIM app
04/17/13   yt      Fix critical KW errors
01/29/13   yt      Support for third SIM card slot
01/14/13   yt      Fix critical KW errors
11/15/12   at      Support for DSDA device configuration
10/08/12   at      Support for ISIM Authentication API
07/09/12   at      Check for NULL path in qcril_uim_extract_file_id
05/10/12   at      Added qcril_uim_find_app_in_slot function
03/22/12   at      Replacing malloc()/free() with qcril_malloc()/qcril_free()
03/20/12   tl      Transition to QCCI
10/26/11   yt      Return appropriate error code if EF-ICCID read fails
09/30/11   yt      Added support for ISIM Refresh
06/16/11   at      Added qcril_uim_convert_slot_id_to_scws_slot_id()
04/11/11   yt      Support for silent PIN1 restart
03/22/11   at      Support for ril.h v6
01/18/11   at      Added new function to fetch slot id, changed slot id
                   parameter type in some functions
11/05/10   at      KW fixes for checking buffer overflow
10/06/10   at      Added new function for allocating original request data
09/21/10   at      Added new function for fetching prov application index
09/09/10   at      Changed the way sessions are returned, added functions for
                   opening & closing non-prov session on demand
09/02/10   at      Added additional check for slot in 2G card app case
08/03/10   at      APIs using aidPtr to be handled properly
07/23/10   at      Conversion to string of buffers of 0 bytes
06/29/10   at      Added functions to support pin verification APIs
05/13/10   at      Clean up for merging with mainline
04/13/10   mib     Initial version
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#if defined (FEATURE_QCRIL_UIM_QMI)

#include "ril.h"
#include "IxErrno.h"
#include "qcril_log.h"
#include "qcril_uim_util.h"
#include "qcril_uim_queue.h"
#include "qcril_uim_qcci.h"
#include <string.h>
#include <cutils/properties.h>


/*===========================================================================

                           DEFINES

===========================================================================*/
/* Word size for memory alignment */
#define QCRIL_UIM_WORD_SIZE                     4

/* Invalid session index */
#define QCRIL_UIM_INVALID_SESSION_VALUE        0xFFFF

/* Maximum number of non provisioning sessions */
#define QCRIL_UIM_NON_PROV_SESSIONS_MAX_COUNT   9

/* File ID for Master File */
#define QCRIL_UIM_MF_ID_HIGH_BYTE              0x3F
#define QCRIL_UIM_MF_ID_LOW_BYTE               0x00

/* File ID for EF-ICCID */
#define QCRIL_UIM_EF_ICCID                     0x2FE2
#define QCRIL_UIM_ICCID_PATH_LEN               2

#define QCRIL_UIM_NUM_BITS_BASE64_CHAR         6
#define QCRIL_UIM_NUM_BITS_CHAR                8

#define QCRIL_UIM_FCI_PROPERTY_NAME            "persist.fci"
#define QCRIL_UIM_PROPERTY_FCP                 '0'
#define QCRIL_UIM_PROPERTY_FCI                 '1'
#define QCRIL_UIM_PROPERTY_FMD                 '2'
#define QCRIL_UIM_PROPERTY_NO_DATA             '3'

/*===========================================================================

                           GLOBALS

===========================================================================*/
struct
{
  boolean                    in_use;
  qmi_uim_session_info_type  session_info;
  RIL_Token                  last_token;
} qcril_uim_non_prov_session_list[QCRIL_UIM_NON_PROV_SESSIONS_MAX_COUNT];


/* -----------------------------------------------------------------------------
   STRUCT:      QCRIL_UIM_AID_FCI_TYPE

   DESCRIPTION:
     Stores the FCI value of a corresponding application through the AID
-------------------------------------------------------------------------------*/
typedef struct
{
  const char                 aid_str_buffer[QMI_UIM_MAX_AID_LEN];
  qcril_uim_fci_value_type   fci_value;
} qcril_uim_aid_fci_type;


static qcril_uim_aid_fci_type qcril_uim_aid_fci_list[] =
{
  {"A0000000041010",                   QCRIL_UIM_FCI_VALUE_FCI}, /* Mastercard */
  {"A000000063504B43532D3135",         QCRIL_UIM_FCI_VALUE_FCP}  /* PKCS15 */
};


/*=========================================================================

  FUNCTION:  qcril_uim_align_size

===========================================================================*/
/*!
    @brief
    Calculates the memory size to keep buffers aligned to word.

    @return
    New memory size
*/
/*=========================================================================*/
uint16 qcril_uim_align_size
(
  uint16  orig_size
)
{
  uint16  mod = 0;

  mod = orig_size % QCRIL_UIM_WORD_SIZE;
  if (mod == 0)
  {
    return orig_size;
  }

  return (orig_size + (QCRIL_UIM_WORD_SIZE - mod));
} /* qcril_uim_align_size*/


/*=========================================================================

  FUNCTION:  qcril_uim_hexchar_to_bin

===========================================================================*/
/*!
    @brief
    Converts a single character from ASCII to binary

    @return
    Binary value of the ASCII characters
*/
/*=========================================================================*/
uint8 qcril_uim_hexchar_to_bin
(
  char ch
)
{
  if (ch >= '0' && ch <= '9')
  {
    return (ch - '0');
  }
  else if (ch >= 'A' && ch <= 'F')  /* A - F */
  {
    return (ch - 'A' + 10);
  }
  else if (ch >= 'a' && ch <= 'f')  /* a - f */
  {
    return (ch - 'a' + 10);
  }
  else
  {
    QCRIL_ASSERT(0);
  }
  return 0;
} /* qcril_uim_hexchar_to_bin */


/*=========================================================================

  FUNCTION:  qcril_uim_bin_to_hexchar

===========================================================================*/
/*!
    @brief
    Converts a single character from ASCII to binary

    @return
    Binary value of the ASCII characters
*/
/*=========================================================================*/
char qcril_uim_bin_to_hexchar
(
  uint8 ch
)
{
  QCRIL_ASSERT(ch < 0x10);

  if (ch < 0x0a)
  {
    return (ch + '0');
  }
  return (ch + 'a' - 10);
} /* qcril_uim_bin_to_hexchar */



/*=========================================================================

  FUNCTION:  qcril_uim_alloc_hexstring_to_bin

===========================================================================*/
/*!
    @brief
    Allocates memory and converts a string from ASCII format into
    binary format.

    @return
    Buffer with binary data
*/
/*=========================================================================*/
uint8* qcril_uim_alloc_hexstring_to_bin
(
  const char * string_ptr,
  uint16     * buffer_size_ptr
)
{
  uint16 buffer_size = 0;
  uint8* out_ptr     = NULL;

  if (string_ptr == NULL || buffer_size_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL pointer");
    QCRIL_ASSERT(0);
    return NULL;
  }

  buffer_size = (strlen(string_ptr) + 1) / 2;
  if (buffer_size == 0)
  {
    return out_ptr;
  }

  out_ptr = qcril_malloc(buffer_size);

  if (out_ptr != NULL)
  {
    *buffer_size_ptr = qcril_uim_hexstring_to_bin(string_ptr, out_ptr, buffer_size);
  }

  return out_ptr;
} /* qcril_uim_alloc_hexstring_to_bin */


/*=========================================================================

  FUNCTION:  qcril_uim_hexstring_to_bin

===========================================================================*/
/*!
    @brief
    Converts a ASCII string into a binary buffer.
    Memory is not allocated for this conversion

    @return
    Size of the data stored in the buffer
*/
/*=========================================================================*/
uint16 qcril_uim_hexstring_to_bin
(
  const char * string_ptr,
  uint8      * buffer_ptr,
  uint16       buffer_size
)
{
  uint16 string_len = 0;
  int    i = 0;

  if (string_ptr == NULL || buffer_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL pointer");
    QCRIL_ASSERT(0);
    return 0;
  }

  string_len = strlen(string_ptr);

  if (buffer_size < (string_len + 1) / 2)
  {
    /* Buffer is too small */
    QCRIL_LOG_ERROR("Buffer is too small for conversion (0x%x < 0x%x)",
                    buffer_size, (string_len + 1) / 2);
    return 0;
  }

  /* Zero the destination buffer */
  memset(buffer_ptr, 0, buffer_size);

  for (i = 0; i < string_len; i++)
  {
    if ((i % 2) == 0)
    {
      buffer_ptr[i / 2] = (qcril_uim_hexchar_to_bin(string_ptr[i]) << 4) & 0xF0;
    }
    else
    {
      buffer_ptr[i / 2] = buffer_ptr[i / 2] | (qcril_uim_hexchar_to_bin(string_ptr[i]) & 0x0F);
    }
  }

  return (string_len + 1) / 2;
} /* qcril_uim_hexstring_to_bin */


/*=========================================================================

  FUNCTION:  qcril_uim_alloc_bin_to_hexstring

===========================================================================*/
/*!
    @brief
    Converts a binary buffer into a string in ASCII format.
    Memory is allocated for the conversion.

    @return
    Pointer to the NULL terminated string
*/
/*=========================================================================*/
char* qcril_uim_alloc_bin_to_hexstring
(
  const uint8*  buffer_ptr,
  uint16        buffer_size
)
{
  char*  out_ptr    = NULL;
  uint16 string_len = 0;

  if(buffer_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL pointer");
    QCRIL_ASSERT(0);
    return NULL;
  }

  string_len = (buffer_size * 2) + sizeof(char);

  out_ptr = qcril_malloc(string_len);

  if (out_ptr != NULL)
  {
    qcril_uim_bin_to_hexstring(buffer_ptr, buffer_size, out_ptr, string_len);
  }

  return out_ptr;
} /* qcril_uim_alloc_bin_to_hexstring */


/*=========================================================================

  FUNCTION:  qcril_uim_bin_to_hexstring

===========================================================================*/
/*!
    @brief
    Converts a binary buffer into a string in ASCII format.
    Memory is not allocated for this conversion.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_bin_to_hexstring
(
  const uint8*  buffer_ptr,
  uint16        buffer_size,
  char*         string_ptr,
  uint16        string_size
)
{
  int    i = 0;

  if(buffer_ptr == NULL || string_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL pointer");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_ASSERT(string_size >= (buffer_size * 2) + sizeof(char));

  memset(string_ptr, 0, string_size);

  for (i = 0; i < buffer_size; i++)
  {
    string_ptr[i * 2] = qcril_uim_bin_to_hexchar((buffer_ptr[i] >> 4) & 0x0F);
    string_ptr[i * 2 + 1] = qcril_uim_bin_to_hexchar(buffer_ptr[i] & 0x0F);
  }
  string_ptr[buffer_size * 2] = 0x0;
} /* qcril_uim_bin_to_hexstring */


/*=========================================================================

  FUNCTION:  qcril_uim_check_aid_with_app_type

===========================================================================*/
/*!
    @brief
    Determines if the given AID belongs to ISIM/USIM/CSIM app type

    @return
    boolean
*/
/*=========================================================================*/
boolean qcril_uim_check_aid_with_app_type
(
  const qmi_uim_data_type * aid_ptr,
  qmi_uim_app_type          app_type
)
{
  const uint8 isim_aid[] = {0xA0, 0x00, 0x00, 0x00, 0x87, 0x10, 0x04};
  const uint8 usim_aid[] = {0xA0, 0x00, 0x00, 0x00, 0x87, 0x10, 0x02};
  const uint8 csim_aid[] = {0xA0, 0x00, 0x00, 0x03, 0x43, 0x10, 0x02};

  const uint8 *app_type_aid = NULL;
  uint8        app_type_aid_len = 0;

  if (aid_ptr == NULL)
  {
    return FALSE;
  }

  if (aid_ptr->data_ptr != NULL)
  {
    switch (app_type)
    {
       case QMI_UIM_APP_USIM:
         app_type_aid_len = sizeof(usim_aid);
         app_type_aid = usim_aid;
         break;
       case QMI_UIM_APP_ISIM:
         app_type_aid_len = sizeof(isim_aid);
         app_type_aid = isim_aid;
         break;
       case QMI_UIM_APP_CSIM:
         app_type_aid_len = sizeof(csim_aid);
         app_type_aid = csim_aid;
         break;
       default:
         QCRIL_LOG_ERROR("app_type not supported: 0x%x", app_type);
         return FALSE;
    }

    if (memcmp(aid_ptr->data_ptr, app_type_aid, app_type_aid_len) == 0)
    {
      return TRUE;
    }
  }

  return FALSE;
} /* qcril_uim_check_aid_with_app_type */


/*===========================================================================

  FUNCTION:  qcril_uim_extract_session_type

===========================================================================*/
/*!
    @brief
    Extracts the session type from a SIM_IO request.
    The AID buffer is optional and is used only for non provisioning
    sessions (in order to avoid allocating memory)

    @return
    RIL_Errno code
*/
/*=========================================================================*/
RIL_Errno qcril_uim_extract_session_type
(
  uint8                       slot,
  const char                * RIL_aid_ptr,
  uint16                      first_level_df_path,
  qmi_uim_session_info_type * session_info_ptr,
  uint8                     * aid_ptr,
  uint16                      max_aid_len
)
{
  uint16        aid_size     = 0;
  uint8         aid_buffer[QMI_UIM_MAX_AID_LEN];
  int           i            = 0;

  if(session_info_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL session_info_ptr");
    QCRIL_ASSERT(0);
    return RIL_E_GENERIC_FAILURE;
  }

  /* Logging */
  QCRIL_LOG_VERBOSE("Session slot: 0x%x", slot);
  QCRIL_LOG_VERBOSE("Session AID: %s",
                    RIL_aid_ptr ? RIL_aid_ptr : "NULL");
  QCRIL_LOG_VERBOSE("First level DF path: 0x%X", first_level_df_path);

  /* Check if slot value is valid */
  if (slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot value: 0x%x", slot);
    return RIL_E_REQUEST_NOT_SUPPORTED;
  }

  /* Verify card status */
  if (qcril_uim.card_status.card[slot].card_state != QMI_UIM_CARD_STATE_PRESENT)
  {
    QCRIL_LOG_ERROR("%s", "Card is not present");
    return RIL_E_REQUEST_NOT_SUPPORTED;
  }

  /* Check DF path to determine session type */
  switch(first_level_df_path)
  {
    case QCRIL_UIM_FILEID_DF_GSM:
      if ((qcril_uim.card_status.index_gw_pri_prov != QCRIL_UIM_INVALID_SESSION_VALUE) &&
          (slot == ((qcril_uim.card_status.index_gw_pri_prov >> 8) & 0xFF)))
      {
        session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_PRI_GW_PROV;
      }
      else if ((qcril_uim.card_status.index_gw_sec_prov != QCRIL_UIM_INVALID_SESSION_VALUE) &&
               (slot == ((qcril_uim.card_status.index_gw_sec_prov >> 8) & 0xFF)))
      {
        session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_SEC_GW_PROV;
      }
      else if ((qcril_uim.card_status.index_gw_ter_prov != QCRIL_UIM_INVALID_SESSION_VALUE) &&
               (slot == ((qcril_uim.card_status.index_gw_ter_prov >> 8) & 0xFF)))
      {
        session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_TER_GW_PROV;
      }
      else
      {
        if (slot == 0)
        {
          session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_CARD_SLOT_1;
        }
        else if (slot == 1)
        {
          session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_CARD_SLOT_2;
        }
        else
        {
          session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_CARD_SLOT_3;
        }
      }
      return RIL_E_SUCCESS;

    case QCRIL_UIM_FILEID_DF_CDMA:
      if ((qcril_uim.card_status.index_1x_pri_prov != QCRIL_UIM_INVALID_SESSION_VALUE) &&
          (slot == ((qcril_uim.card_status.index_1x_pri_prov >> 8) & 0xFF)))
      {
        session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_PRI_1X_PROV;
      }
      else if ((qcril_uim.card_status.index_1x_sec_prov != QCRIL_UIM_INVALID_SESSION_VALUE) &&
               (slot == ((qcril_uim.card_status.index_1x_sec_prov >> 8) & 0xFF)))
      {
        session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_SEC_1X_PROV;
      }
      else if ((qcril_uim.card_status.index_1x_ter_prov != QCRIL_UIM_INVALID_SESSION_VALUE) &&
               (slot == ((qcril_uim.card_status.index_1x_ter_prov >> 8) & 0xFF)))
      {
        session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_TER_1X_PROV;
      }
      else
      {
        if (slot == 0)
        {
          session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_CARD_SLOT_1;
        }
        else if (slot == 1)
        {
          session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_CARD_SLOT_2;
        }
        else
        {
          session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_CARD_SLOT_3;
        }
      }
      return RIL_E_SUCCESS;

    case QCRIL_UIM_FILEID_ADF_USIM_CSIM:
      aid_size = (RIL_aid_ptr == NULL) ? 0 : strlen(RIL_aid_ptr);
      if (aid_size == 0)
      {
        QCRIL_LOG_ERROR("%s", "Invalid AID string for ADF access");
        return RIL_E_REQUEST_NOT_SUPPORTED;
      }
      /* Convert AID string into binary buffer */
      aid_size = qcril_uim_hexstring_to_bin(RIL_aid_ptr,
                                            aid_buffer,
                                            QMI_UIM_MAX_AID_LEN);
      if (aid_size == 0)
      {
        QCRIL_LOG_ERROR("%s", "Error converting AID string into binary");
        return RIL_E_REQUEST_NOT_SUPPORTED;
      }

      for (i = 0; i < qcril_uim.card_status.card[slot].num_app &&
                  i < QMI_UIM_MAX_APP_PER_CARD_COUNT; i++)
      {
        if (qcril_uim.card_status.card[slot].application[i].aid_len == aid_size &&
            aid_size <= sizeof(qcril_uim.card_status.card[slot].application[i].aid_value) &&
            aid_size <= sizeof(aid_buffer))
        {
          if (memcmp(qcril_uim.card_status.card[slot].application[i].aid_value,
                     aid_buffer,
                     aid_size) == 0)
          {
            /* We have found the application! */

            /* According to QMI coding, this is the index that we expect if it's
               a provisioning application. The index has one byte that indicates
               the slot and one byte that indicates the application */
            uint16 index = ((slot & 0xFF) << 8) | (i & 0xFF);

            QCRIL_LOG_VERBOSE("Application found - index: 0x%x", index);

            if (index == qcril_uim.card_status.index_gw_pri_prov)
            {
              session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_PRI_GW_PROV;
              return RIL_E_SUCCESS;
            }
            else if (index == qcril_uim.card_status.index_1x_pri_prov)
            {
              session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_PRI_1X_PROV;
              return RIL_E_SUCCESS;
            }
            else if (index == qcril_uim.card_status.index_gw_sec_prov)
            {
              session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_SEC_GW_PROV;
              return RIL_E_SUCCESS;
            }
            else if (index == qcril_uim.card_status.index_1x_sec_prov)
            {
              session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_SEC_1X_PROV;
              return RIL_E_SUCCESS;
            }
            else if (index == qcril_uim.card_status.index_gw_ter_prov)
            {
              session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_TER_GW_PROV;
              return RIL_E_SUCCESS;
            }
            else if (index == qcril_uim.card_status.index_1x_ter_prov)
            {
              session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_TER_1X_PROV;
              return RIL_E_SUCCESS;
            }
            else
            {
              /* Non provisioning session. We can return success only if AID was
                 provided as input parameter */
              if (aid_ptr != NULL &&
                  max_aid_len >= aid_size)
              {
                memcpy(aid_ptr, aid_buffer, aid_size);
                if (slot == 0)
                {
                  session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_1;
                }
                else if (slot == 1)
                {
                  session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_2;
                }
                else
                {
                  session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_3;
                }
                session_info_ptr->aid.data_ptr = aid_ptr;
                session_info_ptr->aid.data_len = aid_size;

                return RIL_E_SUCCESS;
              }
            }
            /* The application was found, but was impossible to determine the session
               type. Break, so it will return an error */
            break;
          }
        }
      }
      break;

    default:
      /* Use card session for paths outside DF GSM/CDMA, ADF for USIM/CSIM */
      if (slot == 0)
      {
        session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_CARD_SLOT_1;
      }
      else if (slot == 1)
      {
        session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_CARD_SLOT_2;
      }
      else
      {
        session_info_ptr->session_type = QMI_UIM_SESSION_TYPE_CARD_SLOT_3;
      }
      return RIL_E_SUCCESS;
  }

  /* No application matched... return error */
  QCRIL_LOG_ERROR("%s", "Error extracting appropriate session !!");

  return RIL_E_REQUEST_NOT_SUPPORTED;

} /* qcril_uim_extract_session_type */


/*===========================================================================

  FUNCTION:  qcril_uim_extract_file_id

===========================================================================*/
/*!
    @brief
    Extract the file id and path from a SIM_IO request.

    @return
    RIL_Errno code
*/
/*=========================================================================*/
RIL_Errno qcril_uim_extract_file_id
(
  const RIL_SIM_IO_v6      * request_io_ptr,
  qmi_uim_file_id_type     * file_id_ptr,
  uint8                    * path_ptr,
  uint16                     max_path_len
)
{
  size_t path_len = 0;

  if(file_id_ptr == NULL || request_io_ptr == NULL || path_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL pointer");
    QCRIL_ASSERT(0);
    return RIL_E_GENERIC_FAILURE;
  }

  if(max_path_len == 0)
  {
    QCRIL_LOG_ERROR("%s", "max_path_len is 0");
    QCRIL_ASSERT(0);
    return RIL_E_GENERIC_FAILURE;
  }

  /* Sanity check */
  if (request_io_ptr->path == NULL)
  {
    QCRIL_LOG_ERROR( "%s: NULL Path \n", __FUNCTION__);
    return RIL_E_GENERIC_FAILURE;
  }

  /* Logging */
  QCRIL_LOG_VERBOSE("Path: %s", request_io_ptr->path);

  path_len = strlen(request_io_ptr->path);

  /* Path at least 4 digits */
  if (path_len < 4)
  {
    QCRIL_LOG_ERROR( "%s: Path too short len: 0x%x\n", __FUNCTION__, path_len);
    return RIL_E_GENERIC_FAILURE;
  }
  /* Path length is multiple of 4 digits */
  if((path_len % 4) != 0)
  {
    QCRIL_LOG_ERROR( "%s: Path not divisible by 4 len: 0x%x\n", __FUNCTION__, path_len);
    return RIL_E_GENERIC_FAILURE;
  }
  /* Path length not too long */
  if (path_len > max_path_len * 2)
  {
    QCRIL_LOG_ERROR( "%s: Path is too long: 0x%x\n", __FUNCTION__, path_len);
    return RIL_E_GENERIC_FAILURE;
  }

  /* Convert path into binary format */
  qcril_uim_hexstring_to_bin(request_io_ptr->path, path_ptr, max_path_len);

  file_id_ptr->path.data_len = path_len / 2;
  file_id_ptr->path.data_ptr = path_ptr;

  file_id_ptr->file_id = request_io_ptr->fileid & 0xFFFF;

  return RIL_E_SUCCESS;

} /* qcril_uim_extract_file_id */


/*===========================================================================

  FUNCTION:  qcril_uim_extract_pin1_status

===========================================================================*/
/*!
    @brief
    Extract the pin1 status based on the passed application id.

    @return
    RIL_Errno code
*/
/*=========================================================================*/
RIL_Errno qcril_uim_extract_pin1_status
(
  uint8                           slot,
  const char                    * RIL_aid_ptr,
  qmi_uim_pin_status_type       * pin_status_ptr
)
{
  uint16 aid_size     = 0;
  uint8  aid_buffer[QMI_UIM_MAX_AID_LEN];
  int    i            = 0;
  int    prov_slot    = 0;

  if(pin_status_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL pin_status_ptr");
    QCRIL_ASSERT(0);
    return RIL_E_GENERIC_FAILURE;
  }

  /* Check if slot value is valid */
  if (slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot value: 0x%x", slot);
    return RIL_E_REQUEST_NOT_SUPPORTED;
  }

  /* Verify card status */
  if (qcril_uim.card_status.card[slot].card_state != QMI_UIM_CARD_STATE_PRESENT)
  {
    QCRIL_LOG_ERROR("%s", "Card is not present");
    return RIL_E_REQUEST_NOT_SUPPORTED;
  }

  /* If AID in the request is NULL, check if it is SIM or RUIM */
  aid_size = (RIL_aid_ptr == NULL) ? 0 : strlen(RIL_aid_ptr);
  if (aid_size == 0)
  {
    if (qcril_uim.card_status.index_gw_pri_prov != QCRIL_UIM_INVALID_SESSION_VALUE)
    {
      i = qcril_uim.card_status.index_gw_pri_prov & 0xFF;
      prov_slot = (qcril_uim.card_status.index_gw_pri_prov >> 8) & 0xFF;
      /* Passed slot & index must match stored index */
      if ((i >= 0) && (i < QMI_UIM_MAX_APP_PER_CARD_COUNT) && (prov_slot == slot))
      {
        if (qcril_uim.card_status.card[slot].application[i].app_type == QMI_UIM_APP_SIM)
        {
          *pin_status_ptr = qcril_uim.card_status.card[slot].application[i].pin1_state;
          return RIL_E_SUCCESS;
        }
      }
    }

    if (qcril_uim.card_status.index_1x_pri_prov != QCRIL_UIM_INVALID_SESSION_VALUE)
    {
      i = qcril_uim.card_status.index_1x_pri_prov & 0xFF;
      prov_slot = (qcril_uim.card_status.index_1x_pri_prov >> 8) & 0xFF;
      /* Passed slot & index must match stored index */
      if ((i >= 0) && (i < QMI_UIM_MAX_APP_PER_CARD_COUNT) && (prov_slot == slot))
      {
        if (qcril_uim.card_status.card[slot].application[i].app_type == QMI_UIM_APP_RUIM)
        {
          *pin_status_ptr = qcril_uim.card_status.card[slot].application[i].pin1_state;
          return RIL_E_SUCCESS;
        }
      }
    }

    if (qcril_uim.card_status.index_gw_sec_prov != QCRIL_UIM_INVALID_SESSION_VALUE)
    {
      i = qcril_uim.card_status.index_gw_sec_prov & 0xFF;
      prov_slot = (qcril_uim.card_status.index_gw_sec_prov >> 8) & 0xFF;
      /* Passed slot & index must match stored index */
      if ((i >= 0) && (i < QMI_UIM_MAX_APP_PER_CARD_COUNT) && (prov_slot == slot))
      {
        if (qcril_uim.card_status.card[slot].application[i].app_type == QMI_UIM_APP_SIM)
        {
          *pin_status_ptr = qcril_uim.card_status.card[slot].application[i].pin1_state;
          return RIL_E_SUCCESS;
        }
      }
    }

    if (qcril_uim.card_status.index_1x_sec_prov != QCRIL_UIM_INVALID_SESSION_VALUE)
    {
      i = qcril_uim.card_status.index_1x_sec_prov & 0xFF;
      prov_slot = (qcril_uim.card_status.index_1x_sec_prov >> 8) & 0xFF;
      /* Passed slot & index must match stored index */
      if ((i >= 0) && (i < QMI_UIM_MAX_APP_PER_CARD_COUNT) && (prov_slot == slot))
      {
        if (qcril_uim.card_status.card[slot].application[i].app_type == QMI_UIM_APP_RUIM)
        {
          *pin_status_ptr = qcril_uim.card_status.card[slot].application[i].pin1_state;
          return RIL_E_SUCCESS;
        }
      }
    }

    if (qcril_uim.card_status.index_gw_ter_prov != QCRIL_UIM_INVALID_SESSION_VALUE)
    {
      i = qcril_uim.card_status.index_gw_ter_prov & 0xFF;
      prov_slot = (qcril_uim.card_status.index_gw_ter_prov >> 8) & 0xFF;
      /* Passed slot & index must match stored index */
      if ((i >= 0) && (i < QMI_UIM_MAX_APP_PER_CARD_COUNT) && (prov_slot == slot))
      {
        if (qcril_uim.card_status.card[slot].application[i].app_type == QMI_UIM_APP_SIM)
        {
          *pin_status_ptr = qcril_uim.card_status.card[slot].application[i].pin1_state;
          return RIL_E_SUCCESS;
        }
      }
    }

    if (qcril_uim.card_status.index_1x_ter_prov != QCRIL_UIM_INVALID_SESSION_VALUE)
    {
      i = qcril_uim.card_status.index_1x_ter_prov & 0xFF;
      prov_slot = (qcril_uim.card_status.index_1x_ter_prov >> 8) & 0xFF;
      /* Passed slot & index must match stored index */
      if ((i >= 0) && (i < QMI_UIM_MAX_APP_PER_CARD_COUNT) && (prov_slot == slot))
      {
        if (qcril_uim.card_status.card[slot].application[i].app_type == QMI_UIM_APP_RUIM)
        {
          *pin_status_ptr = qcril_uim.card_status.card[slot].application[i].pin1_state;
          return RIL_E_SUCCESS;
        }
      }
    }
    /* If no match for either a GSM or CDMA app, fail the request */
    return RIL_E_GENERIC_FAILURE;
  }

  /* Convert AID string into binary buffer */
  aid_size = qcril_uim_hexstring_to_bin(RIL_aid_ptr,
                                        aid_buffer,
                                        QMI_UIM_MAX_AID_LEN);
  if (aid_size == 0)
  {
    QCRIL_LOG_ERROR("%s", "Error converting AID string into binary");
    return RIL_E_REQUEST_NOT_SUPPORTED;
  }

  for (i = 0; i < qcril_uim.card_status.card[slot].num_app &&
              i < QMI_UIM_MAX_APP_PER_CARD_COUNT; i++)
  {
    if (qcril_uim.card_status.card[slot].application[i].aid_len == aid_size &&
        aid_size <= sizeof(qcril_uim.card_status.card[slot].application[i].aid_value) &&
        aid_size <= sizeof(aid_buffer))
    {
      if (memcmp(qcril_uim.card_status.card[slot].application[i].aid_value,
                 aid_buffer,
                 aid_size) == 0)
      {
        /* We have found the application, fetch appropriate pin state */
#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
        if (qcril_uim.card_status.card[slot].application[i].univ_pin ==
            QCRIL_UIM_UPIN_STATE_REPLACES_PIN1)
        {
          *pin_status_ptr = qcril_uim.card_status.card[slot].upin_state;
          QCRIL_LOG_INFO("%s", "UPIN enabled, sending UPIN state instead of PIN1");
        }
        else
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN */
        {
          *pin_status_ptr = qcril_uim.card_status.card[slot].application[i].pin1_state;
        }
        return RIL_E_SUCCESS;
      }
    }
  }

  /* No application matched... return error */
  QCRIL_LOG_ERROR("%s", "Error finding application for pin1 status !!");

  return RIL_E_REQUEST_NOT_SUPPORTED;
} /* qcril_uim_extract_pin1_status */


/*===========================================================================

  FUNCTION:  qcril_uim_extract_index

===========================================================================*/
/*!
    @brief
    Extracts the provisioning application index & slot from session type.

    @return
    RIL_Errno code
*/
/*=========================================================================*/
RIL_Errno qcril_uim_extract_index
(
  uint8                       * index_ptr,
  uint8                       * slot_ptr,
  qmi_uim_session_type          session_type
)
{
  if(index_ptr == NULL || slot_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL pointer");
    QCRIL_ASSERT(0);
    return RIL_E_GENERIC_FAILURE;
  }

  switch (session_type)
  {
    case QMI_UIM_SESSION_TYPE_PRI_GW_PROV:
      if (qcril_uim.card_status.index_gw_pri_prov != QCRIL_UIM_INVALID_SESSION_VALUE)
      {
        *index_ptr = qcril_uim.card_status.index_gw_pri_prov & 0xFF;
        *slot_ptr = (qcril_uim.card_status.index_gw_pri_prov >> 8) & 0xFF;
        return RIL_E_SUCCESS;
      }
      break;

    case QMI_UIM_SESSION_TYPE_PRI_1X_PROV:
      if (qcril_uim.card_status.index_1x_pri_prov != QCRIL_UIM_INVALID_SESSION_VALUE)
      {
        *index_ptr = qcril_uim.card_status.index_1x_pri_prov & 0xFF;
        *slot_ptr = (qcril_uim.card_status.index_1x_pri_prov >> 8) & 0xFF;
        return RIL_E_SUCCESS;
      }
      break;

    case QMI_UIM_SESSION_TYPE_SEC_GW_PROV:
      if (qcril_uim.card_status.index_gw_sec_prov != QCRIL_UIM_INVALID_SESSION_VALUE)
      {
        *index_ptr = qcril_uim.card_status.index_gw_sec_prov & 0xFF;
        *slot_ptr = (qcril_uim.card_status.index_gw_sec_prov >> 8) & 0xFF;
        return RIL_E_SUCCESS;
      }
      break;

    case QMI_UIM_SESSION_TYPE_SEC_1X_PROV:
      if (qcril_uim.card_status.index_1x_sec_prov != QCRIL_UIM_INVALID_SESSION_VALUE)
      {
        *index_ptr = qcril_uim.card_status.index_1x_sec_prov & 0xFF;
        *slot_ptr = (qcril_uim.card_status.index_1x_sec_prov >> 8) & 0xFF;
        return RIL_E_SUCCESS;
      }
      break;

    case QMI_UIM_SESSION_TYPE_TER_GW_PROV:
      if (qcril_uim.card_status.index_gw_ter_prov != QCRIL_UIM_INVALID_SESSION_VALUE)
      {
        *index_ptr = qcril_uim.card_status.index_gw_ter_prov & 0xFF;
        *slot_ptr = (qcril_uim.card_status.index_gw_ter_prov >> 8) & 0xFF;
        return RIL_E_SUCCESS;
      }
      break;

    case QMI_UIM_SESSION_TYPE_TER_1X_PROV:
      if (qcril_uim.card_status.index_1x_ter_prov != QCRIL_UIM_INVALID_SESSION_VALUE)
      {
        *index_ptr = qcril_uim.card_status.index_1x_ter_prov & 0xFF;
        *slot_ptr = (qcril_uim.card_status.index_1x_ter_prov >> 8) & 0xFF;
        return RIL_E_SUCCESS;
      }
      break;

    default:
      QCRIL_LOG_ERROR("%s", "Only provisioning session supported !!");
      break;
  }

  return RIL_E_REQUEST_NOT_SUPPORTED;
} /* qcril_uim_extract_index */


/*===========================================================================

  FUNCTION:  qcril_uim_add_non_provisioning_session

===========================================================================*/
/*!
    @brief
    Add a non provisioning session to the global array. If the session
    already exists, it simply overwrites the token id

    @return
    RIL_Errno code
*/
/*=========================================================================*/
RIL_Errno qcril_uim_add_non_provisioning_session
(
  const qmi_uim_session_info_type  * session_info_ptr,
  RIL_Token                          token
)
{
  int i     = 0;
  int index = -1;

  if(session_info_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "NULL session_info_ptr");
    QCRIL_ASSERT(0);
    return RIL_E_GENERIC_FAILURE;
  }

  QCRIL_LOG_VERBOSE("Adding token: %d", qcril_log_get_token_id(token));

  /* Check arguments */
  if (session_info_ptr->session_type != QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_1 &&
      session_info_ptr->session_type != QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_2 &&
      session_info_ptr->session_type != QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_3)
  {
    QCRIL_LOG_ERROR("Invalid session type: %d", session_info_ptr->session_type);
    return RIL_E_GENERIC_FAILURE;
  }
  if (session_info_ptr->aid.data_len == 0 ||
      session_info_ptr->aid.data_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid AID to add non prov session!");
    return RIL_E_GENERIC_FAILURE;
  }

  /* Find slot in the global array */
  for (i = 0; i < QCRIL_UIM_NON_PROV_SESSIONS_MAX_COUNT; i++)
  {
    if (!qcril_uim_non_prov_session_list[i].in_use && index < 0)
    {
      index = i;
    }
    else if (qcril_uim_non_prov_session_list[i].in_use)
    {
      if (qcril_uim_non_prov_session_list[i].session_info.aid.data_ptr != NULL &&
          qcril_uim_non_prov_session_list[i].session_info.aid.data_len == session_info_ptr->aid.data_len &&
          memcmp(qcril_uim_non_prov_session_list[i].session_info.aid.data_ptr,
                 session_info_ptr->aid.data_ptr,
                 session_info_ptr->aid.data_len) == 0)
      {
        index = i;
        break;
      }
    }
  }

  /* Check index */
  if (index < 0 || index >= QCRIL_UIM_NON_PROV_SESSIONS_MAX_COUNT)
  {
    QCRIL_LOG_ERROR("%s", "Unable to find index to add non prov session!");
    return RIL_E_GENERIC_FAILURE;
  }

  /* If index used for first time, initialize it */
  if (!qcril_uim_non_prov_session_list[index].in_use)
  {
    qcril_uim_non_prov_session_list[index].session_info.aid.data_ptr =
      qcril_malloc(session_info_ptr->aid.data_len);

    if (qcril_uim_non_prov_session_list[index].session_info.aid.data_ptr == NULL)
    {
      return RIL_E_GENERIC_FAILURE;
    }

    qcril_uim_non_prov_session_list[index].in_use = TRUE;
    qcril_uim_non_prov_session_list[index].session_info.aid.data_len =
      session_info_ptr->aid.data_len;
    memcpy(qcril_uim_non_prov_session_list[index].session_info.aid.data_ptr,
           session_info_ptr->aid.data_ptr,
           session_info_ptr->aid.data_len);
    qcril_uim_non_prov_session_list[index].session_info.session_type =
      session_info_ptr->session_type;
  }

  /* Update the token in all cases */
  qcril_uim_non_prov_session_list[index].last_token = token;

  QCRIL_LOG_VERBOSE("Token %d added successfully. Index = 0x%x",
                    qcril_log_get_token_id(token), index);

  return RIL_E_SUCCESS;
} /* qcril_uim_add_non_provisioning_session */


/*===========================================================================

  FUNCTION:  qcril_uim_remove_non_provisioning_session

===========================================================================*/
/*!
    @brief
    Removes a non provisioning session from the global array, if the
    token is matching with the last one stored in the array. If the session
    does not exist or the token does not exists, nothing is done.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_remove_non_provisioning_session
(
  RIL_Token  token
)
{
  int                                i = 0;
  qmi_uim_rsp_data_type              rsp_data;
  qmi_uim_close_session_params_type  close_params;

  memset(&rsp_data, 0, sizeof(qmi_uim_rsp_data_type));
  memset(&close_params, 0, sizeof(qmi_uim_close_session_params_type));

  for (i = 0; i < QCRIL_UIM_NON_PROV_SESSIONS_MAX_COUNT; i++)
  {
    if (qcril_uim_non_prov_session_list[i].in_use &&
        qcril_log_get_token_id(qcril_uim_non_prov_session_list[i].last_token) == qcril_log_get_token_id(token))
    {
      /* Skip closing the session if it is an ISIM app to avoid losing ISIM
         refresh registration and frequently opening/closing ISIM session. */
      if (!qcril_uim_check_aid_with_app_type((const qmi_uim_data_type *)&qcril_uim_non_prov_session_list[i].session_info.aid,
                                              QMI_UIM_APP_ISIM))
      {
        QCRIL_LOG_VERBOSE("Non prov session to close found - index: 0x%x, token: %d",
                          i, qcril_log_get_token_id(token));

        close_params.session_info = qcril_uim_non_prov_session_list[i].session_info;
        qcril_qmi_uim_close_session(qcril_uim.qmi_handle,
                                    &close_params,
                                    &rsp_data);
      }

      /* Free memory */
      if (qcril_uim_non_prov_session_list[i].session_info.aid.data_ptr)
      {
        qcril_free(qcril_uim_non_prov_session_list[i].session_info.aid.data_ptr);
        qcril_uim_non_prov_session_list[i].session_info.aid.data_ptr = NULL;
      }

      /* Zero the memory for the session */
      memset(&qcril_uim_non_prov_session_list[i],
             0,
             sizeof(qcril_uim_non_prov_session_list[i]));
      return;
    }
  }

  QCRIL_LOG_VERBOSE("Token for non-provisioning session not found: %d",
                    qcril_log_get_token_id(token));
} /* qcril_uim_remove_non_provisioning_session */


/*=========================================================================

  FUNCTION:  qcril_uim_allocate_orig_request

===========================================================================*/
/*!
    @brief
    Allocates the memory for storing the original request parameters.
    This function is used when PIN2 verification callback is called and the
    next step of SIM_IO call is to be performed.

    @return
    Buffer with required request parameters
*/
/*=========================================================================*/
qcril_uim_original_request_type* qcril_uim_allocate_orig_request
(
  qcril_instance_id_e_type                     instance_id,
  qcril_modem_id_e_type                        modem_id,
  RIL_Token                                    token,
  int                                          request_id,
  qmi_uim_session_type                         session_type
)
{
  qcril_uim_original_request_type * ret_ptr = NULL;

  /* Allocate memory */
  ret_ptr = (qcril_uim_original_request_type*)
             qcril_malloc(sizeof(qcril_uim_original_request_type));

  if (ret_ptr == NULL)
  {
    return NULL;
  }

  /* Update request parameters */
  memset(ret_ptr, 0, sizeof(qcril_uim_original_request_type));
  ret_ptr->instance_id  = instance_id;
  ret_ptr->modem_id     = modem_id;
  ret_ptr->token        = token;
  ret_ptr->request_id   = request_id;
  ret_ptr->session_type = session_type;

  return ret_ptr;
} /* qcril_uim_allocate_orig_request */


/*=========================================================================

  FUNCTION:  qcril_uim_instance_id_to_slot

===========================================================================*/
/*!
    @brief
    Returns the slot id based on the input instance id.

    @return
    Mapped slot id.
*/
/*=========================================================================*/
uint8 qcril_uim_instance_id_to_slot
(
  qcril_instance_id_e_type                     instance_id
)
{
  uint8    slot    = QCRIL_UIM_INVALID_SLOT_INDEX_VALUE;
  uint32_t slot_id = 0;

  /* Find slot info, We now have new API where the slot is:
     0 - if it is regular configuration including DSDA (non-DSDS)
     1 - only if it is a DSDS configuration */
  slot_id = qmi_ril_get_sim_slot();
  switch (slot_id)
  {
    case 0:
    case 1:
    case 2:
      slot = slot_id;
      break;

    default:
      QCRIL_LOG_ERROR( " Invalid slot_id returned: 0x%x\n", slot_id);
      /* slot would be QCRIL_UIM_INVALID_SLOT_INDEX_VALUE in this case */
      break;
  }

  QCRIL_LOG_INFO( "Slot found: 0x%X", slot);
  return slot;

} /* qcril_uim_instance_id_to_slot */


/*=========================================================================

  FUNCTION:  qcril_uim_read_iccid

===========================================================================*/
/*!
    @brief
    Reads EF ICCID from card using CARD_SLOT session

    @return
    Contents of EF-ICCID and file size.
*/
/*=========================================================================*/
RIL_Errno qcril_uim_read_iccid
(
  uint8                                      slot,
  uint8                                      *iccid_data_ptr,
  uint8                                      *iccid_len_ptr
)
{
  qmi_uim_rsp_data_type                  read_rsp_data;
  qmi_uim_read_transparent_params_type   read_params;
  uint8                                  iccid_path[QCRIL_UIM_ICCID_PATH_LEN] =
                                           {QCRIL_UIM_MF_ID_HIGH_BYTE,
                                            QCRIL_UIM_MF_ID_LOW_BYTE};
  unsigned short                         file_data_len;
  int                                    i   = 0;
  int                                    res = 0;

  memset(&read_rsp_data, 0, sizeof(qmi_uim_rsp_data_type));
  memset(&read_params, 0, sizeof(qmi_uim_read_transparent_params_type));

  if(iccid_data_ptr == NULL || iccid_len_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL pointer: iccid_data_ptr (0x%x) or iccid_len_ptr (0x%x)",
                    iccid_data_ptr, iccid_len_ptr);
    return RIL_E_GENERIC_FAILURE;
  }

  if(slot == 0)
  {
    read_params.session_info.session_type = QMI_UIM_SESSION_TYPE_CARD_SLOT_1;
  }
  else if(slot == 1)
  {
    read_params.session_info.session_type = QMI_UIM_SESSION_TYPE_CARD_SLOT_2;
  }
  else if(slot == 2)
  {
    read_params.session_info.session_type = QMI_UIM_SESSION_TYPE_CARD_SLOT_3;
  }
  else
  {
    QCRIL_LOG_ERROR("Invalid slot: %d", slot);
    return RIL_E_GENERIC_FAILURE;
  }

  read_params.file_id.file_id       = QCRIL_UIM_EF_ICCID;
  read_params.file_id.path.data_len = QCRIL_UIM_ICCID_PATH_LEN;
  read_params.file_id.path.data_ptr = iccid_path;

  res = qcril_qmi_uim_read_transparent(qcril_uim.qmi_handle,
                                       &read_params,
                                       NULL,
                                       NULL,
                                       &read_rsp_data);

  if((res < 0) ||
     (read_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr == NULL))
  {
    QCRIL_LOG_ERROR("error reading iccid from card; qmi_err_code: 0x%x",
                    read_rsp_data.qmi_err_code);
    if(read_rsp_data.qmi_err_code == QMI_SERVICE_ERR_DEVICE_NOT_READY)
    {
      return RIL_E_RADIO_NOT_AVAILABLE;
    }
    else
    {
      return RIL_E_GENERIC_FAILURE;
    }
  }

  /* Copy contents of ICCID */
  if(read_rsp_data.rsp_data.read_transparent_rsp.content.data_len >
     *iccid_len_ptr)
  {
    file_data_len = *iccid_len_ptr;
  }
  else
  {
    file_data_len =
      read_rsp_data.rsp_data.read_transparent_rsp.content.data_len;
  }
  memcpy(iccid_data_ptr,
         read_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr,
         file_data_len);

  *iccid_len_ptr = file_data_len;

  QCRIL_LOG_DEBUG("%s\n","EF-ICCID contents:");
  for(i = 0; i < file_data_len; i++)
  {
    QCRIL_LOG_VERBOSE("0x%x", iccid_data_ptr[i]);
  }

  /* Client needs to free the memory for raw data */
  qcril_free(read_rsp_data.rsp_data.read_transparent_rsp.content.data_ptr);

  return RIL_E_SUCCESS;
} /* qcril_uim_read_iccid */


/*===========================================================================

  FUNCTION:  qcril_uim_convert_slot_id_to_scws_slot_id

 ==========================================================================*/
/*!
     @brief
      Converts the slot id from cat enum type to scws enum type.

     @return
      Result of the conversion
                                                                           */
/*=========================================================================*/
boolean qcril_uim_convert_slot_id_to_scws_slot_id
(
  int                             uim_slot_id,
  qcril_scws_slot_enum_type     * scws_slot_id_ptr
)
{
  boolean ret = TRUE;

  if( scws_slot_id_ptr == NULL )
  {
    QCRIL_LOG_ERROR("%s", "NULL scws_slot_id_ptr");
    QCRIL_ASSERT(0);
    return FALSE;
  }

  /* Convert cat type to scws type */
  switch (uim_slot_id)
  {
    case 0:
      *scws_slot_id_ptr = QCRIL_SCWS_SLOT_1;
      break;
    case 1:
      *scws_slot_id_ptr = QCRIL_SCWS_SLOT_2;
      break;
    case 2:
      *scws_slot_id_ptr = QCRIL_SCWS_SLOT_3;
      break;
    default:
      QCRIL_LOG_ERROR("Invalid input uim_slot_id: %d \n", uim_slot_id);
      ret = FALSE;
      break;
  }
  return ret;
} /* qcril_uim_convert_slot_id_to_scws_slot_id */


/*===========================================================================

  FUNCTION:  qcril_uim_extract_isim_index

 ==========================================================================*/
/*!
     @brief
      Extracts the app index of ISIM application from slot

     @return
      RIL_Errno code
                                                                           */
/*=========================================================================*/
RIL_Errno qcril_uim_extract_isim_index
(
  uint8                       * index_ptr,
  uint8                         slot
)
{
  uint8       i          = 0;
  const uint8 isim_aid[] = {0xA0, 0x00, 0x00, 0x00, 0x87, 0x10, 0x04};

  for(i = 0; (i < qcril_uim.card_status.card[slot].num_app) && (i < QMI_UIM_MAX_APP_PER_CARD_COUNT); i++)
  {
    if (memcmp(qcril_uim.card_status.card[slot].application[i].aid_value,
               isim_aid,
               sizeof(isim_aid)) == 0)
    {
      *index_ptr = i;
      QCRIL_LOG_INFO("ISIM app found: app_index: 0x%x, slot: 0x%x", i, slot);
      return RIL_E_SUCCESS;
    }
  }

  QCRIL_LOG_INFO("%s", "ISIM app not present on slot 0x%x", slot);
  return RIL_E_GENERIC_FAILURE;
} /* qcril_uim_extract_isim_index */


/*=========================================================================

  FUNCTION:  qcril_uim_find_app_in_slot

===========================================================================*/
/*!
    @brief
    Function to find if the passed application type is present in the slot.

    @return
    TRUE if found, FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_uim_find_app_in_slot
(
  uint8                  slot,
  qmi_uim_app_type       app_type
)
{
  uint8 index = 0;

  if(slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot %d", slot);
    QCRIL_ASSERT(0);
    return FALSE;
  }

  for (index = 0; (index < qcril_uim.card_status.card[slot].num_app) &&
                  (index < QMI_UIM_MAX_APP_PER_CARD_COUNT); index++)
  {
    if (qcril_uim.card_status.card[slot].application[index].app_type == app_type)
    {
      return TRUE;
    }
  }

  return FALSE;
} /* qcril_uim_find_app_in_slot */


/*=========================================================================

  FUNCTION:  qcril_uim_find_base64char_value

===========================================================================*/
/*!
    @brief
    Returns the decoded value of the Base64 character by finding the passed
    ASCII character in the Base 64 table.

    @return
    Converted value of Base 64 character; 0 otherwise.
*/
/*=========================================================================*/
static uint8 qcril_uim_find_base64_values
(
  boolean  find_char,
  char     input_char,
  uint8    input_index
)
{
  const char base64_table[] =
    {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
  const char * value_ptr = NULL;

  if (find_char)
  {
    value_ptr = strchr(base64_table, input_char);
    if (value_ptr)
    {
      return value_ptr - base64_table;
    }
  }
  else
  {
    if (input_index < (sizeof(base64_table)/sizeof(base64_table[0])))
    {
      return (uint8)base64_table[input_index];
    }
  }

  return 0;
} /* qcril_uim_find_base64_values */


/*=========================================================================

  FUNCTION:  qcril_uim_alloc_base64string_to_bin

===========================================================================*/
/*!
    @brief
    Allocates memory and converts a Base64 encoded ASCII string into
    binary format.

    @return
    Buffer with binary data
*/
/*=========================================================================*/
uint8* qcril_uim_alloc_base64string_to_bin
(
  const char   * input_ptr,
  uint16       * output_len_ptr
)
{
  uint8   i           = 0;
  uint8   j           = 0;
  uint16  input_len   = 0;
  uint16  output_len  = 0;
  uint8 * output_ptr  = NULL;

  if (input_ptr == NULL || output_len_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s","NULL pointer");
    QCRIL_ASSERT(0);
    return NULL;
  }

  /* Base64 string have to be multiples of 4 byte-blocks and
     possibly padded to a 4 bytes block */
  input_len = strlen(input_ptr);
  if ((input_len == 0) || (input_len % 4) != 0)
  {
    QCRIL_LOG_ERROR("Invalid Base64 string length: %d", input_len);
    return NULL;
  }

  output_len = (input_len / 4) * 3;

  /* Update output buffer size if input was padded with '='s
     Only 2 '=' padded bytes are allowed per 4 byte-block */
  if (input_ptr[input_len-1] == '=')
  {
    output_len--;
    if (input_ptr[input_len-2] == '=')
    {
      output_len--;
    }
  }

  output_ptr = qcril_malloc(output_len);
  if (output_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Error allocating memory for decoded_buf_ptr");
    return NULL;
  }

  memset(output_ptr, 0, output_len);

  /* Decode the string & convert to binary */
  while ((i+3) < input_len)
  {
    uint8 binary_byte = 0;

    /* Decode each block of 4 Base64 bytes to 3 binary bytes */
    uint32 first  = qcril_uim_find_base64_values(TRUE, input_ptr[i++], 0);
    uint32 second = qcril_uim_find_base64_values(TRUE, input_ptr[i++], 0);
    uint32 third  = qcril_uim_find_base64_values(TRUE, input_ptr[i++], 0);
    uint32 fourth = qcril_uim_find_base64_values(TRUE, input_ptr[i++], 0);

    uint32 all_three = (first  << (3 * QCRIL_UIM_NUM_BITS_BASE64_CHAR)) +
                       (second << (2 * QCRIL_UIM_NUM_BITS_BASE64_CHAR)) +
                       (third  << (1 * QCRIL_UIM_NUM_BITS_BASE64_CHAR)) +
                        fourth;

    /* 3 binary bytes */
    binary_byte = j < output_len ? ((all_three >> (2 * QCRIL_UIM_NUM_BITS_CHAR)) & 0xFF) : 0;
    output_ptr[j++] = binary_byte;
    binary_byte = j < output_len ? ((all_three >> (1 * QCRIL_UIM_NUM_BITS_CHAR)) & 0xFF) : 0;
    output_ptr[j++] = binary_byte;
    binary_byte = j < output_len ? (all_three & 0xFF) : 0;
    output_ptr[j++] = binary_byte;
  }

  *output_len_ptr = output_len;
  return output_ptr;
} /* qcril_uim_alloc_base64string_to_bin */


/*=========================================================================

  FUNCTION:  qcril_uim_alloc_bin_to_base64string

===========================================================================*/
/*!
    @brief
    Allocates memory and converts a binary datastream to a Base64 encoded
    ASCII string format.

    @return
    NULL terminated Base64 string
*/
/*=========================================================================*/
char* qcril_uim_alloc_bin_to_base64string
(
  const uint8   * input_ptr,
  uint16          input_len
)
{
  uint8   i                = 0;
  uint8   j                = 0;
  uint8   extra_bytes      = 0;
  uint16  output_len       = 0;
  char  * output_ptr       = NULL;

  if ((input_ptr == NULL) || (input_len == 0))
  {
    QCRIL_LOG_ERROR("Invalid input parameters: input_ptr: 0x%x, input_len 0x%x",
                    input_ptr, input_len);
    return NULL;
  }

  /* Calculate the max buffer size needed for the encoded Base64 string,
     3 binary bytes make 4 Base64 bytes */
  output_len =  sizeof(char) + ((((input_len % 3 > 0) ? 1 : 0) +
                                  (input_len / 3 )) * 4);
  output_ptr = qcril_malloc(output_len);
  if (output_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Error allocating memory for output_ptr");
    return NULL;
  }

  memset(output_ptr, 0, output_len);

  /* Now encode the ASCII string to Base64 string */
  while (i < input_len)
  {
    /* Encode each block of 4 bytes from 3 ASCII bytes */
    uint32 first  = i < input_len ? input_ptr[i++] : 0;
    uint32 second = i < input_len ? input_ptr[i++] : 0;
    uint32 third  = i < input_len ? input_ptr[i++] : 0;

    uint32 all_three = (first  << (2 * QCRIL_UIM_NUM_BITS_CHAR)) +
                       (second << (1 * QCRIL_UIM_NUM_BITS_CHAR)) +
                        third;

    /* 4 Base64 bytes */
    if ((j+3) < output_len)
    {
      output_ptr[j++] = qcril_uim_find_base64_values(
                          FALSE, 0, (all_three >> (3 * QCRIL_UIM_NUM_BITS_BASE64_CHAR)) & 0x3F);
      output_ptr[j++] = qcril_uim_find_base64_values(
                          FALSE, 0, (all_three >> (2 * QCRIL_UIM_NUM_BITS_BASE64_CHAR)) & 0x3F);
      output_ptr[j++] = qcril_uim_find_base64_values(
                          FALSE, 0, (all_three >> (1 * QCRIL_UIM_NUM_BITS_BASE64_CHAR)) & 0x3F);
      output_ptr[j++] = qcril_uim_find_base64_values(
                          FALSE, 0, all_three & 0x3F);
    }
  }

  /* Update pading if required. It is needed if ASCII string's
     last group has either 1 or 2 bytes */
  extra_bytes = input_len % 3;
  if (extra_bytes)
  {
    uint8 bytes_to_fill = (extra_bytes == 1) ? 2 : 1;
    for (i = 1; i < 3 && bytes_to_fill; i++, --bytes_to_fill)
    {
      output_ptr[output_len - 1 - i] = '=';
    }
  }

  return output_ptr;
} /* qcril_uim_alloc_bin_to_base64string */


/*=========================================================================

  FUNCTION:  qcril_uim_is_prov_app_type

===========================================================================*/
/*!
    @brief
    Function determines if the app type is provision or not.

    @return
    TRUE if provisioning, FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_uim_is_prov_app_type
(
  qmi_uim_app_type       app_type
)
{
  if(app_type == QMI_UIM_APP_SIM  ||
     app_type == QMI_UIM_APP_USIM ||
     app_type == QMI_UIM_APP_RUIM ||
     app_type == QMI_UIM_APP_CSIM)
  {
    return TRUE;
  }

  return FALSE;
} /* qcril_uim_is_prov_app_type */


/*=========================================================================

  FUNCTION:  qcril_uim_is_prov_app_activated

===========================================================================*/
/*!
    @brief
    Function determines if an app on the specified slot is activated.

    @return
    TRUE if activated, FALSE otherwise.
*/
/*=========================================================================*/
boolean qcril_uim_is_prov_app_activated
(
  uint8              slot,
  qmi_uim_app_type   app_type,
  uint8              aid_len,
  char             * aid_ptr
)
{
  uint8           i              = 0;
  uint16          slot_app_index = 0;

  if ((slot >= QMI_UIM_MAX_CARD_COUNT) || (aid_len > QMI_UIM_MAX_AID_LEN) ||
      (aid_ptr == NULL))
  {
    return FALSE;
  }

  for (i = 0; i < QMI_UIM_MAX_APP_PER_CARD_COUNT; i++)
  {
    if ((qcril_uim.card_status.card[slot].application[i].app_type == app_type) &&
        (qcril_uim.card_status.card[slot].application[i].aid_len == aid_len) &&
        (memcmp(qcril_uim.card_status.card[slot].application[i].aid_value,
                aid_ptr, aid_len) == 0))
    {
      break;
    }
  }

  if (i == QMI_UIM_MAX_APP_PER_CARD_COUNT)
  {
    return FALSE;
  }

  slot_app_index = (uint16)(((uint16)slot << 8) | (uint16)i);

  if ((qcril_uim.card_status.index_gw_pri_prov == slot_app_index) ||
      (qcril_uim.card_status.index_1x_pri_prov == slot_app_index) ||
      (qcril_uim.card_status.index_gw_sec_prov == slot_app_index) ||
      (qcril_uim.card_status.index_1x_sec_prov == slot_app_index) ||
      (qcril_uim.card_status.index_gw_ter_prov == slot_app_index) ||
      (qcril_uim.card_status.index_1x_ter_prov == slot_app_index))
  {
    return TRUE;
  }

  return FALSE;
} /* qcril_uim_is_prov_app_activated */


/*===========================================================================

  FUNCTION:  qcril_uim_determine_fci_from_property

===========================================================================*/
/*!
    @brief
    Function converts the FCI value byte which trails the AID into
    its corresponding FCI value.

    @return
    qcril_uim_fci_value_type
*/
/*=========================================================================*/
static qcril_uim_fci_value_type qcril_uim_determine_fci_from_property
(
  char  fci_indicator
)
{
  qcril_uim_fci_value_type fci_value = QCRIL_UIM_FCI_VALUE_FCI_FALLBACK_FCP;

  switch (fci_indicator)
  {
    case QCRIL_UIM_PROPERTY_FCP:
      fci_value = QCRIL_UIM_FCI_VALUE_FCP;
      break;
    case QCRIL_UIM_PROPERTY_FCI:
      fci_value = QCRIL_UIM_FCI_VALUE_FCI;
      break;
    case QCRIL_UIM_PROPERTY_FMD:
      fci_value = QCRIL_UIM_FCI_VALUE_FMD;
      break;
    case QCRIL_UIM_PROPERTY_NO_DATA:
      fci_value = QCRIL_UIM_FCI_VALUE_NO_DATA;
      break;
    default:
      QCRIL_LOG_ERROR("Invalid FCI indicator parameter: 0x%x", fci_indicator);
      return QCRIL_UIM_FCI_VALUE_FCI_FALLBACK_FCP;
  }

  return fci_value;
} /* qcril_uim_determine_fci_from_property */


/*===========================================================================

  FUNCTION:  qcril_uim_determine_select_template_from_aid

===========================================================================*/
/*!
    @brief
    Function will attempt to determine the FCI values based on the
    AID/FCI data pairs sent through the UI. If this information is
    not present or incorrect, then we will check hardcoded AID values
    to try and find a match.

    @return
    qcril_uim_fci_value_type
*/
/*=========================================================================*/
qcril_uim_fci_value_type qcril_uim_determine_select_template_from_aid
(
  const char                      * aid_ptr
)
{
  uint16                                property_len         = 0;
  char                                  property_buffer[PROPERTY_VALUE_MAX];
  uint16                                aid_len              = 0;
  uint8                                 i                    = 0;

  if (aid_ptr == NULL)
  {
    QCRIL_LOG_ERROR("%s", "Invalid AID data");
    return QCRIL_UIM_FCI_VALUE_FCI_FALLBACK_FCP;
  }

  aid_len = strlen(aid_ptr);

  memset(property_buffer, 0x00, sizeof(property_buffer));

  /* First we will check if the input AID matches any of the AIDs
     of the property sets */
  /* The property list, if present should be formated in an ASCII string
     consisting of one or more AID/FCI value pairs. Each set consists of
     the AID values followed immediately by the FCI value which are defined
     with the below values matching spec ISO/IEC 7816:

     '0' - FCP
     '1' - FCI
     '2' - FMD
     '3' - No Data

     Each set of AID/FCI value pair must be terminated by an ASCII semicolon ';'
  */
  do
  {
    property_get( QCRIL_UIM_FCI_PROPERTY_NAME, property_buffer, "" );
    property_len = strlen(property_buffer);
    if (property_len == 0)
    {
      QCRIL_LOG_INFO("%s", "No property or invalid property set continue with hardcoded list");
      break;
    }

    /* A valid set entry in the property buffer must contain at minimum AID
       data (variable), an AID terminator character '=' separating the AID from
       the FCI template indicator, an FCI template indicator (1 byte) and the
       terminating ASCII semicolon or string terminator (1 byte).

       Ex.

       "A0003456=1;A5565345=2"

       AID1 = "A0003456", FCI template = '1' and ';' terminates the set
       AID2 = "A5565345", FCI template = '2' and '\0' terminates the set
    */
    for(i = 0; (i + aid_len + 2) < (property_len + 1); i++)
    {
      /* We only need to check the FCI property if the AID matches that of the property.
         We must also confirm that the AID stored in the property buffer is properly
         terminated as to not compare only a subset of the stored AID. */
      if((memcmp(aid_ptr, &property_buffer[i], aid_len) == 0) &&
         (property_buffer[i + aid_len] == '=') &&
         ((property_buffer[i + aid_len + 2] == ';') ||
          (property_buffer[i + aid_len + 2] == '\0')))
      {
        /* The byte after the AID in the FCI property list will
           contain the application's FCI value */
        return qcril_uim_determine_fci_from_property(property_buffer[i + aid_len + 1]);
      }

      /* ASCII ';' terminates the different sets of AID/FCI pairs in the property
         list. We will move through the buffer until we find an AID that matches. */
      while ((i + aid_len + 2) < property_len &&
             property_buffer[i] != ';')
      {
        i++;
      }
    }
  }
  while(0);

  /* If the properties check fails, we can check if the AID matches our
     internal list of hardcoded AID/FCI value pairs */
  for (i = 0; i < sizeof(qcril_uim_aid_fci_list)/sizeof(qcril_uim_aid_fci_type); i++)
  {
    if (aid_len == strlen(qcril_uim_aid_fci_list[i].aid_str_buffer) &&
        strcasecmp(aid_ptr, qcril_uim_aid_fci_list[i].aid_str_buffer) == 0)
    {
      return qcril_uim_aid_fci_list[i].fci_value;
    }
  }

  /* In the case where FCI value cannot be determined at run-time,
     we will default to try SELECT with FCP and if that fails due
     to an incorrect P2, we will retry the SELECT with FCI. */
  return QCRIL_UIM_FCI_VALUE_FCI_FALLBACK_FCP;
} /* qcril_uim_determine_select_template_from_aid */


/*===========================================================================

  FUNCTION:  qcril_uim_convert_fci_value

===========================================================================*/
/*!
    @brief
    Function converts QCRIL FCI value type to QMI FCI value type

    @return
    qmi_uim_fci_value_type
*/
/*=========================================================================*/
qmi_uim_fci_value_type qcril_uim_convert_fci_value
(
  qcril_uim_fci_value_type  qcril_fci_value
)
{
  qmi_uim_fci_value_type qmi_fci_value = QMI_UIM_FCI_VALUE_FCI;

  switch (qcril_fci_value)
  {
    case QCRIL_UIM_FCI_VALUE_FCP:
      qmi_fci_value = QMI_UIM_FCI_VALUE_FCP;
      break;
    case QCRIL_UIM_FCI_VALUE_FCI:
    case QCRIL_UIM_FCI_VALUE_FCI_FALLBACK_FCP:
      qmi_fci_value = QMI_UIM_FCI_VALUE_FCI;
      break;
    case QCRIL_UIM_FCI_VALUE_FMD:
      qmi_fci_value = QMI_UIM_FCI_VALUE_FMD;
      break;
    case QCRIL_UIM_FCI_VALUE_NO_DATA:
      qmi_fci_value = QMI_UIM_FCI_VALUE_NO_DATA;
      break;
    default:
      QCRIL_LOG_ERROR("Invalid FCI value: 0x%x, default to FCI", qcril_fci_value);
      return QMI_UIM_FCI_VALUE_FCI;
  }

  return qmi_fci_value;
} /* qcril_uim_convert_fci_value */

#endif /* defined (FEATURE_QCRIL_UIM_QMI) */
