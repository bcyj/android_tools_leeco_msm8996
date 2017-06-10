#ifndef _USF_UNIX_DOMAIN_SOCKET_H_
#define _USF_UNIX_DOMAIN_SOCKET_H_

/*============================================================================
                           usf_unix_domain_socket.h

DESCRIPTION:  This file declares the unix domain socket server.

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*-----------------------------------------------------------------------------
  Include Files
-----------------------------------------------------------------------------*/
//includes for socket use
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <semaphore.h>
#include <pthread.h>
#include "usf_geometry.h"

/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/

// Control socket mutex path
static pthread_mutex_t control_socket_mutex = PTHREAD_MUTEX_INITIALIZER;


/**
   Unix domain socket class:
   Base class for unix domain sockets, includes all common functionality of
   initing sockets, and running the helper thread.
 */
class UnSocket
{
  protected:
      int m_socket_fd;
      struct sockaddr_un m_address;
      int m_thread_id;
      // A callback function that gets called when the client gets connected
      void (*m_connected_cb)();
      sem_t m_sem;
  public:
      UnSocket(const char *sock_path, void (*connected_cb)());
      virtual ~UnSocket();
      int get_scoket_fd();
      int start();
      virtual int thread_func() = 0;
      sem_t *get_sem();

};

// Dynamic config coordinate destinations
enum dc_screen_destination
{
  DP_COORD_DESTINATION_MOTION_EVENT = 0,
  DP_COORD_DESTINATION_SOCKET,
  DP_COORD_DESTINATION_BOTH,
  DP_COORD_DESTINATION_NUM
};

// Dynamic config off screen modes
enum dc_off_screen_mode
{
  DC_OFF_SCREEN_MODE_DISABLED = 0,
  DC_OFF_SCREEN_MODE_EXTEND,
  DC_OFF_SCREEN_MODE_DUPLICATE,
  DC_OFF_SCREEN_MODE_NUM
};

// Dynamic config physical eraser button index
enum eraser_button_index_t
{
  ERASER_BUTTON_INDEX_BUTTON1 = 0,
  ERASER_BUTTON_INDEX_BUTTON2,
  ERASER_BUTTON_INDEX_NUM
};

// Eraser button mode
enum eraser_button_mode_t
{
  ERASER_BUTTON_MODE_DISABLED = 0,
  ERASER_BUTTON_MODE_HOLD_TO_ERASE,
  ERASER_BUTTON_MODE_TOGGLE_ERASE,
  ERASER_BUTTON_NUM_MODES
};

/**
   Dynamic configuration parameters struct.
 */
struct dynamic_config_t {
  // True when the parameters changed and were
  // not yet updated in the daemon
  bool config_changed;
  bool angle_changed;
  int on_screen_hover_max_range;
  int event_type;
  dc_off_screen_mode off_screen_mode;
  int on_screen_destination;
  bool on_screen_mapping;
  int off_screen_destination;
  bool off_screen_mapping;
  bool send_all_events_to_side_channel;
  plane_properties off_screen_plane;
  bool smarter_stand_enable;
  double smarter_stand_angle;
  eraser_button_index_t eraser_button_index;
  eraser_button_mode_t eraser_button_mode;
  bool show_on_screen_hover_icon;
  bool show_off_screen_hover_icon;
  int touch_disable_threshold;
};

/**
   Socket parameters to help parse socket data.
 */
struct config_parameters_t
{
  uint8_t off_screen_mode;
  int touch_disable_threshold;
  uint8_t power_save;
  int on_screen_hover_max_range;
  bool on_screen_hover;
  bool show_on_screen_hover_icon;
  int off_screen_hover_max_range;
  bool off_screen_hover;
  bool show_off_screen_hover_icon;
  bool smarter_stand_enable;
  float origin[3];
  float end_x[3];
  float end_y[3];
  uint8_t on_screen_destination;
  bool on_screen_mapping;
  uint8_t off_screen_destination;
  bool off_screen_mapping;
  int eraser_button_index;
  uint8_t eraser_button_mode;
  bool send_all_events_to_side_channel;
} __attribute__((packed));

struct accelerometer_data_t
{
  double angle;
} __attribute__((packed));

/**
   Control unix domain socket class:
   Receives the dynamic configuration parameters in its C'tor, then keeps
   waiting for data from the client to change the dynamic configuration.
 */
class ControlUnSocket : public UnSocket
{
  private:
    struct dynamic_config_t *m_dconfig;
    int m_client_fd;
  public:
    ControlUnSocket(const char *sock_path,
                    struct dynamic_config_t *config,
                    void (*connected_cb)() = NULL);
    virtual ~ControlUnSocket();
    struct dynamic_config_t *get_config();
    bool update_config(config_parameters_t *cp);
    bool update_smarter_stand_angle(const accelerometer_data_t *ad);
    bool validate_dynamic_config();
    void print_config_message(config_parameters_t *cp);
    void update_dynamic_config_params(config_parameters_t *cp);
    virtual int thread_func();
    int send_event(int event_num,
                   int extra1,
                   int extra2);
};

/**
   Data unix domain socket class:
   A unix domain socket for sending data, after the client gets connected, it
   keeps receiving all EPOS points that the daemon sends.
 */
class DataUnSocket : public UnSocket
{
  private:
    int m_client_fd;
  public:
    DataUnSocket(const char *sock_path,
                 void (*connected_cb)() = NULL);
    int send_epos_point(int32_t x,
                        int32_t y,
                        int32_t z,
                        int32_t tilt_x,
                        int32_t tilt_y,
                        int32_t tilt_z,
                        int32_t pressure,
                        int32_t type,
                        int32_t region);
    int send_proximity_event(int timestamp,
                             int seq_num,
                             int result);
    int send_gesture_event(int gesture,
                           int pressure,
                           int event_source);
    int send_hovering_event(int cValid,
                            float fX,
                            float fY,
                            float fZ);
    int send_p2p_event(int myId,
                       int peerId,
                       int azimuth,
                       int distance);
    int send_pairing_event(int status,
                           int penId);
    int send_sw_calib_event(int status,
                            int x,
                            int y);
    int send_sw_calib_tester_event(int msg_type,
                                   int mic_num,
                                   double val);
    virtual int thread_func();
    virtual ~DataUnSocket();
};
#endif // _USF_UNIX_DOMAIN_SOCKET_H_
