/*============================================================================
@file
sns_file.c

@brief
Implements the handling of the open, write, and close requests to the file
service, and manages the list of open files.

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*============================================================================
  INCLUDE FILES
=============================================================================*/

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "sns_fsa.h"
#include "sns_debug_api.h"
#include "sns_file_internal_v01.h"
#include "sns_common.h"
#include "sns_memmgr.h"
#include "sns_debug_str.h"
#include "sns_file.h"
#include "sns_smr_util.h"

/*============================================================================
  Type Declarations
  ============================================================================*/

typedef struct sns_file {
  void* fh;
  int fildes;
  struct sns_file *next;
} sns_file;

/*============================================================================
  Static Variable Definitions
  ============================================================================*/

/* List of files managed by file service.  Mutex not necessary as all requests
 * will be received through sensor1 callback function */
static sns_file *sns_file_head = NULL;

/*============================================================================
  Static Function Definitions and Documentation
  ============================================================================*/

/*===========================================================================
  FUNCTION:   sns_file_lookup
  ==========================================================================*/
/*!
  @brief Searches the file list for the specified client.

  @param[i] fildes

  @return File struct if found, NULL otherwise.
*/
/*==========================================================================*/
static sns_file*
sns_file_lookup( int fildes )
{
  sns_file *file = sns_file_head;
  while( NULL != file )
  {
    if( file->fildes == fildes )
    {
      break;
    }
    file = file->next;
  }

  return file;
}

/*============================================================================
  FUNCTION:   sns_file_add
  ==========================================================================*/
/*!
  @brief Add a file to the file list.  First call to this function will be from
    'init', which must receive the '2' fildes.

  @param[i] fh

  @return -1 upon error; fildes on success.
*/
/*==========================================================================*/
static int32_t
sns_file_add( void *const fh )
{
  sns_file *file = SNS_OS_MALLOC( SNS_DBG_MOD_APPS_FILE, sizeof(sns_file) );
  int i = 2;
  int32_t rv = -1;

  if( NULL == file )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_FILE, "malloc failure" );
  }
  else
  {
    file->fh = fh;
    file->fildes = 0;

    for( ; i < INT_MAX; i++ )
    {
      if( NULL == sns_file_lookup( i ) )
      {
        file->fildes = i;
        break;
      }
    }

    if( 0 != file->fildes )
    {
      file->next = sns_file_head;
      sns_file_head = file;
      rv = file->fildes;
    }
  }

  return rv;
}

/*============================================================================
  FUNCTION:   sns_file_remove
  ==========================================================================*/
/*!
  @brief Removes a file from the file list if it exists.

  @param[i] fildes

  @return 0 upon success.
*/
/*==========================================================================*/
static int
sns_file_remove( int fildes )
{
  sns_file *prev = NULL, *file = sns_file_head;
  while( NULL != file )
  {
    if( file->fildes == fildes )
    {
      if( NULL != prev )
      {
        prev->next = file->next;
      }
      else
      {
        sns_file_head = file->next;
      }
      SNS_OS_FREE( file );
      break;
    }
    prev = file;
    file = file->next;
  }

  return 0;
}

/*============================================================================
  Externalized Function Definitions
  ============================================================================*/

/*============================================================================
  FUNCTION:   sns_file_open
  ==========================================================================*/
