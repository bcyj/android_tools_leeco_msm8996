#ifndef CNE_CET_H
#define CNE_CET_H

/*=============================================================================
               Copyright (c) 2009-2011 Qualcomm Technologies, Inc.
               All Rights Reserved.
               Qualcomm Technologies Confidential and Proprietary
=============================================================================*/

#include "CneDefs.h"
#include "queue.h"

typedef enum
{
  CNE_SPM_ID,
  CNE_SRM_ID,
  CNE_CDE_ID,
  CNE_REF_ID
} cne_comp_id_type;

/* queue item in CneQ */
typedef struct
{
  int fd;
  cne_cmd_enum_type cmdId;
  int dataLen;
  void *data;
  //void *reqInfo;
} cne_cmd_data_type;


typedef struct
{
  q_link_type          link;
  cne_cmd_data_type   cmdInfo;
} cne_queue_item_data_type;


typedef void (*cne_commandsNotificationCbType)
(
  int fd,
  cne_cmd_enum_type cmdId,
  int dataLen,
  void *data
);

extern "C" {
  void
  cne_regCommandsNotificationCb
  (
    int clientId,
    cne_commandsNotificationCbType cbFn,
    int interestedCommandsMask
  );

  void
  cne_processCommand
  (
    int fd,
    cne_cmd_enum_type command,
    void *data,
    size_t datalen
  );

  int
  cne_getCneClientFd
  (
    void
  );
}

#endif
