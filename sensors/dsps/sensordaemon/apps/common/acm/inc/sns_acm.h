#ifndef _SNS_ACM_H_
#define _SNS_ACM_H_
/*============================================================================
  @file sns_acm.h

  @brief
  This contains ACM preprocessor definitions.

  <br><br>

  DEPENDENCIES: None.

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/

#define SNS_ACM_MAX_CLIENTS 100

/* Maximum service ID for usage tracking */
#define SNS_ACM_MAX_SVC_ID 60

/* Maximum number of messages in the processing queue.  Overflow is blocked
 * in QMI context
 */
#define SNS_ACM_MAX_MSG_QUEUE 20

#endif /* _SNS_ACM_H_ */
