/**
  \file **************************************************************************
 *
 *  A U D I O   C A L I B R A T I O N   P A C K E T   H A N D L E R
 *
 *DESCRIPTION
 * This file contains the implementation of ACPH
 *
 *REFERENCES
 * None.
 *
 * Copyright (c) 2010-2014 by Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *******************************************************************************
 */
/**
  \file ***************************************************************************
 *
 *                      EDIT HISTORY FOR FILE
 *
 *  This section contains comments describing changes made to this file.
 *  Notice that changes are listed in reverse chronological order.
 *
 *  $Header: acph.h
 *
 *when         who     what, where, why
 *--------     ---     ----------------------------------------------------------
 *05/28/14     mh      SW migration from 32-bit to 64-bit architecture
 *05/28/10     ayin    initial draft
 ********************************************************************************
 */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal4/acdb_sw/main/latest/acph/src/acph.c#6 $ */
/*
   -------------------------------
   |Include Files                |
   -------------------------------
   */
#include "acdb_os_includes.h"
#include "acph.h"
#include "acph_online.h"

/*------------------------------------------
 ** flag, page size, and buffer length definition
 *-------------------------------------------*/
#define ACPH_SUC_FLAG_TRUE          0x1
#define ACPH_SUC_FLAG_FALSE         0x0
#define ACPH_BUFFER_LENGTH          0x2500
#define ACPH_HEADER_LENGTH          6
#define ACPH_COMMAND_ID_POSITION    0
#define ACPH_COMMAND_ID_LENGTH      2
#define ACPH_DATA_LENGTH_POSITION   2
#define ACPH_DATA_LENGTH_LENGTH     4
#define ACPH_ERROR_FRAME_LENGTH     11
#define ACPH_ERROR_FLAG_LENGTH      1
#define ACPH_SUC_FLAG_LENGTH        1
#define ACPH_ERROR_CODE_LENGTH      4
#define ACPH_CAL_DATA_UNIT_LENGTH   4
#define ACPH_ACDB_BUFFER_POSITION   7

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

typedef struct AcphFcnLinklistStruct{
   uint32_t serviceId;
   ACPH_CALLBACK_PTR fcnPtr;
   struct AcphFcnLinklistStruct *pNext;
}AcphFcnLinklistType;

typedef struct AcphFcnNodeStruct{
   AcphFcnLinklistType *pNodeHead;
   AcphFcnLinklistType *pNodeTail;
}AcphFcnNodeType;

/**
 * FUNCTION : acph_execute_command
 *
 * DESCRIPTION : Interpret request and operate ACDB correspondingly, response with
 * the result.
 *
 * DEPENDENCIES : ACDB needs to be initialized before this function is called
 *
 * PARAMS:
 *   req_buf_ptr - the ACPH request command buffer
 *   req_buf_length - the lenght of request command buffer
 *   resp_buf_ptr - pointer of pointer to the response buffer
 *   resp_buf_length - pointer to the length of response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
void acph_execute_command(uint8_t *req_buf_ptr,
                          uint32_t req_buf_length,
                          uint8_t **resp_buf_ptr,
                          uint32_t *resp_buf_length
                          );

/*
   -------------------------------
   |Global variables              |
   -------------------------------
   */
uint8_t * acph_main_buffer = NULL;

/*
   -------------------------------
   |external function            |
   -------------------------------
   */
extern void actp_diag_init(
        void (*callback_function)(uint8_t*, uint32_t, uint8_t**, uint32_t*)
        );

/*
   ---------------------------------
   |Static Variable Definitions    |
   ---------------------------------
   */
static AcphFcnNodeType *g_pCmdTbl = NULL;
static uint32_t acph_init_count = 0;

/**
 * FUNCTION : get_command_length
 *
 * DESCRIPTION : Get request data length from a request buffer
 *
 * DEPENDENCIES : the address of a uint32_t variable need to be passed in to
 * contain the data length
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   req_buf_length - length of the request buffer
 *   data_length - pointer to the length of data
 *
 * RETURN VALUE : return false if the there is no data length information
 * or the data length read from the request buffer is not consistent with
 * the total buffer length.
 * return true otherwise and data length is returned in the last parameter.
 *
 * SIDE EFFECTS : None
 */
