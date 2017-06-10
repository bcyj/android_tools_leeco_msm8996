#ifndef __UAL_UTIL_H__
#define __UAL_UTIL_H__

/*============================================================================
                           ual_util.h

DESCRIPTION:  Types and function definitions for the common use
              of all usf daemons.
              Containe parsing and pattern file handle.

Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*-----------------------------------------------------------------------------
  Include Files
-----------------------------------------------------------------------------*/
#include <ual.h>
#include <math.h>

/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
const uint16_t MAX_TRANSPARENT_DATA_SIZE = 1024;
const uint16_t FILE_PATH_MAX_LEN = 500;
const uint16_t MAX_FILE_NAME_LEN  = 500;
const uint16_t TRANSPARENT_DATA_MAX_SIZE = 200;
const uint16_t US_FORM_FACTOR_CONFIG_MAX_MICS = 10;
const uint16_t US_FORM_FACTOR_CONFIG_MAX_SPEAKERS = 4;

#define TSC_LOGICAL_MAX_X 65000
#define TSC_LOGICAL_MAX_Y TSC_LOGICAL_MAX_X

// min, max array dimension
#define MIN_MAX_DIM 2

// The coordinates indexes
const uint16_t X_IND  = 0;
const uint16_t Y_IND  = 1;
const uint16_t Z_IND  = 2;

const uint16_t BYTE_WIDTH = 8;
/**
  indexes of min, max values
*/
const uint16_t MIN_IND = 0;
const uint16_t MAX_IND = 1;

// PS info string size
const uint16_t PS_INFO_SIZE = 100;

// Calibration files defines
#define EPOS_PRODUCT_FILE     0
#define EPOS_UNIT_FILE        1
#define EPOS_PERSISTENT_FILE  2
#define EPOS_PEN_SERIES_FILE  3
#define EPOS_CALIB_FILE_COUNT 4

// Output type shift
#define OUTPUT_TYPE_RAW_SHIFT           0
#define OUTPUT_TYPE_GESTURE_EVENT_SHIFT 3
#define OUTPUT_TYPE_P2P_EVENT_SHIFT     4
// Output type masks
#define OUTPUT_TYPE_RAW_MASK           (1 << OUTPUT_TYPE_RAW_SHIFT)
#define OUTPUT_TYPE_GESTURE_EVENT_MASK (1 << OUTPUT_TYPE_GESTURE_EVENT_SHIFT)
#define OUTPUT_TYPE_P2P_EVENT_MASK     (1 << OUTPUT_TYPE_P2P_EVENT_SHIFT)

#define PAIRING_PRODUCT_FILE     0
#define PAIRING_UNIT_FILE        1
#define PAIRING_DEFAULT_CALIB_FILE_COUNT 2

// Buttons masks to create the
// requested buttons bitmap
#define BTN_STYLUS_SHIFT                0
#define BTN_STYLUS2_SHIFT               1
#define BTN_TOOL_PEN_SHIFT              2
#define BTN_TOOL_RUBBER_SHIFT           3
#define BTN_TOOL_FINGER_SHIFT           4
#define BTN_USF_HOVERING_ICON_SHIFT     5
#define BTN_STYLUS_MASK             (1 << BTN_STYLUS_SHIFT)
#define BTN_STYLUS2_MASK            (1 << BTN_STYLUS2_SHIFT)
#define BTN_TOOL_PEN_MASK           (1 << BTN_TOOL_PEN_SHIFT)
#define BTN_TOOL_RUBBER_MASK        (1 << BTN_TOOL_RUBBER_SHIFT)
#define BTN_TOOL_FINGER_MASK        (1 << BTN_TOOL_FINGER_SHIFT)
#define BTN_USF_HOVERING_ICON_MASK  (1 << BTN_USF_HOVERING_ICON_SHIFT)

// SW calib calibration files
#define SW_CALIB_PRODUCT_FILE             0
#define SW_CALIB_UNIT_FILE                1
#define SW_CALIB_PERSISTENT_FILE          2
#define SW_CALIB_PEN_SERIES_FILE          3
#define SW_CALIB_CALIB_FILE               4
#define SW_CALIB_DEFAULT_CALIB_FILE_COUNT 5

// P2P max users
// should match QC_US_P2P_MAX_USERS in P2PExports.h
#define P2P_MAX_USERS                   4