/*!
  @brief Handles a request to open a file.

  @param[i] req_msg
  @param[o] resp_msg
*/
/*==========================================================================*/
void
sns_file_open( sns_file_open_req_msg_v01 const *req_msg,
               sns_file_open_resp_msg_v01* const resp_msg )
{
  void *new_fh = NULL;

  char file_path[ SNS_FILE_MAX_FILENAME_SIZE_V01 + DEBUG_FILE_DIR_BASE_LEN ]
    = DEBUG_FILE_DIR_BASE;

  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_FILE, "rcv req" );
  new_fh = sns_fsa_open( strncat( file_path,
                                  req_msg->path_name,
                                  SNS_FILE_MAX_FILENAME_SIZE_V01),
                         req_msg->mode );

  if( NULL == new_fh )
  {
    SNS_PRINTF_STRING_HIGH_0( SNS_DBG_MOD_APPS_FILE, "open failed" );
    resp_msg->fildes_valid = false;
  }
  else if( -1 == (resp_msg->fildes = sns_file_add( new_fh )) )
  {
    SNS_PRINTF_STRING_HIGH_0( SNS_DBG_MOD_APPS_FILE, "file add failed" );
    resp_msg->fildes_valid = false;
  }
  else
  {
    resp_msg->fildes_valid = true;
  }

  resp_msg->resp.sns_err_t = SENSOR1_SUCCESS;
  resp_msg->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;
}

/*============================================================================
  FUNCTION:   sns_file_write
  ==========================================================================*/
/*!
  @brief Handles a request to write to a file.

  @param[i] req_msg
  @param[o] resp_msg
*/
/*==========================================================================*/
void
sns_file_write( sns_file_write_req_msg_v01 const *req_msg,
                sns_file_write_resp_msg_v01* const resp_msg )
{
  sns_file *file;
  sns_err_code_e err;
  SNS_PRINTF_STRING_LOW_1( SNS_DBG_MOD_APPS_FILE, "rcv req (%i)",
                           req_msg->fildes );

  if( NULL == ( file = sns_file_lookup( req_msg->fildes )) )
  {
    SNS_PRINTF_STRING_HIGH_0( SNS_DBG_MOD_APPS_FILE, "file not found" );
    resp_msg->bytes_written_valid = false;
  }
  else
  {
    err = sns_fsa_write( file->fh, req_msg->buf,
                         req_msg->buf_len, &resp_msg->bytes_written );
    err |= sns_fsa_flush( file->fh );
    if( (SNS_SUCCESS != err) ||
        (req_msg->buf_len != resp_msg->bytes_written) )
    {
      SNS_PRINTF_STRING_HIGH_3( SNS_DBG_MOD_APPS_FILE,
                                "write failed: err %i, req %i, written %i",
                                err, req_msg->buf_len, resp_msg->bytes_written );
    }
    resp_msg->bytes_written_valid = true;
  }

  resp_msg->resp.sns_err_t = SENSOR1_SUCCESS;
  resp_msg->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;
}

/*============================================================================
  FUNCTION:   sns_file_close
  ==========================================================================*/
/*!
  @brief Handles a request to close a file.

  @param[i] req_msg
  @param[o] resp_msg
*/
/*==========================================================================*/
void
sns_file_close( sns_file_close_req_msg_v01 const *req_msg,
                sns_file_close_resp_msg_v01* const resp_msg )
{
  sns_err_code_e err;
  sns_file *file;

  SNS_PRINTF_STRING_LOW_1( SNS_DBG_MOD_APPS_FILE, "rcv req (%i)", req_msg->fildes );

  file = sns_file_lookup( (int)req_msg->fildes );
  if( 2 == req_msg->fildes )
  {
    SNS_PRINTF_STRING_HIGH_0( SNS_DBG_MOD_APPS_FILE, "Invalid file descriptor" );
  }
  else if( NULL == file )
  {
    SNS_PRINTF_STRING_HIGH_0( SNS_DBG_MOD_APPS_FILE, "file not found" );
  }
  else
  {
    if( SNS_SUCCESS == (err = sns_fsa_close( file->fh )) )
    {
      sns_file_remove( (int)req_msg->fildes );
    }
    else
    {
      SNS_PRINTF_STRING_HIGH_1( SNS_DBG_MOD_APPS_FILE, "close failed %i", err );
    }
  }

  resp_msg->resp.sns_err_t = SENSOR1_SUCCESS;
  resp_msg->resp.sns_result_t = SNS_RESULT_SUCCESS_V01;
}

/*============================================================================
  FUNCTION:   sns_file_init
  ==========================================================================*/
