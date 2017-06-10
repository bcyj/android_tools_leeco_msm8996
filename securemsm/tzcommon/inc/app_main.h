#ifndef _APP_MAIN_H_
#define _APP_MAIN_H_
/*===========================================================================
  Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

  $Header: //source/qcom/qct/core/pkg/bootloaders/dev/chenxiz.playready.use.tzos/boot_images/core/securemsm/sampletzosapp/core/inc/app_main.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/23/14    wt     Added AOSTLM cmd handler
05/01/14    rk     Defined new structs to handle non-contiguous buffers with 64bit support
04/03/14    tp     Formatted by running "atyle --style=allman" command.
01/16/14    dm     Reverting back the backward compatibility support added for TZ_OUT_BUF_MAX macro
05/20/13    dm     Modified TZ_OUT_BUF_MAX macro to get the value from QSEECOM
08/08/12    rk     Added SVC ID for drm provisioning commands.
02/26/12    ib     Added the ISDB-Tmm BKM application
05/11/11    cz     Added a path in tz_prov_provision_rsp_t to support chmod/chown
04/28/10   chm     Added support for decryption using TZBSP Crypto Driver.
04/28/11   chm     Added support for Memory protection API's.
03/24/11   jct     Added testdir request and response structures
03/03/11   jct     Added fs and time command id's
02/09/11   ssm     Initial Version.

===========================================================================*/

/*===========================================================================

                     INCLUDE FILES FOR MODULE

===========================================================================*/
#include "comdef.h"
#include "stdlib.h"

#define APP_CORE_SVC_ID             0x07000000

/* Add your Services here ;
   you could have a single service that
   handles all your API's or you can create
   different handlers for each service that
   your application provides */
#define SVC_TZCOMMMON_ID            0x00000000

#define TZ_OUT_BUF_MAX              512

/* tzcommon_cmd_handler for tzcommon services
   by adding a weak link we allow for a service to
   be conditionally compiled
*/
//__weak void tzcommon_cmd_handler(void* cmd,uint32 cmdlen,void* rsp,uint32 rsplen);
void tzcommon_cmd_handler(void* cmd,uint32 cmdlen,void* rsp,uint32 rsplen);

/* playready_cmd_handler for playready services */
#define SVC_PLAYREADY_ID             0x00010000
//__weak void playready_cmd_handler(void* cmd,uint32 cmdlen,void* rsp,uint32 rsplen);
void playready_cmd_handler(void* cmd,uint32 cmdlen,void* rsp,uint32 rsplen);


/* widevine_cmd_handler for widevine services */
#define SVC_WIDEVINE_ID              0x00020000
//__weak void widevine_cmd_handler(void* cmd,uint32 cmdlen,void* rsp,uint32 rsplen);
void widevine_cmd_handler(void* cmd,uint32 cmdlen,void* rsp,uint32 rsplen);

/* isdbtmm_cmd_handler for isdbtmm services */
#define SVC_ISDBTMM_ID              0x00030000
//__weak void isdbtmm_cmd_handler(void* cmd,uint32 cmdlen,void* rsp,uint32 rsplen);
void isdbtmm_cmd_handler(void* cmd,uint32 cmdlen,void* rsp,uint32 rsplen);

/* drmprov_cmd_handler for drm key provisioning */
#define SVC_DRMPROV_ID              0x00050000
void drmprov_cmd_handler(void* cmd,uint32 cmdlen,void* rsp,uint32 rsplen);

/* aostlm_cmd_handler for aostlm services */
#define SVC_AOSTLM_ID              0x00070000
void aostlm_cmd_handler(void* cmd,uint32 cmdlen,void* rsp,uint32 rsplen);

/**
  Commands for TZ OS Core
 */
#define APP_CORE_CREATE_CMD(x)             (APP_CORE_SVC_ID | x)

#define APP_CORE_CMD_INVALID                APP_CORE_CREATE_CMD(0)
#define APP_CORE_CMD_INIT_FN_TABLE          APP_CORE_CREATE_CMD(1)     /**< Initialize the function table from TZBSP */

/* undefined commands */
#define APP_CMD_UNKNOWN          0x7FFFFFFE
#define APP_CMD_MAX              0x7FFFFFFF


/* A common structure that can be used by any service to
   respond to an unsupported command request */
typedef struct app_unknown_rsp_s
{
    /** First 4 bytes should always be command id */
    uint32          core_cmd;
} __attribute__ ((packed)) app_unknown_rsp_t;

// sturcture for non-contiguous buffers
struct tz_buf_seg
{
    uint32 address;
    uint32 size;
} __attribute__ ((packed));

// sturcture for non-contiguous buffers with 64bit address
struct tz_buf_64bit_seg
{
  uint64 address;
  uint32 size;
} __attribute__ ((packed));


typedef struct tz_buf_array_s
{
    struct tz_buf_seg buf_seg[TZ_OUT_BUF_MAX];
    uint32 seg_num;
} __attribute__ ((packed)) tz_buf_array_s_t;

typedef struct tz_buf_64bit_array_s
{
  struct tz_buf_64bit_seg buf_seg[TZ_OUT_BUF_MAX];
  uint32 seg_num;
} __attribute__ ((packed)) tz_buf_64bit_array_s_t;


typedef enum
{
  TZ_SUPPORTS_32BIT = 0x00000000,
  TZ_SUPPORTS_64BIT = 0x00000001,
  UNKNOWN_CAPABILITY= 0x7FFFFFFF
}tz_capability_t;

#endif /* _APP_MAIN_H_ */