/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/
/**
  Off screen mode values
 */
enum off_screen_mode_t
{
  OFF_SCREEN_MODE_DUPLICATE = 0,
  OFF_SCREEN_MODE_EXTEND,
  OFF_SCREEN_NUM_MODES
};

/**
 Hover icon mode
 */
enum hover_icon_mode_t
{
    // hover icon not shown
    HOVER_ICON_DISABLED = 0,
    // hover icon shown in palm-rejection area
    HOVER_ICON_PALM_AREA,
    HOVER_ICON_NUM_MODES
};

/**
  The calibration file is a file used by EPOS library to save/load data specific
  to each platform/device/etc...
*/
struct calib_file {
  // File path
  char     path[FILE_PATH_MAX_LEN];
  // Pointer to memory where data resides
  void     *calib_packet;
  uint32_t calib_packet_len;
  // When mandatory, daemon will exit on loading error
  bool     mandatory;
};

/**
  This struct is filled by parsing function and contains all the
  info from the cfg file.
*/
typedef struct {
    bool use_rx; // True if there is usf_rx_sample_rate in the cfg file
    bool use_tx; // True if there is usf_tx_sample_rate in the cfg file

    // Tx params
    uint32_t usf_device_id;
    uint32_t usf_tx_data_format;
    uint32_t usf_tx_sample_rate;
    uint16_t usf_tx_sample_width;
    uint16_t usf_tx_port_count;
    char usf_tx_ports[FILE_PATH_MAX_LEN];
    uint16_t usf_tx_frame_hdr_size;
    uint16_t usf_tx_port_data_size;
    uint16_t usf_tx_queue_capacity;
    uint32_t usf_tx_transparent_data_size;
    char usf_tx_transparent_data[MAX_TRANSPARENT_DATA_SIZE];
    uint32_t usf_tx_buf_size;
    uint16_t usf_tx_skip;
    uint16_t usf_tx_group;
    uint32_t usf_tx_max_get_set_param_buf_size;

    // Rx params
    char usf_rx_pattern[FILE_PATH_MAX_LEN];
    uint32_t usf_rx_data_format;
    uint32_t usf_rx_sample_rate;
    uint16_t usf_rx_sample_width;
    uint16_t usf_rx_port_count;
    char usf_rx_ports[FILE_PATH_MAX_LEN];
    uint16_t usf_rx_frame_hdr_size;
    uint16_t usf_rx_port_data_size;
    uint16_t usf_rx_queue_capacity;
    uint32_t usf_rx_pattern_size;
    uint32_t usf_rx_transparent_data_size;
    char usf_rx_transparent_data[MAX_TRANSPARENT_DATA_SIZE];
    uint32_t usf_rx_buf_size;
    uint16_t usf_rx_group;
    uint32_t usf_rx_max_get_set_param_buf_size;

    // Common params
    char usf_frame_file[FILE_PATH_MAX_LEN];
    uint32_t usf_frame_file_format;
    uint32_t usf_frame_count;
    char usf_fuzz_params[FILE_PATH_MAX_LEN];
    char usf_adapter_lib[FILE_PATH_MAX_LEN];

    uint32_t ual_work_mode;
    uint32_t usf_append_timestamp;


    int usf_x_tilt[MIN_MAX_DIM];
    int usf_y_tilt[MIN_MAX_DIM];

    int usf_tsc_pressure[MIN_MAX_DIM];
    uint32_t  usf_event_type; // event_type to be sent (touch, mouse, etc...)
    // Bitmap of types of events from devs, conflicting with USF
    uint16_t conflicting_event_types;

    uint16_t req_buttons_bitmap;

    char usf_socket_path[FILE_PATH_MAX_LEN];

    // Private Epos
    uint32_t usf_epos_on_screen_event_dest;
    uint32_t usf_epos_off_screen_event_dest;
    uint32_t usf_epos_coord_type_on_disp;
    uint32_t usf_epos_coord_type_off_disp;
    uint16_t eraser_button_mode;
    uint16_t eraser_button_index;
    hover_icon_mode_t usf_epos_on_screen_hover_icon_mode;
    hover_icon_mode_t usf_epos_off_screen_hover_icon_mode;
    uint32_t usf_epos_touch_disable_threshold;
    uint32_t usf_epos_battery_low_level_threshold;
    // On screen transformation matrix points
    char usf_on_screen_transform_origin[FILE_PATH_MAX_LEN];
    char usf_on_screen_transform_end_X[FILE_PATH_MAX_LEN];
    char usf_on_screen_transform_end_Y[FILE_PATH_MAX_LEN];
    uint32_t usf_epos_on_screen_hover_max_range;
    // Off screen transformation matrix points
    char usf_epos_off_screen_transform_origin[FILE_PATH_MAX_LEN];
    char usf_epos_off_screen_transform_end_X[FILE_PATH_MAX_LEN];
    char usf_epos_off_screen_transform_end_Y[FILE_PATH_MAX_LEN];
    uint32_t usf_epos_off_screen_hover_max_range;
    // Active zone distance from on/off draw area.
    char usf_epos_on_screen_act_zone_border[FILE_PATH_MAX_LEN];
    char usf_epos_off_screen_act_zone_border[FILE_PATH_MAX_LEN];
    uint32_t usf_epos_cfg_point_downscale;
    char usf_epos_calib_files[EPOS_CALIB_FILE_COUNT][FILE_PATH_MAX_LEN];
    char usf_epos_coord_file[FILE_PATH_MAX_LEN];
    uint32_t usf_epos_coord_count;
    uint32_t usf_epos_timeout_to_coord_rec;
    off_screen_mode_t usf_epos_off_screen_mode;
    uint32_t usf_epos_debug_print_interval;
    uint16_t no_act_zone_sleep_duration;
    uint16_t no_act_zone_probe_duration;
    uint32_t no_act_zone_empty_frames_count;
    uint16_t epos_lib_max_trace_level;
    char usf_epos_lib_path[FILE_PATH_MAX_LEN];
    // Smarter stand parameters
    double usf_epos_smarter_stand_angle;
    double usf_epos_zero_angle_thres;
    char usf_epos_rotation_axis_origin[FILE_PATH_MAX_LEN];
    char usf_epos_rotation_axis_direction[FILE_PATH_MAX_LEN];
    // Input events simulation
    char usf_epos_durations_file[FILE_PATH_MAX_LEN];

    // Private P2P
    uint32_t usf_p2p_device_uid;
    uint32_t usf_p2p_event_dest;
    uint32_t usf_p2p_pattern_type;
    uint32_t usf_p2p_event_out_port;
    uint16_t usf_p2p_app_type;
    uint16_t usf_p2p_pos_alg_type;
    uint16_t usf_p2p_data_alg_type;
    uint16_t usf_p2p_num_users;
    uint16_t usf_p2p_user_index;
    uint32_t usf_p2p_samples_per_frame;
    uint16_t usf_p2p_sequence_index[P2P_MAX_USERS];
    uint16_t usf_p2p_first_bin;
    uint16_t usf_p2p_last_bin;
    uint32_t usf_p2p_resampler_freq_ppm;
    uint16_t usf_p2p_library_mode;
    uint16_t usf_p2p_fft_size;
    uint16_t usf_p2p_rx_pattern_mode;
    uint16_t usf_p2p_p2a_threshold;
    uint16_t usf_p2p_los_window_len_ratio;
    uint16_t usf_p2p_los_peak_threashold_ratio;

    // Private Hovering
    uint32_t usf_hovering_event_dest;
    uint32_t usf_hovering_event_out_port;

    // Private Gesture
    uint32_t usf_gesture_event_dest;
    uint32_t usf_gesture_event_out_port;
    char usf_gesture_keys[FILE_PATH_MAX_LEN];
    uint32_t usf_gesture_app_lib_bypass;
    uint32_t usf_algo_transparent_data_size;
    char usf_algo_transparent_data[MAX_TRANSPARENT_DATA_SIZE];

    // Private DSP calculated events daemons
    uint32_t usf_output_type;

    // Private Pairing
    uint32_t usf_pairing_max_aquisition_time;
    char usf_pairing_calib_files_path_prefix[FILE_PATH_MAX_LEN];
    char usf_pairing_default_calib_files[PAIRING_DEFAULT_CALIB_FILE_COUNT][FILE_PATH_MAX_LEN];
    uint32_t usf_pairing_circle_x;
    uint32_t usf_pairing_circle_y;
    uint32_t usf_pairing_circle_r;

    // Private SW Calib
    struct calib_file usf_sw_calib_calibration_file[SW_CALIB_DEFAULT_CALIB_FILE_COUNT];
    uint32_t usf_sw_calib_timeout_msec;
    bool usf_sw_calib_is_tester_mode;
    char usf_tester_power_min_scales[FILE_PATH_MAX_LEN];
    char usf_tester_power_max_scales[FILE_PATH_MAX_LEN];
    char usf_tester_power_thresholds[FILE_PATH_MAX_LEN];
    char usf_tester_quality_min_scales[FILE_PATH_MAX_LEN];
    char usf_tester_quality_max_scales[FILE_PATH_MAX_LEN];
    char usf_tester_quality_thresholds[FILE_PATH_MAX_LEN];

    // Power save
    char ps_act_state_params[PS_INFO_SIZE];
    char ps_standby_state_params[PS_INFO_SIZE];
    char ps_idle_state_params[PS_INFO_SIZE];
    // US detection info for standby & idle states
    char ps_standby_detect_info[PS_INFO_SIZE];
    char ps_idle_detect_info[PS_INFO_SIZE];
    // US detection calibration files for standby & idle states
    char ps_standby_detect_calibration[FILE_PATH_MAX_LEN];
    char ps_idle_detect_calibration[FILE_PATH_MAX_LEN];
    // US detection port in idle state
    uint32_t ps_idle_detect_port;
    // US detection period (sec) in idle state
    // Value 0 - contininue detection mode
    // Other value - one shot detection mode
    uint32_t ps_idle_detect_period;
}  us_all_info;

