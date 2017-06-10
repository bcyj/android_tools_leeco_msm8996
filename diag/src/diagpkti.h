//# Copyright (c) 2007-2011 by Qualcomm Technologies, Inc.  All Rights Reserved.
//# Qualcomm Technologies Proprietary and Confidential.

#ifndef DIAGPKTI_H
#define DIAGPKTI_H

#include "diagi.h"
#include "diag.h"

typedef struct
{
  diag_cmd_rsp rsp_func; /* If !NULL, this is called in lieu of comm layer */
  void *rsp_func_param;

  diagpkt_rsp_type rsp; /* see diagi.h */
}
diagpkt_lsm_rsp_type;
/* Initializes critical section that protects global Rx buffer */

	boolean diagpkt_rx_buffer_cs_init(void);

boolean diagpkt_get_delayed_rsp_id(byte* out_buf_ptr, unsigned long outbuf_max_bytes, unsigned long* pbytes_written);

#endif /* DIAGPKTI_H */