static bool_t get_command_length(uint8_t *req_buf_ptr,
                                 uint32_t req_buf_length,
                                 uint32_t *data_length
                                 )
{
   bool_t bResult = FALSE;
   if (req_buf_length < ACPH_HEADER_LENGTH)
   {
      /**there is no data length information*/
      bResult = FALSE;
   }
   else
   {
      /** copy data length*/
      ACDB_MEM_CPY(data_length,ACPH_DATA_LENGTH_LENGTH,
             req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
             ACPH_DATA_LENGTH_LENGTH);
      if (req_buf_length - ACPH_HEADER_LENGTH == *data_length)
      {
         bResult = TRUE;
      }
      else
      {
         /**the data length read from the request buffer is not consistent
         with the total buffer length.*/
         bResult = FALSE;
      }
   }
   return bResult;
}

/*
   ----------------------------------
   | Externalized Function Definitions    |
   ----------------------------------
   */
/**
 * FUNCTION : acph_create_error_resp
 *
 * DESCRIPTION : Create a response error buffer with the specified error code
 * and request command id.
 *
 * DEPENDENCIES : None
 *
 * PARAMS:
 *   error_code - response error code
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
static void acph_create_error_resp(uint32_t error_code,
                            uint8_t *req_buf_ptr,
                            uint8_t *resp_buf_ptr,
                            uint32_t *resp_buf_length
                            )
{
   uint8_t suc_flag = ACPH_SUC_FLAG_FALSE;
   uint32_t resp_data_length;
   if (NULL == (resp_buf_ptr))
   {
      /**not initialized*/
      ACDB_DEBUG_LOG("resp buffer ptr address[%p]\n",resp_buf_ptr);
      *resp_buf_length = 0;
      return;
   }
   *resp_buf_length = ACPH_ERROR_FRAME_LENGTH;
   resp_data_length = ACPH_ERROR_FRAME_LENGTH - ACPH_HEADER_LENGTH;
   /**copy command id */
   ACDB_MEM_CPY(resp_buf_ptr,ACPH_COMMAND_ID_LENGTH, req_buf_ptr, ACPH_COMMAND_ID_LENGTH);
   /**copy data length */
   ACDB_MEM_CPY(resp_buf_ptr + ACPH_COMMAND_ID_LENGTH,ACPH_DATA_LENGTH_LENGTH,
          &resp_data_length,
          ACPH_DATA_LENGTH_LENGTH);
   /**copy suc flag */
   ACDB_MEM_CPY(resp_buf_ptr + ACPH_HEADER_LENGTH,ACPH_ERROR_FLAG_LENGTH,
          &suc_flag,
          ACPH_ERROR_FLAG_LENGTH);
   /**copy error code */
   ACDB_MEM_CPY(resp_buf_ptr + ACPH_HEADER_LENGTH + ACPH_ERROR_FLAG_LENGTH,ACPH_ERROR_CODE_LENGTH,
          &error_code,
          ACPH_ERROR_CODE_LENGTH);
}

/**
 * FUNCTION : acph_create_suc_resp
 *
 * DESCRIPTION : Create a response buffer with the specified response data
 * and request command id.
 *
 * DEPENDENCIES : None
 *
 * PARAMS:
 *   data_length - response data length
 *   req_buf_ptr - pointer to request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
static void acph_create_suc_resp(uint32_t data_length,
                          uint8_t *req_buf_ptr,
                          uint8_t *resp_buf_ptr,
                          uint32_t *resp_buf_length
                          )
{
   uint8_t suc_flag = ACPH_SUC_FLAG_TRUE;
   uint32_t resp_data_length;
   *resp_buf_length = ACPH_HEADER_LENGTH + ACPH_SUC_FLAG_LENGTH + data_length;
   if (NULL == resp_buf_ptr)
   {
      /**not initialized*/
      ACDB_DEBUG_LOG("resp buffer ptr address[%p]\n", (void *)resp_buf_ptr);
      *resp_buf_length = 0;
      return;
   }
   resp_data_length = ACPH_SUC_FLAG_LENGTH + data_length;
   /**copy command id */
   ACDB_MEM_CPY(resp_buf_ptr,ACPH_COMMAND_ID_LENGTH, req_buf_ptr, ACPH_COMMAND_ID_LENGTH);
   /**copy response data length */
   ACDB_MEM_CPY(resp_buf_ptr + ACPH_COMMAND_ID_LENGTH,ACPH_DATA_LENGTH_LENGTH,
          &resp_data_length,
          ACPH_DATA_LENGTH_LENGTH);
   /**copy suc flag */
   ACDB_MEM_CPY(resp_buf_ptr + ACPH_HEADER_LENGTH,ACPH_SUC_FLAG_LENGTH,
          &suc_flag,
          ACPH_SUC_FLAG_LENGTH);
}

