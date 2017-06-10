/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "module_denoise_dbg.h"
#include "module_imglib_common.h"
#include "module_denoise.h"
#include "module_denoise_port.h"
#include "module_denoise_lib.h"

static boolean module_denoise_stream_on_event(mct_module_t *module,
  mct_event_t *event);
static boolean module_denoise_stream_off_event(mct_module_t *module,
  mct_event_t *event);
static boolean module_denoise_forward_event_to_port(mct_module_t *module,
  mct_event_t *event);
static boolean module_denoise_start_session(mct_module_t *module,
  uint32_t sessionid);
static boolean module_denoise_stop_session(mct_module_t *module,
  uint32_t sessionid);

/** module_denoise_context_t:
 *    @dummy_port: dummy port used to simulate external peer
 *    @lib_handle: library handle
 *
 *  This structure defines the context of denoise module
 **/
typedef struct {
  mct_port_t* dummy_port;
  void* lib_handle;
} module_denoise_context_t;

/** module_denoise_event_lut_entry_t:
 *    @type: binded command
 *    @event_handler: binded event handler
 *
 *  This structure defines entry in the denoise module LUT for event handlers
 **/
typedef struct {
  union {
    uint32_t type;
    mct_event_module_type_t event_type;
    mct_event_control_type_t cmd_type;
  };
  mct_module_process_event_func event_handler;
} module_denoise_event_lut_entry_t;

/** module_denoise_cmd_lut:
 *
 *  This array defines denoise module LUT for command handlers
 **/
static module_denoise_event_lut_entry_t module_denoise_cmd_lut[] = {
  {{MCT_EVENT_CONTROL_STREAMON}, module_denoise_stream_on_event},
  {{MCT_EVENT_CONTROL_STREAMOFF}, module_denoise_stream_off_event},
  {{MCT_EVENT_CONTROL_PARM_STREAM_BUF}, module_denoise_forward_event_to_port},
  // Dummy entry. In needs always at the end of the LUT
  {{MCT_EVENT_CONTROL_MAX}, NULL}
};

/** module_denoise_event_lut:
 *
 *  This array defines denoise module LUT for event handlers
 **/
static module_denoise_event_lut_entry_t module_denoise_event_lut[] = {
  // Dummy entry. In needs always at the end of the LUT
  {{MCT_EVENT_MODULE_MAX}, NULL}
};

/** module_denoise_check_reserve_port
 *    @data1: mct_port_t object
 *    @data2: stream info
 *
 *  Checks if this port has can be reserved
 *
*  Return TRUE if the port has the same identity and is linked
 **/
