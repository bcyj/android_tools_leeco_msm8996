/*===========================================================================
                           usf_unix_domain_socket.cpp

DESCRIPTION: This file implements the unix domain socket server.

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#define LOG_TAG "usf_unix_domain_socket"

/*-----------------------------------------------------------------------------
  Includes
-----------------------------------------------------------------------------*/

#include "usf_unix_domain_socket.h"
#include "usf_epos_defs.h"
#include "usf_log.h"
#include <math.h>
#include <sys/stat.h>

/*-----------------------------------------------------------------------------
  Constants
-----------------------------------------------------------------------------*/
static const int MAX_CONTROL_MSG_LENGTH = 4096;
static const int CONTROL_MSG_ID_LOAD_CONFIG = 1;
static const int CONTROL_MSG_ID_ACCELEROMETER_DATA = 2;

static const int DC_ERASER_BUTTON_INDEX_DISABLED = -1;
static const int DC_ERASER_BUTTON_INDEX_BUTTON1 = 0;
static const int DC_ERASER_BUTTON_INDEX_BUTTON2 = 1;
static const int DC_ERASER_BUTTON_MODE_HOLD = 0;
static const int DC_ERASER_BUTTON_MODE_TOGGLE = 1;

/*-----------------------------------------------------------------------------
  Functions
-----------------------------------------------------------------------------*/

/*============================================================================
  FUNCTION:  UnSocket::UnSocket
============================================================================*/
/**
  C'tor for unix domain socket class,takes as argument the path for the socket
*/
UnSocket::UnSocket(const char *sock_path,
                   void (*connected_cb)())
:m_thread_id(-1)
{
  LOGD("%s: got %s as socket path",
       __FUNCTION__,
       sock_path);
  // Create, initialize semaphore
  if (0 > sem_init(&m_sem,
                   0,
                   0))
  {
    LOGE("%s: semaphor init failed",
         __FUNCTION__);
    return;
  }

  if (NULL == sock_path)
  {
    LOGE("%s: socket_path is null",
         __FUNCTION__);
    // TODO: Check if we need to throw an exception here
    return;
  }

  m_address.sun_family = AF_UNIX;
  // sock_path length is already checked in another code
  strlcpy(m_address.sun_path,
          sock_path,
          sizeof(m_address.sun_path));

  // Unlink the name so that the bind won't fail
  unlink(m_address.sun_path);

  m_connected_cb = connected_cb;
}

/*============================================================================
  FUNCTION:  UnSocket::~UnSocket
============================================================================*/
/**
  D'tor for unix domain socket class
*/
UnSocket::~UnSocket()
{
  sem_destroy(&m_sem);
  if (0 <= m_socket_fd)
    close(m_socket_fd);
}

/*============================================================================
  FUNCTION:  global_thread_func
============================================================================*/
/**
  This function given as the helper thread global function, it forwards the
  call to the thread_func() member function in the unix domain socket class.
*/
static void *global_thread_func(void *param)
{
  // The param sent is the class, we should get a pointer for it, then run its
  // thread_func()
  UnSocket *un_socket = (UnSocket *) param;
  sem_post(un_socket->get_sem());
  un_socket->thread_func();
  return NULL;
}

/*============================================================================
  FUNCTION:  interrupt_func
============================================================================*/
/**
  This function is a signal handling function which closes the thread when
  called.
*/
static void interrupt_func(int sig)
{
  pthread_exit(NULL);
}

/*============================================================================
  FUNCTION:  UnSocket::start
============================================================================*/
/**
  Open the socket, bind to it, then run the socket helper thread.
*/
int UnSocket::start()
{
  // Initialize signals
  struct sigaction act;
  act.sa_handler = interrupt_func;
  sigaction(SIGINT,
            &act,
            NULL);

  int len;
  pthread_attr_t attr;

  m_socket_fd = socket(AF_UNIX,
                       SOCK_STREAM,
                       0);
  if (m_socket_fd < 0)
  {
    LOGE("%s: socket() failed, %s",
         __FUNCTION__,
         strerror(errno));
    return -1;
  }
  // The length of the packet.
  len = sizeof(m_address.sun_family) + strlen(m_address.sun_path);

  if (bind(m_socket_fd,
           (struct sockaddr *)&m_address,
           len) < 0)
  {
    LOGE("%s: bind() failed, %s",
         __FUNCTION__,
         strerror(errno));
    return -1;
  }

  // Chnage the socket permissions. Only the owner has permissions
  // to read from and write to the socket.
  chmod(m_address.sun_path, 0600);

  // Run the helper thread
  pthread_attr_init(&attr);
  return pthread_create((pthread_t*)&m_thread_id,
                        &attr,
                        global_thread_func,
                        this);
}

