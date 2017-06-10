#ifndef QUEUE_WRAPPER_H
#define QUEUE_WRAPPER_H
/*============================================================================

  @file sns_queue_wrapper.h

  @brief
  This file contains the wrapper of the queue util that the sensors framework uses

            Copyright (c) 2011 Qualcomm Technologies, Inc.
            All Rights Reserved.
            Qualcomm Technologies Confidential and Proprietary

============================================================================*/
#ifdef USE_STANDARD_QCOM_QUEUE
#include "queue.h"
#else
#include "sns_queue.h"
#endif

/*===========================================================================

                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

$PVCSPath: O:/src/asw/COMMON/vcs/queue.h_v   1.3   16 May 2002 15:43:00   rajeevg  $
$Header: //components/dev/ssc.adsp/2.6/gulansey.ssc.adsp.2.6.gulansey_sam31/common/util/queue/inc/sns_queue_wrapper.h#1 $ $DateTime: 2014/09/30 13:32:03 $ $Author: gulansey $
   
when       who    what, where, why
--------   ---    ----------------------------------------------------------
2011-04-30 Br     initial release

===========================================================================*/
#if defined(SNS_DSPS_BUILD)
# define USE_STANDARD_QCOM_QUEUE
#endif

#ifdef USE_STANDARD_QCOM_QUEUE
# define SNS_Q_LINK_S          q_link_type
# define SNS_Q_S               q_type
# define SNS_Q_INIT            q_init
# define SNS_Q_LINK            q_link
# define SNS_Q_PUT             q_put
# define SNS_Q_GET             q_get
# define SNS_Q_LAST_GET        q_last_get
# define SNS_Q_CNT             q_cnt
# define SNS_Q_CHECK           q_check
# define SNS_Q_LAST_CHECK      q_last_check
# define SNS_Q_NEXT            q_next
# define SNS_Q_INSERT          q_insert
# define SNS_Q_DELETE          q_delete
# define SNS_Q_LINEAR_SEARCH   q_linear_search
# define SNS_Q_LINEAR_DELETE   q_linear_delete
#else
# define SNS_Q_LINK_S          sns_q_link_type
# define SNS_Q_S               sns_q_type
# define SNS_Q_INIT            sns_q_init
# define SNS_Q_LINK            sns_q_link
# define SNS_Q_PUT             sns_q_put
# define SNS_Q_GET             sns_q_get
# define SNS_Q_LAST_GET        sns_q_last_get
# define SNS_Q_CNT             sns_q_cnt
# define SNS_Q_CHECK           sns_q_check
# define SNS_Q_LAST_CHECK      sns_q_last_check
# define SNS_Q_NEXT            sns_q_next
# define SNS_Q_INSERT          sns_q_insert
# define SNS_Q_DELETE          sns_q_delete
# define SNS_Q_LINEAR_SEARCH   sns_q_linear_search
# define SNS_Q_LINEAR_DELETE   sns_q_linear_delete
#endif  /* USE_STANDARD_QCOM_QUEUE */

#endif /* QUEUE_WRAPPER_H */