/**
* FUNCTION : IsServiceRegistered
*
* DESCRIPTION : Checks if the given service is already registered.
*
* DEPENDENCIES : None
*
* PARAMS:
*   nServiceId - Service ID
*
* RETURN VALUE : returns true or false.
*
* SIDE EFFECTS : None
*/
static bool_t IsServiceRegistered(uint32_t nServiceId)
{
   if (g_pCmdTbl != NULL)
   {
      AcphFcnLinklistType *pCur = g_pCmdTbl->pNodeHead;
      while (pCur)
      {
         if(pCur->serviceId == nServiceId)
         {
            return TRUE;
         }
         else
         {
            pCur = pCur->pNext;
         }
      }
   }
   return FALSE;
}

/**
 * FUNCTION : acph_init
 *
 * DESCRIPTION : Initilize ACPH. Allocate memory.
 *
 * DEPENDENCIES : None
 *
 * RETURN VALUE : ACPH_SUCCESS when success, ACPH_FAILURE otherwise
 *
 * SIDE EFFECTS : None
 */
int32_t acph_init(void)
{
    int32_t result = ACPH_SUCCESS;

    if (acph_init_count)
    {
       return ACPH_SUCCESS;
    }

    if (NULL == acph_main_buffer)
    {
        acph_main_buffer = (uint8_t *) ACDB_MALLOC(ACPH_ACDB_BUFFER_POSITION + ACPH_BUFFER_LENGTH);
        g_pCmdTbl = (AcphFcnNodeType *) ACDB_MALLOC(sizeof(AcphFcnNodeType));

        if (NULL == acph_main_buffer || NULL == g_pCmdTbl)
        {
            /**ACDB_MALLOC failed*/
            result = ACPH_FAILURE;
        }
        else
        {
            g_pCmdTbl->pNodeHead = NULL;
            g_pCmdTbl->pNodeTail = NULL;
        }
    }
    if (ACPH_SUCCESS != result)
    {
        ACDB_DEBUG_LOG("[ACPH]->acph_init->memory allocation failed\n");
        goto end;
    }

    result = acph_online_init();
    if (result != ACPH_SUCCESS)
    {
       ACDB_DEBUG_LOG("[ACPH]->acph_init->acph_online_intf_init failed\n");
       goto end;
    }

    //This part will be handled by Platform team
/*    result = acph_rtc_dsp_init();
    if (ACPH_SUCCESS != result)
    {
       ACDB_DEBUG_LOG("[ACPH]->acph_init->rtc_audio_voice_intf_init failed\n");
       goto end;
    }
    result = acph_rtc_adie_init();
    if (ACPH_SUCCESS != result)
    {
       ACDB_DEBUG_LOG("[ACPH]->acph_init->adie_rtc_intf_init failed\n");
       goto end;
    }
*/
    actp_diag_init(acph_execute_command);

    acph_init_count ++;
end:

    if (result != ACPH_SUCCESS)
    {
       ACDB_DEBUG_LOG("[ACPH]->ACPH init failed!\n");
    }
    else
    {
       ACDB_DEBUG_LOG("[ACPH]->ACPH init success\n");
    }

    return result;
}

