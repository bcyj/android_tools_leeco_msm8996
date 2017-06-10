/*============================================================================
  @file sns_main.c

  @brief Implements the "main" function for the Linux Android sensors library.

  <br><br>

  DEPENDENCIES:

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ============================================================================*/

/*============================================================================

  INCLUDE FILES

  ============================================================================*/
#include "libsensor1.h"
#include "sensor1.h"
#include "sns_common_v01.h" /* For common response structure */
#include "sns_debug_str.h"
#include "sns_debug_api.h"
#include "sns_smr_util.h"
#include "sns_main.h"

# include "qmi_client.h"
#include "qmi_idl_lib.h"

#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <inttypes.h>
#include <pthread.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <utime.h>

/*============================================================================
  Preprocessor Definitions and Constants
  ============================================================================*/
/* Maximum number of services that can be in the SDK whitelist */
#define MAX_SDK_WHITELIST 64

/* Maximum number of allowed unprivileged connections */
#define MAX_UNPRIVILEGED_CONN 13

/* Maximum number of clients which will wait in a queue before connecting */
#define MAX_CONN_Q 1

/* Timeout in ms for SMRG service */
#define SNS_SMGR_SVC_TIMEOUT_MS 10000

/* Retry a standard library command if the error code is EINTR */
#define  RETRY_ON_EINTR(ret,cond) \
    do { \
        ret = (cond); \
    } while (ret < 0 && errno == EINTR)

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif /* MAX */

#define FD_ADD( fd, fdset, max )                \
  do {                                          \
    FD_SET((fd),(fdset));                       \
    (max) = MAX((max),(fd));                    \
  } while(0);

#ifdef SNS_LA_SIM
#define strlcpy strncpy
#endif /* SNS_LA_SIM */

/* Android defines PRIxPTR incorrectly. Fix it here */
#ifdef SNS_LA
#  undef PRIxPTR
#  define PRIxPTR "x"
#endif /* SNS_LA */

/* file path for sensors settings file */
/* If file exists and contains '0', then disable external sensor clients */
#define SENSORS_SETTINGS_DIR  "/persist/sensors"
#define SENSORS_SETTINGS_FILE "sensors_settings"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/*============================================================================
  Type Declarations
  ============================================================================*/

typedef struct sns_ctl_q_s {
  int                 sock_fd; /**< FD for the control socket */
  bool                privileged_user;
  sensor1_handle_s   *sensor1_hndl; /**< Sensor1 Handle */
  struct sns_ctl_q_s *next;
  bool                use_qmi;
  int32_t             outstanding_req; /**< Count of outstanding request
                                          messages */
} sns_ctl_q_s;

typedef struct sns_main_data_s {
  int          ctl_fd;  /**< Control socket server file descriptor */
  sns_ctl_q_s *ctl_p;  /**< List of client control sockets */

  /** If set to false, do not accept incoming connecitons, and close all existing
   * client connections */
  bool enabled;
  /** File descriptor for inotify events relating to sensor1 socket */
  int inotify_fd;
} sns_main_data_s;

/*============================================================================
 * Global Data Definitions
 ============================================================================*/

/*============================================================================
  Static Variable Definitions
  ============================================================================*/

/* Whitelisted services available to SDK are set to true.  Services are indexed by their value
 * defined in sns_common_v01.h */
static bool sdk_white_list[MAX_SDK_WHITELIST] = {false, false, false, false, true, true, false, false,   //AMD=4,RMD=5
                                                    false, false, true, false, false, false, false, false,  //BTE=10
                                                    false, false, false, false, false, false, false, false,
                                                    false, true, true, true, false, false, false, false,    //BASIC_GESTURES=25,TAP=26,FACING=27
                                                    false, false, false, false, false, false, false, false,
                                                    true, true, false, false, false, false, true, false,   //SMD=40,CMC=41,TILT=46
                                                    true, false, false, false, false, false, false, false, //DPC=48
                                                    false, false, false, false, false, false, false, false};

static pthread_mutex_t sns_main_data_mutex;

/* Wakeup file descriptors, used to wake up and exit the main thread */
static int wakeup_pipe[2];

/* Flag used to indicate if sensor1 is initialized or not */
static volatile bool initialized = false;

/*============================================================================
  Function Definitions
  ============================================================================*/

/*===========================================================================

  FUNCTION:   sns_main_set_not_blocking

  ===========================================================================*/
/*!
  @brief Puts a file descriptor into non-blocking mode

  @param[i] fd: file descriptor to set to non-blocking mode

  @return
  The return code from ioctl or fcntl

*/
/*=========================================================================*/
static int sns_main_set_non_blocking(int fd)
{
  int flags;

#if defined(O_NONBLOCK)
  /* Supports POSIX O_NONBLOCK */
  flags = fcntl(fd, F_GETFL, 0);
  if (-1 == flags ) {
    flags = 0;
  }
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
  /* No POSIX support */
  flags = 1;
  return ioctl(fd, FIOBIO, &flags);
#endif /* defined(O_NONBLOCK) */
}

/*===========================================================================

  FUNCTION:   sns_main_check_enabled

  ===========================================================================*/
/*!
  @brief Checks the settings file to see if sensors are enabled.

  Sets the enabled flag, and modifies the watch list to be notified when
  the settings file changes.

  @param[o] srv_data: Server data.

  If the settings file exists and starts with '1', then enable sensors.
  Otherwise sensors are disabled.

*/
/*=========================================================================*/
static void sns_main_check_enabled( sns_main_data_s *srv_data )
{
#ifdef SNS_LA_SIM
  srv_data->enabled = true;
#else
  static int settings_fd = -1;
  char enabled;

  if( settings_fd == -1 ) {
    settings_fd = open( SENSORS_SETTINGS_DIR "/" SENSORS_SETTINGS_FILE, O_RDONLY | O_NOFOLLOW );
  }
  if( settings_fd != -1 ) {
    /* File exists. */
    /* Set up inotify to be notified of file changes */
    inotify_add_watch( srv_data->inotify_fd,
                       SENSORS_SETTINGS_DIR "/" SENSORS_SETTINGS_FILE,
                       IN_CLOSE_WRITE );

    /* Read value from settings file, and enable/disable as needed */
    lseek( settings_fd, 0, SEEK_SET );
    if( 1 == read( settings_fd, &enabled, 1 ) ) {
      if( enabled == '1' ) {
        SNS_PRINTF_STRING_HIGH_0( SNS_DBG_MOD_APPS_MAIN,
                                  "enabled=true");
        srv_data->enabled = true;
        return;
      }
    } else {
      close( settings_fd );
      settings_fd = -1;
    }
  } else {
    /* File does not exist */
    SNS_PRINTF_STRING_ERROR_2( SNS_DBG_MOD_APPS_MAIN,
                               "open error: settings file \"%s\", errno %d",
                               (SENSORS_SETTINGS_DIR "/" SENSORS_SETTINGS_FILE),
                               errno );
    if( -1 == inotify_add_watch( srv_data->inotify_fd,
                                 SENSORS_SETTINGS_DIR,
                                 IN_CREATE ) ) {
      /* Error creating the watch */
      SNS_PRINTF_STRING_ERROR_2( SNS_DBG_MOD_APPS_MAIN,
                                 "inotify error: settings path \"%s\", errno %d",
                                 SENSORS_SETTINGS_DIR,
                                 errno );
      SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_MAIN,
                                 "***Sensor external clients cannot be enabled!!!***" )
    }
  }
  SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_MAIN,
                             "enabled=false");
  srv_data->enabled = false;
