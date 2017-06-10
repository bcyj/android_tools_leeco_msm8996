/*===========================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved
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

$Header:  $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/25/13   tl      Initial version

===========================================================================*/

/*===========================================================================

                            INCLUDE FILES

===========================================================================*/

#include "cri_uim_core.h"
#include "cri_uim_utils.h"
#include "cri_core.h"

/*===========================================================================

                            FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  cri_uim_util_has_card_status_changed

===========================================================================*/
/*
    @brief
    Function determines if the status of the card has changed

    @return
    int
      TRUE   - changed card status
      FALSE  - cannot change card status
*/
/*=========================================================================*/
int cri_uim_util_has_card_status_changed
(
  const uim_card_status_type_v01        * cache_card_status_ptr,
  const uim_card_status_type_v01        * new_card_status_ptr,
  unsigned short                        * slot_index_ptr
)
{
  unsigned short i               = 0;
  unsigned short j               = 0;
  unsigned short card_info_len   = 0;

  if (cache_card_status_ptr == NULL ||
      new_card_status_ptr == NULL ||
      slot_index_ptr == NULL)
  {
    UTIL_LOG_MSG("cri_uim_util_has_card_status_changed: invalid input\n");
    return FALSE;
  }

  if (cache_card_status_ptr->index_gw_pri != new_card_status_ptr->index_gw_pri)
  {
    UTIL_LOG_MSG("GW primary index changed: %d -> %d\n",
                 cache_card_status_ptr->index_gw_pri, new_card_status_ptr->index_gw_pri);
    if ((new_card_status_ptr->index_gw_pri >> 8) != 0xFF)
    {
      *slot_index_ptr = (new_card_status_ptr->index_gw_pri >> 8);
    }
    else
    {
      *slot_index_ptr = (cache_card_status_ptr->index_gw_pri >> 8);
    }
    return TRUE;
  }

  if (cache_card_status_ptr->index_1x_pri != new_card_status_ptr->index_1x_pri)
  {
    UTIL_LOG_MSG("1x primary index changed: %d -> %d\n",
                 cache_card_status_ptr->index_1x_pri, new_card_status_ptr->index_1x_pri);
    if ((new_card_status_ptr->index_1x_pri >> 8) != 0xFF)
    {
      *slot_index_ptr = (new_card_status_ptr->index_1x_pri >> 8);
    }
    else
    {
      *slot_index_ptr = (cache_card_status_ptr->index_1x_pri >> 8);
    }
    return TRUE;
  }

  if (cache_card_status_ptr->index_gw_sec != new_card_status_ptr->index_gw_sec)
  {
    UTIL_LOG_MSG("GW secondary index changed: %d -> %d\n",
                 cache_card_status_ptr->index_gw_sec, new_card_status_ptr->index_gw_sec);
    if ((new_card_status_ptr->index_gw_sec >> 8) != 0xFF)
    {
      *slot_index_ptr = (new_card_status_ptr->index_gw_sec >> 8);
    }
    else
    {
      *slot_index_ptr = (cache_card_status_ptr->index_gw_sec >> 8);
    }
    return TRUE;
  }

  if (cache_card_status_ptr->index_1x_sec != new_card_status_ptr->index_1x_sec)
  {
    UTIL_LOG_MSG("1x secondary index changed: %d -> %d\n",
                 cache_card_status_ptr->index_1x_sec, new_card_status_ptr->index_1x_sec);
    if ((new_card_status_ptr->index_1x_sec >> 8) != 0xFF)
    {
      *slot_index_ptr = (new_card_status_ptr->index_1x_sec >> 8);
    }
    else
    {
      *slot_index_ptr = (cache_card_status_ptr->index_1x_sec >> 8);
    }
    return TRUE;
  }

  if (cache_card_status_ptr->card_info_len >=  QMI_UIM_CARDS_MAX_V01 ||
      new_card_status_ptr->card_info_len >= QMI_UIM_CARDS_MAX_V01)
  {
    UTIL_LOG_MSG("Invalid card info length\n");
    return FALSE;
  }

  for (i = 0; i < QMI_UIM_CARDS_MAX_V01; i++)
  {
    if (cache_card_status_ptr->card_info[i].card_state != new_card_status_ptr->card_info[i].card_state)
    {
      UTIL_LOG_MSG("Card state changed: %d -> %d\n",
                   cache_card_status_ptr->card_info[i].card_state,
                   new_card_status_ptr->card_info[i].card_state);
      *slot_index_ptr = i;
      return TRUE;
    }

    if ((cache_card_status_ptr->card_info[i].card_state == UIM_CARD_STATE_ERROR_V01) &&
        (cache_card_status_ptr->card_info[i].error_code != new_card_status_ptr->card_info[i].error_code))
    {
      UTIL_LOG_MSG("Card error changed: %d -> %d\n",
                   cache_card_status_ptr->card_info[i].error_code,
                   new_card_status_ptr->card_info[i].error_code);
      *slot_index_ptr = i;
      return TRUE;
    }

    if (cache_card_status_ptr->card_info[i].app_info_len != new_card_status_ptr->card_info[i].app_info_len ||
        new_card_status_ptr->card_info[i].app_info_len >= QMI_UIM_APPS_MAX_V01)
    {
      UTIL_LOG_MSG("Application information length changed: %d -> %d\n",
                   cache_card_status_ptr->card_info[i].app_info_len,
                   new_card_status_ptr->card_info[i].app_info_len);
      *slot_index_ptr = i;
      return TRUE;
    }

    for (j = 0; j < new_card_status_ptr->card_info[i].app_info_len; j++)
    {
      if (cache_card_status_ptr->card_info[i].app_info[j].app_type !=
          new_card_status_ptr->card_info[i].app_info[j].app_type)
      {
        UTIL_LOG_MSG("Application type changed: %d -> %d\n",
                     cache_card_status_ptr->card_info[i].app_info[j].app_type,
                     new_card_status_ptr->card_info[i].app_info[j].app_type);
        *slot_index_ptr = i;
        return TRUE;
      }

      if (cache_card_status_ptr->card_info[i].app_info[j].app_state !=
          new_card_status_ptr->card_info[i].app_info[j].app_state)
      {
        UTIL_LOG_MSG("Application state changed: %d -> %d\n",
                     cache_card_status_ptr->card_info[i].app_info[j].app_state,
                     new_card_status_ptr->card_info[i].app_info[j].app_state);
        *slot_index_ptr = i;
        return TRUE;
      }

      if (cache_card_status_ptr->card_info[i].app_info[j].perso_state !=
          new_card_status_ptr->card_info[i].app_info[j].perso_state)
      {
        UTIL_LOG_MSG("Application personalization changed: %d -> %d\n",
                     cache_card_status_ptr->card_info[i].app_info[j].perso_state,
                     new_card_status_ptr->card_info[i].app_info[j].perso_state);
        *slot_index_ptr = i;
        return TRUE;
      }

      if (cache_card_status_ptr->card_info[i].app_info[j].perso_feature !=
          new_card_status_ptr->card_info[i].app_info[j].perso_feature)
      {
        UTIL_LOG_MSG("Application personalization feature changed: %d -> %d\n",
                     cache_card_status_ptr->card_info[i].app_info[j].perso_feature,
                     new_card_status_ptr->card_info[i].app_info[j].perso_feature);
        *slot_index_ptr = i;
        return TRUE;
      }

      if (cache_card_status_ptr->card_info[i].app_info[j].perso_retries !=
          new_card_status_ptr->card_info[i].app_info[j].perso_retries)
      {
        UTIL_LOG_MSG("Application personalization retries changed: %d -> %d\n",
                     cache_card_status_ptr->card_info[i].app_info[j].perso_retries,
                     new_card_status_ptr->card_info[i].app_info[j].perso_retries);
        *slot_index_ptr = i;
        return TRUE;
      }

      if (cache_card_status_ptr->card_info[i].app_info[j].perso_unblock_retries !=
          new_card_status_ptr->card_info[i].app_info[j].perso_unblock_retries)
      {
        UTIL_LOG_MSG("Application personalization PUK retries changed: %d -> %d\n",
                     cache_card_status_ptr->card_info[i].app_info[j].perso_unblock_retries,
                     new_card_status_ptr->card_info[i].app_info[j].perso_unblock_retries);
        *slot_index_ptr = i;
        return TRUE;
      }

      if (cache_card_status_ptr->card_info[i].app_info[j].aid_value_len !=
          new_card_status_ptr->card_info[i].app_info[j].aid_value_len ||
          new_card_status_ptr->card_info[i].app_info[j].aid_value_len >=
          QMI_UIM_AID_MAX_V01)
      {
        UTIL_LOG_MSG("Application ID len changed: %d -> %d\n",
                     cache_card_status_ptr->card_info[i].app_info[j].aid_value_len,
                     new_card_status_ptr->card_info[i].app_info[j].aid_value_len);
        *slot_index_ptr = i;
        return TRUE;
      }

      if (memcmp(cache_card_status_ptr->card_info[i].app_info[j].aid_value,
                 new_card_status_ptr->card_info[i].app_info[j].aid_value,
                 new_card_status_ptr->card_info[i].app_info[j].aid_value_len) != 0)
      {
        UTIL_LOG_MSG("Application ID values changed\n");
        *slot_index_ptr = i;
        return TRUE;
      }

      if (cache_card_status_ptr->card_info[i].app_info[j].pin1.pin_state !=
          new_card_status_ptr->card_info[i].app_info[j].pin1.pin_state)
      {
        UTIL_LOG_MSG("PIN 1 state changed: %d -> %d\n",
                     cache_card_status_ptr->card_info[i].app_info[j].pin1.pin_state,
                     new_card_status_ptr->card_info[i].app_info[j].pin1.pin_state);
        *slot_index_ptr = i;
        return TRUE;
      }

      if (cache_card_status_ptr->card_info[i].app_info[j].pin1.pin_retries !=
          new_card_status_ptr->card_info[i].app_info[j].pin1.pin_retries)
      {
        UTIL_LOG_MSG("PIN 1 retries changed: %d -> %d\n",
                     cache_card_status_ptr->card_info[i].app_info[j].pin1.pin_retries,
                     new_card_status_ptr->card_info[i].app_info[j].pin1.pin_retries);
        *slot_index_ptr = i;
        return TRUE;
      }

      if (cache_card_status_ptr->card_info[i].app_info[j].pin1.puk_retries !=
          new_card_status_ptr->card_info[i].app_info[j].pin1.puk_retries)
      {
        UTIL_LOG_MSG("PIN 1 PUK changed: %d -> %d\n",
                     cache_card_status_ptr->card_info[i].app_info[j].pin1.puk_retries,
                     new_card_status_ptr->card_info[i].app_info[j].pin1.puk_retries);
        *slot_index_ptr = i;
        return TRUE;
      }

      if (cache_card_status_ptr->card_info[i].app_info[j].pin2.pin_state !=
          new_card_status_ptr->card_info[i].app_info[j].pin2.pin_state)
      {
        UTIL_LOG_MSG("PIN 2 state changed: %d -> %d\n",
                     cache_card_status_ptr->card_info[i].app_info[j].pin2.pin_state,
                     new_card_status_ptr->card_info[i].app_info[j].pin2.pin_state);
        *slot_index_ptr = i;
        return TRUE;
      }

      if (cache_card_status_ptr->card_info[i].app_info[j].pin2.pin_retries !=
          new_card_status_ptr->card_info[i].app_info[j].pin2.pin_retries)
      {
        UTIL_LOG_MSG("PIN 2 retries changed: %d -> %d\n",
                     cache_card_status_ptr->card_info[i].app_info[j].pin2.pin_retries,
                     new_card_status_ptr->card_info[i].app_info[j].pin2.pin_retries);
        *slot_index_ptr = i;
        return TRUE;
      }

      if (cache_card_status_ptr->card_info[i].app_info[j].pin2.puk_retries !=
          new_card_status_ptr->card_info[i].app_info[j].pin2.puk_retries)
      {
        UTIL_LOG_MSG("PIN 2 PUK changed: %d -> %d\n",
                     cache_card_status_ptr->card_info[i].app_info[j].pin2.puk_retries,
                     new_card_status_ptr->card_info[i].app_info[j].pin2.puk_retries);
        *slot_index_ptr = i;
        return TRUE;
      }
    }
  }

  UTIL_LOG_MSG("No card information change\n");

  return FALSE;
} /* cri_uim_util_has_card_status_changed */


/*===========================================================================

  FUNCTION:  cri_uim_util_retrieve_card_status_global

===========================================================================*/
/*
    @brief
    Returns pointer to the cached card status.

    @return
    uim_card_status_type_v01*
*/
/*=========================================================================*/
uim_card_status_type_v01* cri_uim_util_retrieve_card_status_global
(
  void
)
{
  return &uim_card_status;
} /* cri_uim_util_retrieve_card_status_global */


/*===========================================================================

  FUNCTION:  cri_uim_util_retrieve_client_id

===========================================================================*/
/*
    @brief
    Returns the cached client id.

    @return
    int
*/
/*=========================================================================*/
int cri_uim_util_retrieve_client_id
(
  void
)
{
  return uim_client_id;
} /* cri_uim_util_retrieve_client_id */

