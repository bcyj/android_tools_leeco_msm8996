#ifndef _SNS_OSA_H_
#define _SNS_OSA_H_
/*============================================================================
  @file sns_osa.h

  @brief
  OS Abstraction layer for sensors.

  This is a common header file; however, each processor & target OS will have
  a specific implementation.

  <br><br>

  Copyright (c) 2010-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*=====================================================================
  INCLUDES
  =======================================================================*/
#if defined(SNS_PCSIM) || defined (SNS_LA) || defined(SNS_LA_SIM) || defined(SNS_BLAST) || defined(SNS_QDSP_SIM)
#include <includes.h>
#else
#include "os_qcom_port.h"
#endif
#include "sensor1.h"

/*=====================================================================
  INTERNAL DEFINITIONS AND TYPES
  =======================================================================*/
/* Critical Section */
#define SNS_OS_ENTER_CRITICAL() OS_ENTER_CRITICAL()
#define SNS_OS_EXIT_CRITICAL() OS_EXIT_CRITICAL()

#ifdef SNS_PCSIM
#define OS_CPU_INT_LOCK()
#endif

/*=====================================================================
  INTERNAL FUNCTION PROTOTYPES
  =======================================================================*/
/* Task */
uint8_t       sns_os_task_create           (void           (*task)(void *p_arg),
                                            void            *p_arg,
                                            OS_STK          *ptos,
                                            uint8_t          prio);

uint8_t       sns_os_task_create_ext       (void           (*task)(void *p_arg),
                                            void            *p_arg,
                                            OS_STK          *ptos,
                                            uint8_t          prio,
                                            uint16_t         id,
                                            OS_STK          *pbos,
                                            uint32_t         stk_size,
                                            void            *pext,
                                            uint16_t         opt,
                                            uint8_t          *name);

uint8_t       sns_os_task_del               (uint8_t          prio);
uint8_t       sns_os_task_del_req           (uint8_t          prio);

/* Flag */

OS_FLAG_GRP  *sns_os_sigs_create           (OS_FLAGS         flags,
                                            uint8_t         *perr);

OS_FLAG_GRP  *sns_os_sigs_del              (OS_FLAG_GRP     *pgrp,
                                            uint8_t          opt,
                                            uint8_t         *perr);

OS_FLAGS      sns_os_sigs_pend             (OS_FLAG_GRP     *pgrp,
                                            OS_FLAGS         flags,
                                            uint8_t          wait_type,
                                            uint32_t         timeout,
                                            uint8_t         *perr);

OS_FLAGS      sns_os_sigs_post             (OS_FLAG_GRP     *pgrp,
                                            OS_FLAGS         flags,
                                            uint8_t          opt,
                                            uint8_t         *perr);


OS_FLAGS      sns_os_sigs_accept           (OS_FLAG_GRP     *pgrp,
                                            OS_FLAGS         flags,
                                            uint8_t          wait_type,
                                            uint8_t         *perr);

void         *sns_os_sigs_add              (OS_FLAG_GRP     *pgrp,
                                            OS_FLAGS         flags);

/* Mutex */

OS_EVENT     *sns_os_mutex_create          (uint8_t          prio,
                                            uint8_t         *perr);

OS_EVENT     *sns_os_mutex_del             (OS_EVENT        *pevent,
                                            uint8_t          opt,
                                            uint8_t         *perr);

void          sns_os_mutex_pend            (OS_EVENT        *pevent,
                                            uint32_t         timeout,
                                            uint8_t         *perr);

uint8_t       sns_os_mutex_post            (OS_EVENT        *pevent);

OS_EVENT     *sns_os_mutex_create_uimg     (uint8_t          prio,
                                            uint8_t         *perr);

#ifdef SNS_DSPS_PROFILE_ON
void          sns_os_time_dly( uint16_t ticks);
#endif

#endif /* _SNS_OSA_H_ */