#endif /* SNS_LA_SIM */
}

/*===========================================================================

  FUNCTION:   sns_main_send_open_resp

  ===========================================================================*/
/*!
  Sends to a libsensor1 client the result of the sensor1_open operation.

  @param[i] ctl_fd: Control socket file descriptor
  @param[i] sns_err: Error code, one of SENSOR1_SUCCESS or SENSOR1_EWOULDBLOCK.

*/
/*=========================================================================*/
static void
sns_main_send_open_resp( int ctl_fd,
                         sensor1_error_e sns_err )
{
  int err;
  libsensor_ctl_read_s resp_pkt_p;

  resp_pkt_p.msg_type = 0;
  resp_pkt_p.svc_num  = 0;
  resp_pkt_p.msg_id   = -1;
  resp_pkt_p.txn_id   = 0;
  resp_pkt_p.reserved = 0;

  resp_pkt_p.socket_cmd = (sns_err == SENSOR1_SUCCESS) ? LIBSENSOR_SOCKET_CMD_OPEN_SUCCESS :
                          (sns_err == SENSOR1_EWOULDBLOCK) ? LIBSENSOR_SOCKET_CMD_OPEN_BLOCK :
                          -1;

  err = write( ctl_fd, &resp_pkt_p, sizeof(resp_pkt_p) );
  if( -1 == err ) {
    SNS_PRINTF_STRING_ERROR_2( SNS_DBG_MOD_APPS_MAIN,
                               "Error writing to ctl sock fd %d, err %d",
                               ctl_fd, errno );
  }
}

/*===========================================================================

  FUNCTION:   sns_main_send_err_resp

  ===========================================================================*/
/*!
  @brief Sends an error response message to a client.

  This should be used when an internal error happens when processing a request,
  and the cilent must be informed that some error has occurred.

  @param[i] ctl_fd: File descriptor of the control socket on which to send the
            response.

  @param[i] req_pkt_p: Pointer to the request packet which generated this error
            response.

  @param[i] err: Error code which should be contained in the response.

  @return
  None.

  @sideeffects
  None.
*/
/*=========================================================================*/
static void
sns_main_send_err_resp( int ctl_fd,
                        libsensor_ctl_write_s *req_pkt_p,
                        sensor1_error_e sns_err )
{
  int err;
  int msg_sz;
  libsensor_ctl_read_s *resp_pkt_p;
  libsensor_ctl_read_s internal_err_resp_pkt;

  resp_pkt_p = malloc( sizeof(*resp_pkt_p) -1 + sizeof(sns_common_resp_s_v01) );

  if( NULL != resp_pkt_p ) {
    sns_common_resp_s_v01 *resp_data_p;// The common response within resp_pkt_p
    resp_data_p = (sns_common_resp_s_v01*)(resp_pkt_p->data);
    resp_data_p->sns_result_t = 1;
    resp_data_p->sns_err_t = sns_err;
    resp_pkt_p->msg_type = SENSOR1_MSG_TYPE_RESP;
    msg_sz = sizeof(*resp_pkt_p) -1 + sizeof(resp_data_p);
  } else {
    resp_pkt_p = &internal_err_resp_pkt;
    msg_sz = sizeof(internal_err_resp_pkt);
    resp_pkt_p->msg_type = SENSOR1_MSG_TYPE_RESP_INT_ERR;
  }

  resp_pkt_p->svc_num  = req_pkt_p->svc_num;
  resp_pkt_p->msg_id   = req_pkt_p->msg_id;
  resp_pkt_p->txn_id   = req_pkt_p->txn_id;
  resp_pkt_p->reserved = 0;
  /* Sent as a RAW message to prevent QMI encoding/decoding from attempting
     to encode/decode the full response, rather than just the header */
  resp_pkt_p->socket_cmd = LIBSENSOR_SOCKET_CMD_WRITE_RAW;

  err = write( ctl_fd, resp_pkt_p, msg_sz );
  if( -1 == err ) {
    SNS_PRINTF_STRING_ERROR_2( SNS_DBG_MOD_APPS_MAIN,
                               "Error writing to ctl sock fd %d, err %d",
                               ctl_fd, errno );
  }

  if( SENSOR1_MSG_TYPE_RESP_INT_ERR != resp_pkt_p->msg_type )
  {
    free(resp_pkt_p);
  }
}


/*===========================================================================

  FUNCTION:   sns_main_add_ctl_cli

  ===========================================================================*/
/*!
  @brief Adds a ctl client to the global linked list of ctl clients

  @param[i] data: pointer to state containing the list of all connected sockets

  @param[i] new_ctl: pointer to an allocated ctl socket to add to the list.

  @return
  None.

  @sideeffects
  None.
*/
/*=========================================================================*/

static void
sns_main_add_ctl_cli( sns_main_data_s *data, sns_ctl_q_s *new_ctl )
{
  sns_ctl_q_s *ctl_p;
  ctl_p = data->ctl_p;
  if( NULL == ctl_p ) {
    data->ctl_p = new_ctl;
  } else {
    while( NULL != ctl_p->next ) {
      ctl_p = ctl_p->next;
    }
    ctl_p->next = new_ctl;
  }
  new_ctl->use_qmi = true;
  new_ctl->outstanding_req = 0;
}

/*===========================================================================

  FUNCTION:   sns_main_notify_cb

  ===========================================================================*/