/**
  This struct contains info about mics/speakers form factor.
*/
typedef struct
{
  int num_of_mics;
  // Mics coordinates (x, y, z?) in  0.1 mm (NW of the portrait display)
  float mics_info[US_FORM_FACTOR_CONFIG_MAX_MICS][COORDINATES_DIM];
  // The mics identification in well-defined  enumeration
  int mics_id[US_FORM_FACTOR_CONFIG_MAX_MICS];
  int num_of_speakers;
  // Speakers coordinates (x, y, z?) in  0.1 mm (NW of the portrait display)
  float speakers_info[US_FORM_FACTOR_CONFIG_MAX_SPEAKERS][COORDINATES_DIM];
  // The speakers identification in well-defined enumeration
  int speakers_id[US_FORM_FACTOR_CONFIG_MAX_SPEAKERS];
} FormStruct;

/*------------------------------------------------------------------------------
  Function declarations
------------------------------------------------------------------------------*/

/*==============================================================================
  FUNCTION:  ual_util_parse_cfg_file
==============================================================================*/
/**
  This function receives file to parse and returns the parsing file
. as struct in us_all_info.
  Return value is the number of recognized fields from the cfg file                       .
  or -1 for failure.
*/
int ual_util_parse_cfg_file (FILE *cfgFile,
                             us_all_info *paramsStruct);