/**
 * FUNCTION : acph_deinit
 *
 * DESCRIPTION : Deinitilize ACPH. ACDB_MEM_FREE memory.
 *
 * DEPENDENCIES : None
 *
 * RETURN VALUE : ACPH_SUCCESS when success, ACPH_FAILURE otherwise
 *
 * SIDE EFFECTS : None
 */
int32_t acph_deinit(void)
{
    int32_t result = ACPH_SUCCESS;

    ACDB_DEBUG_LOG("[ACPH]->acph_deinit->is called\n");
    if (!acph_init_count)
    {
       return result;
    }

    if (NULL != acph_main_buffer)
    {
        ACDB_MEM_FREE(acph_main_buffer);
        acph_main_buffer = NULL;
    }

    {
       if (g_pCmdTbl != NULL && g_pCmdTbl->pNodeHead != NULL)
       {
          int32_t count = 0;
          AcphFcnLinklistType *pCur = g_pCmdTbl->pNodeHead;
          ACDB_DEBUG_LOG("g_pCmdTbl is not NULL, g_pCmdTbl->pNodeHead is not NULL\n");

          while (pCur)
          {
             count ++;
             ACDB_DEBUG_LOG("Node%d is not empty, address[%p]\n",count,pCur);
             pCur = pCur->pNext;
          }
          if (g_pCmdTbl->pNodeTail != NULL)
          {
             ACDB_DEBUG_LOG("g_pCmdTbl->pNodeTail is not NULL, address[%p]\n",g_pCmdTbl->pNodeTail);
          }
       }
    }

    (void) acph_online_deinit();
    //This part will be handled by Platform team
//    (void) acph_rtc_adie_deinit();
//    (void) acph_rtc_dsp_deinit();

    if (NULL != g_pCmdTbl)
    {
       if (g_pCmdTbl->pNodeHead != NULL)
       {
          AcphFcnLinklistType *pCur = g_pCmdTbl->pNodeHead->pNext;
          while (pCur)
          {
             g_pCmdTbl->pNodeHead->pNext = pCur->pNext;
             ACDB_MEM_FREE(pCur);
             pCur = g_pCmdTbl->pNodeHead->pNext;
          }
          ACDB_MEM_FREE(g_pCmdTbl->pNodeHead);
       }
       ACDB_MEM_FREE(g_pCmdTbl);
       g_pCmdTbl = NULL;
    }
    acph_init_count = 0;

    return result;
}

/**
 * FUNCTION : acph_execute_command
 *
 * DESCRIPTION : Interpret request and operate ACDB correspondingly, response with
 * the result.
 *
 * DEPENDENCIES : ACDB needs to be initialized before this function is called
 *
 * PARAMS:
 *   req_buf_ptr - pointer to request buffer
 *   req_buf_length - length of the request buffer
 *   resp_buf_ptr - pointer to response buffer
 *   resp_buf_length - length of the response buffer
 *
 * RETURN VALUE : None
 *
 * SIDE EFFECTS : None
 */
