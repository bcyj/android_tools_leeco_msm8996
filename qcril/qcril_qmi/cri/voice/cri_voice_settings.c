/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <time.h>
#include "cri_voice_settings.h"

void cri_voice_settings_init(cri_voice_settings_type *settings_ptr)
{
    if (settings_ptr)
    {
        settings_ptr->wait_1x_num = TRUE;
        settings_ptr->wait_1x_num_timeout.tv_sec = 1;

        settings_ptr->support_subaddr = FALSE;

        settings_ptr->hlos_user_data_deleter = NULL;
    }
}

void cri_voice_settings_set_wait_1x_num(cri_voice_settings_type *settings_ptr, boolean wait)
{
    if (settings_ptr)
    {
        settings_ptr->wait_1x_num = wait;
    }
}
boolean cri_voice_settings_get_wait_1x_num(const cri_voice_settings_type *settings_ptr)
{
    if (settings_ptr)
    {
        return settings_ptr->wait_1x_num;
    }
    else
    {
        return 0;
    }
}

void cri_voice_settings_set_wait_1x_num_timeout(cri_voice_settings_type *settings_ptr, uint32 sec, uint32 usec)
{
    if (settings_ptr)
    {
        settings_ptr->wait_1x_num_timeout.tv_sec = sec;
        settings_ptr->wait_1x_num_timeout.tv_usec = usec;
    }
}

const struct timeval* cri_voice_settings_get_wait_1x_num_timeout(const cri_voice_settings_type *settings_ptr)
{
    if (settings_ptr)
    {
        return &settings_ptr->wait_1x_num_timeout;
    }
    else
    {
        return NULL;
    }
}


void cri_voice_settings_set_subaddr_support(cri_voice_settings_type *settings_ptr, boolean support)
{
    if (settings_ptr)
    {
        settings_ptr->support_subaddr = support;
    }
}

boolean cri_voice_settings_get_subaddr_support(const cri_voice_settings_type *settings_ptr)
{
    if (settings_ptr)
    {
        return settings_ptr->support_subaddr;
    }
    else
    {
        return 0;
    }
}

void cri_voice_settings_set_hlos_user_data_deleter(cri_voice_settings_type *settings_ptr, hlos_user_data_deleter_type deleter)
{
    if (settings_ptr)
    {
        settings_ptr->hlos_user_data_deleter = deleter;
    }
}

hlos_user_data_deleter_type cri_voice_settings_get_default_hlos_user_data_deleter(const cri_voice_settings_type *settings_ptr)
{
    if (settings_ptr)
    {
        return settings_ptr->hlos_user_data_deleter;
    }
    else
    {
        return NULL;
    }
}


