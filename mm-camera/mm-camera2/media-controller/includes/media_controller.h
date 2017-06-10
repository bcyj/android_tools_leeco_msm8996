/* media_controller.h
 *                               .
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MEDIA_CONTROLLER_H__
#define __MEDIA_CONTROLLER_H__

#include "mct_list.h"
#include "mtype.h"
#include "cam_types.h"

/* put mct forward declarations here due to cross dependencies */
typedef struct _mct_object mct_object_t;
typedef struct _mct_event mct_event_t;
typedef struct _mct_bus mct_bus_t;
typedef struct _mct_pipeline mct_pipeline_t;
typedef struct _mct_stream mct_stream_t;
typedef struct _mct_module mct_module_t;
typedef struct _mct_port mct_port_t;

#define MCT_MODULE_CAST(obj) ((mct_module_t *)(obj))
#define MCT_PORT_CAST(obj)   ((mct_port_t *)(obj))

#define pack_identity(sessionid, streamid) \
  ((sessionid & 0x0000FFFF) << 16) | (streamid & 0x0000FFFF)

#endif /* __MEDIA_CONTROLLER_H__ */
