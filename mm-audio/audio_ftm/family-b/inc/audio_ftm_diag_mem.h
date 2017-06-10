#ifndef FTM_AUDIO_DIAG_MEM_H
#define FTM_AUDIO_DISPATCH_H

/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                      F T M    A U D I O    D I S P A T C H

GENERAL DESCRIPTION
  This is the header file for the embedded FTM Audio Commands

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================
Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/inc/audio_ftm_diag_mem.h#1 $ $DateTime: 2011/04/05 20:05:46 $ $Author: zhongl $

when       who    what, where, why
--------   ---    ----------------------------------------------------------
===========================================================================*/

#include "event.h"
#include "msg.h"
#include "log.h"

#include "diag_lsm.h"
#include "diagpkt.h"
#include "diagcmd.h"
#include "diag.h"
//#include "diagpkti.h"


/*===========================================================================

                              DEFINITIONS

===========================================================================*/

PACK(void *) audio_ftm_diagpkt_subsys_alloc (diagpkt_subsys_id_type id,
              diagpkt_subsys_cmd_code_type code, unsigned int length);


#endif /* FTM_AUDIO_DIAG_MEM_H */

