/*******************************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#ifndef EZTUNE_INTERFACE_H
#define EZTUNE_INTERFACE_H

typedef enum {
    EZTUNE_METADATA_NOTIFY = 0,
    EZTUNE_PREVIEW_NOTIFY = 1,
    EZTUNE_SNAPSHOT_NOTIFY = 2,
    EZTUNE_3ALIVE_NOTIFY = 3,
    EZTUNE_STOP_NOTIFY = 4,
    EZTUNE_RELOAD_TUNING_NOTIFY = 5,
    EZTUNE_SNAPSHOT_PENDING_NOTIFY = 6,
    EZTUNE_MAX_NOTFY
} EztuneNotify;

typedef enum {
    EZTUNE_SERVER_CONTROL,
    EZTUNE_SERVER_PREVIEW,
    EZTUNE_SERVER_MAX
} eztune_server_t;

typedef enum {
    EZTUNE_INTF_ASYNC_RESP = 0,
    EZTUNE_INTF_STOP = 1,
    EZTUNE_INTF_MAX
} eztune_intf_notify_t;

#if defined(__cplusplus)
extern "C" {
#endif

//! Creats and starts the eztune Server
/*!
   Server runs in a separate thread, hence the function returns
   immediately. The return value is opaque handle which the client
   should pass when calling eztune_delete_server

   The input is defined as void pointer to keep the implementation
   generic. In the current version of code this pointer must be set to
   mct_pipeline_t structure

   The implementation uses a singleton class, so multiple calls to
   this API returns the same handle back. But if the client ptr is
   different in subsequent calls, the server would start using the
   new client handle to access camera internal structure

   \param in client : Pointer to camera structure to enable access to
    camera APIs (mct_pipeline_t)
   \param in type : Type of server
   \return Opaque pointer which should be passed when calling eztune_delete_server
*/
void *eztune_create_server(void *client, eztune_server_t type);

//! Stops eztune server
/*!
    Stops the internal server thread and deletes the instance

    \param in handle: Opaque pointer to server (returned by eztune_create_server)
*/
void eztune_delete_server(void *handle);

//! Notify eztune server of events or set params on server
/*!

    \param in handle: Opaque pointer to server (returned by eztune_create_server)
    \param in type: Event type
    \param in data: Data associated with event
*/
void eztune_notify_server(void *handle, uint32_t type, void *data);

#if defined(__cplusplus)
}
#endif

#endif
