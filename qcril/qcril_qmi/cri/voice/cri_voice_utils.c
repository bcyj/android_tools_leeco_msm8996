/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_voice_utils.h"
//#include "qcril_qmi_voice.h"
#include "cri_voice_temp_defs.h"
cri_core_error_type cri_voice_util_get_qmi_call_type_info
(
    cri_voice_call_type_type           cri_call_type,
    cri_voice_call_domain_type         cri_call_domain,
    int                                is_emergency,
    call_type_enum_v02                 *qmi_call_type,
    uint8_t                            *qmi_audio_attrib_valid,
    voice_call_attribute_type_mask_v02 *qmi_audio_attrib,
    uint8_t                            *qmi_video_attrib_valid,
    voice_call_attribute_type_mask_v02 *qmi_video_attrib
)
{
   cri_core_error_type result = TRUE;


       /* both of following fields are mandatory */
       if( qmi_call_type == NULL )
       {
          result = FALSE;
          return result;
       }

       switch( cri_call_type )
       {
       case CRI_VOICE_CALL_TYPE_VOICE:
          if( cri_call_domain == CRI_VOICE_CALL_DOMAIN_PS )
          {
             *qmi_call_type = CALL_TYPE_VOICE_IP_V02;
          }
          else
          {
             *qmi_call_type = CALL_TYPE_VOICE_V02;
          }

          if( qmi_audio_attrib_valid != NULL && qmi_audio_attrib != NULL )
          {
             *qmi_audio_attrib_valid = TRUE;
             *qmi_audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
          }

          if( qmi_video_attrib_valid != NULL && qmi_video_attrib != NULL )
          {
             *qmi_video_attrib_valid = TRUE;
             *qmi_video_attrib = 0;
          }
          break;

       case CRI_VOICE_CALL_TYPE_VT_RX:
          /* Video is receive only */
          if( cri_call_domain == CRI_VOICE_CALL_DOMAIN_PS || CRI_VOICE_CALL_DOMAIN_AUTOMATIC == cri_call_domain )
          {
             if( qmi_audio_attrib_valid == NULL || qmi_audio_attrib == NULL || qmi_video_attrib_valid == NULL || qmi_video_attrib == NULL )
             {
                result = FALSE;
             }
             else
             {
                *qmi_audio_attrib_valid = TRUE;
                *qmi_audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
                *qmi_video_attrib_valid = TRUE;
                *qmi_video_attrib = VOICE_CALL_ATTRIB_RX_V02;
                *qmi_call_type = CALL_TYPE_VT_V02;
             }
          }
          else
          {
             /* other domain values are not supported currently */
             result = FALSE;
          }
          break;

       case CRI_VOICE_CALL_TYPE_VT_TX:
          /* Video is transmit only */
          if( cri_call_domain == CRI_VOICE_CALL_DOMAIN_PS || CRI_VOICE_CALL_DOMAIN_AUTOMATIC == cri_call_domain )
          {
             if( qmi_audio_attrib_valid == NULL || qmi_audio_attrib == NULL || qmi_video_attrib_valid == NULL || qmi_video_attrib == NULL )
             {
                result = FALSE;
             }
             else
             {
                *qmi_audio_attrib_valid = TRUE;
                *qmi_audio_attrib = (VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02);
                *qmi_video_attrib_valid = TRUE;
                *qmi_video_attrib = VOICE_CALL_ATTRIB_TX_V02;
                *qmi_call_type = CALL_TYPE_VT_V02;
             }
          }
          else
          {
             /* other domain values are not supported currently */
             result = FALSE;
          }
          break;

       case CRI_VOICE_CALL_TYPE_VT:
          /* Video is transmit only */
          if( cri_call_domain == CRI_VOICE_CALL_DOMAIN_PS || CRI_VOICE_CALL_DOMAIN_AUTOMATIC == cri_call_domain )
          {
             if( qmi_audio_attrib_valid == NULL || qmi_audio_attrib == NULL || qmi_video_attrib_valid == NULL || qmi_video_attrib == NULL )
             {
                result = FALSE;
             }
             else
             {
                *qmi_audio_attrib_valid = TRUE;
                *qmi_audio_attrib = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
                *qmi_video_attrib_valid = TRUE;
                *qmi_video_attrib = VOICE_CALL_ATTRIB_TX_V02 | VOICE_CALL_ATTRIB_RX_V02;
                *qmi_call_type = CALL_TYPE_VT_V02;
             }
          }
          else
          {
             /* other domain values are not supported currently */
             result = FALSE;
          }
          break;

       default:
          result = FALSE;
          break;
       }

       QCRIL_LOG_DEBUG( "result = %d, modem call type = %d, ril call type = %d, ril call domain = %d",
                        result, *qmi_call_type, cri_call_type, cri_call_domain );

       if( ( qmi_audio_attrib != NULL ) && (qmi_audio_attrib_valid != NULL ) )
       {
          QCRIL_LOG_DEBUG(" qmi_audio_attrib_valid = %d, qmi_audio_attrib = %d", *qmi_audio_attrib_valid, *qmi_audio_attrib);
       }

       if( ( qmi_video_attrib != NULL ) && ( qmi_video_attrib_valid != NULL ) )
       {
          QCRIL_LOG_DEBUG(" qmi_video_attrib_valid = %d, qmi_video_attrib = %d", *qmi_video_attrib_valid, *qmi_video_attrib);
       }


   return result;
}

void cri_voice_util_free_call_list(cri_voice_call_list_type** call_list_dptr)
{
    if(call_list_dptr)
    {
        if(*call_list_dptr)
        {
            if ((*call_list_dptr)->calls_dptr)
            {
                util_memory_free((void**)&(*call_list_dptr)->calls_dptr);
            }
            util_memory_free((void**)call_list_dptr);
        }
        else
        {
            UTIL_LOG_MSG("*call_list_dptr is NULL");
        }
    }
    else
    {
        UTIL_LOG_MSG("call_list_dptr is NULL");
    }
}

uint32_t cri_voice_utils_call_num_copy_with_toa_check(
    char* dest,
    uint32_t dest_buffer_size,
    const char* src,
    uint32_t src_size,
    voice_num_type_enum_v02 num_type)
{
    uint32_t ret_size = 0;
    int offset = 0;

    if ( NULL == src || NULL == dest || src_size + 1 >= dest_buffer_size )
    {
        QCRIL_LOG_ERROR("function paramenter incorrect");
    }
    else
    {
        if ( QMI_VOICE_NUM_TYPE_INTERNATIONAL_V02 != num_type )
        {
            ret_size = src_size;
            memcpy(dest, src, src_size);
        }
        else
        {
            if ( CRI_VOICE_SS_TA_INTER_PREFIX == src[0] )
            {
                ret_size = src_size;
                memcpy(dest, src, src_size);
            }
            else
            {
                if (src_size > 1 && src[0] == '0' && src[1] == '0')
                {
                    QCRIL_LOG_INFO("Removing 00 prefix");
                    offset = 2;
                    src_size-=2;
                }
                ret_size = src_size + 1;
                dest[0] = CRI_VOICE_SS_TA_INTER_PREFIX;
                memcpy(dest+1, src + offset, src_size);
            }
        }
        dest[ret_size] = 0;
    }
//    QCRIL_LOG_FUNC_RETURN_WITH_RET(ret_size);
    return ret_size;
}
