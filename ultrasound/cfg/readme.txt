------------------------
  Cfg File Clarification
------------------------
Every line in the file which starts with '#' or space or
Enter (empty line) is considered as comment line and ignored.
Every line with relevant info is of the form: <paramName>  [<paramValue>].
<paramName> and <paramValue> shouldn't contain any spaces.
Between <paramName> and <paramValue> there is spaces or tabs.


------------------------
  General
------------------------
usf_device_id           -       should be [1] for both Echo and Epos


------------------------
  TX Params
------------------------
usf_tx_data_format      -       should be [0] for USF_POINT_EPOS_FORMAT or [1] for USF_RAW_FORMAT or [2] USF_PROX_FORMAT
                                or [3] USF_SYNC_GESTURE_FORMAT or [4] USF_SYNC_FORMAT
usf_tx_sample_rate      -       [96000] for Echo or [192000] for Epos. If this param dosen't exist the calculator
                                concludes there is no TX.
usf_tx_sample_width     -       [<bits_per_sample>]. <bits_per_sample> is the number of bits in one sample.
usf_tx_port_count       -       [<port_count>]. <port_count> is the number of mics in use.
usf_tx_ports            -       for echo [1,2,5]. The serial numbers of the mics in use.
                                for epos [2,1,5] (must be in this order!!!).
usf_tx_port_data_size   -       for Echo [768] samples, for Epos [522] samples, for P2P [12288] samples.
usf_tx_frame_hdr_size   -       for Echo [12] bytes, for Epos [0] bytes
usf_tx_queue_capacity   -       [<queue_capacity>]. In the code this param assigned to the tx_info.us_xx_info.buf_num
usf_tx_max_get_set_param_buf_size - max buffer size for getting and setting tx parameter


------------------------
  Tx Transparent Data
------------------------
usf_tx_transparent_data_size    -       should be [8] bytes for Echo and [4] for Epos or [40] for proximity.
usf_tx_transparent_data         -       Example for Echo [2,0,3,0,0,3,0,0].
                                        format: [skip - 2 bytes, lower byte first e.g. (2,0) for skip 2,
                                                 group - 2 bytes, lower byte first e.g. (3,0) for group 3,
                                                 frame size - 4 bytes, lower byte first e.g. (0,3,0,0) for 768]
                                        note: 768 = 300 in HEX and byte is 8 bits so 00 in HEX is 0 byte.
                                        Example for Epos [2,0,3,0].
                                        format: [skip - 2 bytes, lower byte first e.g. (2,0) for skip 2,
                                        group - 2 bytes, lower byte first e.g. (3,0) for group 3]
                                        Example for P2P [1,0,1,0,0,48,0,0].
                                        format: [skip - 2 bytes, lower byte first e.g. (1,0) for skip 1,
                                                 group - 2 bytes, lower byte first e.g. (1,0) for group 1,
                                                 frame size - 4 bytes, lower byte first e.g. (0,48,0,0) for 12288]
                                        note: 12288 = 3000 in HEX and byte is 8 bits so 00 in HEX is 0 byte (in DEC) and
                                        30 is 48 in byte (in DEC).
                                        For Proximity, following is the list of the parameters, each is 4 bytes in size:
                                        - round_trip_delay_comp
                                        - residual_backoff
                                        - hpf_backoff_samples
                                        - jammer_check_len_log2
                                        - jammer_skip_frames
                                        - jammer_frequency_extra_hz
                                        - notch_backoff_samples
                                        - notch_q
                                        - direct_path_max_index
                                        - indirect_path_min_index
                                        - indirect_path_max_index
                                        - direct_threshold
                                        - direct_2_indirect_ratio
                                        - motion_direct_path_max_index
                                        - motion_indirect_path_min_index
                                        - motion_indirect_path_max_index
                                        - proximity_fine_tuning
                                        - proximity_calibration