/*!
  @brief Handles the notify data callback from the Sensor1 API.

  This function handles indications & responses, and sends them to the
  appropriate socket for the associated client.

  Indications are sent via the data channel, or the ctl channel if there are
  no data channels.

  Responses are sent via the ctl channel.

  @param[i] data: Pointer to the control channel associated with this client

  @param[i] msg_hdr_p: Sensor1 message header

  @param[i] msg_type: Sensor1 message type

  @param[i] msg_p: Sensor1 message body

  @return
  None.

  @sideeffects
  None.
*/
/*=========================================================================*/
static void
sns_main_notify_cb( intptr_t data,
                    sensor1_msg_header_s *msg_hdr_p,
                    sensor1_msg_type_e msg_type,
                    void *msg_p )
{
  sns_ctl_q_s                *ctl_p;
  uint32_t                    encoded_msg_sz;
  uint32_t                    encoded_msg_max;
  int32_t                     err;
  qmi_idl_service_object_type service;

  ctl_p = (sns_ctl_q_s*)data;

  if( NULL != msg_hdr_p ) {
    pthread_mutex_lock( &sns_main_data_mutex );
    if( NULL != ctl_p ) {
      service = sns_smr_get_svc_obj( msg_hdr_p->service_number );
      if( ctl_p->use_qmi ) {
        err =
          qmi_idl_get_max_message_len( service,
                                       (qmi_idl_type_of_message_type)msg_type,
                                       msg_hdr_p->msg_id,
                                       &encoded_msg_max );
        if( QMI_NO_ERR != err ) {
          sensor1_free_msg_buf( ctl_p->sensor1_hndl, msg_p );
          SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN,
                                     "QMI err gettting msg len %d",
                                     err );
          pthread_mutex_unlock( &sns_main_data_mutex );
          return;
        }
      } else {
        encoded_msg_max = msg_hdr_p->msg_size;
      }

      if( SENSOR1_MSG_TYPE_RESP == msg_type ) {
        libsensor_ctl_read_s *pkt_p;

        ctl_p->outstanding_req--;

        pkt_p = malloc( sizeof(*pkt_p)-1 + encoded_msg_max );
        if( NULL != pkt_p ) {
          pkt_p->svc_num    = msg_hdr_p->service_number;
          pkt_p->msg_id     = msg_hdr_p->msg_id;
          pkt_p->txn_id     = msg_hdr_p->txn_id;
          pkt_p->msg_type   = msg_type;
          pkt_p->reserved   = 0;

          if( ctl_p->use_qmi ) {
            pkt_p->socket_cmd = LIBSENSOR_SOCKET_CMD_WRITE_QMI;
            err = qmi_idl_message_encode( service,
                                          QMI_IDL_RESPONSE,
                                          msg_hdr_p->msg_id,
                                          msg_p,
                                          msg_hdr_p->msg_size,
                                          pkt_p->data,
                                          encoded_msg_max,
                                          &encoded_msg_sz );
            if( QMI_NO_ERR != err ) {
              sensor1_free_msg_buf( ctl_p->sensor1_hndl, msg_p );
              SNS_PRINTF_STRING_LOW_1( SNS_DBG_MOD_APPS_MAIN,
                                       "QMI err encoding msg %d",
                                       err );
              free( pkt_p );
              pthread_mutex_unlock( &sns_main_data_mutex );
              return;
            }
          } else {
            pkt_p->socket_cmd = LIBSENSOR_SOCKET_CMD_WRITE_RAW;
            encoded_msg_sz = msg_hdr_p->msg_size;
            memcpy(pkt_p->data, msg_p, msg_hdr_p->msg_size);
          }
          err = write( ctl_p->sock_fd, pkt_p, sizeof(*pkt_p)-1+encoded_msg_sz );
          if( -1 == err ) {
            SNS_PRINTF_STRING_ERROR_2( SNS_DBG_MOD_APPS_MAIN,
                                       "Error writing to ctl sock fd %d, err %d",
                                       ctl_p->sock_fd, errno );
          }
          free( pkt_p );
        }
      } else if( SENSOR1_MSG_TYPE_IND == msg_type ) {
        libsensor_data_read_s *pkt_p;
        pkt_p = malloc( sizeof(*pkt_p)-1 + encoded_msg_max );
        if( NULL != pkt_p ) {
          pkt_p->svc_num    = msg_hdr_p->service_number;
          pkt_p->msg_id     = msg_hdr_p->msg_id;
          pkt_p->txn_id     = msg_hdr_p->txn_id;
          pkt_p->msg_type   = msg_type;
          pkt_p->reserved   = 0;

          if( ctl_p->use_qmi ) {
            service = sns_smr_get_svc_obj( msg_hdr_p->service_number );
            pkt_p->socket_cmd = LIBSENSOR_SOCKET_CMD_WRITE_QMI;
            err = qmi_idl_message_encode( service,
                                          QMI_IDL_INDICATION,
                                          msg_hdr_p->msg_id,
                                          msg_p,
                                          msg_hdr_p->msg_size,
                                          pkt_p->data,
                                          encoded_msg_max,
                                          &encoded_msg_sz );
            if( QMI_NO_ERR != err ) {
              sensor1_free_msg_buf( ctl_p->sensor1_hndl, msg_p );
              SNS_PRINTF_STRING_LOW_1( SNS_DBG_MOD_APPS_MAIN,
                                       "QMI err encoding msg %d",
                                       err );
              free( pkt_p );
              pthread_mutex_unlock( &sns_main_data_mutex );
              return;
            }
          } else {
            pkt_p->socket_cmd = LIBSENSOR_SOCKET_CMD_WRITE_RAW;
            encoded_msg_sz = msg_hdr_p->msg_size;
            memcpy(pkt_p->data, msg_p, msg_hdr_p->msg_size);
          }
          err = write( ctl_p->sock_fd,
                       pkt_p,
                       sizeof(*pkt_p)-1+encoded_msg_sz );
          if( -1 == err ) {
            SNS_PRINTF_STRING_ERROR_2( SNS_DBG_MOD_APPS_MAIN,
                                       "Error writing to ctl sock fd %d, "
                                       "err %d",
                                       ctl_p->sock_fd, errno );
          }
          free( pkt_p );
        }
      }
      sensor1_free_msg_buf( ctl_p->sensor1_hndl, msg_p );
    }
    pthread_mutex_unlock( &sns_main_data_mutex );
  }
}

/*===========================================================================

  FUNCTION:   sns_main_daemonize

  ===========================================================================*/
/*!
  @brief Turns the current process into a background daemon.

  @return
  None.

  @sideeffects
  Many associated with becoming a daemon
  - Forks off a child process (the child returns, the parent exits)
  - Sets the umask to 0
  - Creates a new SID
  - Changes the path to root
  - Closes all of the stdio handles.

*/
/*=========================================================================*/
static void
sns_main_daemonize( void )
{
  pid_t pid;
  pid_t sid;
  int error;

  error = getppid();
  if( 1 == error ) {
    /* Parent is init; already a daemon */
    return;
  }

  /* Fork off the daemon process */
  pid = fork();
  if( 0 > pid ) {
    /* uh oh... */
    exit( 1 );
  } else if( pid > 0 ) {
    /* This is the parent process. Exit here, and let the child continue */
    printf("Background daemon process starting... PID %d\n", pid);
    exit( 0 );
  }

  /* Set the file mask */
  umask(0);

  /* Create a SID to disassociate ourselves from the parent process &
     terminal */
  sid = setsid();
  if( sid < 0 ) {
    exit(2);
  }

  /* Set the cwd to root, so we don't keep subdirs locked */
  error = chdir("/");
  if( 0 > error ) {
    exit(3);
  }

  /* Redirect stdio to /dev/null */
  freopen( "/dev/null", "r", stdin);
  freopen( "/dev/null", "w", stdout);
  freopen( "/dev/null", "w", stderr);
}

/*===========================================================================

  FUNCTION:   sns_main_get_usr

  ===========================================================================*/
/*!
  @return The passwd struct corresponding to either the sensors user or nobody.
*/
/*=========================================================================*/
static struct passwd*
sns_main_get_usr()
{
  static struct passwd *sensors_pwent = NULL;

  if( NULL != sensors_pwent )
  {
    return sensors_pwent;
  }
  else
  {
    sensors_pwent = getpwnam( SNS_USERNAME );
  }

  if( NULL == sensors_pwent ) /* Can't find sensors user; try 'nobody' instead */
  {
    sensors_pwent = getpwnam( USERNAME_NOBODY );

    if( NULL == sensors_pwent ) {
      SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_MAIN,
          "Unable to get user entry for 'sensors' or 'nobody'" );
    } else {
      SNS_PRINTF_STRING_HIGH_0( SNS_DBG_MOD_APPS_MAIN,
         "Usr 'sensors' does not exist. Switching to 'nobody' instead." );
    }
  }
  return sensors_pwent;
}

