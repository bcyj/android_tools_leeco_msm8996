/*===========================================================================

  Copyright (c) 2010-2011 Qualcomm Technologies, Inc. All Rights Reserved. 
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

===========================================================================*/

#ifndef QMI_PROXY_H
#define QMI_PROXY_H


/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "comdef.h"
#include "qmi.h"
#include "../src/qmi_client.h"
#include "../src/qmi_service.h"
#include "common_log.h"


/*===========================================================================

                        EXTERNAL DEFINITIONS AND TYPES


===========================================================================*/

/* Timeout for Synchronous QMI Proxy Service Request */
#define QMI_PROXY_SYNC_REQ_SANITY_TIMEOUT  3000

typedef void (*qmi_proxy_qmi_srvc_rx_msg_hdlr_type)( qmi_connection_id_type conn_id,
                                                     qmi_service_id_type    srvc_id,
                                                     qmi_client_id_type     client_id,
                                                     unsigned char          control_flags,
                                                     unsigned char          *rx_msg, 
                                                     int                    rx_msg_len
                                                   );

/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/

extern void qmi_proxy_init
(
  void
);

extern void qmi_proxy_release
(
  void
);

#endif /* QMI_PROXY_H */