/*============================================================================
  FUNCTION:  UnSocket::get_socket_fd
============================================================================*/
/**
  Returns the socket file descriptor number.
*/
int UnSocket::get_scoket_fd()
{
  return m_socket_fd;
}

/*============================================================================
  FUNCTION:  UnSocket::get_sem
============================================================================*/
/**
  Returns the semaphor's address.
*/
sem_t *UnSocket::get_sem()
{
  return &m_sem;
}

/*============================================================================
  FUNCTION:  ControlUnSocket::ControlUnSocket
============================================================================*/
/**
  C'tor for the control unix domain socket, it takes as an argument the socket
  path and a struct which has the pointers to the configuration parameters
  so that we would be able to change them dynamicly.
*/
ControlUnSocket::ControlUnSocket(const char *sock_path,
                                 struct dynamic_config_t *dconfig,
                                 void (*connected_cb)()):
UnSocket(sock_path, connected_cb),
m_client_fd(-1)
{
  // This activates default copy C'tor of struct dynamic_config_t
  m_dconfig = dconfig;
}

/*============================================================================
  FUNCTION:  ControlUnSocket::thread_func
============================================================================*/
/**
  The abstract function thread_func() implementation, helper thread runs this
  function after it gets created.
  The function waits for a client to connect, then waits for new configuration
  data to be sent, the client should send us 4 bytes each time it wants the
  configuration to be changed.
*/
int ControlUnSocket::thread_func()
{
  struct sockaddr_un faddress;
  int fromlen;
  LOGD("%s: Control unix domain socket, thread_func invoked",
       __FUNCTION__);
  // Listen first (that's how socket works)
  if (listen(m_socket_fd, 5) < 0) {
    LOGE("%s: listen() failed, %s",
         __FUNCTION__,
         strerror(errno));
    return -1;
  }
  fromlen = sizeof(faddress);
  // Waiting for a connection...
  if ((m_client_fd = accept(m_socket_fd,
                          (struct sockaddr*)&faddress,
                          &fromlen)) < 0) {
    LOGE("%s: accept() failed, %s",
         __FUNCTION__,
         strerror(errno));
    return -1;
  }

  if (NULL != m_connected_cb)
  {
    // Client is connected, invoking the callback function
    m_connected_cb();
  }
  //send_event(0, 0, 3); // A hackish way to report aliveness 3 - OFF, 0 - ACTIVE
  while (true)
  {
    int retval;
    struct {
      int msgId;
      int msgLength;
    } header;

    // Get input from client; first read header: id + length...
    retval = recv(m_client_fd,
                  (void*) &header,
                  sizeof(header),
                  MSG_WAITALL);
    if (retval == -1)
    {
      LOGE("%s: recv() failed, %s",
           __FUNCTION__,
           strerror(errno));
      return -1;
    }

    if (header.msgLength > MAX_CONTROL_MSG_LENGTH)
    {
      LOGE("%s: received too-long message: %d",
           __FUNCTION__,
           header.msgLength);
      // TODO: not sure how to recover at this point; payload still waiting in socket
      return -1;
    }

    // now read payload into buffer
    char buf[MAX_CONTROL_MSG_LENGTH];
    retval = recv(m_client_fd,
                  (void*) &buf,
                  header.msgLength,
                  MSG_WAITALL);
    if (retval == -1)
    {
      LOGE("%s: recv() failed, %s",
           __FUNCTION__,
           strerror(errno));
      return -1;
    }

    // switch on message ID, validate length and forward to processing function
    switch (header.msgId) {
    case CONTROL_MSG_ID_LOAD_CONFIG:
      if (header.msgLength != sizeof(config_parameters_t))
      {
        LOGE("%s: bad message length %d, expected message length: %d",
             __FUNCTION__,
             header.msgLength,
             sizeof(config_parameters_t));
        return -1;
      }
      if (!update_config((config_parameters_t*) buf))
      {
        return -1;
      }
      break;
    case CONTROL_MSG_ID_ACCELEROMETER_DATA:
      if (header.msgLength != sizeof(accelerometer_data_t))
      {
        LOGE("%s: bad message length %d, expected message length: %d",
             __FUNCTION__,
             header.msgLength,
             sizeof(accelerometer_data_t));
        return -1;
      }
      if (!update_smarter_stand_angle((accelerometer_data_t*)buf))
      {
        return -1;
      }
      break;
    default:
      LOGE("%s: unknown message: %d, length: %d",
           __FUNCTION__,
           header.msgId,
           header.msgLength);
      return -1;
    }
  }
  return 0;
}