void acph_execute_command(uint8_t *req_buf_ptr,
                          uint32_t req_buf_length,
                          uint8_t **resp_buf_ptr,
                          uint32_t *resp_buf_length
                          )
{
   uint32_t ulData_Length;
   int32_t result = ACPH_FAILURE;
   uint16_t nCommandId = 0;
   uint32_t nServiceId=0;
   uint8_t *temp_resp_buf = NULL;
   ACPH_CALLBACK_PTR pfnRequested_Operation_Function = NULL;
   uint32_t resp_buf_size = ACPH_BUFFER_LENGTH-ACPH_ACDB_BUFFER_POSITION;
   *resp_buf_ptr = acph_main_buffer;
   temp_resp_buf = acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;

   //int32_t (*pfnRequested_Operation_Function)(uint32_t, char_t*, char_t**, uint32_t*);

   if (FALSE == get_command_length(req_buf_ptr, req_buf_length, &ulData_Length))
   {
      /**return error response: ERR_LENGTH_NOT_MATCH */
      acph_create_error_resp(ACPH_ERR_LENGTH_NOT_MATCH, req_buf_ptr, *resp_buf_ptr, resp_buf_length);
      return;
   }

   //copy command id
   ACDB_MEM_CPY((void*)&nCommandId,ACPH_COMMAND_ID_LENGTH, (void*)(req_buf_ptr + ACPH_COMMAND_ID_POSITION),
          ACPH_COMMAND_ID_LENGTH);

   if(nCommandId >= ACPH_ONLINE_CMD_ID_START && nCommandId <= ACPH_ONLINE_CMD_ID_END)
   {
      nServiceId = ACPH_ONLINE_REG_SERVICEID;
   }
   else if(nCommandId >= ACPH_DSP_RTC_CMD_ID_START && nCommandId <= ACPH_DSP_RTC_CMD_ID_END)
   {
      nServiceId = ACPH_DSP_RTC_REG_SERVICEID;
   }
   else if(nCommandId >= ACPH_ADIE_RTC_CMD_ID_START && nCommandId <= ACPH_ADIE_RTC_CMD_ID_END)
   {
      nServiceId = ACPH_ADIE_RTC_REG_SERVICEID;
   }
   else if(nCommandId >= ACPH_MEDIA_CONTROL_CMD_ID_START && nCommandId <= ACPH_MEDIA_CONTROL_CMD_ID_END)
   {
      nServiceId = ACPH_MEDIA_CONTROL_REG_SERVICEID;
   }
   else if(nCommandId >= ACPH_FILE_TRANSFER_CMD_ID_START && nCommandId <= ACPH_FILE_TRANSFER_CMD_ID_END)
   {
      nServiceId = ACPH_FILE_TRANSFER_REG_SERVICEID;
   }
   else
   {
      ACDB_DEBUG_LOG("[ACPH ERROR]->acph_execute_command->The command id provided do not belong to any service category [%d]\n",(uint32_t)nCommandId);
      acph_create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, *resp_buf_ptr, resp_buf_length);
       return;
   }

   if (g_pCmdTbl != NULL)
   {
      AcphFcnLinklistType *pCur = g_pCmdTbl->pNodeHead;
      while (pCur)
      {
         if(pCur->serviceId == nServiceId)
         {
              uint32_t temp_req_buf_len = req_buf_length-ACPH_HEADER_LENGTH;
         pfnRequested_Operation_Function = pCur->fcnPtr;
              if(temp_req_buf_len == 0)
              {
             result = pfnRequested_Operation_Function(nCommandId, NULL, temp_req_buf_len,temp_resp_buf,resp_buf_size,resp_buf_length);
         }
              else
         {
             result = pfnRequested_Operation_Function(nCommandId, req_buf_ptr+ACPH_HEADER_LENGTH, temp_req_buf_len,temp_resp_buf,resp_buf_size,resp_buf_length);
              }
         break;
      }
                else
      {
          pCur = pCur->pNext;
      }
      }

      if (result == ACPH_ERR_INVALID_COMMAND)
      {
         ACDB_DEBUG_LOG("[ACPH ERROR]->acph_execute_command->Received Invalid commandId[%08X], result[%08X]\n",
                        nCommandId, result);
         acph_create_error_resp(result, req_buf_ptr, *resp_buf_ptr, resp_buf_length);
      }
      else if (result != ACPH_SUCCESS)
      {
         ACDB_DEBUG_LOG("[ACPH ERROR]->acph_execute_command->Error received while executing commandId[%08X] with result[%08X]\n",
                        nCommandId, result);
         acph_create_error_resp(result, req_buf_ptr, *resp_buf_ptr, resp_buf_length);
      }
      else
      {
          acph_create_suc_resp(*resp_buf_length,req_buf_ptr,*resp_buf_ptr,resp_buf_length);
      }
   }
   else
   {
      ACDB_DEBUG_LOG("[ACPH ERROR]->acph_execute_command->acph registry table was not initialized. Check if acph_init is called.\n");
      result = ACPH_ERR_UNKNOWN_REASON;
   }
}

/**
 * FUNCTION : acph_register_command
 *
 * DESCRIPTION : register command id into acph registry table
 *
 *
 * DEPENDENCIES : get_command_length is supposed to be called before this function,
 * which will check the command length and make sure command id is there
 *
 * PARAMS: *
 *   nService_id - corresponding to command id to the function name,
*                 client must use this commandId to de-resiger the command
 *   function_ptr - a pointer to sumbit operation function name
 *
 * RETURN VALUE : ACPH_SUCCESS if the register with function pointer success;
 *                ACPH_FAILURE otherwise.
 *
 * SIDE EFFECTS : None
 */