/*===========================================================================

  FUNCTION:   sns_main_get_grp

  ===========================================================================*/
/*!
  @return The group struct corresponding to either the sensors group or nobody.
*/
/*=========================================================================*/
static struct group*
sns_main_get_grp()
{
  static struct group *sensors_grent = NULL;

  if( NULL != sensors_grent )
  {
    return sensors_grent;
  }
  else
  {
    sensors_grent = getgrnam( SNS_GROUPNAME );
  }

  if( NULL == sensors_grent ) /* Can't find sensors group; try 'nobody' instead */
  {
    sensors_grent = getgrnam( GROUPNAME_NET_RAW );

    if( NULL == sensors_grent )
    {
      SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_MAIN,
          "Unable to get group entry for 'sensors' or 'nobody' or 'net_raw'" );
      sensors_grent = NULL;
    }
    else
    {
      SNS_PRINTF_STRING_HIGH_0( SNS_DBG_MOD_APPS_MAIN,
          "Grp 'sensors' does not exist. Switching to 'net_raw' instead." );
    }
  }

  return sensors_grent;
}

/*===========================================================================

  FUNCTION:   sns_main_drop_root

  ===========================================================================*/
/*!
  @brief Drops root permissions, and sets uid/gid to SNS_USERNAME/SNS_GROUPNAME

  @return
  TRUE: Error
  FALSE: No error

*/
/*=========================================================================*/
static bool
sns_main_drop_root( void )
{
  struct passwd *sensors_pwent; /* Sensors password entry */
  struct group *sensors_grent; /* Sensors group entry */

  bool rv = false; /* return value */
  int error;

  if( getuid() != 0 ) {
    /* Not root. No need to drop root privs */
    rv = false;
  } else {
    sensors_pwent = sns_main_get_usr();
    sensors_grent = sns_main_get_grp();

    if( NULL == sensors_pwent || NULL == sensors_grent ) {
      SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_MAIN,
          "Exiting! Unable to get user or group entry for 'sensors' or 'nobody'" );
      rv = true;
      return rv;
    }

    error = setresgid( sensors_grent->gr_gid,
                       sensors_grent->gr_gid,
                       sensors_grent->gr_gid );
    if( -1 == error ) {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN,
                                 "Exiting! Unable to set gid to %d",
                                 sensors_grent->gr_gid );
      rv = true;
    }
    error = setresuid( sensors_pwent->pw_uid,
                       sensors_pwent->pw_uid,
                       sensors_pwent->pw_uid );
    if( -1 == error ) {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN,
                                 "Exiting! Unable to set uid to %d",
                                 sensors_pwent->pw_uid );
      rv = true;
    }
  }
  return rv;
}


/*===========================================================================

  FUNCTION:   sns_main_setup_srv_sock

  ===========================================================================*/
/*!
  @brief Initializes sensor daemon server socket

  Creates the main server socket to listen for incomming connections.

  @param[o] srv_data: server data structure

  @return 0 if successful
*/
/*=========================================================================*/
static int
sns_main_setup_srv_sock( sns_main_data_s* srv_data )
{
  struct sockaddr_un ctl_addr;
  int error, optval;
  struct stat statbuf;

  /* Just in case, remove any old sockets which may be open */
  unlink(SENSOR_CTL_SOCKET);

  srv_data->ctl_fd = socket( AF_UNIX, SOCK_SEQPACKET, 0 );

  if( 0 > srv_data->ctl_fd ) {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN,
                               "socket error %d", errno );
    return 1;
  }

  // set SO_REUSEADDR on the control socket to true (1):
  optval = 1;
  setsockopt(srv_data->ctl_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  ctl_addr.sun_family = AF_UNIX;
  strlcpy(ctl_addr.sun_path, SENSOR_CTL_SOCKET, strlen(SENSOR_CTL_SOCKET)+1);
  error = bind(srv_data->ctl_fd, (struct sockaddr*)&ctl_addr,
               sizeof(sa_family_t)+strlen(SENSOR_CTL_SOCKET)+1 );
  if( 0 > error ) {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN, "bind error %d", errno );
    close( srv_data->ctl_fd );
    srv_data->ctl_fd = -1;
    return 0;
  }

  if ( stat( SENSOR_CTL_SOCKET, &statbuf ) == -1 ) {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN, "stat error %d", errno );
    return 1;
  }
  if ( S_ISSOCK(statbuf.st_mode) ) {
    error = chmod( SENSOR_CTL_SOCKET,
                   S_IRUSR | S_IWUSR | S_IXUSR |
                   S_IRGRP | S_IWGRP |
                   S_IROTH | S_IWOTH );
    if( 0 > error ) {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN, "chmod error %d", errno );
      return 1;
    }
  }
  error = listen(srv_data->ctl_fd, MAX_CONN_Q);
  if( 0 > error ) {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN,
                               "listen error %d", errno );
    return 1;
  }
  return 0;
}

/*===========================================================================

  FUNCTION:   sns_main_reopen_ctl_socket_if_needed

  ===========================================================================*/
static void
sns_main_reopen_ctl_socket_if_needed(sns_main_data_s *data)
{
  if( data->ctl_fd < 0 ) {
    sns_main_setup_srv_sock(data);
  }

  utime(SENSOR_CTL_SOCKET, NULL);
}

/*===========================================================================

  FUNCTION:   sns_main_setup

  ===========================================================================*/
/*!
  @brief Initializes sensor daemon.

  Creates the main server socket to listen for incomming connections.

  @return
  Pointer to the main data structure representing all clients.

*/
/*=========================================================================*/
static sns_main_data_s*
sns_main_setup( void )
{
  sns_main_data_s *srv_data;
  int error;

  srv_data = malloc( sizeof(*srv_data) );
  if( NULL == srv_data ) {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_MAIN, "malloc error" );
    return NULL;
  }

  memset( srv_data, 0, sizeof(*srv_data) );

  if( 0 != sns_main_setup_srv_sock( srv_data ) ) {
    free(srv_data);
    return NULL;
  }

  srv_data->enabled = true;
  srv_data->inotify_fd = inotify_init();
  if( srv_data->inotify_fd == -1 ) {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN,
                               "inotify_init error %d", errno );
    free(srv_data);
    return NULL;
  }

  error = pipe2( wakeup_pipe, O_NONBLOCK );
  if( error != 0 ) {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN,
                               "pipe2 error %d", errno );
    free(srv_data);
    return NULL;
  }

  return srv_data;
}

/*===========================================================================

  FUNCTION:   sns_main_validate_group_access

  ===========================================================================*/
static bool
sns_main_validate_group_access(struct ucred *cred)
{
  struct group *allowed_grent = NULL;
  char* allowed_groups[] = {SNS_GROUPNAME,
                            ROOT_GROUPNAME,
                            SYSTEM_GROUPNAME,
                            GPS_GROUPNAME,
                            CAMERA_GROUPNAME};
  unsigned int i;

  /*Check if the remote process is part of
    sensors/system/root/gps/camera group.
    Check for GPS_GROUPNAME and CAMERA_GROUPNAME should
    removed once a good way to enumerate the list of remote
    supplementary groups is devised.
    Eventually the check should be done on
    a. If the process is part of the allowed group or
    b. If the process has access to the allowed group(s)*/
  for( i=0; i< ARRAY_SIZE(allowed_groups); i++ )
  {
    if( ((NULL != (allowed_grent = getgrnam( allowed_groups[i] ))) &&
         ((int)cred->gid == (int)allowed_grent->gr_gid)
         )
       )
       return true;
  }

  return false;
}