/*!
  @brief Initializes files and structures required for file request handling.

  @return Error code
*/
/*==========================================================================*/
sns_err_code_e
sns_file_init()
{
  sns_err_code_e smr_err;
  void *new_fh = NULL;
  uint8_t os_err;

#ifndef _WIN32
  char file_path[ SNS_FILE_MAX_FILENAME_SIZE_V01 + DEBUG_FILE_DIR_BASE_LEN ]
    = DEBUG_FILE_DIR_BASE;
  sns_fsa_stat_s fsa_stat;
#endif /* _WIN32 */

  smr_err = sns_file_mr_init();
  if( SNS_SUCCESS != smr_err )
  {
    SNS_PRINTF_STRING_FATAL_1( SNS_DBG_MOD_APPS_FILE, "mr init failure %i", smr_err );
    return SNS_ERR_FAILED;
  }

  SNS_PRINTF_STRING_MEDIUM_0( SNS_DBG_MOD_APPS_FILE, "sns_file_init" );

#ifndef _WIN32
  if( SNS_SUCCESS != sns_fsa_stat( file_path, &fsa_stat ) &&
      SNS_SUCCESS != (smr_err = sns_fsa_mkdir( file_path )) )
  {
    SNS_PRINTF_STRING_HIGH_1( SNS_DBG_MOD_APPS_FILE, "sns_fsa_mkdir failed %i", smr_err );
    return SNS_ERR_FAILED;
  }
#ifndef SNS_LA_SIM
  if( SNS_SUCCESS != (smr_err = sns_fsa_chown( file_path, NULL )) )
  {
    SNS_PRINTF_STRING_HIGH_1( SNS_DBG_MOD_APPS_FILE, "sns_fsa_chown failed %i", smr_err );
    return SNS_ERR_FAILED;
  }
  if( SNS_SUCCESS != (smr_err = sns_fsa_chgrp( file_path, NULL )) )
  {
    SNS_PRINTF_STRING_HIGH_1( SNS_DBG_MOD_APPS_FILE, "sns_fsa_chgrp failed %i", smr_err );
    return SNS_ERR_FAILED;
  }
#endif /* SNS_LA_SIM */
  new_fh = sns_fsa_open( strncat( file_path,
                                  ERR_LOG_FILE_NAME,
                                  SNS_FILE_MAX_FILENAME_SIZE_V01),
                         "a+" );

  if( NULL == new_fh )
  {
    SNS_PRINTF_STRING_HIGH_0( SNS_DBG_MOD_APPS_FILE, "open failed" );
    return SNS_ERR_FAILED;
  }
  else if( -1 == sns_file_add( new_fh ) )
  {
    SNS_PRINTF_STRING_HIGH_0( SNS_DBG_MOD_APPS_FILE, "file add failed" );
    return SNS_ERR_FAILED;
  }
#endif /* _WIN32 */

  os_err = sns_os_task_create( sns_file_mr_thread, NULL, NULL,
                               SNS_MODULE_PRI_APPS_FILE );
  if( OS_ERR_NONE != os_err )
  {
    SNS_PRINTF_STRING_FATAL_0( SNS_MODULE_APPS_TIME, "Task create failed" );
    sns_fsa_close( new_fh );
    return SNS_ERR_FAILED;
  }

  return SNS_SUCCESS;
}

/*============================================================================
  FUNCTION:   sns_file_deinit
  ==========================================================================*/
/*!
  @brief Deinitializes files and structures required for file request handling.

  @return Error code
*/
/*==========================================================================*/
sns_err_code_e
sns_file_deinit()
{
  sns_err_code_e err;
  sns_file *file;

  SNS_PRINTF_STRING_MEDIUM_0( SNS_DBG_MOD_APPS_FILE, "sns_file_deinit" );

  err = sns_file_mr_deinit();
  if( SNS_SUCCESS != err )
  {
    SNS_PRINTF_STRING_FATAL_1( SNS_DBG_MOD_APPS_FILE, "mr deinit failure %i", err );
  }
  SNS_PRINTF_STRING_LOW_0( SNS_DBG_MOD_APPS_FILE, "closing all files");

  while(NULL != sns_file_head)
  {
    file = sns_file_head;

    if( SNS_SUCCESS == (err = sns_fsa_close( file->fh )) )
    {
      sns_file_remove(file->fildes);
    }
    else
    {
      SNS_PRINTF_STRING_HIGH_1( SNS_DBG_MOD_APPS_FILE, "close failed %i", err );
    }
  }

  return err;
}

