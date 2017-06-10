/* pp_buf_mgr.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __PP_BUF_MGR_H__
#define __PP_BUF_MGR_H__

#include "mtype.h"
#include "mct_stream.h"

/** pp_buf_mgr_open: buffer manager open
 *
 *  On first call, create buffer manager instance, call function
 *  to open buffer manager subdevice and increment ref count.
 *
 *  Return: buffer manager instance on SUCCESS
 *          NULL on failure **/
void *pp_buf_mgr_open(void);

/** pp_buf_mgr_close: buffer manager close
 *
 *  @buf_mgr: buf mgr instance
 *
 *  Decrement buffer manager ref count. If ref count is zero,
 *  close sub device and free memory
 *
 *  Return: TRUE on SUCCESS
 *          FALSE on failure **/
boolean pp_buf_mgr_close(void *v_buf_mgr);

/** pp_buf_mgr_get_buf: get buf API to acquire buffer
 *
 *  @v_buf_mgr: buf mgr instance
 *  @stream_info: stream info handle
 *
 *  Get buffer from kernel buffer manager and match its index
 *  from stream info's img buf list to extract other params like
 *  fd, vaddr
 *
 *  Return: SUCCESS - stream map struct representing buffer
 *          FAILURE - NULL **/
mct_stream_map_buf_t *pp_buf_mgr_get_buf(void *v_buf_mgr,
  mct_stream_info_t *stream_info);

/** pp_buf_mgr_put_buf: put buf API to release buffer to buf mgr
 *  without doing buf done on HAL
 *
 *  @v_buf_mgr: buf mgr instance
 *  @stream_info: stream info handle
 *
 *  Invoke put buf on buffer manager node
 *
 *  Return: SUCCESS - TRUE
 *          FAILURE - FALSE **/
boolean pp_buf_mgr_put_buf(void *v_buf_mgr, uint32_t identity,
  uint32_t buff_idx, uint32_t frameid, struct timeval timestamp);

/** pp_buf_mgr_buf_done: buf done API to release buffer to buf
 *  mgr + buf done on HAL
 *
 *  @v_buf_mgr: buf mgr instance
 *  @stream_info: stream info handle
 *
 *  Invoke buf done on buffer manager node
 *
 *  Return: SUCCESS - TRUE
 *          FAILURE - FALSE **/
boolean pp_buf_mgr_buf_done(void *v_buf_mgr, uint32_t identity,
  uint32_t buff_idx, uint32_t frameid, struct timeval timestamp);

/** pp_buf_mgr_get_vaddr: get virtual address from buffer index
 *  and associated stream info
 *
 *  @v_buf_mgr: [INPUT] buf mgr instance
 *  @buff_idx: [INPUT] buf index
 *  @stream_info: [INPUT] stream info handle
 *  @vaddr: [OUTPUT] address of virtual address
 *
 *  Get virtual address from buffer index and associated stream
 *  info
 *
 *  Return: SUCCESS - TRUE
 *          FAILURE - FALSE **/
boolean pp_buf_mgr_get_vaddr(void *v_buf_mgr, uint32_t buff_idx,
  mct_stream_info_t *stream_info, unsigned long *vaddr);

#endif
