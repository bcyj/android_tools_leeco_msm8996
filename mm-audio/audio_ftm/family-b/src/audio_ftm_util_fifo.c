#ifdef __cplusplus
extern "C" {
#endif

/**

  @file   audio_ftm_util_fifo.c
  @brief  Utility: FIFO buffer
====================================================================================================
Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/src/audio_ftm_util_fifo.c#1 $
$DateTime: 2011/04/05 20:05:46 $
$Author: zhongl $

Revision History:
                            Modification     Tracking
Author (core ID)                Date         CR Number   Description of Changes
-------------------------   ------------    ----------   -------------------------------------------
ZhongL                      05/30/2010                    File creation.


====================================================================================================
                                        INCLUDE FILES
==================================================================================================*/

#include "audio_ftm_util_fifo.h"

/*==================================================================================================
                                     LOCAL CONSTANTS
==================================================================================================*/


/*==================================================================================================
                          LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
==================================================================================================*/



/*==================================================================================================
                                        LOCAL MACROS
==================================================================================================*/



/*==================================================================================================
                                      LOCAL VARIABLES
==================================================================================================*/


/*==================================================================================================
                                     GLOBAL VARIABLES
==================================================================================================*/


/*==================================================================================================
                                 LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
                                     LOCAL FUNCTIONS
==================================================================================================*/


/*==================================================================================================
                                       GLOBAL FUNCTIONS
==================================================================================================*/

int32 cbuf_data_size (AUD_FTM_CIRCULAR_BUF_T *rb)
{
   int32 len;
   if(rb->wr_pointer >= rb->rd_pointer)
   {
     len=(int32)(rb->wr_pointer - rb->rd_pointer);
   }
   else
   {
      len=(int32)(rb->wr_pointer + rb->size - rb->rd_pointer);
   }
   return len;
}

int32 cbuf_capacity(AUD_FTM_CIRCULAR_BUF_T *rb)
{
  return rb->size;
}


DALBOOL aud_ftm_cbuf_init(AUD_FTM_CIRCULAR_BUF_T *cbuf, uint16 sz)
{
    if(DAL_SUCCESS != DALSYS_Malloc(sz, (void **)&(cbuf->buffer)))
    {
      if(cbuf->buffer != NULL)     DALSYS_Free(cbuf->buffer);
       return FALSE;
    }

    if(cbuf->buffer == NULL)
        return FALSE;

    DALSYS_memset (cbuf->buffer, 0, sz);
    cbuf->size = sz;
    cbuf->rd_pointer = 0;
    cbuf->wr_pointer = 0;
       if(DAL_SUCCESS != DALSYS_SyncCreate(DALSYS_SYNC_ATTR_RESOURCE, &(cbuf->hSync),    NULL))
      {
          return FALSE;
      }
      return TRUE;
}


int32
aud_ftm_cbuf_write (AUD_FTM_CIRCULAR_BUF_T *cbuf, uint8 * buf, int32 len)
{
    int32 total;
    int32 i;

    DALSYS_SyncEnter(cbuf->hSync);
    total = cbuf->size - 1 - cbuf_data_size(cbuf);
    if(len > total)
        len = total;
    else
        total = len;

    i = cbuf->wr_pointer;
    if((i + len) > cbuf->size)
    {
        DALSYS_memcpy((uint8 *)(cbuf->buffer + i), (uint8 *)buf, cbuf->size - i);
        buf += cbuf->size - i;
        len -= cbuf->size - i;
        i = 0;
    }
    DALSYS_memcpy((uint8 *)(cbuf->buffer + i), (uint8 *)buf, len);
    cbuf->wr_pointer = i + len;
      DALSYS_SyncLeave(cbuf->hSync);
    return total;
}


int32
aud_ftm_cbuf_read (AUD_FTM_CIRCULAR_BUF_T *cbuf, uint8 * buf, int max)
{
    int32 total;
    int32 i;

    DALSYS_SyncEnter(cbuf->hSync);

    total = cbuf_data_size(cbuf);

    if(max > total)
        max = total;
    else
        total = max;

    i = cbuf->rd_pointer;
    if(i + max > cbuf->size)
    {
        DALSYS_memcpy((uint8 *)buf,(uint8 *)(cbuf->buffer + i), cbuf->size - i);
        buf += cbuf->size - i;
        max -= cbuf->size - i;
        i = 0;
    }
    DALSYS_memcpy((uint8 *)buf,(uint8 *)(cbuf->buffer + i), max);
    cbuf->rd_pointer = i + max;

      DALSYS_SyncLeave(cbuf->hSync);

    return total;

}

DALBOOL aud_ftm_cbuf_deinit(AUD_FTM_CIRCULAR_BUF_T *cbuf)
{
    DALSYS_Free(cbuf->buffer);
    cbuf->buffer=NULL;

      if(DAL_SUCCESS != DALSYS_DestroySyncObject(cbuf->hSync))
      {
          return FALSE;
      }
    return TRUE;
}

void
aud_ftm_cbuf_clear (AUD_FTM_CIRCULAR_BUF_T *cbuf)
{
    DALSYS_memset (cbuf->buffer, 0, cbuf->size);
    cbuf->rd_pointer = 0;
    cbuf->wr_pointer = 0;
}


#ifdef __cplusplus
}
#endif