static boolean module_denoise_check_reserve_port(void *data1,
  void *data2)
{
  boolean ret_val = FALSE;
  mct_module_t *module;
  mct_port_t *port = (mct_port_t *)data1;
  mct_stream_info_t *info = (mct_stream_info_t *)data2;
  module_denoise_context_t* denoise_context;

  IDBG_MED("%s +", __func__);

  if (port && info && MCT_PORT_PARENT(port) && (MCT_PORT_PARENT(port))->data) {
    module = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
    denoise_context = module->module_private;
    if (denoise_context && denoise_context->dummy_port) {
        ret_val = port->check_caps_reserve(port,
          &denoise_context->dummy_port->caps, info);
    }
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/**
 * module_denoise_stream_on_event:
 *   @module: the module instance
 *   @event: mct event
 *
 * Stream on Event handler function for the denoise module
 *
 * This function executes in Imaging Server context
 *
 * Return values: TRUE in case of success
 **/
static boolean module_denoise_stream_on_event(mct_module_t *module,
  mct_event_t *event)
{
  boolean ret_val = FALSE;
  mct_list_t *list_match;
  mct_port_t *port = NULL;
  mct_stream_info_t *info;
  module_denoise_context_t* denoise_context = module->module_private;
  mct_stream_t* stream;
  uint32_t dummy;

  IDBG_LOW("%s +", __func__);

  stream = mod_imglib_find_module_parent(event->identity, module);

  if (stream) {
    info = &stream->streaminfo;

    if (info) {
      list_match = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
        info, module_denoise_check_reserve_port);

      if (list_match && list_match->data)
        port = MCT_PORT_CAST(list_match->data);
      else
        port = module->request_new_port(info, MCT_PORT_SINK, module,
          &denoise_context->dummy_port->caps);

      if (port) {
        ret_val = port->ext_link(event->identity, port,
          denoise_context->dummy_port);

        if (ret_val) {
          ret_val = port->event_func(port, event);
        }
      }
    }
  }

  if (!ret_val) {
    CDBG_ERROR("Cannot process stream on in %s\n", __func__);

    if (port) {
      port->un_link(event->identity, port, port);
      port->check_caps_unreserve(port, event->identity);
    }
  }

  IDBG_LOW("%s -", __func__);

  return ret_val;
}

/**
 * module_denoise_stream_off_event:
 *   @module: the module instance
 *   @event: mct event
 *
 * Stream off Event handler function for the denoise module
 *
 * This function executes in Imaging Server context
 *
 * Return values: TRUE in case of success
 **/
static boolean module_denoise_stream_off_event(mct_module_t *module,
  mct_event_t *event)
{
  boolean ret_val = FALSE;
  mct_list_t *list_match;
  mct_port_t *port;
  module_denoise_context_t* denoise_context = module->module_private;

  IDBG_LOW("%s +", __func__);

  list_match = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
      &event->identity, module_denoise_port_check_linked_port_identity);

  if (list_match && list_match->data) {
    port = MCT_PORT_CAST(list_match->data);
    if (port) {
      ret_val = port->event_func(port, event);

      port->un_link(event->identity, port, denoise_context->dummy_port);

      ret_val &= port->check_caps_unreserve(port, event->identity);
    } else
      CDBG_ERROR("Port pointer corrupted in %s\n", __func__);

    if (!ret_val)
      CDBG_ERROR("Cannot process stream off in %s\n", __func__);
  } else
    CDBG_ERROR("Cannot find port with identity 0x%x in %s\n",
      event->identity, __func__);

  IDBG_LOW("%s -", __func__);

  return ret_val;
}

/**
 * module_denoise_forward_event_to_port:
 *   @module: the module instance
 *   @event: mct event
 *
 * Handler function in denoise module for forwarding events
 *   to corresponding ports
 *
 * This function executes in Imaging Server context
 *
 * Return values: TRUE in case of success
 **/
static boolean module_denoise_forward_event_to_port(mct_module_t *module,
  mct_event_t *event)
{
  boolean ret_val = FALSE;
  mct_list_t *list_match;
  mct_port_t *port;

  IDBG_LOW("%s +", __func__);

  list_match = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
      &event->identity, module_denoise_port_check_linked_port_identity);

  if (list_match && list_match->data) {
    port = MCT_PORT_CAST(list_match->data);
    if (port) {
      ret_val = port->event_func(port, event);
    } else
      CDBG_ERROR("Port pointer corrupted in %s\n", __func__);
    if (!ret_val)
      CDBG_ERROR("Cannot process event in %s\n", __func__);
  } else
    ret_val = TRUE;

  IDBG_LOW("%s -", __func__);

  return ret_val;
}

/** module_denoise_find_event_handler
 *    @id: event id
 *    @denoise_handlers_lut: denoise handlers lut
 *
 * Returns event handler binded to corresponded event id
 *
 * Returns event handler or null (if event handler is not found)
 **/
static void* module_denoise_find_event_handler(uint32_t id,
  module_denoise_event_lut_entry_t denoise_handlers_lut[])
{
  void* ret_val = NULL;
  uint32_t i;

  IDBG_LOW("%s +", __func__);

  for (i=0; denoise_handlers_lut[i].event_handler; i++) {
    if (denoise_handlers_lut[i].type == id) {
      ret_val = denoise_handlers_lut[i].event_handler;
      break;
    }
  }

  IDBG_LOW("%s -", __func__);

  return ret_val;
}

/**
 * module_denoise_process_event:
 *   @module: the module instance
 *   @event: mct event
 *
 * Event handler function for the denoise module
 *
 * This function executes in Imaging Server context
 *
 * Return values: TRUE in case of success
 **/