int32_t acph_register_command(uint32_t nService_id,
                              ACPH_CALLBACK_PTR fcn_ptr
                              )
{
   int32_t result = ACPH_SUCCESS;

   if( (nService_id != ACPH_ONLINE_REG_SERVICEID) &&
      (nService_id != ACPH_DSP_RTC_REG_SERVICEID) &&
      (nService_id != ACPH_ADIE_RTC_REG_SERVICEID) &&
       (nService_id != ACPH_MEDIA_CONTROL_REG_SERVICEID) &&
      (nService_id != ACPH_FILE_TRANSFER_REG_SERVICEID))
   {
      ACDB_DEBUG_LOG("[ACPH]->Invalid service id Received for ACPH registration - %x\n", nService_id);
      return ACPH_ERR_INVALID_SERVICE_ID;
   }

   //ACDB_DEBUG_LOG("[ACPH]->registering the command - %x\n", nService_id);

   if (g_pCmdTbl == NULL)
   {
      ACDB_DEBUG_LOG("[ACPH ERROR]->acph_execute_command->acph registry table was not initialized\n");
      result = ACPH_FAILURE;
      return result;
   }

   if(IsServiceRegistered(nService_id))
   {
     ACDB_DEBUG_LOG("[ACPH]->Requested service already registered - %x\n", nService_id);
     return ACPH_FAILURE;
   }

   if (g_pCmdTbl->pNodeTail != NULL)
   {
      AcphFcnLinklistType* pCur = g_pCmdTbl->pNodeTail;
      pCur->pNext = (AcphFcnLinklistType*)ACDB_MALLOC(sizeof(AcphFcnLinklistType));
      if (pCur->pNext != NULL)
      {
         pCur->pNext->serviceId = nService_id;
         pCur->pNext->fcnPtr = fcn_ptr;
         pCur->pNext->pNext = NULL;
         g_pCmdTbl->pNodeTail = pCur->pNext;
      }
      else
      {
         ACDB_DEBUG_LOG("[ACPH ERROR]->acph_register_command->fail to ACDB_MALLOC memory\n");
         result = ACPH_FAILURE;
      }
   }
   else
   {
      /* register with cmdId and function pointer */
      g_pCmdTbl->pNodeHead = (AcphFcnLinklistType*)ACDB_MALLOC(sizeof(AcphFcnLinklistType));
      if (g_pCmdTbl->pNodeHead != NULL)
      {
         g_pCmdTbl->pNodeHead->serviceId = nService_id;
         g_pCmdTbl->pNodeHead->fcnPtr = fcn_ptr;
         g_pCmdTbl->pNodeHead->pNext = NULL;
         g_pCmdTbl->pNodeTail = g_pCmdTbl->pNodeHead;
      }
      else
      {
         ACDB_DEBUG_LOG("[ACPH ERROR]->acph_register_command->fail to ACDB_MALLOC memory\n");
         result = ACPH_FAILURE;
      }
   }

   if(result == ACPH_SUCCESS)
   {
      switch(nService_id)
      {
      case ACPH_ONLINE_REG_SERVICEID:
         ACDB_DEBUG_LOG("[ACPH]->Online service registered with ACPH\n");
         break;
      case ACPH_DSP_RTC_REG_SERVICEID:
         ACDB_DEBUG_LOG("[ACPH]->DSP RTC service registered with ACPH\n");
         break;
      case ACPH_ADIE_RTC_REG_SERVICEID:
         ACDB_DEBUG_LOG("[ACPH]->ADIE RTC service registered with ACPH\n");
         break;
      case ACPH_FILE_TRANSFER_REG_SERVICEID:
         ACDB_DEBUG_LOG("[ACPH]->FTS RTC service registered with ACPH\n");
         break;
      case ACPH_MEDIA_CONTROL_REG_SERVICEID:
         ACDB_DEBUG_LOG("[ACPH]->MCS RTC service registered with ACPH\n");
         break;
      default:
         ACDB_DEBUG_LOG("[ACPH]->Unknown service registered with ACPH\n");
         break;
      }
   }

   return result;
}