usf_tx_buf_size                 -       for Echo [13860] bytes
                                        calculation:
                                        usf_tx_buf_size = (usf_tx_port_data_size * (usf_tx_sample_width/8) *
                                                           usf_tx_port_count + usf_tx_frame_hdr_size) * usf_tx_group;
                                        =>  usf_tx_buf_size = (768 * (16/8) * 3 + 12) * 3 = 13860;
                                        for Epos [9396] bytes
                                        calculation:
                                        usf_tx_buf_size = (usf_tx_port_data_size * (usf_tx_sample_width/8) *
                                                           usf_tx_port_count + usf_tx_frame_hdr_size) * usf_tx_group;
                                        =>  usf_tx_buf_size = (522 * (16/8) * 3 + 0) * 3 = 9396;
                                        for P2P [73740] bytes
                                        calculation:
                                        usf_tx_buf_size = (usf_tx_port_data_size * (usf_tx_sample_width/8) *
                                                           usf_tx_port_count + usf_tx_frame_hdr_size) * usf_tx_group;
                                        =>  usf_tx_buf_size = (12288 * (16/8) * 3 + 12) * 1 = 73740;

                                        Note: for family B: (usf_tx_buf_size % 32) = 0; e.g. for Epos usf_tx_buf_size = 9408


------------------------
  RX Params
------------------------
Epos has only TX (no transmission in Epos mode).
Therefore RX params are for Echo and P2P only.

usf_rx_data_format      -       [1] for USF_RAW_FORMAT or [2] for USF_PROX_FORMAT or [3] USF_SYNC_GESTURE_FORMAT
usf_rx_sample_rate      -       [96000] for Echo, [192000] for P2P. If this param dosen't exist the clculator
                                concludes there is no RX.
usf_rx_sample_width     -       [<bits_per_sample>]. <bits_per_sample> is the number of bits in one sample.
usf_rx_port_count       -       [<port_count>]. <port_count> is the number of speakers in use.
usf_rx_ports            -       for example [1].  The serial numbers of the speakers in use.
usf_rx_port_data_size   -       for Echo [768] samples, for P2P [24576] samples.
usf_rx_frame_hdr_size   -       [0] bytes.
usf_rx_queue_capacity   -       [<queue_capacity>]. in the code this param assigned to the rx_info.us_xx_info.buf_num
usf_rx_pattern_size     -       for Echo [768] samples.
                                calculation:
                                pattern size in samples = usf_rx_port_data_size * usf_rx_port_count * usf_rx_group
                                => pattern size in samples = 768 * 1 * 1 = 768
                                for P2P [24576] samples.
                                calculation:
                                pattern size in samples = usf_rx_port_data_size * usf_rx_port_count * usf_rx_group
                                => pattern size in samples = 24576 * 1 * 1 = 24576
usf_rx_pattern          -       [<patternFileName>] while <patternFileName> is name
                                of a pattern file located in /data/usf/pattern/ dir.
                                For tester, if rx exist, this param is obligation.
                                For P2P this param is optional. If it does not exist then pattern is taken
                                from the P2P lib.
usf_rx_max_get_set_param_buf_size - max buffer size for getting and setting rx parameter


------------------------
  Rx Transparent Data
------------------------
usf_rx_transparent_data_size    -       [4] bytes for USF_RAW_FORMAT, [12] bytes for USF_PROX_FORMAT.
usf_rx_transparent_data         -       Example for Echo [0,3,1,0].
                                        format: [frame size - 2 bytes, lower byte first e.g. (0,3) for 768,
                                                 group - 2 bytes, lower byte first e.g. (1,0) for group 1]
                                        note: 768 = 300 in HEX and byte is 8 bits so 00 in HEX
                                        (each 0 is 4 bits) is only one 0 in byte.
                                        Example for P2P [0,96,1,0].
                                        format: [frame size - 2 bytes, lower byte first e.g. (0,96) for 24576,
                                                 group - 2 bytes, lower byte first e.g. (1,0) for group 1]
                                        note: 24576 = 6000 in HEX and byte is 8 bits so 00 in HEX
                                        (each 0 is 4 bits) is only one 0 in byte (in DEC) and 60 in HEX is
                                        96 in byte (in DEC).
                                        For Proximity, following is the list of the parameters, each is 4 bytes in size:
                                        - general_config
                                        - rx_pattern_length_log2
                                        - frequency_start_hz
                                        - bandwith_hz
                                        - gauss_quad_m

usf_rx_buf_size                 -       [1536] bytes.
                                        calculation:
                                        frame_size = usf_rx_port_data_size * (usf_rx_sample_width/8) *
                                                     usf_rx_port_count + usf_rx_frame_hdr_size;
                                        =>  frame_size = 768 * (16/8) * 1 + 0 = 1536;
                                        usf_rx_buf_size = frame_size * usf_rx_group;
                                        =>  usf_rx_buf_size = 1536 * 1 = 1536;

                                        Note: for family B: (usf_rx_buf_size % 32) = 0; e.g. for Epos usf_rx_buf_size = 1536