static boolean module_denoise_process_event(mct_module_t *module,
  mct_event_t *event)
{
  boolean ret_val = FALSE;
  mct_stream_info_t *info;
  mct_stream_t* stream;
  mct_module_process_event_func event_handler;

  IDBG_LOW("%s +", __func__);

  if (module && event && module->module_private) {
    if ((mct_module_find_type(module, event->identity)) == MCT_MODULE_FLAG_SOURCE) {
      if (MODULE_DENOISE_VALIDATE_NAME(module)) {

        stream = mod_imglib_find_module_parent(event->identity, module);
        if (stream) {
          info = &stream->streaminfo;
          if (info->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
            if (MCT_EVENT_DIRECTION(event) == MCT_EVENT_DOWNSTREAM) {
              if (MCT_EVENT_CONTROL_CMD == event->type) {

                mct_event_control_t *mct_event_control = &event->u.ctrl_event;
                event_handler =
                  module_denoise_find_event_handler(mct_event_control->type,
                    module_denoise_cmd_lut);
                if (event_handler)
                  ret_val = event_handler(module, event);
                else
                  ret_val = module_denoise_forward_event_to_port(module,
                    event);

              } else if (MCT_EVENT_MODULE_EVENT == event->type) {

                mct_event_module_t *mct_event_module = &event->u.module_event;
                event_handler =
                  module_denoise_find_event_handler(mct_event_module->type,
                    module_denoise_event_lut);
                if (event_handler)
                  ret_val = event_handler(module, event);
                else
                  ret_val = module_denoise_forward_event_to_port(module,
                    event);

              } else
                CDBG_ERROR("Received mct_event->type %d is not supported\n",
                  event->type);
            } else
              CDBG_ERROR("Wrong event->direction %d in %s\n",
                MCT_EVENT_DIRECTION(event), __func__);
          } else
            CDBG_ERROR("Wrong info->stream_type %d in %s\n", info->stream_type,
              __func__);
        } else
          CDBG_ERROR("Cannot find module parent with identity 0x%x in %s\n",
            event->identity, __func__);
      } else {
        CDBG_ERROR("Requested module name is %s\n", MCT_OBJECT_NAME(module));
        CDBG_ERROR("Module name needs to be %s\n", MODULE_DENOISE_NAME);
      }
    } else
      CDBG_ERROR("Module type should be %d, but it is %d %s\n",
        MCT_MODULE_FLAG_SOURCE, (mct_module_find_type(module, event->identity)),
        __func__);
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_LOW("%s -", __func__);

  return ret_val;
}

/** module_denoise_set_mod:
 *    @module: the module instance
 *    @module_type: new type of this module
 *    @identity: stream identity
 *
 * Specifies new type of existng module
 *
 * This function executes in Imaging Server context
 *
 * Returns nothing
 **/
static void module_denoise_set_mod(mct_module_t *module, uint32_t module_type,
  uint32_t identity)
{
  IDBG_MED("%s +", __func__);

  if (module) {
    mct_module_add_type(module, module_type, identity);
  }

  IDBG_MED("%s -", __func__);

  return;
}

/** module_denoise_query_mod_func:
 *    @module: the module instance
 *    @query_buf: querry capabilities data
 *    @sessionid: session for which querry data is requested
 *
 * Requests module capabilities data for specified session
 *
 * This function executes in Imaging Server context
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_query_mod(mct_module_t *module,
  void *query_buf, uint32_t sessionid)
{
  boolean ret_val = FALSE;
  mct_pipeline_cap_t *buf = query_buf;

  IDBG_MED("%s +", __func__);

  if (module && query_buf) {
    if (MODULE_DENOISE_VALIDATE_NAME(module)) {

    ret_val = module_denoise_lib_query_mod(buf);

    } else {
      CDBG_ERROR("Requested module name is %s\n", MCT_OBJECT_NAME(module));
      CDBG_ERROR("Module name needs to be %s\n", MODULE_DENOISE_NAME);
    }
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_start_session:
 *    @module: the module instance
 *    @session_id: session id to be started
 *
 * Starts specified session
 *
 * This function executes in Imaging Server context
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_start_session(mct_module_t *module,
  uint32_t session_id)
{
  boolean ret_val = FALSE;
  mct_port_t *port = NULL;

  IDBG_MED("%s +", __func__);

  if (module) {
    if (MODULE_DENOISE_VALIDATE_NAME(module)) {
      IDBG_HIGH("Session id 0x%x started", session_id);
      ret_val = TRUE;
    } else {
      CDBG_ERROR("Requested module name is %s\n", MCT_OBJECT_NAME(module));
      CDBG_ERROR("Module name needs to be %s\n", MODULE_DENOISE_NAME);
    }
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_remove_port
 *    @port: mct_port_t object
 *    @module: mct_module_t module
 *
 * Removes port from module
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_remove_port(mct_port_t *port, mct_module_t *module)
{
  boolean ret_val = FALSE;

  IDBG_MED("%s +", __func__);

  if (port && module && MODULE_DENOISE_VALIDATE_NAME(port)
    && MODULE_DENOISE_VALIDATE_NAME(module)) {

    mct_module_remove_port(module, port);
    module_denoise_port_deinit(port);
    mct_port_destroy(port);

    ret_val = TRUE;
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_stop_session:
 *    @module: the module instance
 *    @session_id: session id to be stopped
 *
 * Stops specified session
 *
 * This function executes in Imaging Server context
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_stop_session(mct_module_t *module,
  uint32_t session_id)
{
  boolean ret_val = FALSE;
  mct_list_t* list;

  IDBG_MED("%s +", __func__);

  if (module) {
    if (MODULE_DENOISE_VALIDATE_NAME(module)) {
      MCT_OBJECT_LOCK(module);

      do {
        list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &session_id,
          module_denoise_port_validate_port_session_id);
        if (list)
          module_denoise_remove_port(list->data, module);
      } while (list);

      do {
        list = mct_list_find_custom(MCT_MODULE_SRCPORTS(module), &session_id,
          module_denoise_port_validate_port_session_id);
        if (list)
          module_denoise_remove_port(list->data, module);
      } while (list);

      ret_val = TRUE;
      IDBG_HIGH("Session id 0x%x stopped", session_id);

      MCT_OBJECT_UNLOCK(module);
    } else {
      CDBG_ERROR("Requested module name is %s\n", MCT_OBJECT_NAME(module));
      CDBG_ERROR("Module name needs to be %s\n", MODULE_DENOISE_NAME);
    }
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_request_new_port:
 *    @stream_info: pointer to stream info
 *    @direction: source / sink
 *    @module: the module instance
 *    @peer_caps: capabilites of the peer port
 *
 *  This function creates new port dynamically and adds it to specified module
 *
 *  This function executes in Imaging Server context
 *
 *  Return Instance to new port in case of success, otherwise returns NULL
 **/
static mct_port_t *module_denoise_request_new_port(void *stream_info,
  mct_port_direction_t direction, mct_module_t *module, void *peer_caps)
{
  boolean port_created = FALSE;
  mct_port_t *port = NULL;
  mct_stream_info_t *info = stream_info;
  mct_port_caps_t *caps = peer_caps;
  module_denoise_context_t* denoise_context;
  char port_name[32];

  IDBG_MED("%s +", __func__);

  if (stream_info && module && module->module_private) {
    denoise_context = module->module_private;
    if (MODULE_DENOISE_VALIDATE_NAME(module)) {
      if (!caps || caps->port_caps_type == MCT_PORT_CAPS_FRAME) {
        if (MCT_PORT_SINK == direction)
          snprintf(port_name, sizeof(port_name), "%s_%d",
            MODULE_DENOISE_PORT_SINK_NAME, module->numsinkports);
        else
          snprintf(port_name, sizeof(port_name), "%s_%d",
            MODULE_DENOISE_PORT_SRC_NAME, module->numsrcports);
        port = mct_port_create(port_name);
        if (port) {
          if (module_denoise_port_init(port, direction, &info->identity,
            denoise_context->lib_handle)) {
              MCT_OBJECT_LOCK(module);
              if (mct_module_add_port(module, port)) {
                if (port->check_caps_reserve(port,
                  &denoise_context->dummy_port->caps, stream_info))
                    port_created = TRUE;
                else
                  mct_module_remove_port(module, port);
              }

              if (!port_created) {
                module_denoise_port_deinit(port);
              }
              MCT_OBJECT_UNLOCK(module);
          }

          if (port_created)
            IDBG_HIGH("New port %s created", MCT_OBJECT_NAME(port));
          else {
            CDBG_ERROR("Cannot initialize new port in %s\n", __func__);
            mct_port_destroy(port);
            port = NULL;
          }
        } else
        CDBG_ERROR("Cannot create new port in %s\n", __func__);
      } else
        CDBG_ERROR("Peer caps->port_caps_type %d are not %d in %s\n",
          caps->port_caps_type, MCT_PORT_CAPS_FRAME, __func__);
    } else {
      CDBG_ERROR("Requested module name is %s\n", MCT_OBJECT_NAME(module));
      CDBG_ERROR("Module name needs to be %s\n", MODULE_DENOISE_NAME);
    }
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return port;
}

/** module_denoise_get_next_from_list
 *    @data1: not used
 *    @data2: not used
 *
 *  Gets next element from the list
 *
 *  Return TRUE always
 **/
boolean module_denoise_get_next_from_list(void *data1,
  void *data2)
{
  IDBG_LOW("%s +", __func__);

  IDBG_LOW("%s -", __func__);

  return TRUE;
}

/** module_denoise_deinit:
 *    @module: the instance to be deinitialized
 *
 * Deinitializes instance of denoise module
 *
 * This function executes in Imaging Server context
 *
 * Returns nothing
 **/
void module_denoise_deinit(mct_module_t *module)
{
  module_denoise_context_t* denoise_context;
  mct_list_t* list;

  IDBG_MED("%s +", __func__);

  if (module) {
    if (MODULE_DENOISE_VALIDATE_NAME(module)) {
      MCT_OBJECT_LOCK(module);

      if (module->module_private) {
        denoise_context = module->module_private;
        if (denoise_context->dummy_port) {
          module_denoise_port_deinit(denoise_context->dummy_port);
          mct_port_destroy(denoise_context->dummy_port);
        }
        if (denoise_context->lib_handle) {
          module_denoise_lib_unload(denoise_context->lib_handle);
        }
        free(module->module_private);
      }

      do {
        list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), module,
          module_denoise_get_next_from_list);
        if (list)
          module_denoise_remove_port(list->data, module);
      } while (list);

      do {
        list = mct_list_find_custom(MCT_MODULE_SRCPORTS(module), module,
          module_denoise_get_next_from_list);
        if (list)
          module_denoise_remove_port(list->data, module);
      } while (list);

      mct_module_destroy(module);
    }
  }

  IDBG_MED("%s -", __func__);
}