/*============================================================================
  FUNCTION:  is_in_enum_range
============================================================================*/
/**
  Checks if the given enum value is in the given range
*/
bool is_in_enum_range(int value, int min_including, int max_excluding)
{
  return ((value >= min_including) &&
          (value < max_excluding));
}

/*============================================================================
  FUNCTION:  ControlUnSocket::validate_dynamic_config
============================================================================*/
/**
  Validates the dynamic config.
*/
bool ControlUnSocket::validate_dynamic_config()
{
  if (!is_in_enum_range(m_dconfig->off_screen_mode,
                       DC_OFF_SCREEN_MODE_DISABLED,
                       DC_OFF_SCREEN_MODE_NUM))
  {
    LOGW("Dynamic config validation failed for parameter: off_screen_mode value: %d",
         m_dconfig->off_screen_mode);
    return false;
  }

  if (!is_in_enum_range(m_dconfig->on_screen_destination,
                       DP_COORD_DESTINATION_MOTION_EVENT,
                       DP_COORD_DESTINATION_NUM))
  {
    LOGW("Dynamic config validation failed for parameter: on_screen_destination value: %d",
         m_dconfig->on_screen_destination);
    return false;
  }

  if (!is_in_enum_range(m_dconfig->off_screen_destination,
                       DP_COORD_DESTINATION_MOTION_EVENT,
                       DP_COORD_DESTINATION_NUM))
  {
    LOGW("Dynamic config validation failed for parameter: off_screen_destination value: %d",
         m_dconfig->off_screen_destination);
    return false;
  }

  if (!is_in_enum_range(m_dconfig->eraser_button_index,
                       ERASER_BUTTON_INDEX_BUTTON1,
                       ERASER_BUTTON_INDEX_NUM))
  {
    LOGW("Dynamic config validation failed for parameter: eraser_button_index value: %d",
         m_dconfig->eraser_button_index);
    return false;
  }

  if (!is_in_enum_range(m_dconfig->eraser_button_mode,
                       ERASER_BUTTON_MODE_DISABLED,
                       ERASER_BUTTON_NUM_MODES))
  {
    LOGW("Dynamic config validation failed for parameter: eraser_button_mode value: %d",
         m_dconfig->eraser_button_mode);
    return false;
  }

  return true;
}

/*============================================================================
  FUNCTION:  ControlUnSocket::print_config_message
============================================================================*/
/**
  Prints socket fields to the log.
*/
void ControlUnSocket::print_config_message(config_parameters_t *cp)
{
  LOGD("VERIFY off_screen_mode: %d", cp->off_screen_mode);
  LOGD("VERIFY touch_disable_threshold: %d", cp->touch_disable_threshold);
  LOGD("VERIFY power_save: %d", cp->power_save);
  LOGD("VERIFY on_screen_hover_max_range: %d", cp->on_screen_hover_max_range);
  LOGD("VERIFY on_screen_hover: %d", cp->on_screen_hover);
  LOGD("VERIFY showing_on_screen_hover_icon: %d", cp->show_on_screen_hover_icon);
  LOGD("VERIFY off_screen_hover_max_range: %d", cp->off_screen_hover_max_range);
  LOGD("VERIFY off_screen_hover: %d", cp->off_screen_hover);
  LOGD("VERIFY showing_off_screen_hover_icon: %d", cp->show_off_screen_hover_icon);
  LOGD("VERIFY local_smarter_stand_enable: %s",
       cp->smarter_stand_enable ? "true" : "false");
  LOGD("VERIFY local_off_screen_plane: %f %f %f %f %f %f %f %f %f",
       cp->origin[0],
       cp->origin[1],
       cp->origin[2],
       cp->end_x[0],
       cp->end_x[1],
       cp->end_x[2],
       cp->end_y[0],
       cp->end_y[1],
       cp->end_y[2]);
  LOGD("VERIFY on_screen_destination: %d", cp->on_screen_destination);
  LOGD("VERIFY on_screen_mapping: %d", cp->on_screen_mapping);
  LOGD("VERIFY off_screen_destination: %d", cp->off_screen_destination);
  LOGD("VERIFY off_screen_mapping: %d", cp->off_screen_mapping);
  LOGD("VERIFY erase_button_index: %d", cp->eraser_button_index);
  LOGD("VERIFY erase_button_mode: %d", cp->eraser_button_mode);
  LOGD("VERIFY send_all_events_to_side_channel: %d", cp->send_all_events_to_side_channel);
}