/*==============================================================================
  FUNCTION:  ual_util_read_pattern
==============================================================================*/
/**
  This function receives ptr to store content of pattern file,
. us_all_info struct for info and the path of the pattern dir.
  Returns 0 for success or -1 for failure.
*/
int ual_util_read_pattern (uint8_t *pBuf,
                           us_all_info *paramsStruct,
                           char *patternDirPath);

/*==============================================================================
  FUNCTION:  ual_util_parse_string2array
==============================================================================*/
/**
  Parse data string (format <byte1>, <byte2>,..) into output bytes array
  Pay Attention! This function changes the content of the data_string.
*/
bool ual_util_parse_string2array(uint16_t data_size,
                                 char *data_string,
                                 uint8_t *data);

/*==============================================================================
  FUNCTION:  ual_util_tx_config
==============================================================================*/
/**
  ual_util_tx_config function config the ual tx.
  Returns 0 for success, -1 for failure or 1 if no tx params in the
  the cfg file.
*/
int ual_util_tx_config (us_all_info *paramsStruct,
                        char *clientName);

/*==============================================================================
  FUNCTION:  ual_util_rx_config
==============================================================================*/
/**
  ual_util_rx_config function config the ual tx.
  Returns 0 for success, -1 for failure or 1 if no rx params in
  the cfg file.
*/
int ual_util_rx_config (us_all_info *paramsStruct,
                        char *clientName);