/** module_denoise_init:
 *    @name: name of this denoise interface module ("denoise")
 *
 * Initializes new instance of denoise module
 *
 * This function executes in Imaging Server context
 *
 * Returns new instance on success or NULL on fail
 **/
mct_module_t *module_denoise_init(const char *name)
{
  boolean status = FALSE;
  mct_module_t* module_denoise = NULL;
  module_denoise_context_t* denoise_context;
  uint32_t sessionid = 0;

  IDBG_MED("%s +", __func__);

  if (name) {
    if (!strncmp(name, MODULE_DENOISE_NAME, sizeof(MODULE_DENOISE_NAME))) {

      module_denoise = mct_module_create(name);
      if (module_denoise) {

        denoise_context = malloc(sizeof(module_denoise_context_t));
        if (denoise_context) {
          module_denoise->module_private = denoise_context;

          denoise_context->lib_handle = module_denoise_lib_load();
          if (denoise_context->lib_handle) {
            denoise_context->dummy_port =
              mct_port_create(MODULE_DENOISE_PORT_DUMMY_NAME);
            if (denoise_context->dummy_port) {
              if (module_denoise_port_init(denoise_context->dummy_port,
                MCT_PORT_SRC, &sessionid, denoise_context->lib_handle)) {

                mct_module_set_process_event_func(module_denoise,
                  module_denoise_process_event);
                mct_module_set_set_mod_func(module_denoise,
                  module_denoise_set_mod);
                mct_module_set_query_mod_func(module_denoise,
                  module_denoise_query_mod);
                mct_module_set_start_session_func(module_denoise,
                  module_denoise_start_session);
                mct_module_set_stop_session_func(module_denoise,
                  module_denoise_stop_session);
                mct_module_set_request_new_port_func(module_denoise,
                  module_denoise_request_new_port);

                status = TRUE;
              } else
                CDBG_ERROR("Cannot init dummy port in %s module\n", name);
            } else
              CDBG_ERROR("Cannot create dummy port in %s module\n", name);
          } else
            CDBG_ERROR("Cannot load library in %s module\n", name);
        } else
          CDBG_ERROR("Cannot allocate private data for %s module\n", name);
        if (!status)
          module_denoise_deinit(module_denoise);
      } else
        CDBG_ERROR("Cannot create %s module\n", name);
    } else {
      CDBG_ERROR("Requested module name is %s\n", name);
      CDBG_ERROR("Module name needs to be %s\n", MODULE_DENOISE_NAME);
    }
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return module_denoise;
}
