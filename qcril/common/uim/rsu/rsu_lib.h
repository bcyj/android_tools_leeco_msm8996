#ifndef RSU_LIB_H
#define RSU_LIB_H
/*===========================================================================
  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#define MODEM_REQUEST_GET_SHARED_KEY           1
#define MODEM_REQUEST_UPDATE_SIMLOCK_SETTINGS  2
#define MODEM_REQUEST_GET_SIMLOCK_VERSION      3
#define MODEM_REQUEST_RESET_SIMLOCK_SETTINGS   4
#define MODEM_REQUEST_GET_MODEM_STATUS         5

#define MODEM_STATUS_REBOOT_REQUEST            1
#define MODEM_STATUS_OK                        0
#define MODEM_STATUS_CONNECTION_FAILED        -1
#define MODEM_STATUS_UNSUPPORTED_COMMAND      -2
#define MODEM_STATUS_VERIFICATION_FAILED      -3
#define MODEM_STATUS_BUFFER_TOO_SHORT         -4
#define MODEM_STATUS_COMMAND_FAILED           -5
#define MODEM_STATUS_GET_TIME_FAILED          -6

enum ModemLockState
{
  MODEM_LOCK_STATE_LOCKED                     = 0,
  MODEM_LOCK_STATE_TEMPORARY_UNLOCK           = 1,
  MODEM_LOCK_STATE_PERMANENT_UNLOCK           = 2,
  // For testing a different-length modem state blob that is nonetheless valid
  MODEM_LOCK_STATE_PERMANENT_UNLOCK_DUALSIM   = 255
};

/*===========================================================================

                           FUNCTIONS

===========================================================================*/

extern int32_t Connect_To_Modem(
  void
);

extern int32_t Disconnect_From_Modem(
  void
);

extern int32_t ModemWrapper_Send_request (
  uint32_t            request_type,
  uint8_t           * buffer_ptr,
  uint32_t            buffer_len,
  uint32_t            payload_len
);

#endif /* RSU_LIB_H */
