/******************************************************************************
  @file:  xtra_config_api.h
  @brief:

  DESCRIPTION

  public api of xtra configuration

  -----------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technology Incoporated.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
 ******************************************************************************/

#ifndef _XTRA_SYSTEM_INTERFACE_H_
#define _XTRA_SYSTEM_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_URL_LEN 256
#define NUMBER_OF_SNTP_SERVERS 1
#define NUMBER_OF_XTRA_SERVERS 3
#define USER_AGENT_STRING_LEN 256

/** Xtra URL type **/
typedef char XtraUrlType[MAX_URL_LEN];

/** Callback types for when data and time are ready to be injected **/
typedef void (* XtraDataCallback)(char * data, int length);
typedef void (* XtraTimeCallback)(int64_t utcTime, int64_t timeReference, int uncertainty);

/** Callbacks for when data is ready to be injected **/
typedef struct Xtra_System_Data_Callbacks {

   XtraDataCallback xtraDataCallback;

} XtraClientDataCallbacksType;

/** Callbacks for when time is ready to be injected **/
typedef struct Xtra_System_Time_Callbacks {

    XtraTimeCallback xtraTimeCallback;

} XtraClientTimeCallbacksType;

/** Xtra System Configuration **/
typedef struct Xtra_System_Config {
    XtraUrlType xtra_sntp_server_url[NUMBER_OF_SNTP_SERVERS];
    XtraUrlType xtra_server_url[NUMBER_OF_XTRA_SERVERS];
    char user_agent_string[USER_AGENT_STRING_LEN];
} XtraClientConfigType;


/** Represents the Xtra System interface **/
typedef struct Xtra_System_Interface {

    /** set to size of this structure */
    size_t size;
    /** Initializes the interface and registers the callbaks **/
    int (*init) (XtraClientDataCallbacksType * dataCallbacks, XtraClientTimeCallbacksType * timeCallbacks, XtraClientConfigType *pConfig);
    /** Gracefully stops the system and won't return until system exits **/
    void (*stop) ();
    /** To indicate to Xtra System that data has been requested **/
    int (*onDataRequest)(void);
    /** To indicate to Xtra System that time has been requested **/
    int (*onTimeRequest)(void);

} XtraClientInterfaceType;

/** returns a pointer to the XtraClientInterface **/
XtraClientInterfaceType const * get_xtra_client_interface();

#ifdef __cplusplus
}
#endif

#endif