/*===========================================================================

  FUNCTION:   sns_main_handle_ctl_srv_sock

  ===========================================================================*/
/*!
  @brief Handles incoming connections on the control server socket.

  This will add a new ctl connection to the list of ctl connections, call
  sensor1_open, and associate the new ctl connection with the sensor1
  client.

  @param[i] data: Pointer to data structure representing all connections

  @return
  SENSOR1_SUCCESS -- no error
  SENSOR1_ENOMEM -- too many clients
  SENSOR1_ENOTALLOWED -- Client has insufficient permissions
  Or other error.

*/
/*=========================================================================*/
static sensor1_error_e
sns_main_handle_ctl_srv_sock( sns_main_data_s *data )
{
  sensor1_error_e sns_err;
  sns_ctl_q_s *new_ctl = malloc(sizeof(sns_ctl_q_s));
  struct ucred cred;
  socklen_t len;
  int sns_ctl_unprivileged = 0;

  if( NULL == new_ctl ) {
    return SENSOR1_EFAILED;
  }
  new_ctl->next = NULL;
  new_ctl->privileged_user = false;
  new_ctl->sock_fd = accept( data->ctl_fd, NULL, NULL );
  if( -1 == new_ctl->sock_fd ) {
    free(new_ctl);
    return SENSOR1_EUNKNOWN;
  } else {
    sns_main_set_non_blocking( new_ctl->sock_fd );

    /* get peer credentials- uid, gid, pid */
    len = sizeof(struct ucred);
    if (getsockopt(new_ctl->sock_fd, SOL_SOCKET, SO_PEERCRED, &cred, &len) != -1) {
      SNS_PRINTF_STRING_LOW_3( SNS_DBG_MOD_APPS_MAIN,
                               "Client PID=%d GID=%d UID=%d",(int)cred.pid,(int)cred.gid,(int)cred.uid );
      if (sns_main_validate_group_access(&cred)) {
        new_ctl->privileged_user = true;
      } else {
        sns_ctl_q_s *ctl_p = data->ctl_p;
        while (NULL != ctl_p) {
          if (false == ctl_p->privileged_user) {
            sns_ctl_unprivileged++;
          }
          ctl_p = ctl_p->next;
        }
        if (sns_ctl_unprivileged > MAX_UNPRIVILEGED_CONN) {
          return SENSOR1_ENOTALLOWED;
        }
      }
    } else {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN,
                               "getsockopt returned error %d",errno );
      close(new_ctl->sock_fd);
      free(new_ctl);
      return SENSOR1_ENOTALLOWED;
    }

    sns_err = sensor1_open( &new_ctl->sensor1_hndl,
                            sns_main_notify_cb,
                            (intptr_t)new_ctl );
    if( SENSOR1_SUCCESS != sns_err ) {
      if(SENSOR1_ENOMEM == sns_err ) {
        sns_main_send_open_resp( new_ctl->sock_fd, SENSOR1_EWOULDBLOCK );
      }
      close(new_ctl->sock_fd);
      free(new_ctl);
    } else {
      SNS_PRINTF_STRING_LOW_2( SNS_DBG_MOD_APPS_MAIN,
                               "Incoming connection success! fd %d."
                               "sensor1 handle 0x%"PRIxPTR,
                               new_ctl->sock_fd,
                               (uintptr_t)new_ctl->sensor1_hndl );
      sns_main_add_ctl_cli( data, new_ctl );

      sns_main_send_open_resp( new_ctl->sock_fd, SENSOR1_SUCCESS );
    }
  }

  return sns_err;
}

/*===========================================================================

  FUNCTION:   sns_main_close_all_clients

  ===========================================================================*/
/*!
  @brief Closes all open clients.

  @param[i] data: Pointer to data structure representing all connections
*/
/*=========================================================================*/
static void
sns_main_close_all_clients( sns_main_data_s *data )
{
  sns_ctl_q_s *ctl_p;
  sns_ctl_q_s *tmp_p;

  ctl_p = data->ctl_p;

  while( ctl_p != NULL ) {
    close( ctl_p->sock_fd );
    tmp_p = ctl_p;
    ctl_p = ctl_p->next;

    sensor1_close( tmp_p->sensor1_hndl );
    free( tmp_p );
  }
  data->ctl_p = NULL;
}

/*===========================================================================

  FUNCTION:   sns_main_handle_ctl_sock

  ===========================================================================*/