------------------------------
  common parameters of all daemons
------------------------------
usf_frame_file           -       [<frameFileName>]. In java UI is of type EditText.
                                 The file will be created in /data/usf/tester/rec/ dir.
usf_frame_file_format    -       Specifies the format of the output frame file recording:
                                     0 = raw format,
                                     1 = wave format,
                                 When this parameter is omitted, 0 is taken as default
usf_frame_count          -       [<numberOfFrames>]. In java UI is of type EditText.
usf_fuzz_params                  [100,100,100] - fuzz factors (in "0.01 mm" Epos' units)  (X, Y, Z)
                                                 define whether 2 points should be considered as different.
usf_event_type           -       Specifies the type of the event to be sent for the input module:
                                      0 = no event,
                                      1 = touch event,
                                      2 = mouse event,
                                 When this parameter is omitted, 0 is taken as default.
usf_adapter_lib          -       Path to the external framework adapter to use.
ual_work_mode            -       Specifies the mode of work for ual for power studies:
                                      0 = regular,
                                      1 = no calculation of events,
                                      2 = no inject of events to input module,
                                      3 = idle usf data path,
                                          Configs data path, but doesn't read US data.
                                      4 = idle all data path,
                                          Perform only device switch.
                                 When this parameter is omitted, 0 is taken as default.
usf_append_timestamp             Specifies whether to append timestamp to the recording file or not.
                                 0 = does not append timestamp,
                                 1 = appends timestamp to recording file,
                                 When this parameter is ommited, 0 is taken as default.
req_buttons_bitmap       -       A bitmap with the stylus buttons to use.
                                 [1] - use library's primary switch output.
                                 [2] - use library's secondary switch output.
                                 [4] - BTN_TOOL_PEN, the pen stylus pen tool type.
                                 [8] - BTN_TOOL_RUBBER, the tool type for eraser.
                                 [16] - BTN_TOOL_FINGER, for future use.
                                 [32] - hover indicator virtual button (show hover marker below certain Z value).

------------------------------
  private parameters of EPOS
------------------------------
usf_epos_on_screen_event_dest           -        [1] for UAL, [2] for out (socket), [3] for both.
                                                 This param is referenced as bitmask - for future use, if more destinations
                                                 is needed it will be [4] then [8] etc.
                                                 Any combination of destinations is a sum of the basic destinations
                                                 (for example 3 for UAL and socket = 1 for UAL + 2 for socket).
usf_epos_off_screen_event_dest           -       [1] for UAL, [2] for out (socket), [3] for both
usf_epos_event_out_port                  -       [<portNumber>]. This is the port number of the socket if
                                                 event_dest is 2 or 3.
usf_epos_coord_type_on_disp              -       [0] for mapped to the screen or [1] for raw.
usf_epos_coord_type_off_disp             -       [0] for mapped to the screen or [1] for raw.
usf_on_screen_transform_origin           -       On-screen plane origin in mm (x0,y0,z0)
                                                 This is the offset from EPOS origin (0,0,0) in mm.
                                                 Notice that the maximum allowed origin values are 500 for each dimension,
                                                 maximum width and height screen size is 50 cm, and the two vectors created by the points
                                                 origin-end_x and origin-end_y must be perpenducular to each other.
usf_on_screen_transform_end_X            -       On-screen plane end of x axis (x1,y1,z1).
                                                 This is the offset from EPOS origin.
                                                 Please see above description.
usf_on_screen_transform_end_Y            -       On-screen plane end of y axis (x2,y2,z2).
                                                 This is the offset from EPOS origin.
                                                 Please see above description.
usf_epos_on_screen_hover_max_range       -       On screen max hovering range.
usf_epos_off_screen_transform_origin     -       Off screen origin in mm (x0,y0,z0).
                                                 This is the offset from EPOS origin (0,0,0) in mm
                                                 Please see above description.
usf_epos_off_screen_transform_end_X      -       Off screen end of x axis (x1,y1,z1).
                                                 This is the offset from EPOS origin in mm.
                                                 Please see above description.
usf_epos_off_screen_transform_end_Y      -       Off screen end of y axis (x2,y2,z2).
                                                 This is the offset from EPOS origin in mm.
                                                 Please see above description.
usf_epos_off_screen_hover_max_range      -       Off screen max hovering range.
usf_epos_on_screen_act_zone_border       -       The vector of distances from the on screen draw area which will be considered
                                                 as on active zone draw area. (x,y,z)
usf_epos_off_screen_act_zone_border      -       The vector of distances from the off screen draw area which will be considered
                                                 as off active zone draw area. (x,y,z)
usf_epos_skip                            -       [<skip_factor>]. This is for statistics. The <skip_factor> number is match
                                                 to the skip number in the usf_rx_transparent_data.
usf_epos_cfg_point_downscale             -       [<point_downscale>]. Every <point_downscale> measures of epos one point will
                                                 be printed out. This is an OPTIONAL parameter. If this param isn't found
                                                 then cfg_point_downscale will be set to 0.
usf_epos_product_packet                  -       [<productcalibFile>]. <productcalibFile> is the full name to the epos product calibration packet
                                                 file. This is an optional parameter.  If this param isn't found then default
                                                 hard coded location is taken - "/persist/usf/epos/product_calib.dat".
usf_epos_unit_packet                     -       [<unitcalibFile>]. <unitcalibFile> is the full name to the epos unit calibration packet
                                                 file. This is an optional parameter.  If this param isn't found then default
                                                 hard coded location is taken - "/persist/usf/epos/unit_calib.dat".
usf_epos_persistent_packet               -       [<persistentcalibFile>]. <persistentcalibFile> is the full name to the epos persistent calibration packet
                                                 file. This is an optional parameter.  If this param isn't found then default
                                                 hard coded location is taken - "/data/usf/epos/persistent_calib.dat".
usf_epos_coord_file                      -       [<coordFileName>]. In java UI is of type EditText.
                                                 The file will be created in /data/usf/epos/rec/ dir or in absolute path if the name is with '/'.
usf_epos_coord_count                     -       number of coordinate to record from epos lib.
usf_epos_timeout_to_coord_rec            -       Recording timeout in seconds. After recording timeout is reached, daemon will be
                                                 turned off.
usf_epos_off_screen_mode                 -       [0] (default) For duplicate mode (one-to-one mapping)
                                                 [1] Extend mode
usf_epos_debug_print_interval            -       The interval of frames between every point logged. This is for epos points debugging.
                                                 [0] (default) No points will be logged.
no_act_zone_sleep_duration               -       Duration (msec) of sleep stage of "no active zone" behavior
                                                 Zero value means, the "no active zone" behavior is disabled
no_act_zone_probe_duration               -       Duration (msec) of probe stage of "no active zone" behavior
no_act_zone_empty_frames_count           -       Number of "empty" frames trigerring start of "no active zone" behavior
eraser_button_mode                       -       behavior of eraser button.
                                                 [0] DISABLED. The Eraser tooltype will no be sent at all.
                                                 [1] HOLD_TO_ERASE. Eraser mode is activated only when the designated button is held down.
                                                 [2] TOGGLE_ERASE. Pressing the button toggles between eraser mode and normal mode.
eraser_button_index                      -       Physical button for eraser functionality.
                                                 [0] BUTTON1.
                                                 [1] BUTTON2. This is the only possible one for now.
epos_lib_max_trace_level                 -       Maximum trace level to be printed by the epos library. All traces upto and including selected
                                                 selected level will be printed.
                                                 [0] Error.
                                                 [1] Warning.
                                                 [2] Info.
                                                 [3] Debug.
usf_epos_smarter_stand_angle             -       Smarter stand angle to be used. Default value is 0.
usf_epos_rotation_axis_origin            -       The origin of the rotation axis of the tablet (x,y,z).
usf_epos_rotation_axis_direction         -       The normalized direction of the rotation axis of the tablet (x,y,z).
usf_epos_zero_angle_thres                -       This is the threshold for treating the angle as zero.
                                                 In smart stand mode library can calculate some of the
                                                 parameters less accurately therefore it is suggested to
                                                 report zero angle when device is almost parallel to
                                                 the off screen surface.
                                                 This means that when:
                                                 0 < angle < 0 + thres OR 360 > angle > 360 - thres
                                                 the angle is treated as 0.
usf_epos_hover_cursor_mode               -       Controls the behavior of the pen hovering cursor.
                                                 [0] hovering cursor is not shown
                                                 [1] hovering cursor will be shown when pen hovers in the
                                                     "palm rejection" area (the Z area near the screen
                                                     where touch is disabled)
usf_epos_battery_low_level_threshold     -       Battery level which indicates of low battery level.
                                                 The value is in percents.


------------------------------
  Epos: Power saving parameters
------------------------------
ps_act_state_params                             1) Destination "power save" (PS) states:
                                                             PS_STATE_ACTIVE =0,
                                                             PS_STATE_STANDBY,
                                                             PS_STATE_IDLE,
                                                             PS_STATE_OFF,
                                                2) Destination timeout (sec)
                                                Example:
                                                   ps_act_state_params     [0,10]
                                                   ps_standby_state_params [2,30]
                                                   ps_idle_state_params    [3,60]