/*============================================================================
  FUNCTION:   sns_file_handle
  ==========================================================================*/
sns_err_code_e
sns_file_handle( sns_smr_header_s *smr_header,
                 void *req_msg_ptr, void **resp_msg_ptr )
{
  sns_err_code_e rv = SNS_SUCCESS;
  uint8_t msg_size =  SNS_FILE_INTERNAL_OPEN_REQ_V01 == smr_header->msg_id ? sizeof(sns_file_open_resp_msg_v01) :
                      SNS_FILE_INTERNAL_WRITE_REQ_V01 == smr_header->msg_id ? sizeof(sns_file_write_resp_msg_v01) :
                      SNS_FILE_INTERNAL_CLOSE_REQ_V01 == smr_header->msg_id ? sizeof(sns_file_close_resp_msg_v01) : 0;

  if( NULL == resp_msg_ptr )
  {
    return SNS_ERR_BAD_PTR;
  }

  *resp_msg_ptr = sns_smr_msg_alloc( SNS_DBG_MOD_APPS_FILE, msg_size );
  if( NULL == *resp_msg_ptr )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_FILE, "msg alloc failed" );
    *resp_msg_ptr = req_msg_ptr; /* re-use request message */
    smr_header->msg_type = SNS_SMR_MSG_TYPE_RESP_INT_ERR;
    smr_header->body_len = 0;
    rv = SNS_ERR_NOMEM;
  }
  else
  {
    smr_header->msg_type = SNS_SMR_MSG_TYPE_RESP;
    smr_header->body_len = msg_size;
    if( SNS_FILE_INTERNAL_OPEN_REQ_V01 == smr_header->msg_id )
    {
      sns_file_open( (sns_file_open_req_msg_v01*)req_msg_ptr,
                     (sns_file_open_resp_msg_v01*)*resp_msg_ptr );
      smr_header->msg_id = SNS_FILE_INTERNAL_OPEN_RESP_V01;
    }
    else if( SNS_FILE_INTERNAL_WRITE_REQ_V01 == smr_header->msg_id )
    {
      sns_file_write( (sns_file_write_req_msg_v01*)req_msg_ptr,
                            (sns_file_write_resp_msg_v01*)*resp_msg_ptr );
      smr_header->msg_id = SNS_FILE_INTERNAL_WRITE_RESP_V01;
    }
    else if( SNS_FILE_INTERNAL_CLOSE_REQ_V01 == smr_header->msg_id )
    {
      sns_file_close( (sns_file_close_req_msg_v01*)req_msg_ptr,
                            (sns_file_close_resp_msg_v01*)*resp_msg_ptr );
      smr_header->msg_id = SNS_FILE_INTERNAL_CLOSE_RESP_V01;
    }
    else
    {
      SNS_PRINTF_STRING_HIGH_2( SNS_DBG_MOD_APPS_FILE,
                                "SNS Diag Rcv: unknown msg id %i %i",
                                smr_header->svc_num,
                                smr_header->msg_id);
      *resp_msg_ptr = req_msg_ptr; /* re-use request message */
      smr_header->msg_type = SNS_SMR_MSG_TYPE_RESP_INT_ERR;
      smr_header->body_len = 0;
      rv = SNS_ERR_FAILED;
    }

    smr_header->dst_module = smr_header->src_module;
    smr_header->src_module = SNS_MODULE_APPS_FILE;
    smr_header->svc_num = SNS_FILE_INTERNAL_SVC_ID_V01;
  }

  return rv;
}
