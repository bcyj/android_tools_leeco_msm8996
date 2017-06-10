#ifndef USB_UICC_QMI_H
#define USB_UICC_QMI_H
/*===========================================================================
  Copyright (c) 2013, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/30/13   xj      Initial version
03/27/14   xj      correct typo
===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <properties.h>
#include "qmi_idl_lib.h"
#include "comdef.h"
#include "qmi_client.h"
#include "qmi_idl_lib_internal.h"
#include "qmi_idl_lib.h"
#include "user_identity_module_remote_v01.h"

/*===========================================================================

                           DEFINES

===========================================================================*/
#ifdef USB_UICC_QMI_UT
#define USB_UICC_UT
#endif /* USB_UICC_QMI_UT */

/*=========================================================================

  FUNCTION:  usb_uicc_send_write

===========================================================================*/

/*!
    @brief
    Performs to print data buffer.

    @return
    boolean.
*/
/*=========================================================================*/
void usb_uicc_print_data
(
  const uint8                  *data_ptr,
  uint32                        data_len
);

/*=========================================================================

  FUNCTION:  usb_uicc_cancel_thread

===========================================================================*/
/*!
    @brief
    To cancel open/ioctl thread.

    @return
    None.
*/
/*=========================================================================*/
void usb_uicc_cancel_thread
(
  pthread_t pthread_id
);

/*=========================================================================

  FUNCTION:  usb_uicc_create_and_send_remote_evt_req

===========================================================================*/

/*!
    @brief
    Performs to create and send event to qmi remote uim server.

    @return
    boolean.
*/
/*=========================================================================*/
void usb_uicc_create_and_send_remote_evt_req
(
  uim_remote_event_type_enum_v01 evt_type
);

/*=========================================================================

  FUNCTION:  usb_uicc_create_and_send_remote_apdu_req

===========================================================================*/

/*!
    @brief
    Performs to create and send apdu response to qmi remote uim server.

    @return
    None.
*/
/*=========================================================================*/
void usb_uicc_create_and_send_remote_apdu_req
(
  qmi_result_type_v01 qmi_result_type
);

/*=========================================================================

  FUNCTION:  usb_uicc_init_qmi

===========================================================================*/
/*!
    @brief
    Finds out the QMI port to be used based on the modem type & initializes
    QMI_UIM service.

    @return
    TRUE if successful, else FALSE
*/
/*=========================================================================*/
boolean usb_uicc_init_qmi
(
  void
);

/*=========================================================================

  FUNCTION:  usb_uicc_open_ccid_device

===========================================================================*/
/*!
    @brief
    write to hub enable fd and open ccid.

    @return
    -1     failed
    others file descriptor
*/
/*=========================================================================*/
int usb_uicc_open_ccid_device
(
  const char * pathname,
  int          flags
);

/*=========================================================================

  FUNCTION:  usb_uicc_close_ccid_device

===========================================================================*/
/*!
    @brief
    close ccid and write card removal flag to hub enable fd.

    @return
    none.
*/
/*=========================================================================*/
void usb_uicc_close_ccid_device
(
  int fd
);

/*=========================================================================

  FUNCTION:  usb_uicc_detach_usb

===========================================================================*/
/*!
    @brief
    Deattach usb if any error when read or write.

    @return
    None.
*/
/*=========================================================================*/
void usb_uicc_detach_usb
(
  void
);

/*=========================================================================

  FUNCTION:  usb_uicc_reset_global_data

===========================================================================*/
/*!
    @brief
    To reset global data except qmi_msg_lib_handle and qmi_uim_svc_client_ptr.

    @return
    None.
*/
/*=========================================================================*/
void usb_uicc_reset_global_data
(
  boolean detach_usb
);

#ifdef USB_UICC_QMI_UT
/*=========================================================================

  FUNCTION:  usb_uicc_ut_thread

===========================================================================*/
/*!
    @brief
    Perform to UT thread

    @return
    None.
*/
/*=========================================================================*/
void usb_uicc_create_ut_thread
(
  void
);
#endif /* USB_UICC_QMI_UT */

#endif /* USB_UICC_QMI_H */