/*!
  @brief Handles a connected ctl socket.

  This will handle all incoming data, which may be:
  - Sensor1 requests
  - Notification that this ctl socket has closed.

  @param[i] data: Pointer to data structure representing all connections

  @param[i/o] ctl_pp: Pointer to pointer to the data for the ctl connetion
              which has data.

  @param[i/o] prev_ctl_pp: Pointer to a pointer to the previous ctl
              connection in the linked list of ctl connections.

  @return
  true - error has occured
  false - no error

  @sideeffects
  If the ctl socket is closed, this will clean-up/close all associated data
  sockets, and remove this ctl socket from the linked list in 'data'.

  Advances the ctl_pp and prev_ctl_pp pointers to the next in the list if
  there is an error.

*/
/*=========================================================================*/
static bool
sns_main_handle_ctl_sock( sns_main_data_s *data,
                          sns_ctl_q_s **ctl_pp,
                          sns_ctl_q_s **prev_ctl_pp )
{
  bool error = false;
  sensor1_error_e sns_err;
  libsensor_ctl_write_s *ctl_pkt_p;
  ssize_t num_bytes;
  struct msghdr msg;
  struct iovec  msg_vec;
  char ccmsg[CMSG_SPACE(sizeof(int))];
  qmi_idl_service_object_type service = NULL;
  uint32_t err;

  ctl_pkt_p = malloc( sizeof(*ctl_pkt_p) + SENSOR_MAX_MSG_SIZE );
  msg_vec.iov_base = ctl_pkt_p;
  msg_vec.iov_len = sizeof(*ctl_pkt_p) + SENSOR_MAX_MSG_SIZE;

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &msg_vec;
  msg.msg_iovlen = 1;
  msg.msg_control = ccmsg;
  msg.msg_controllen = sizeof(ccmsg);
  msg.msg_flags = 0;

  if( NULL == ctl_pkt_p ) {
    pthread_mutex_lock( &sns_main_data_mutex );
    if( NULL == (*prev_ctl_pp) ) {
      data->ctl_p = (*ctl_pp)->next;
      (*ctl_pp) = data->ctl_p;
    } else {
      (*prev_ctl_pp)->next = (*ctl_pp);
      (*ctl_pp) = (*ctl_pp)->next;
    }
    pthread_mutex_unlock( &sns_main_data_mutex );
    return(error);
  }
  num_bytes = recvmsg( (*ctl_pp)->sock_fd, &msg, 0 );

  if( (ssize_t)(sizeof(*ctl_pkt_p)-1) > num_bytes ) {
    sns_ctl_q_s *tmp_p = *ctl_pp;

    pthread_mutex_lock( &sns_main_data_mutex );
    /* Error reading data. Close the ctl socket */
    SNS_PRINTF_STRING_LOW_1( SNS_DBG_MOD_APPS_MAIN,
                             "Error reading ctl socket %d; Closing.",
                             (*ctl_pp)->sock_fd );
    close( (*ctl_pp)->sock_fd );

    if( NULL == (*prev_ctl_pp) ) {
      data->ctl_p = (*ctl_pp)->next;
      (*ctl_pp) = data->ctl_p;
    } else {
      (*prev_ctl_pp)->next = (*ctl_pp)->next;
      (*ctl_pp) = (*ctl_pp)->next;
    }
    pthread_mutex_unlock( &sns_main_data_mutex );

    sensor1_close( tmp_p->sensor1_hndl );
    sns_main_reopen_ctl_socket_if_needed(data);

    free( tmp_p );
    error = true;
  } else {
    void                *payload = NULL;
    uint32_t             decoded_ctype_sz;
    sensor1_msg_header_s msg_hdr;
    if( ctl_pkt_p->socket_cmd == LIBSENSOR_SOCKET_CMD_WRITE_RAW ||
        ctl_pkt_p->socket_cmd == LIBSENSOR_SOCKET_CMD_WRITE_QMI ) {

      SNS_PRINTF_STRING_LOW_1( SNS_DBG_MOD_APPS_MAIN,
                               "Incoming CMD_WRITE on ctl fd %d",
                               (*ctl_pp)->sock_fd );

      if( ctl_pkt_p->socket_cmd == LIBSENSOR_SOCKET_CMD_WRITE_QMI ) {
        service = sns_smr_get_svc_obj( ctl_pkt_p->svc_num );
        decoded_ctype_sz = 0;
        err = qmi_idl_get_message_c_struct_len( service,
                                                QMI_IDL_REQUEST,
                                                ctl_pkt_p->msg_id,
                                                &decoded_ctype_sz );
      } else {
        /* Reading a raw (non-QMI encoded) message.
           Set the decoded ctype size to the size of the recived data,
           and set a QMI error */
        decoded_ctype_sz = num_bytes - sizeof(*ctl_pkt_p) + 1;
        err = QMI_IDL_LIB_UNRECOGNIZED_SERVICE_VERSION;
      }

      if( QMI_NO_ERR != err &&
          ctl_pkt_p->socket_cmd == LIBSENSOR_SOCKET_CMD_WRITE_QMI ) {
        /* There is a QMI error, and QMI is enabled for this packet */
        SNS_PRINTF_STRING_ERROR_2( SNS_DBG_MOD_APPS_MAIN,
                                   "QMI get c struct len error %d on incoming "
                                   "CMD_WRITE on ctl fd %d",
                                   err,
                                   (*ctl_pp)->sock_fd );
        sns_main_send_err_resp( (*ctl_pp)->sock_fd,
                                ctl_pkt_p,
                                SENSOR1_EBUFFER );

      } else {
        /* No QMI error, or QMI not enabled */
        sensor1_alloc_msg_buf( (*ctl_pp)->sensor1_hndl,
                               decoded_ctype_sz,
                               &payload );

        if( NULL == payload ) {
          sns_main_send_err_resp( (*ctl_pp)->sock_fd,
                                  ctl_pkt_p,
                                  SENSOR1_ENOMEM );
        } else {
            /* get options about current socket FD to inspect access privileges */
            /* depends on system call to getsockopt for each inbound message */
            struct ucred cred;
            socklen_t len = sizeof(struct ucred);
            if (getsockopt((*ctl_pp)->sock_fd, SOL_SOCKET, SO_PEERCRED, &cred, &len) != -1) {
              if (!sns_main_validate_group_access(&cred)) {
                if ((MAX_SDK_WHITELIST - 1) < ctl_pkt_p->svc_num || false == sdk_white_list[ctl_pkt_p->svc_num]) {
#ifdef SNS_LA_SIM
                  SNS_PRINTF_STRING_ERROR_2( SNS_DBG_MOD_APPS_MAIN,"Group Access for pid %d gid %d would not qualify on target",(int)cred.pid,(int)cred.gid );
#else
                  SNS_PRINTF_STRING_ERROR_2( SNS_DBG_MOD_APPS_MAIN,"No Group Access for pid %d gid %d. Disconnecting",(int)cred.pid,(int)cred.gid );
#endif /* SNS_LA_SIM */
                  error = true;
                }
              }
            } else {
              SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN,"getsockopt returned error %d",errno );
              error = true;
            }

          msg_hdr.service_number = ctl_pkt_p->svc_num;
          msg_hdr.msg_id = ctl_pkt_p->msg_id;
          msg_hdr.msg_size = decoded_ctype_sz;
          msg_hdr.txn_id = ctl_pkt_p->txn_id;
          if( ctl_pkt_p->socket_cmd == LIBSENSOR_SOCKET_CMD_WRITE_RAW ) {
            memcpy( payload, ctl_pkt_p->data, decoded_ctype_sz );
            pthread_mutex_lock( &sns_main_data_mutex );
            (*ctl_pp)->use_qmi = false;
            pthread_mutex_unlock( &sns_main_data_mutex );
          } else if( 0 != decoded_ctype_sz &&
                     ctl_pkt_p->socket_cmd == LIBSENSOR_SOCKET_CMD_WRITE_QMI ){
            err = qmi_idl_message_decode( service,
                                          QMI_IDL_REQUEST,
                                          ctl_pkt_p->msg_id,
                                          ctl_pkt_p->data,
                                          num_bytes - sizeof(*ctl_pkt_p) + 1,
                                          payload,
                                          decoded_ctype_sz );
            if( QMI_NO_ERR != err ) {
              error = true;
              SNS_PRINTF_STRING_ERROR_2( SNS_DBG_MOD_APPS_MAIN,
                                         "QMI decode error %d on incoming "
                                         "CMD_WRITE on ctl fd %d",
                                         err,
                                         (*ctl_pp)->sock_fd );
              sns_main_send_err_resp( (*ctl_pp)->sock_fd,
                                      ctl_pkt_p,
                                      SENSOR1_EBUFFER );
              sensor1_free_msg_buf( (*ctl_pp)->sensor1_hndl, payload );
            }
          }

          if( !error ) {
            sns_err = sensor1_write( (*ctl_pp)->sensor1_hndl, &msg_hdr,
                  payload );
            if( SENSOR1_SUCCESS != sns_err ) {
              sns_main_send_err_resp( (*ctl_pp)->sock_fd,
                                      ctl_pkt_p,
                                      sns_err );
              sensor1_free_msg_buf( (*ctl_pp)->sensor1_hndl, payload );
              SNS_PRINTF_STRING_ERROR_3( SNS_DBG_MOD_APPS_MAIN,
                                         "Error writing request. sns_err %d "
                                         "error %d qmi_err %d",
                                         sns_err, error, err);
            }
          }

          if( !error && SENSOR1_SUCCESS == sns_err ) {
            pthread_mutex_lock( &sns_main_data_mutex );
            (*ctl_pp)->outstanding_req++;
            pthread_mutex_unlock( &sns_main_data_mutex );
          }
        }
      }
      *prev_ctl_pp = *ctl_pp;
      *ctl_pp = (*ctl_pp)->next;
    } else if( ctl_pkt_p->socket_cmd == LIBSENSOR_SOCKET_CMD_DISCON_CTL ) {
      sns_ctl_q_s *tmp_ctl_p = *ctl_pp;

      pthread_mutex_lock( &sns_main_data_mutex );
      SNS_PRINTF_STRING_LOW_1( SNS_DBG_MOD_APPS_MAIN,
                               "Incoming CMD_DISCON_CTL on ctl fd %d. Closing",
                               (*ctl_pp)->sock_fd );
      close((*ctl_pp)->sock_fd);
      (*ctl_pp)->sock_fd = -1;
      if( NULL == (*prev_ctl_pp) ) {
        data->ctl_p = (*ctl_pp)->next;
        (*ctl_pp) = data->ctl_p;
      } else {
        (*prev_ctl_pp)->next = (*ctl_pp)->next;
        (*ctl_pp) = (*ctl_pp)->next;
      }
      pthread_mutex_unlock( &sns_main_data_mutex );
      sensor1_close( tmp_ctl_p->sensor1_hndl );
      sns_main_reopen_ctl_socket_if_needed(data);

      free( tmp_ctl_p );
      error = true;
    }
  }
  if( NULL != ctl_pkt_p ) {
    free( ctl_pkt_p );
  }

  return error;
}

