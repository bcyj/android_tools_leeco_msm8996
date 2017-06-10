/* mct_port.h
 *  														 .
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MCT_PORT_H__
#define __MCT_PORT_H__

#include "mct_object.h"

/* definintions of mct_port_caps_t.flag */

#define MCT_PORT_CAP_FORMAT_BAYER   (0x00000001 << 0)
#define MCT_PORT_CAP_FORMAT_YCBCR   (0x00000001 << 1)
#define MCT_PORT_CAP_STATS_Q3A      (0x00000001 << 3)
#define MCT_PORT_CAP_STATS_CS_RS    (0x00000001 << 4)
#define MCT_PORT_CAP_FORMAT_YCBYCR  (0x00000001 << 5)
#define MCT_PORT_CAP_FORMAT_YCRYCB  (0x00000001 << 6)
#define MCT_PORT_CAP_FORMAT_CBYCRY  (0x00000001 << 7)
#define MCT_PORT_CAP_FORMAT_CRYCBY  (0x00000001 << 8)
#define MCT_PORT_CAP_FORMAT_NV12    (0x00000001 << 9)
#define MCT_PORT_CAP_FORMAT_NV21    (0x00000001 << 10)
#define MCT_PORT_CAP_FORMAT_NV16    (0x00000001 << 11)
#define MCT_PORT_CAP_FORMAT_NV61    (0x00000001 << 12)


#define MCT_PORT_CAP_SIZE_20MB 20

#define MCT_PORT_CAP_ANY  \
  (MCT_PORT_CAP_FORMAT_BAYER | \
   MCT_PORT_CAP_FORMAT_YCBCR | \
   MCT_PORT_CAP_STATS_Q3A    | \
   MCT_PORT_CAP_STATS_CS_RS)

#define MCT_PORT_CAP_INTERLEAVED  \
  (MCT_PORT_CAP_FORMAT_YCBYCR | \
   MCT_PORT_CAP_FORMAT_YCRYCB | \
   MCT_PORT_CAP_FORMAT_CBYCRY | \
   MCT_PORT_CAP_FORMAT_CRYCBY)

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef enum _mct_port_caps_type
{
  MCT_PORT_CAPS_OPAQUE,
  MCT_PORT_CAPS_STATS,
  MCT_PORT_CAPS_FRAME,
  MCT_PORT_CAPS_SENSOR, /* Not needed*/
} mct_port_caps_type_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef struct _mct_port_caps_stats {
  unsigned int flag;
} mct_port_caps_stats_t;

typedef struct _mct_port_caps_frame {
  unsigned int format_flag;
  unsigned int size_flag;
  unsigned int priv_data; /* private data uniquely identify the port */
} mct_port_caps_frame_t;

/** mct_port_caps_t:
 *    @flag: bitmask of this port's basic capability
 *    @frame_size: supported frame size
 **/
typedef struct _mct_port_caps {
  mct_port_caps_type_t port_caps_type;
  union {
    void *data;
    mct_port_caps_stats_t stats;
    mct_port_caps_frame_t frame;
  } u;
} mct_port_caps_t;

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
typedef enum {
  MCT_PORT_UNKNOWN,
  MCT_PORT_SRC,
  MCT_PORT_SINK
} mct_port_direction_t;

typedef boolean (*mct_port_event_func)(mct_port_t *port, mct_event_t *event);

typedef mct_list_t *(*mct_port_int_link_func)(unsigned int identity,
  mct_port_t *port);

typedef boolean (*mct_port_ext_link_func)(unsigned int identity,
 mct_port_t* port, mct_port_t *peer);

typedef void (*mct_port_unlink_func)(unsigned int identity,
 mct_port_t *port, mct_port_t *peer);

typedef boolean (*mct_port_set_caps_func)(mct_port_t *port,
  mct_port_caps_t *caps);

typedef boolean (*mct_port_check_caps_reserve_func)
  (mct_port_t *port, void *peer_caps, void *stream_info);

typedef boolean (*mct_port_check_caps_unreserve_func)
(mct_port_t *port, unsigned int identity);

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
struct _mct_port {
  mct_object_t          object;
  mct_port_caps_t       caps;
  mct_port_direction_t  direction;
  mct_port_t           *peer; /*one port should have one external peer*/
  void                 *port_private;

  /* virtual functions must be defined by invidual port */
  mct_port_event_func                event_func;
  mct_port_int_link_func             int_link;
  mct_port_ext_link_func             ext_link;
  mct_port_unlink_func               un_link;
  mct_port_set_caps_func             set_caps;
  mct_port_check_caps_reserve_func   check_caps_reserve;
  mct_port_check_caps_unreserve_func check_caps_unreserve;
};

#define MCT_PORT_PEER(port)        (MCT_PORT_CAST(port)->peer)
#define MCT_PORT_DIRECTION(port)   (MCT_PORT_CAST(port)->direction)
#define MCT_PORT_IS_SINK(port)     (MCT_PORT_DIRECTION(port) == MCT_PORT_SINK)
#define MCT_PORT_IS_SRC(port)      (MCT_PORT_DIRECTION(port) == MCT_PORT_SRC)
#define MCT_PORT_PARENT(port)      (MCT_OBJECT_PARENT(port))
#define MCT_PORT_NAME(mod)         (MCT_OBJECT_NAME(mod))
#define MCT_PORT_CHILDREN(mod)     (MCT_OBJECT_CHILDREN(mod))
#define MCT_PORT_NUM_CHILDREN(mod) (MCT_OBJECT_NUM_CHILDREN(mod))
#define MCT_PORT_INTLINKFUNC(port) (MCT_PORT_CAST(port)->int_link)
#define MCT_PORT_EVENT_FUNC(port)  (MCT_PORT_CAST(port)->event_func)
#define MCT_PORT_EXTLINKFUNC(port) (MCT_PORT_CAST(port)->ext_link)
#define MCT_PORT_EXTUNLINKFUNC(port) (MCT_PORT_CAST(port)->un_link)

#define mct_port_set_event_func(port, f) \
  MCT_PORT_CAST(port)->event_func = f

#define mct_port_set_int_link_func(port, f) \
  MCT_PORT_CAST (port)->int_link = f

#define mct_port_set_ext_link_func(port, f) \
  MCT_PORT_CAST(port)->ext_link = f

#define mct_port_set_unlink_func(port, f) \
  MCT_PORT_CAST(port)->un_link = f

#define mct_port_set_set_caps_func(port, f) \
  MCT_PORT_CAST(port)->set_caps = f

#define mct_port_set_check_caps_reserve_func(port, f) \
  MCT_PORT_CAST(port)->check_caps_reserve = f

#define mct_port_set_check_caps_unreserve_func(port, f) \
  MCT_PORT_CAST(port)->check_caps_unreserve = f


mct_port_t *mct_port_create(const char *name);

void mct_port_destroy(mct_port_t *port);

boolean mct_port_check_link(mct_port_t *srcport, mct_port_t *sinkport);

boolean mct_port_establish_link(unsigned int identity,
  mct_port_t *src, mct_port_t *sink);

boolean mct_port_send_event_to_peer(mct_port_t *port, mct_event_t *event);

void mct_port_destroy_link(unsigned int identity,
  mct_port_t* srcport, mct_port_t* sinkport);

void mct_port_remove_child(unsigned int identity, mct_port_t *port);
boolean mct_port_add_child(unsigned int identity, mct_port_t *port);

#endif /* __MCT_PORT_H__ */