/*============================================================================
  FUNCTION:  ControlUnSocket::update_dynamic_config_params
============================================================================*/
/**
  Updates the dynamic config parameters
*/
void ControlUnSocket::update_dynamic_config_params(config_parameters_t *cp)
{
  m_dconfig->smarter_stand_enable = cp->smarter_stand_enable;
  m_dconfig->event_type = USF_TSC_EVENT;
  m_dconfig->off_screen_mode = (dc_off_screen_mode)cp->off_screen_mode;
  m_dconfig->on_screen_destination = (int)cp->on_screen_destination;
  m_dconfig->on_screen_mapping = cp->on_screen_mapping;
  m_dconfig->off_screen_destination = (int)cp->off_screen_destination;
  m_dconfig->off_screen_mapping = cp->off_screen_mapping;
  m_dconfig->send_all_events_to_side_channel = cp->send_all_events_to_side_channel;
  // Off screen coordinates
  Vector origin(cp->origin[0],
                cp->origin[1],
                cp->origin[2]);
  Vector end_x(cp->end_x[0],
               cp->end_x[1],
               cp->end_x[2]);
  Vector end_y(cp->end_y[0],
               cp->end_y[1],
               cp->end_y[2]);
  m_dconfig->off_screen_plane.origin = origin;
  m_dconfig->off_screen_plane.point_end_x = end_x;
  m_dconfig->off_screen_plane.point_end_y = end_y;
  m_dconfig->off_screen_plane.hover_max_range =
      (cp->off_screen_hover ? cp->off_screen_hover_max_range * MM_TO_LIB_UNIT : 0);

  m_dconfig->on_screen_hover_max_range =
      (cp->on_screen_hover ? cp->on_screen_hover_max_range * MM_TO_LIB_UNIT : 0);

  m_dconfig->show_on_screen_hover_icon = cp->show_on_screen_hover_icon;
  m_dconfig->show_off_screen_hover_icon = cp->show_off_screen_hover_icon;

  if (DC_ERASER_BUTTON_INDEX_DISABLED == cp->eraser_button_index) {
      m_dconfig->eraser_button_mode = ERASER_BUTTON_MODE_DISABLED;
  } else {
      switch (cp->eraser_button_index) {
      case DC_ERASER_BUTTON_INDEX_BUTTON1:
          m_dconfig->eraser_button_index = ERASER_BUTTON_INDEX_BUTTON1;
          break;
      case DC_ERASER_BUTTON_INDEX_BUTTON2:
          m_dconfig->eraser_button_index = ERASER_BUTTON_INDEX_BUTTON2;
          break;
      default:
          LOGE("%s: Got illegal eraser_button_index",
               __FUNCTION__);
      }
      switch (cp->eraser_button_mode) {
      case DC_ERASER_BUTTON_MODE_HOLD:
          m_dconfig->eraser_button_mode = ERASER_BUTTON_MODE_HOLD_TO_ERASE;
          break;
      case DC_ERASER_BUTTON_MODE_TOGGLE:
          m_dconfig->eraser_button_mode = ERASER_BUTTON_MODE_TOGGLE_ERASE;
          break;
      default:
          LOGE("%s: Got illegal eraser_button_mode",
               __FUNCTION__);
      }
  }

  m_dconfig->touch_disable_threshold =
    cp->touch_disable_threshold * MM_TO_LIB_UNIT;
}
/*============================================================================
  FUNCTION:  ControlUnSocket::update_smarter_stand_angle
============================================================================*/
/**
  Updates the smarter stand angle.
*/
bool ControlUnSocket::update_smarter_stand_angle(const accelerometer_data_t *ad)
{
  int rc = pthread_mutex_lock(&control_socket_mutex);
  if(rc)
  {
    LOGE("%s: pthread_mutex_lock failed: %d",
         __FUNCTION__,
         rc);
    return false;
  }

  m_dconfig->smarter_stand_angle = ad->angle;

  m_dconfig->angle_changed = true;

  rc = pthread_mutex_unlock(&control_socket_mutex);
  if(rc)
  {
    LOGE("%s: pthread_mutex_unlock failed: %d",
         __FUNCTION__,
         rc);
    return false;
  }
  return true;
}