/*===========================================================================

  FUNCTION:   sns_main_sock_handle

  ===========================================================================*/
/*!
  @brief Handles all incoming socket communication. Blocks until a socket
  has an event to handle.

  @param[i] data: Pointer to data structure representing all connections

  @return
  true - error has occured
  false - no error
*/
/*=========================================================================*/
static bool
sns_main_sock_handle( sns_main_data_s *data )
{
  fd_set read_fd;
  fd_set error_fd;
  int max_fd;
  sns_ctl_q_s *ctl_p;
  sns_ctl_q_s *prev_ctl_p;
  sensor1_error_e sns_err;
  int sel_err;
  bool error = true;

  max_fd = 0;
  FD_ZERO( &read_fd );
  FD_ZERO( &error_fd );

  FD_ADD( data->inotify_fd, &read_fd, max_fd );
  FD_ADD( wakeup_pipe[0], &read_fd, max_fd );
  if( data->enabled ) {
    /* Make sure server socket is opened */
    if( data->ctl_fd == -1 ) {
      if( sns_main_setup_srv_sock( data ) != 0 ) {
        return true;
      }
    }
    if( data->ctl_fd >= 0 ) {
      FD_ADD( data->ctl_fd, &read_fd, max_fd );
    }
    for( ctl_p = data->ctl_p; NULL != ctl_p; ctl_p = ctl_p->next ) {
      FD_ADD( ctl_p->sock_fd, &read_fd, max_fd );
    }
  } else {
    /* Not enabled. Make sure server socket is closed */
    if( data->ctl_fd >= 0 ) {
      unlink(SENSOR_CTL_SOCKET);
      close( data->ctl_fd );
    }
    data->ctl_fd = -1;
  }

  error_fd = read_fd;

  max_fd++;

  errno = 0;
  RETRY_ON_EINTR(sel_err, select( max_fd, &read_fd, (fd_set *)NULL, &error_fd,
                                  (struct timeval*) NULL));
  if (sel_err < 0 ) {
    return error;
  }

  if( FD_ISSET( wakeup_pipe[0], &read_fd ) ) {
    return error;
  }

  if( FD_ISSET( data->inotify_fd, &read_fd ) ) {
    /* Change in settings file */
    char buf[500];
    struct inotify_event *evt = (struct inotify_event *)buf;
    read(data->inotify_fd, evt, 500);
    if(evt->mask & IN_IGNORED) {
      /* Previous watch was removed. Nothing to do here */
    } else if(evt->len == 0 ||
              ( (evt->mask & IN_CREATE) &&
                (0 == strncmp( evt->name, SENSORS_SETTINGS_FILE, evt->len)))) {
      inotify_rm_watch( data->inotify_fd, evt->wd );
      sns_main_check_enabled( data );
    }
  }

  /* Check to make sure sensors is enabled */
  if( !data->enabled ) {
    sns_main_close_all_clients( data );
    return false;
  }

  if( FD_ISSET( data->ctl_fd, &read_fd ) ) {
    /* incoming connection on ctl socket */
    sns_err =  sns_main_handle_ctl_srv_sock( data );

    if( SENSOR1_ENOMEM != sns_err &&
        SENSOR1_ENOTALLOWED != sns_err &&
        SENSOR1_SUCCESS != sns_err ) {
      return true;
    }
  }

  prev_ctl_p = NULL;
  ctl_p = data->ctl_p;
  while( NULL != ctl_p ) {
    bool ctl_sock_err = false;
    if( FD_ISSET( ctl_p->sock_fd, &read_fd ) ) {
      /* Incoming data on a control socket */
      ctl_sock_err = sns_main_handle_ctl_sock( data, &ctl_p, &prev_ctl_p );
    } else {
      prev_ctl_p = ctl_p;
      ctl_p = ctl_p->next;
    }
  }
  error = false;
  return error;
}

/*===========================================================================

  FUNCTION:   sns_main_exit

  ===========================================================================*/
/*!
  @brief Causes the main thread to exit.
*/
/*=========================================================================*/
void sns_main_exit( void )
{
  char wr_data = 1;
  SNS_PRINTF_STRING_FATAL_0( SNS_DBG_MOD_APPS_MAIN,
                             "sns_main_exit called");
  write( wakeup_pipe[1], &wr_data, 1 );
  if( initialized == false ) {
    abort();
  }
}

/*===========================================================================

  FUNCTION:   sns_smgr_svc_error_cb

  ===========================================================================*/
/*!
  @brief Handles the QCCI error events, this is called by the
  QCCI infrastructure when the service is no longer available.

  Notified when SMGR service disconnected to detect
  ADSP going down. Exit the Sensors Daemon upon ADSP reset

  @param[i] user_handle: Unused
  @param[i] error: Unused
  @param[i] err_cb_data: Unused

  @return void
*/
/*=========================================================================*/
static void
sns_smgr_svc_error_cb
(
  qmi_client_type user_handle,
  qmi_client_error_type error,
  void *err_cb_data
)
{
  UNREFERENCED_PARAMETER(user_handle);
  UNREFERENCED_PARAMETER(err_cb_data);

  SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN,
                             "SMGR service error %d received",
                             error );

  if( error == QMI_SERVICE_ERR )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_MAIN,
                             "ADSP reset. Exit Sensors Daemon!" );
    sns_main_exit();
  }
}

/*===========================================================================

  FUNCTION:   sns_handle_adsp_restart

  ===========================================================================*/
