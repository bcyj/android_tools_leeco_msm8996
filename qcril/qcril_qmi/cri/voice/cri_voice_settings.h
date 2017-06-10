/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_VOICE_SETTINGS
#define CRI_VOICE_SETTINGS

#include "comdef.h"
#include "cri_voice.h"

typedef struct
{
    boolean wait_1x_num;
    struct timeval wait_1x_num_timeout;

    boolean support_subaddr;

    hlos_user_data_deleter_type hlos_user_data_deleter;
} cri_voice_settings_type;


void cri_voice_settings_init(cri_voice_settings_type *settings_ptr);

void    cri_voice_settings_set_wait_1x_num(cri_voice_settings_type *settings_ptr, boolean wait_1x_num);
boolean cri_voice_settings_get_wait_1x_num(const cri_voice_settings_type *settings_ptr);
void            cri_voice_settings_set_wait_1x_num_timeout(cri_voice_settings_type *settings_ptr, uint32 sec, uint32 usec);
const struct timeval* cri_voice_settings_get_wait_1x_num_timeout(const cri_voice_settings_type *settings_ptr);

void    cri_voice_settings_set_subaddr_support(cri_voice_settings_type *settings_ptr, boolean support);
boolean cri_voice_settings_get_subaddr_support(const cri_voice_settings_type *settings_ptr);

void cri_voice_settings_set_hlos_user_data_deleter(cri_voice_settings_type *settings_ptr, hlos_user_data_deleter_type deleter);
hlos_user_data_deleter_type cri_voice_settings_get_default_hlos_user_data_deleter(const cri_voice_settings_type *settings_ptr);

#endif