/*============================================================================
  FUNCTION:  ControlUnSocket::update_config
============================================================================*/
/**
  Updates the dynamic config.
*/
bool ControlUnSocket::update_config(config_parameters_t *cp)
{
  int rc = pthread_mutex_lock(&control_socket_mutex);
  if(rc)
  {
    LOGE("%s: pthread_mutex_lock failed: %d",
         __FUNCTION__,
         rc);
    return false;
  }

  // NOTE: need "VERIFY" logs until we can read config (e.g. through binder)
  print_config_message(cp);

  // Assign necessary socket parameters to dynamic config
  update_dynamic_config_params(cp);

  if (validate_dynamic_config())
  {
    m_dconfig->config_changed = true;
    LOGD("GOT NEW CONFIG: on_screen_hover_range:%d, off_screen_hover_range: %d, off_screen_mode:%d, on_screen_destination:%d, off_screen_destination:%d, off screen origin(%lf,%lf,%lf), endX(%lf,%lf,%lf), endY(%lf,%lf,%lf), eraser_button_index:%d, eraser_button_mode:%d, show_on_screen_hover_icon:%d, show_off_screen_hover_icon:%d",
         m_dconfig->on_screen_hover_max_range, m_dconfig->off_screen_plane.hover_max_range,
         m_dconfig->off_screen_mode,
         m_dconfig->on_screen_destination,
         m_dconfig->off_screen_destination,
         m_dconfig->off_screen_plane.origin.get_element(X),
         m_dconfig->off_screen_plane.origin.get_element(Y),
         m_dconfig->off_screen_plane.origin.get_element(Z),
         m_dconfig->off_screen_plane.point_end_x.get_element(X),
         m_dconfig->off_screen_plane.point_end_x.get_element(Y),
         m_dconfig->off_screen_plane.point_end_x.get_element(Z),
         m_dconfig->off_screen_plane.point_end_y.get_element(X),
         m_dconfig->off_screen_plane.point_end_y.get_element(Y),
         m_dconfig->off_screen_plane.point_end_y.get_element(Z),
         m_dconfig->eraser_button_index,
         m_dconfig->eraser_button_mode,
         m_dconfig->show_on_screen_hover_icon,
         m_dconfig->show_off_screen_hover_icon);
  }
  else
  {
    LOGW("Dynamic config received is invalid.");
  }

  rc = pthread_mutex_unlock(&control_socket_mutex);
  if(rc)
  {
    LOGE("%s: pthread_mutex_unlock failed: %d",
         __FUNCTION__,
         rc);
    return false;
  }
  return true;
}

/*============================================================================
  FUNCTION:  ControlUnSocket::get_config
============================================================================*/
/**
  Returns the struct dynamic_config parameter of this class.
*/
struct dynamic_config_t *ControlUnSocket::get_config()
{
  return m_dconfig;
}

/*============================================================================
  FUNCTION:  ControlUnSocket::send_event
============================================================================*/
/**
  This function is used to send an event for the service, please Check:
  "/ultrasound/sdk/sdk/src/com/qti/snapdragon/digitalpen/util/DigitalPenEvent.java"
  to know more about the format of the events that should be sent.
*/
int ControlUnSocket::send_event(
  int event_num,
  int extra1,
  int extra2
)
{
  // TODO: Another way is to send a struct and using "sizeof" on it
  const int ARGS_NUM = 3;
  int buff[ARGS_NUM];
  int nbytes = 0;
  int len    = 0;

  if (0 > m_client_fd)
  {
    LOGD("%s: Socket is not open",
         __FUNCTION__);
    return -1;
  }

  buff[0] = event_num;
  buff[1] = extra1;
  buff[2] = extra2;

  len = sizeof(buff);
  while ((nbytes = write(m_client_fd,
                         (char *)buff + (sizeof(buff) - len),
                         len)) < len)
  {
    if (0 >= nbytes)
    {
      LOGE("%s: write() failed, %s",
           __FUNCTION__,
           strerror(errno));
      break;
    }
    len -= nbytes;
  }

  return nbytes;
}

