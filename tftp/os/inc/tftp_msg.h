/***********************************************************************
 * tftp_msg.h
 *
 * Short description
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Verbose description.
 *
 ***********************************************************************/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  $Header$ $DateTime$ $Author$

when         who   what, where, why
----------   ---   ---------------------------------------------------------
2013-12-03   rp    Create

===========================================================================*/

#ifndef __TFTP_MSG_H__
#define __TFTP_MSG_H__

#include "tftp_config_i.h"
#include "tftp_log.h"


void tftp_msg_init (void);

void tftp_msg_print (enum tftp_log_msg_level_type, const char *msg);



#endif /* not __TFTP_MSG_H__ */