ps_idle_detect_port                             US detection port (in mics enumeration) in idle state
                                                Example:
                                                   ps_idle_detect_port [17]

ps_idle_detect_period                           US detection period (in sec) in idle state.
                                                Value 0 - contininue detection mode.
                                                Other value - one shot detection mode
                                                Example:
                                                   ps_idle_detect_period [20]

ps_standby_detect_calibration                   Path of calibration data file for US detection
                                                Example:
                                                   ps_standby_detect_calibration [/data/usf/epos/liquid_ps_tuning1.bin]
                                                   ps_idle_detect_calibration    [/data/usf/epos/liquid_ps_tuning1.bin]

------------------------------
  private parameters of P2P
------------------------------
usf_p2p_device_uid                      -      [<deviceUID>]. <deviceUID> can be number between 0 to 2 (included).
                                               This param gives the device a unique id to distinct it
                                               from the other two devices in a group of 3 devices.
usf_p2p_event_dest                      -      [1] for UAL, [2] for out (socket), [3] for both.
                                               This param is referenced as bitmask - for future use, if more destinations
                                               is needed it will be [4] then [8] etc.
                                               Any combination of destinations is a sum of the basic destinations
                                               (for example 3 for UAL and socket = 1 for UAL + 2 for socket).
usf_p2p_pattern_type                    -      [<patternType>]. <patternType> can be number between 0 to 2 (included)
                                               Each number (0-2) represent family of orthogonal patterns.