ControlUnSocket::~ControlUnSocket()
{
  if (m_thread_id > 0)
  {
    // If thread got created, we should wait for him to start running, or else
    // pthread_kill will fail.
    sem_wait(&m_sem);
    pthread_kill(m_thread_id,
                 SIGINT);
    pthread_join(m_thread_id,
                 NULL);
  }
  if (0 <= m_client_fd)
  {
    close(m_client_fd);
  }
}

/*============================================================================
  FUNCTION:  DataUnSocket::DataUnSocket
============================================================================*/
/**
  C'tor for the data unix domain socket, takes as an argument the socket path.
  And a callback function to be called when the socket is connected.
*/
DataUnSocket::DataUnSocket(const char *sock_path,
                           void (*connected_cb)()):
 UnSocket(sock_path, connected_cb),
 m_client_fd(-1)
{
}

/*============================================================================
  FUNCTION:  DataUnSocket::~DataUnSocket
============================================================================*/
/**
  D'tor for the data unix domain socket.
*/
DataUnSocket::~DataUnSocket()
{
  if (m_thread_id > 0)
  {
    // If thread got created, we should wait for him to start running, or else
    // pthread_kill will fail.
    sem_wait(&m_sem);
    pthread_kill(m_thread_id,
                 SIGINT);
    pthread_join(m_thread_id,
                 NULL);
  }
  if (0 <= m_client_fd)
  {
    close(m_client_fd);
  }
}

/*============================================================================
  FUNCTION:  DataUnSocket::thread_func
============================================================================*/
/**
  The abstract function thread_func() implementation, helper thread runs this
  function after it gets created.
  Waits for a client to connect, saves his socket number, then exists.
*/
int DataUnSocket::thread_func()
{
  struct sockaddr_un faddress;
  int fromlen;
  // Listen first (that's how socket works)
  if (listen(m_socket_fd, 5) < 0) {
    LOGE("%s: listen() failed, %s",
         __FUNCTION__,
         strerror(errno));
    return -1;
  }
  fromlen = sizeof(faddress);
  // Waiting for a connection...
  if ((m_client_fd = accept(m_socket_fd,
                            (struct sockaddr*)&faddress,
                            &fromlen)) < 0) {
    LOGE("%s: accept() failed, %s",
         __FUNCTION__,
         strerror(errno));
    return -1;
  }

  LOGD("%s: Connection accepted is socket %d",
       __FUNCTION__,
       m_socket_fd);

  if (NULL != m_connected_cb)
  {
    // Client is connected, invoking the callback function
    m_connected_cb();
  }

  return 0;
}

/*============================================================================
  FUNCTION:  send
============================================================================*/
/**
  Writes the given byte array to the socket.
  Returns number of bytes written, or a negative value on faliure.
*/
static inline int send(
  int   client_fd,
  char *arr,
  int   size
)
{
  int nbytes = 0;
  int bytes_left = size;

  if (0 > client_fd)
  {
    return -1;
  }

  while ((nbytes = write(client_fd,
                         arr + (size - bytes_left),
                         bytes_left)) < bytes_left)
  {
    if (0 >= nbytes)
    {
      LOGE("%s: write() failed, %s",
           __FUNCTION__,
           strerror(errno));
      break;
    }
    // 'nbytes' were written, so we subtract them from 'bytes_left'
    bytes_left -= nbytes;
  }
  return nbytes;
}