/*==============================================================================
  FUNCTION:  ual_util_daemon_init
==============================================================================*/
/**
  This function implements common initialization to all the daemons.
  It also calls to parser for the cfg file.
  Returns 0 for success and -1 for failure.
*/
int ual_util_daemon_init (us_all_info *paramsStruct,
                          char *cfgLinkFileLocation,
                          FILE *cfgFile,
                          char *daemonName);

/*==============================================================================
  FUNCTION:  ual_util_get_mic_config
==============================================================================*/
/**
  Receives mic index (start from 0) and returns this mic form in micCfg.
  Returns -1 for failure, 0 for success.
*/
int ual_util_get_mic_config (int micIndex,
                             float *micCfg);

/*==============================================================================
  FUNCTION:  ual_util_get_speaker_config
==============================================================================*/
/**
  Receives speaker index (start from 0) and returns this mspeaker form in           .
  speakerCfg. Returns -1 for failure, 0 for success.
*/
int ual_util_get_speaker_config (int speakerIndex,
                                 float *speakerCfg);

/*==============================================================================
  FUNCTION:  ual_util_get_file
==============================================================================*/
/**
  Receives file name, default dir to store the file and whether the file
  binary or not.
  If the file name contains '/' the file name is absolute path,
  else the file will be stored in the default dir.
  Returens pointer to the open file or NULL if any problem
  occurs.
*/
FILE *ual_util_get_file(us_all_info const *paramsStruct,
                        char *defaultFrameDirName,
                        bool isNotBinary);

/*==============================================================================
  FUNCTION:  ual_util_get_frame_file
==============================================================================*/
/**
  Receives frame file name and default dir to store the file.
  If the file name contains '/' the file name is absolute path,
  else the file will be stored in the default dir.
  Returens pointer to the open file or NULL if any problem
  occurs.
*/
FILE *ual_util_get_frame_file(us_all_info const *paramsStruct,
                              char *defaultFrameDirName);

/*==============================================================================
  FUNCTION:  ual_util_malloc_read
==============================================================================*/
/**
  Allocates memory for the required file and reads file into the memory.
  The caller must free the allocated memory.
*/
void *ual_util_malloc_read(const char *file_name,
                           uint32_t &data_size);

/*==============================================================================
  FUNCTION:  ual_util_write_file
==============================================================================*/
/**
  Writes the given data to the file path given, note that data_size is in long.
*/
int ual_util_write_file(const char *file_path,
                        void *data,
                        uint32_t data_size,
                        int element_size);

/*==============================================================================
  FUNCTION:  ual_util_print_US_version
==============================================================================*/
/**
  Prints the USF driver, UAL, calculator versions
*/
void ual_util_print_US_version(const char *calculator_name,
                               const char *calculator_version);

/*==============================================================================
  FUNCTION:  ual_util_declare_pid
==============================================================================*/
/**
 * This function creates a file containing the pid of the daemon
 *
 * @param pid the pid of the daemon
 * @param pid_filename the pid filename to store the pid in
 *
 * @return int returns 0 on success
 *                     -1 on failure
 */
int ual_util_declare_pid(int pid,
                         const char *pid_filename);

/*==============================================================================
  FUNCTION:  ual_util_remove_declare_pid
==============================================================================*/
/**
 * This function removes the file containing the pid of the daemon
 *
 *
 * @param pid_filename the pid filename to store the pid in
 *
 * @return int -1 on case on an error
 *              0 on success
 */
int ual_util_remove_declare_pid(const char *pid_filename);

/*==============================================================================
  FUNCTION: ual_util_prefill_ports_num_and_id
==============================================================================*/
/**
 * Config s_req_mics_num, s_req_mics_id, s_req_speakers_num, s_req_speakers_id
 * required for ual_util_get_mic_config & ual_util_get_speaker_config.
 * Returns 0 for success, -1 for failure
 */
int ual_util_prefill_ports_num_and_id(us_all_info *paramsStruct);

/*==============================================================================
  FUNCTION: ual_util_is_supported
==============================================================================*/
/**
 * This function checks whether the current daemon is supported and returns the
 * answer.
 *
 * @param daemon_name The name of current daemon.
 *
 * @return bool true - The daemon is supported
 *              false - The daemon is not supported.
 */
