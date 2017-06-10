/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef GPSONE_GLUE_PIPE_H
#define GPSONE_GLUE_PIPE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <linux/types.h>

int gpsone_glue_pipeget(const char * pipe_name, int mode);
int gpsone_glue_piperemove(const char * pipe_name, int fd);
int gpsone_glue_pipewrite(int fd, const void * buf, size_t sz);
int gpsone_glue_piperead(int fd, void * buf, size_t sz);

int gpsone_glue_pipeflush(int fd);
int gpsone_glue_pipeunblock(int fd);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GPSONE_GLUE_PIPE_H */
