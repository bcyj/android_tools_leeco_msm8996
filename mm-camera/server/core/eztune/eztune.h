/**********************************************************************
* Copyright (c) 2010-2012 Qualcomm Technologies, Inc.  All Rights Reserved.*
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#ifndef __EZTUNE_H__
#define __EZTUNE_H__

#include "chromatix.h"
#include "eztune_diagnostics.h"
#include "mctl_ez.h"


#define CAM_EZ_DEBUG 0
#if (CAM_EZ_DEBUG)
#ifdef _ANDROID_
#include <utils/Log.h>
#endif
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "msm_camera_EZTune"
  #define CDBG_EZ(fmt, args...) LOGE(fmt, ##args)
#else
  #define CDBG_EZ(fmt, args...) do{}while(0)
#endif

#define EZTUNE_GET_LIST  1014
#define EZTUNE_GET_PARMS 1015
#define EZTUNE_SET_PARMS 1016
#define EZTUNE_MISC_CMDS 1021

#define EZTUNE_MAX_RECV  2048
#define EZTUNE_MAX_SEND  2048

#define EZTUNE_FORMAT_MAX 20
#define EZTUNE_FORMAT_STR 50

#define POISITION1  ((void *) 0x00100100)
#define POISITION2  ((void *) 0x00200200)


typedef enum {
  EZTUNE_RECV_COMMAND = 1,
  EZTUNE_RECV_PAYLOAD_SIZE,
  EZTUNE_RECV_PAYLOAD,
  EZTUNE_RECV_RESPONSE,
  EZTUNE_RECV_INVALID
} eztune_recv_cmd_t;

typedef struct {
  uint16_t          current_cmd;
  eztune_recv_cmd_t next_recv_code;
  uint32_t          next_recv_len;
  void              *recv_buf;
  uint8_t           send;
  uint32_t          send_len;
  void              *send_buf;
} eztune_protocol_t;

typedef enum {
  EZTUNE_R_STOP = 1,
  EZTUNE_R_CONTINUE,
  EZTUNE_R_MAX
} eztune_r_status_t;

typedef enum {
  EZTUNE_MISC_GET_VERSION,
  EZTUNE_MISC_APPLY_CHANGES,
  EZTUNE_MISC_WRITE_INI,
  EZTUNE_MISC_READ_INI,
  EZTUNE_MISC_LIST_INI,
} eztune_misc_message_t;

typedef struct {
  int item_num;
  int table_index;
  char * value_string;
} eztune_set_val_t;

struct ezlist {
  struct ezlist  *next, *prev;
};

typedef struct my_list {
  struct ezlist list;
  eztune_set_val_t data;
}my_list_t;

static inline void init_list (struct ezlist *ptr)
{
  ptr->next = ptr ;
  ptr->prev = ptr ;
}

static inline void add_node ( struct ezlist *tmp , struct ezlist *head)
{
  struct ezlist *prev =  head->prev ;

  head->prev = tmp ;
  tmp->next = head ;
  tmp->prev = prev;
  prev->next = tmp ;
}

static inline void del_node (struct ezlist *ptr)
{
  struct ezlist *prev = ptr->prev ;
  struct ezlist *next = ptr->next ;

  next->prev = ptr->prev;
  prev->next = ptr->next;
  ptr->next = POISITION1;
  ptr->prev = POISITION2;
}

typedef struct {
  int8_t(*eztune_status) (void);
} eztune_function_table_t;


typedef struct {
  uint32_t clientsocket_id;
  uint32_t pipewrite_fd;
  uint32_t status;
  mctl_config_ctrl_t* mctl_ptr;
  eztune_protocol_t *protocol_ptr;
  eztune_function_table_t fn_table;
  chromatix_parms_type *chromatix_ptr;
  af_tune_parms_t *af_tune_ptr;
  vfe_diagnostics_t *diagnostics_ptr;
  ez_3a_params_t diagnostics_3a;
  ez_pp_params_t diagnostics_pp;
  ez_sensor_params_t diagnostics_sens;
  ez_config_params_t diagnostics_conf;
  ez_af_tuning_params_t af_tuning;
} eztune_t;
void eztune_init(eztune_t *ez_ctrl);
int32_t eztune_server_connect(eztune_t *ez_ctrl);
int32_t eztune_server_readwrite(eztune_t *ez_ctrl);

#endif /* __EZTUNE_H__ */