bool ual_util_is_supported(const char *daemon_name);

/*==============================================================================
  FUNCTION:  ual_util_alarm_handler
==============================================================================*/
/**
 * A recursive alarm handler, keeps sending alarm signals every 1 second.
 * Alarm signals are used to make processes leave blocking-functions in their
 * way to exiting.
 * @param sig the number of the signal.
 */
void ual_util_alarm_handler(int sig);

/*==============================================================================
  FUNCTION:  ual_util_close_and_sync_file
==============================================================================*/
/**
 * Closes the given file and syncs it to the disk.
 *
 * @param file The file to close and sync
 */
void ual_util_close_and_sync_file(FILE *file);

/*==============================================================================
  FUNCTION:  ual_util_set_buf_size
==============================================================================*/
/**
 * calculates and sets the buf size according to the given parameters.
 */
void ual_util_set_buf_size(uint32_t *usf_buf_size,
                           uint16_t usf_port_data_size,
                           uint16_t usf_sample_width,
                           uint16_t usf_port_count,
                           uint16_t usf_frame_hdr_size,
                           uint16_t usf_group,
                           const char *type,
                           bool has_event,
                           bool has_raw,
                           uint32_t event_size);

/*==============================================================================
  FUNCTION:  ual_util_set_tx_buf_size
==============================================================================*/
/**
 * This function uses data in the param struct to calculate the tx buf size and
 * set it in the params struct
 */
void ual_util_set_tx_buf_size(us_all_info *paramsStruct);

/*==============================================================================
  FUNCTION:  ual_util_set_rx_buf_size
==============================================================================*/
/**
 * This function uses data in the param struct to calculate the rx buf size and
 * set it in the params struct
 */
void ual_util_set_rx_buf_size(us_all_info *paramsStruct);

/*==============================================================================
  FUNCTION:  ual_util_set_epos_tx_transparent_data
==============================================================================*/
/**
 * Sets the transparent data in the params struct based on cfg params (e.g. skip,group)
 */
void ual_util_set_epos_tx_transparent_data(us_all_info *paramsStruct);

/*==============================================================================
  FUNCTION:  ual_util_set_echo_tx_transparent_data
==============================================================================*/
/**
 * Sets the transparent data in the params struct based on cfg params (e.g. skip,group)
 */
void ual_util_set_echo_tx_transparent_data(us_all_info *paramsStruct);

/*==============================================================================
  FUNCTION:  ual_util_set_echo_rx_transparent_data
==============================================================================*/
/**
 * Sets the transparent data in the params struct based on cfg params (e.g. skip,group)
 */
void ual_util_set_echo_rx_transparent_data(us_all_info *paramsStruct);

/*==============================================================================
  FUNCTION:  ual_util_inject_to_trans_data
==============================================================================*/
/**
  Injects the given value to the transparent data given, adding the data to the end.
  params:
  trans_data - transparent data bytes
  size       - size of the trans_data in bytes, gets updated to the new size
  max_size   - max size allowed for the transparent data file
  value      - value to inject
  num_bytes  - how much bytes should be injected (look for information about
               transparent data for more details)
*/
int ual_util_inject_to_trans_data(char      *trans_data,
                                  uint32_t  *size,
                                  size_t     max_size,
                                  int        value,
                                  int        num_bytes);

/*==============================================================================
  FUNCTION:  ual_util_inject_to_trans_data
==============================================================================*/
/**
 * Tries to opens ual, OPEN_RETRIES times, unless fails.
 *
 * @return int 0 - success
 *             1 - failure
 */
int ual_util_ual_open_retries(us_all_info const *paramsStruct);

/*==============================================================================
  FUNCTION:  ual_util_cfg_parse_uint16_array
==============================================================================*/
/**
 * Parse a parameter value into an array of uint16_t values
 * @param value the parameter value string
 * @param out pointer to the array for placing values
 * @param maxSize maximum size of the out array
 * @param actualSize on success fill with number of values parsed
 *
 * @return 0 success
 *         other value error occured
 */
int ual_util_cfg_parse_uint16_array(char* value, uint16_t* out, uint32_t maxSize, uint32_t* actualSize);

#endif // __UAL_UTIL_H__
