#ifdef __cplusplus
extern "C" {
#endif

/**

  @file   DALSYS_common.c
  @brief  DALSYS OS dependent functions mapping to Linux
====================================================================================================
Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/src/DALSYS_common.c#1 $
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

#include "DALSYS_common.h"

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

/*=============================================================================
  @brief use malloc as memory allocation function

  @param path:

  @return return:
===============================================================================*/


DALResult
DALSYS_Malloc(uint32 dwSize, void **ppMem)
{
    *ppMem = malloc(dwSize);
    if (NULL != *ppMem)
    {
        return DAL_SUCCESS;
    }
    return DAL_ERROR;
}

/*=============================================================================
  @brief use free as memory free function

  @param path:

  @return return:
===============================================================================*/

DALResult
DALSYS_Free(void *pMem)
{
    free(pMem);
    return DAL_SUCCESS;
}

/*=============================================================================
  @brief use Metux as sync object

  @param path:

  @return return:
===============================================================================*/

DALResult
DALSYS_SyncCreate(uint32 dwAttribs,
                  DALSYSSyncHandle *phSync,
                  DALSYSSyncObj *pObj)
{
   pthread_mutex_t *hMutex;

   *phSync=NULL;
   if(DAL_SUCCESS != DALSYS_Malloc(sizeof(pthread_mutex_t), (void**)&hMutex))
   {
      DALSYS_Log_Err("failed to alloc memroy\n");
      return DAL_ERROR;
   }

   pthread_mutex_init(hMutex, NULL);

   *phSync=(DALSYSSyncHandle)hMutex;
   return DAL_SUCCESS;
}

/*=============================================================================
  @brief enter lock

  @param path:

  @return return:
===============================================================================*/

void
DALSYS_SyncEnter(DALSYSSyncHandle hSync)
{
   if( hSync == NULL)
       DALSYS_Log_Err("Null Mutex pointer\n");
   else
      pthread_mutex_lock((pthread_mutex_t *) hSync);
}

/*=============================================================================
  @brief exit lock

  @param path:

  @return return:
===============================================================================*/

void
DALSYS_SyncLeave(DALSYSSyncHandle hSync)
{
   if( hSync == NULL)
       DALSYS_Log_Err("Null Mutex pointer\n");
   else
      pthread_mutex_unlock((pthread_mutex_t *) hSync);
}

/*=============================================================================
  @brief release Mutex

  @param path:

  @return return:
===============================================================================*/

DALResult
DALSYS_DestroySyncObject(DALSYSSyncHandle hSync)
{
   if( hSync == NULL)
   {
       DALSYS_Log_Err("Null Mutex pointer\n");
       return DAL_ERROR;
   }
   else
      DALSYS_Free((void *)hSync);
      return DAL_SUCCESS;
}

/*=============================================================================
  @brief Use Semaphore to simulate event

  @param path:

  @return return:
===============================================================================*/

DALResult
DALSYS_EventCreate(uint32 dwEvtAttrib,  DALSYSEventHandle *phEvent,
                                   DALSYSEventObj *pObj)
{
     sem_t *hSem;

   *phEvent=NULL;
   if(DAL_SUCCESS != DALSYS_Malloc(sizeof(sem_t), (void**)&hSem))
   {
      DALSYS_Log_Err("failed to alloc memroy\n");
      return DAL_ERROR;
   }

   sem_init(hSem,0,0);   /* internal process semphore with value 0 */

   *phEvent=(DALSYSEventHandle)hSem;
   return DAL_SUCCESS;
}

/*=============================================================================
  @brief Trigger the event ( indicate completion to release the waiting )

  @param path:

  @return return:
===============================================================================*/

DALResult
DALSYS_EventCtrl(DALSYSEventHandle hEvent, uint32 dwCtrl,uint32 dwParam,
                                  void *pPayload, uint32 dwPayloadSize)
{
   if( hEvent == NULL)
   {
       DALSYS_Log_Err("Null Completion Event pointer\n");
       return DAL_ERROR;
   }
   else
      sem_post((sem_t *)hEvent);
   return DAL_SUCCESS;
}

/*=============================================================================
  @brief Wait for event coming

  @param path:

  @return return:
===============================================================================*/

DALResult
DALSYS_EventWait(DALSYSEventHandle hEvent)
{
   if( hEvent == NULL)
   {
       DALSYS_Log_Err("Null Completion Event pointer\n");
       return DAL_ERROR;
   }
   else
      sem_wait((sem_t *)hEvent);
   return DAL_SUCCESS;

}

/*=============================================================================
  @brief Wait for event coming with timeout enabled

  @param path:  timeout in ms

  @return return:
===============================================================================*/

DALResult
DALSYS_EventWaitTimeout(DALSYSEventHandle hEvent, uint32 timeout_ms)
{
   struct timespec ts;
   if( hEvent == NULL)
   {
       DALSYS_Log_Err("Null Completion Event pointer\n");
       return DAL_ERROR;
   }
   else
   {

     time_t sec=(int)(timeout_ms/1000);

     timeout_ms=timeout_ms-(sec*1000);

     if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        return DAL_ERROR;
         ts.tv_sec  += sec;
         ts.tv_nsec += timeout_ms*1000000L;   /* convert to ms */
     sem_timedwait((sem_t *)hEvent, &ts);
   }
   return DAL_SUCCESS;
}

/*=============================================================================
  @brief Release the Event resource

  @param path:

  @return return:
===============================================================================*/

DALResult
DALSYS_DestroyEventObject(DALSYSEventHandle hEvent)
{
   if( hEvent == NULL)
   {
       DALSYS_Log_Err("Null Completion Event pointer\n");
       return DAL_ERROR;
   }
   else
         sem_destroy((sem_t *)hEvent);
      DALSYS_Free((void *)hEvent);
      return DAL_SUCCESS;
}

/*=============================================================================
  @brief Delays (sleeps) in milliseconds

  @param path:

  @return return:
===============================================================================*/

void DALSYS_Delay(uint32 delay_ms)
{
    struct timespec req={0};

    time_t sec=(int)(delay_ms/1000);

    delay_ms=delay_ms-(sec*1000);

    req.tv_sec=sec;

    req.tv_nsec=delay_ms*1000000L;

    while(nanosleep(&req,&req)==-1)

         continue;

    return 1;
}


#ifdef __cplusplus
}
#endif
