/* mct_module.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MCT_MODULE_H__
#define __MCT_MODULE_H__

#include "mct_bus.h"
#include "mct_object.h"
#include "mct_port.h"
#include "mct_stream.h"

/*
 * MctModuleFlags:
 * pipeline.
 *
 * @MCT_MODULE_FLAG_LOCKED_STATE : ignore state changes from parent
 * @MCT_MODULE_FLAG_SINK         : the module is a sink
 * @MCT_MODULE_FLAG_SOURCE       : the module is a source.
 * @MCT_MODULE_FLAG_INDEXABLE    : the module can be used an index
 * @MCT_MODULE_FLAG_CONTAINER    : the module contains other modules
 *                                 that actually process the events.
 * @MCT_MODULE_FLAG_LAST         : offset to define more flags
 *
 * The standard flags that any Module may have.
 */
typedef enum
{
  MCT_MODULE_FLAG_INVALID = (MCT_OBJECT_FLAG_LAST << 0),
  MCT_MODULE_FLAG_SINK = (MCT_OBJECT_FLAG_LAST << 1),
  MCT_MODULE_FLAG_SOURCE = (MCT_OBJECT_FLAG_LAST << 2),
  MCT_MODULE_FLAG_INDEXABLE = (MCT_OBJECT_FLAG_LAST << 3),
  MCT_MODULE_FLAG_PEERLESS = (MCT_OBJECT_FLAG_LAST << 4),

  /* padding */
  MCT_MODULE_FLAG_LAST = (MCT_OBJECT_FLAG_LAST << 9),
} mct_module_type_t;

typedef boolean (*mct_module_process_event_func)(mct_module_t *module,
  mct_event_t *event);

typedef void (*mct_module_set_mod_func)(mct_module_t *module,
  unsigned int module_type, unsigned int identity);

/*
 typedef void (* mct_module_free_mod_func)
 (mct_module_t *module);
 */

typedef boolean (*mct_module_query_mod_func)(mct_module_t *module,
  void *query_buf, unsigned int sessionid);

typedef mct_port_t *(*mct_module_request_port_function)(void *stream_info,
  mct_port_direction_t direction, mct_module_t *module, void *peer_caps);

typedef boolean (*mct_module_start_session)(mct_module_t *module,
  unsigned int sessionid);

typedef boolean (*mct_module_stop_session)(mct_module_t *module,
  unsigned int sessionid);

typedef struct {
  unsigned int identity;
  mct_module_type_t type;
} mct_module_type_identity_t;
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
struct _mct_module {
  mct_object_t      object;
  mct_list_t        *type_list;
  mct_list_t        *srcports;
  unsigned short    numsrcports;
  mct_list_t        *sinkports;
  unsigned short    numsinkports;
  void              *module_private;

  /* virtual functions - MUST be implemented by each module */
  mct_module_process_event_func process_event;
  mct_module_set_mod_func set_mod;
  mct_module_query_mod_func query_mod;
  mct_module_request_port_function request_new_port;
  mct_module_start_session start_session;
  mct_module_stop_session stop_session;
};

#define MCT_MODULE_NAME(mod)          (MCT_OBJECT_NAME(mod))
#define MCT_MODULE_PARENT(mod)        (MCT_OBJECT_PARENT(mod))
#define MCT_MODULE_CHILDREN(mod)      (MCT_OBJECT_CHILDREN(mod))
#define MCT_MODULE_NUM_CHILDREN(mod)  (MCT_OBJECT_NUM_CHILDREN(mod))
#define MCT_MODULE_NUM_SINKPORTS(mod) (MCT_MODULE_CAST(mod)->numsinkports)
#define MCT_MODULE_NUM_SRCPORTS(mod)  (MCT_MODULE_CAST(mod)->numsrcports)
#define MCT_MODULE_SINKPORTS(mod)     (MCT_MODULE_CAST(mod)->sinkports)
#define MCT_MODULE_SRCPORTS(mod)      (MCT_MODULE_CAST(mod)->srcports)

#define mct_module_set_process_event_func(mod,f)  \
  MCT_MODULE_CAST(mod)->process_event = f

#define mct_module_set_set_mod_func(mod,f)  \
  MCT_MODULE_CAST(mod)->set_mod = f

#define mct_module_set_query_mod_func(mod,f)  \
  MCT_MODULE_CAST(mod)->query_mod = f

#define mct_module_set_request_new_port_func(mod,f)  \
  MCT_MODULE_CAST(mod)->request_new_port = f

#define mct_module_set_start_session_func(mod,f)  \
  MCT_MODULE_CAST(mod)->start_session = f

#define mct_module_set_stop_session_func(mod,f)  \
  MCT_MODULE_CAST(mod)->stop_session = f

mct_module_t *mct_module_create(const char *name);

void mct_module_destroy(mct_module_t *module);

boolean mct_module_link(void *stream_info, mct_module_t *src,
  mct_module_t *dest);

boolean mct_module_send_event(mct_module_t *module, mct_event_t *event);

void mct_module_unlink(unsigned int identity, mct_module_t *src,
  mct_module_t *dest);

boolean mct_module_post_bus_msg(mct_module_t *module, mct_bus_msg_t *bus_msg);

boolean mct_module_add_port(mct_module_t *module, mct_port_t *port);

boolean mct_module_remove_port(mct_module_t *module, mct_port_t *port);

typedef mct_module_t* (*mct_module_init)(const char *name);
typedef void (*mct_module_deinit)(mct_module_t *module);

typedef struct _mct_module_init_name
{
  const char *name;
  mct_module_init init_mod;
  mct_module_deinit deinit_mod;
  mct_module_t *module;
} mct_module_init_name_t;

mct_module_t* module_sensor_init(const char *name);
void module_sensor_deinit(mct_module_t *module);
mct_module_t* module_isp_init(const char *name);
void module_isp_deinit(mct_module_t *module);
mct_module_t* module_iface_init(const char *name);
void module_iface_deinit(mct_module_t *module);
mct_module_t* stats_module_init(const char *name);
void stats_module_deinit(mct_module_t *module);

mct_module_t* stats_module_init(const char *name);
void stats_module_deinit(mct_module_t *mod);

mct_module_t* pproc_module_init(const char *name);
void pproc_module_deinit(mct_module_t *mod);

mct_module_t *module_imglib_init(const char *name);
void module_imglib_deinit(mct_module_t *p_mct_mod);

mct_module_t* module_faceproc_init(const char *name);
void module_faceproc_deinit(mct_module_t *mod);

void *mct_module_get_buffer_ptr(uint32_t buf_idx, mct_module_t *module,
  unsigned int session_id, unsigned int stream_id);
void *mct_module_get_buffer(uint32_t buf_idx, mct_module_t *module,
  unsigned int session_id, unsigned int stream_id);

void mct_module_add_type(mct_module_t *module, mct_module_type_t type,
  unsigned int identity);
mct_module_type_t mct_module_find_type(mct_module_t *module,
  unsigned int identity);
void mct_module_remove_type(mct_module_t *module, unsigned int identity);

void *mct_module_get_stream_info(mct_module_t *module, unsigned int session_id,
  int32_t stream_id);
mct_module_t *module_cac_init(const char *name);
void module_cac_deinit(mct_module_t *p_mct_mod);
mct_module_t *module_wnr_init(const char *name);
void module_wnr_deinit(mct_module_t *p_mct_mod);


mct_module_t *module_hdr_init(const char *name);
void module_hdr_deinit(mct_module_t *module);

#endif /* __MCT_MODULE_H__ */