/*!
  @brief Handles ADSP restart for SSR.

  @return void
*/
/*=========================================================================*/
static void*
sns_handle_adsp_restart( void *unused )
{
  qmi_client_error_type qmi_err;
  qmi_service_info service_info;
  qmi_client_type smgr_client;
  qmi_idl_service_object_type service;
  qmi_client_type notifier_handle;
  qmi_cci_os_signal_type os_params;

  UNREFERENCED_PARAMETER(unused);

  SNS_PRINTF_STRING_MEDIUM_0( SNS_DBG_MOD_APPS_MAIN,
                              "Register for SMRG service");
  service = sns_smr_get_svc_obj( SNS_SMGR_SVC_ID_V01 );
  if( service == NULL )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_MAIN,
                               "Error: SMGR service is NULL" );
    return (void*)0;
  }

  qmi_err = qmi_client_notifier_init( service, &os_params, &notifier_handle );
  if( QMI_NO_ERR != qmi_err )
  {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN,
                               "qmi_client_notifier_init for SMGR failed, err=%d",
                               qmi_err );
    return (void*)0;
  }
  SNS_PRINTF_STRING_MEDIUM_0( SNS_DBG_MOD_APPS_MAIN,
                              "Waiting for SMGR service up" );
  QMI_CCI_OS_SIGNAL_WAIT( &os_params, SNS_SMGR_SVC_TIMEOUT_MS );
  if( QMI_CCI_OS_SIGNAL_TIMED_OUT(&os_params) )
  {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_MAIN,
                               "Timeout waiting for SMGR service. Exit sensors daemon!" );
    sns_main_exit();
  }
  else
  {
    SNS_PRINTF_STRING_MEDIUM_0( SNS_DBG_MOD_APPS_MAIN,
                                "Get SMGR servive info" );
    qmi_err = qmi_client_get_any_service( service, &service_info );
    if( qmi_err != QMI_NO_ERR )
    {
      SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN,
                                 "qmi_client_get_any_service for SMGR failed, err=%d",
                                 qmi_err );
    }
    else
    {
      SNS_PRINTF_STRING_MEDIUM_0( SNS_DBG_MOD_APPS_MAIN,
                                  "Initialize client for SMRG" );
      qmi_err = qmi_client_init( &service_info, service, NULL, NULL, NULL, &smgr_client );
      if( QMI_NO_ERR != qmi_err )
      {
        SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN,
                                   "qmi_client_init for SMGR failed, err=%d",
                                   qmi_err);
      }
      else
      {
        SNS_PRINTF_STRING_MEDIUM_0( SNS_DBG_MOD_APPS_MAIN,
                                    "Register for SMGR error notification" );
        qmi_err = qmi_client_register_error_cb( smgr_client, sns_smgr_svc_error_cb, NULL );
        if( QMI_NO_ERR != qmi_err )
        {
          SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN,
                                     "qmi_client_register_error_cb for SMGR failed, err=%d",
                                     qmi_err );
          qmi_client_release( smgr_client );
        }
      }
    }
  }

  qmi_client_release( notifier_handle );

  return (void*)0;
}

/*===========================================================================

  FUNCTION:   sns_monitor_adsp_restart

  ===========================================================================*/
/*!
  @brief Start a thread to monitor ADSP restart.

  @return 0
*/
/*=========================================================================*/
static int
sns_monitor_adsp_restart()
{
  pthread_t thread_id;
  pthread_attr_t attr;

  SNS_PRINTF_STRING_MEDIUM_0( SNS_DBG_MOD_APPS_MAIN,
                           "Start thread to monitor ADSP restart");
  pthread_attr_init( &attr );
  pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
  pthread_create( &thread_id,  &attr,
                  sns_handle_adsp_restart,  NULL );
  pthread_attr_destroy( &attr );

  return 0;
}

/*===========================================================================

  FUNCTION:   main

  ===========================================================================*/
/*!
  @brief "Main" function for the sensor daemon process. Handles starting
  the process, initializing sockets and sensor1 API, and blocking for
  incoming data on sockets.

  @param[i] argc: Count of arguments on the command line.

  @param[i] argv: Array of strings contaning command line arguments.

  @return
  0 - no error
*/
/*=========================================================================*/
int
main( int argc, char *argv[] )
{
  sensor1_error_e sns_err;
  sns_main_data_s *srv_data;
  bool error = FALSE;
  pthread_mutexattr_t attr;

  if( 'd' == getopt( argc, argv, "d" ) ) {
    /* Put ourselves in a background daemon process */
    sns_main_daemonize();
  } else {
    printf("Use option \"-d\" to run as background process.\n");
    printf("Running in foreground...\n");
  }

  /* Start up the sensor library */
  sns_err = sensor1_init();
  if( SENSOR1_SUCCESS != sns_err ) {
    SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN,
                               "Exiting! sensor1_init failed with %d",
                               sns_err );
    exit(6);
  }

  /* Initialize mutex */
  pthread_mutexattr_init( &attr );
  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );
  pthread_mutex_init( &sns_main_data_mutex, &attr );

  /* Initialize server */
  srv_data = sns_main_setup();

  if( NULL == srv_data ) {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_MAIN,
                               "Exiting! Can't setup server socket" );
    exit(4);
  }

  /* Check to ensure sensors are enabled */
  sns_main_check_enabled( srv_data );

  /* Drop all root privs */
  error = sns_main_drop_root();
  if( error != false ) {
    SNS_PRINTF_STRING_ERROR_0( SNS_DBG_MOD_APPS_MAIN,
                               "Error dropping root privs. Exiting" );
    exit(5);
  }

  initialized = true;

  sns_monitor_adsp_restart();

  utime(SENSOR_CTL_SOCKET, NULL);

  while( !error ) {
    error = sns_main_sock_handle( srv_data );
  }

  SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_APPS_MAIN, "Exiting! Err %d",
                             error );

  return 0;
}

/*===========================================================================

  FUNCTION:   smr_print_heap_summary

===========================================================================*/
/*!
  @brief This function prints the summary of the heap memory

  @detail

  @param[i] none

  @return
   None
*/
/*=========================================================================*/
#ifdef USE_NATIVE_MALLOC
void sns_print_heap_summary (void)
{
  return;
}
#else
#include "oi_support_init.h"
#define NUM_BLOCK 10
extern uint32_t PoolFreeCnt[];
extern const OI_MEMMGR_POOL_CONFIG memmgrPoolTable[];

void sns_print_heap_summary (void)
{
  static bool     is_block_counted = false;
  static uint32_t tot_block_cnt = 0;
  uint32_t        i, tot_free_cnt = 0;

  SNS_PRINTF_STRING_FATAL_0(SNS_DBG_MOD_APPS_SMR,
                            "prints heap_summary");
  if ( !is_block_counted )
  {
    for ( i = 0; i < NUM_BLOCK; i++)
    {
      tot_block_cnt += memmgrPoolTable[i].numBlocks;
    }
    is_block_counted = true;
  }
  for ( i = 0; i < NUM_BLOCK; i++)
  {
    tot_free_cnt += PoolFreeCnt[i];
    SNS_PRINTF_STRING_FATAL_3(SNS_DBG_MOD_APPS_SMR,
                              "Free Cnt[%5d] = %d/%d",
                              memmgrPoolTable[i].blockSize, PoolFreeCnt[i], memmgrPoolTable[i].numBlocks);
  }
  SNS_PRINTF_STRING_FATAL_2(SNS_DBG_MOD_APPS_SMR,
                            "Total Free Cnt  = %d/%d", tot_free_cnt, tot_block_cnt);
}
#endif