usf_p2p_event_out_port                  -      [<portNumber>]. This is the port number of the socket if
                                               event_dest is 2 or 3.
usf_p2p_skip                            -      [<skip_factor>]. This is for statistics. The <skip_factor> number is match
                                               to the skip number in the usf_rx_transparent_data.


------------------------------
  private parameters of Hovering
------------------------------
usf_hovering_event_dest                 -      [1] for UAL, [2] for out (socket), [3] for both.
                                               This param is referenced as bitmask - for future use, if more destinations
                                               is needed it will be [4] then [8] etc.
                                               Any combination of destinations is a sum of the basic destinations
                                               (for example 3 for UAL and socket = 1 for UAL + 2 for socket).
usf_hovering_event_out_port             -      [<portNumber>]. This is the port number of the socket if
                                               event_dest is 2 or 3.
usf_hovering_skip                       -      [<skip_factor>]. This is for statistics. The <skip_factor> number is match
                                               to the skip number in the usf_rx_transparent_data.

------------------------------
  private parameters of Gesture
------------------------------
usf_gesture_event_dest                  -      [1] for UAL, [2] for out (Socket or External framework), [3] for both.
                                               This param is referenced as bitmask - for future use, if more destinations
                                               is needed it will be [4] then [8] etc.
                                               Any combination of destinations is a sum of the basic destinations
                                               (for example 3 for UAL and socket = 1 for UAL + 2 for socket).
usf_gesture_event_out_port              -      [<portNumber>]. This is the port number of the socket if
                                               event_dest is 2 or 3.
usf_gesture_skip                        -      [<skip_factor>]. This is for statistics. The <skip_factor> number is match
                                               to the skip number in the usf_rx_transparent_data.
usf_gesture_keys                        -      [<key1>,<key2>,<key3>,<key4>]. <keyx> is the key x identifier.
usf_gesture_app_lib_bypass              -      A parameter to disable apps library and report only lpass events.
                                               The parameter is used for lpass library recording.
                                               [1] - bypass apps library.
                                               [0] (default) - do not bypass apps library.
usf_algo_transparent_data_file          -      File name of the sync gesture algorithm transparent data. This data is passed to the sync
                                               gesture library which uses this data as configuration data.

------------------------------
  private parameters for DSP
  calculated events
------------------------------
usf_output_type                         -      Output type of the DSP, when set, first bit is for raw data, second bit is for
                                               proximity static, and third bit is for proximity motion, fourth bit is for
                                               sync gesture event from the DSP.
