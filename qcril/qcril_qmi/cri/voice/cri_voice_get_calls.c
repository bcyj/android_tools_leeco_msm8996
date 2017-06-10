/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_voice_get_calls.h"
#include "cri_voice_core.h"
#include "cri_voice_cache.h"

cri_core_error_type cri_voice_get_calls_request_get_current_all_calls(cri_voice_call_list_type** call_list_dptr)
{
    *call_list_dptr = util_memory_alloc(sizeof(cri_voice_call_list_type));

    uint32 num_of_calls;
    cri_voice_call_obj_type** call_obj_dptr;

    if(*call_list_dptr)
    {
        cri_voice_cache_type *call_info_ptr = cri_voice_core_get_call_info();
        util_list_info_type *call_list_ptr = cri_voice_cache_get_call_list(call_info_ptr);

        cri_voice_call_list_get_filtered_call_objects(call_list_ptr, cri_voice_call_obj_is_hlos_call, &num_of_calls, &call_obj_dptr);

        (*call_list_dptr)->num_of_calls = num_of_calls;
        QCRIL_LOG_INFO("num_of_calls: %d", (*call_list_dptr)->num_of_calls);

        if ((*call_list_dptr)->num_of_calls)
        {
            (*call_list_dptr)->calls_dptr = util_memory_alloc(sizeof(cri_voice_call_obj_type *) * (*call_list_dptr)->num_of_calls);
            if((*call_list_dptr)->calls_dptr)
            {
                uint32 i;
                if(call_obj_dptr)
                {
                    for (i=0; i<num_of_calls; i++)
                    {
                        (*call_list_dptr)->calls_dptr[i] = (call_obj_dptr[i]);
                    }
                }
            }
        }
    }

    if (call_obj_dptr)
    {
        util_memory_free((void**) &call_obj_dptr);
    }
    return 0;
}

cri_core_error_type cri_voice_get_calls_request_get_current_specific_calls(cri_voice_call_list_type** call_list_dptr, cri_voice_is_specific_call is_specific_call_checker)
{
    cri_voice_call_list_type *tmp_call_list_ptr = NULL;
    cri_core_error_type err = cri_voice_get_calls_request_get_current_all_calls(&tmp_call_list_ptr);
    if (!err && tmp_call_list_ptr)
    {
        *call_list_dptr = util_memory_alloc(sizeof(cri_voice_call_list_type));
        if(*call_list_dptr)
        {
            (*call_list_dptr)->num_of_calls = tmp_call_list_ptr->num_of_calls;
            if (is_specific_call_checker)
            {
                uint32 i;
                for (i=0; i<tmp_call_list_ptr->num_of_calls; i++)
                {
                    if (!is_specific_call_checker(tmp_call_list_ptr->calls_dptr[i]))
                    {
                        (*call_list_dptr)->num_of_calls--;
                    }
                }
            }

            if ((*call_list_dptr)->num_of_calls)
            {
                (*call_list_dptr)->calls_dptr = util_memory_alloc(sizeof(cri_voice_call_obj_type *) * (*call_list_dptr)->num_of_calls);
                if((*call_list_dptr)->calls_dptr)
                {
                    uint32 iter = 0;
                    uint32 i;
                    for (i=0; i<tmp_call_list_ptr->num_of_calls; i++)
                    {
                        if ( !is_specific_call_checker || is_specific_call_checker(tmp_call_list_ptr->calls_dptr[i]) )
                        {
                            (*call_list_dptr)->calls_dptr[iter] = (tmp_call_list_ptr->calls_dptr[i]);
                            iter++;
                        }
                    }
                }
            }
        }

        if (tmp_call_list_ptr)
        {
            cri_voice_free_call_list(&tmp_call_list_ptr);
        }
    }
    return err;
}

cri_core_error_type cri_voice_get_calls_request_get_current_specific_calls_with_param(
    cri_voice_call_list_type** call_list_dptr,
    cri_voice_is_specific_call_with_param is_specific_call_checker,
    const void* param
)
{
    cri_voice_call_list_type *tmp_call_list_ptr = NULL;
    cri_core_error_type err = cri_voice_get_calls_request_get_current_all_calls(&tmp_call_list_ptr);
    if (!err && tmp_call_list_ptr)
    {
        *call_list_dptr = util_memory_alloc(sizeof(cri_voice_call_list_type));
        if(*call_list_dptr)
        {
            (*call_list_dptr)->num_of_calls = tmp_call_list_ptr->num_of_calls;
            if (is_specific_call_checker)
            {
                uint32 i;
                for (i=0; i<tmp_call_list_ptr->num_of_calls; i++)
                {
                    if (!is_specific_call_checker(tmp_call_list_ptr->calls_dptr[i], param))
                    {
                        (*call_list_dptr)->num_of_calls--;
                    }
                }
            }

            if ((*call_list_dptr)->num_of_calls)
            {
                (*call_list_dptr)->calls_dptr = util_memory_alloc(sizeof(cri_voice_call_obj_type *) * (*call_list_dptr)->num_of_calls);
                if((*call_list_dptr)->calls_dptr)
                {
                    uint32 iter = 0;
                    uint32 i;
                    for (i=0; i<tmp_call_list_ptr->num_of_calls; i++)
                    {
                        if ( !is_specific_call_checker || is_specific_call_checker(tmp_call_list_ptr->calls_dptr[i], param) )
                        {
                            (*call_list_dptr)->calls_dptr[iter] = (tmp_call_list_ptr->calls_dptr[i]);
                            iter++;
                        }
                    }
                }
            }

        }
        if (tmp_call_list_ptr)
        {
            cri_voice_free_call_list(&tmp_call_list_ptr);
        }
    }
    return err;
}