/*============================================================================
  FUNCTION:  DataUnSocket::send_epos_point
============================================================================*/
/**
  Sends an EPOS point event to the client connected on the data socket.
*/
int DataUnSocket::send_epos_point(
  int32_t x,
  int32_t y,
  int32_t z,
  int32_t tilt_x,
  int32_t tilt_y,
  int32_t tilt_z,
  int32_t pressure,
  int32_t type,
  int32_t region
)
{
  int32_t buff[12];

  buff[0] = x;
  buff[1] = y;
  buff[2] = z;
  buff[3] = tilt_x;
  buff[4] = tilt_y;
  buff[5] = tilt_z;
  buff[6] = pressure;
  buff[7] = !!(type & EPOS_TYPE_PEN_DOWN_BIT);
  buff[8] = !!(type & COORD_SW2);
  buff[9] = !!(type & COORD_SW1);
  buff[10] = !!(type & COORD_SW0);
  buff[11] = region;

  return send(m_client_fd,
              (char*)buff,
              sizeof(buff));

  return 0;
}

/*============================================================================
  FUNCTION:  DataUnSocket::send_proximity_event
============================================================================*/
/**
  Sends a proximity event to the client connected on the data socket.
*/
int DataUnSocket::send_proximity_event(
  int timestamp,
  int seq_num,
  int result
)
{
  int buff[3];

  buff[0] = timestamp;
  buff[1] = seq_num;
  buff[2] = result;

  return send(m_client_fd,
              (char*)buff,
              sizeof(buff));
}

/*============================================================================
  FUNCTION:  DataUnSocket::send_pairing_event
============================================================================*/
/**
  Sends a pairing event to the client connected on the data socket.
*/
int DataUnSocket::send_pairing_event(
  int status,
  int penId
)
{
  LOGD("%s: sending pairing event: %d , %d",
       __FUNCTION__,
       status,
       penId);

  int buff[2];

  buff[0] = status;
  buff[1] = penId;

  return send(m_client_fd,
              (char*)buff,
              sizeof(buff));
}

/*============================================================================
  FUNCTION:  DataUnSocket::send_sw_calib_event
============================================================================*/
/**
  Sends a pairing event to the client connected on the data socket.
*/
int DataUnSocket::send_sw_calib_event(int status,
int x,
int y)
{
  int buff[3];

  buff[0] = status;
  buff[1] = x;
  buff[2] = y;

  return send(m_client_fd,
              (char*)buff,
              sizeof(buff));
}

/*============================================================================
  FUNCTION:  DataUnSocket::send_sw_calib_tester_event
============================================================================*/
/**
  Sends a statictics event to the client connected on the data socket.
*/
int DataUnSocket::send_sw_calib_tester_event(int msg_type,
                                             int mic_num,
                                             double val) {
    struct {
        int msg_type;
        int mic_num;
        float val;
    } buff;

    buff.msg_type = msg_type;
    buff.mic_num = mic_num;
    buff.val = (float) val;

    return send(m_client_fd,
                (char*)(&buff),
                sizeof(buff));
}

/*============================================================================
  FUNCTION:  DataUnSocket::send_gesture_event
============================================================================*/
/**
  Sends a gesture event to the client connected on the data socket.
*/
int DataUnSocket::send_gesture_event(int gesture,
                                     int pressure,
                                     int event_source)
{
  int buff[3];

  buff[0] = gesture;
  buff[1] = pressure;
  buff[2] = event_source;

  return send(m_client_fd,
              (char*)buff,
              sizeof(buff));
}

/*============================================================================
  FUNCTION:  DataUnSocket::send_hovering_event
============================================================================*/
/**
  Sends a hovering event to the client connected on the data socket.
*/
int DataUnSocket::send_hovering_event(int cValid,
                                      float fX,
                                      float fY,
                                      float fZ)
{
  int buff[4];

  buff[0] = cValid;
  buff[1] = round(fX);
  buff[2] = round(fY);
  buff[3] = round(fZ);

  return send(m_client_fd,
              (char*)buff,
              sizeof(buff));
}

/*============================================================================
  FUNCTION:  DataUnSocket::send_p2p_event
============================================================================*/
/**
  Sends a p2p event to the client connected on the data socket.
  This updates the presence status of a single user. Call this function multiple times
  to update the status of all users.
  distance is -1 if the user with peerId is not present
*/
int DataUnSocket::send_p2p_event(int myId,
                                 int peerId,
                                 int azimuth,
                                 int distance)
{
  int buff[4];

  buff[0] = (char)myId;
  buff[1] = (char)peerId;
  buff[2] = azimuth;
  buff[3] = distance;

  return send(m_client_fd,
              (char*)buff,
              sizeof(buff));
}