/**
 * FUNCTION : acph_deregister_command
 *
 * DESCRIPTION : deregister command into acph registry table
 *
 *
 * DEPENDENCIES : get_command_length is supposed to be called before this function,
 * which will check the command length and make sure command id is there
 *
 * PARAMS: *
 *   nService_id - corresponding to command id to the function name, it is dynamically
 *                created and client must use this commandId to de-resiger the command
 *
 * RETURN VALUE : ACPH_SUCCESS if the register with function pointer success;
 *                ACPH_FAILURE otherwise.
 *
 * SIDE EFFECTS : None
 */
int32_t acph_deregister_command(uint32_t nService_id
                                )
{
   int32_t result = ACPH_FAILURE;

   if (g_pCmdTbl != NULL && g_pCmdTbl->pNodeHead != NULL)
   {
      AcphFcnLinklistType* pCur = g_pCmdTbl->pNodeHead;
      AcphFcnLinklistType* pPrev = g_pCmdTbl->pNodeHead;
      //first Node
      if (pCur->serviceId == nService_id)
      {
         if (pCur == g_pCmdTbl->pNodeTail)
         {
           ACDB_MEM_FREE(pCur);
           g_pCmdTbl->pNodeHead = NULL;
            g_pCmdTbl->pNodeTail = g_pCmdTbl->pNodeHead;
         }
         else
         {
            g_pCmdTbl->pNodeHead = pCur->pNext;
            ACDB_DEBUG_LOG("Free first node, pNodeHead[%p],pCur[%p],pNext[%p]\n",
                  g_pCmdTbl->pNodeHead,pCur,pCur->pNext);
            ACDB_MEM_FREE(pCur);
         }

         result = ACPH_SUCCESS;
      }
      else
      {
         pCur = pCur->pNext;

         //2nd or ... Node
         while (pCur)
         {
            if (pCur->serviceId == nService_id)
            {
               if (pCur == g_pCmdTbl->pNodeTail)
               {
                 g_pCmdTbl->pNodeTail = pPrev;
               }
               pPrev->pNext = pCur->pNext;
               ACDB_MEM_FREE(pCur);
               result = ACPH_SUCCESS;
               break;
            }
            else
            {
               pPrev = pPrev->pNext;
               pCur = pCur->pNext;
            }
         }
      }
      if (result == ACPH_FAILURE)
      {
         ACDB_DEBUG_LOG("[ACPH ERROR]->acph_deregister_command->given service Id not found to deregister\n");
      }
   }
   else
   {
      ACDB_DEBUG_LOG("[ACPH ERROR]->acph_execute_command->acph registry table was not initialized\n");
      result = ACPH_FAILURE;
   }

   if(result == ACPH_SUCCESS)
   {
      switch(nService_id)
      {
      case ACPH_ONLINE_REG_SERVICEID:
         ACDB_DEBUG_LOG("[ACPH]->Online service Unregistered with ACPH\n");
         break;
      case ACPH_DSP_RTC_REG_SERVICEID:
         ACDB_DEBUG_LOG("[ACPH]->DSP RTC service Unregistered with ACPH\n");
         break;
      case ACPH_ADIE_RTC_REG_SERVICEID:
         ACDB_DEBUG_LOG("[ACPH]->ADIE RTC service Unregistered with ACPH\n");
         break;
       case ACPH_MEDIA_CONTROL_REG_SERVICEID:
         ACDB_DEBUG_LOG("[ACPH]->MCS RTC service Unregistered with ACPH\n");
         break;
      case ACPH_FILE_TRANSFER_REG_SERVICEID:
         ACDB_DEBUG_LOG("[ACPH]->FTS RTC service Unregistered with ACPH\n");
         break;
      default:
         ACDB_DEBUG_LOG("[ACPH]->Unknown service Unregistered with ACPH\n");
         break;
      }
   }

   return result;
}

