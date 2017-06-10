/*==========================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

===========================================================*/
{
  {
    {
      /* header_version */
      0x208,
      /* cam_name */
      ACTUATOR_MAIN_CAM_0,
      /* module_name */
      "sony",
      /* actuator_name */
      "iu074",
    },
  /* af_tune_parms */
/******************************************************************************/
// Feature name : The following parameters are auto focus related. They should be
//           obtained during AF characterization process.
// Applicable chipset: All.
// Applicable operation mode: Viewfinder.
//
// Variable name: num_gross_steps_between_stat_points.
// Used to control how rough initial AF search (coarse search) is.
// 3A version: 3.0
// Default value: 4.
// Data range: 3 to 6.
// Constraints: None.
// Effect: Larger value means more displacement between initial sampled
//         points.  This would require more num_fine_search_points during
//         subsequent search (fine search) to locate optimal ending AF lens
//         position.
//
// Variable name: num_fine_steps_between_stat_points.
// Used to control how precise subsequent AF search (fine search) is.
// 3A version: 3.0
// Default value: 1.
// Data range: 1 or 2.
// Constraints: Less than num_gross_steps_between_stat_points
// Effect: The bigger the value is, the less likely AF lens ends in optimal
//         position.
//
// Variable name: num_fine_search_points.
// Used to control how many search points to be gather in fine search
// 3A version: 3.0
// Default value: 8.
// Data range: Fixed as below.
// Constraints: It is set to 2*num_gross_steps_between_stat_points to cover
//              entire range of coarse search's neighboring sampled points.
// Effect: If it is less than 2*num_gross_steps_between_stat_points, AF
//         precision maybe lost.
//
// Variable name: position_near_end.
// Used to control how far lens can move away from mechanical stop.  It is
// defined in term of indices, where position_far_end > position_near_end.
// 3A version: 3.0
// Default value: 0.
// Data range: 0 to (position_far_end – 1).
// Constraints: Less than position_far_end. Total steps for AF lens to travel
//              = position_far_end - position_near_end.  For sanity check, it
//              should be more than 20 steps.
// Effect: Non-zero means we are limiting AF travel range even more than the
//         values obtained from AF tuning.  For example, if AF lens on the final
//         production modules move 8 steps beyond the necessary MACRO focused
//         distance, we can reduce travel range by setting position_near_end
//         to 8 (or less to account for module-to-module variation).
//
// Variable name: position_default_in_macro.
// Placeholder. For future use.
//
// variable name: position_boundary
// Used to control how far lens can move away from mechanical stop in NORMAL search
// mode.
// 3A version: 3.0
// Default value: 0 (To be tuned later).
// Data range: 0 to (position_far_end – 1).
// Constraints: Less than position_far_end.
// Effect: The closer it is to position_far_end, the less steps AF lens is allowed
//         to travel in NORMAL search mode.
//
// variable name: position_default_in_normal.
// Placeholder. For future use
//
// variable name: undershoot_protect
// Boolean flag to enable/disable the feature
// 3A version: 3.0
// default value: 0 (disable)
// data range: 0 (enable) or 1 (disable)
// constraints: the degree of protection from undershoot will be depends on undershoot_adjust
//              variable
// effect: If this feature is enabled, lens will move more in one direction over the other direction.
//                               This is needed when it is determined that AF actuator has severe hysteresis on its movement
//         during characterization process. The feature compensate hysteresis by moving the lens more
//         in either forward or backward direction.
//
//
// variable name: undershoot_adjust;
// Used when undershoot protection is enabled.
// 3A version: 3.0
// default value: 0
// data range: 0 to (coarse step size - 1)
// constraints: As noted above, number greater than or equal to coarse step size is not
//              recommended.
// effect: When feature is turned on, the feature will compensate the undershoot movement
//         of lens (mainly due to severe hysteresis) by moving extra step specified in this
//         variable.
//
/******************************************************************************/

    4, /* Num Gross Steps Between Stat Points */
    2, /* Num Fine Steps Between Stat Points */
    8, /* Num Fine Search Points */
    1, /* Process Type */
    0, /* Near End */
    32, /* Default In Macro */
    15, /* Boundary */
    41, /* Defult In Normal */
    41, /* Far End */
    1, /* Undershoot Protect */
    0, /* Undershoot Adjust */

/******************************************************************************/
// applicable chipset: All.
// applicable operation mode: Snapshot Auto Focus
//
// variable name: reset_lens_after_snap
// Type: boolean
// Forces the AF lens to reset to infinity position after snapshot capture
// 3A version: All
// default value: 1
// data range: 0 to 1
// constraints: None
// effect: If 0 is selected, AF lens will remain in last AF position after
// after snapshot.
//         If 1 is selected, AF lens will move back to infinity or
// RESET position after snapshot.  This could save power for certain actuators
// like voice coil.  It will help reduce AF latency by setting starting position
// at infinity prior to AF search.
/******************************************************************************/

    1, /* Reset Lens After Snap */
/* Config */
    {

/*********************************************************************/
// Feature name :
// Min Y and max Y are used to limit pixels being used for focus value
// calculation to only pixels with Y values between minY and maxY.
/*********************************************************************/

     0, /* Minimum Y */
     255, /* Maximum Y */

/*********************************************************************/
// Feature name :
// This group of parameters defines the coordinates of single AF window.
//
// Horizontal Offset Ratio:
// Horizontal location of first pixel in terms of the ratio to the whole frame size.
// For example, image width is 1000, we want to use the middle 500
// as AF window. Horizontal offset ratio is 250/1000=0.25.
// default: 0.25.
//
// Vertical Offset Ratio:
// Similar to Horizontal Offset Ratio, but this is in the veritcal direction.
// default: 0.25.
//
// Horizontal Clip Ratio:
// AF window horizontal size in terms of ratio to the whole image. For the
// same example above, Horizontal Clip Ratio is 500/1000=0.5.
// default: 0.5.
//
// Vertical Clip Ratio:
// AF window veritical size in terms of ratio to the whole image. For the
// same example above, Vertical Clip Ratio is 500/1000=0.5.
// default: 0.5.
//
/*********************************************************************/

      0.250000f, /* Horizontal Offset Ratio */
      0.250000f, /* Vertical Offset Ratio */
      0.500000f, /* Horizontal Clip Ratio */
      0.500000f, /* Vertical Clip Ratio */
    },

/*********************************************************************/
// Feature name :
//
// Focus value metric. 0 means sum, 1 means max.
/*********************************************************************/

    0, /* FV Metric */

/*********************************************************************/
// Feature name :
// Highpass filter coeffs used for focus value calculation.
// range: -16 to 15.
/*********************************************************************/

/* AF VFE HPF */
    {
      0, /* A00 */
      -2, /* A02 */
      -2, /* A04 */
      15, /* A20 */
      0, /* A21 */
      -2, /* A22 */
      -4, /* A23 */
      -5, /* A24 */
    },
/* AF Shake Resistant */
    {

/******************************************************************************/
// Feature name: AF shake resistant.
// Applicable chipset: all chipsets.
// 3A version: 1.8.
// Applicable operation mode: Viewfinder and snapshot.
//
// Variable name: AF_shake_resistant_enable.
// Enables and disables the feature.
// Default value: 1.
// Data range: 0 or 1.
// Constraints: None.
// Effect: Enables or disables the featue.
//
// Variable name: AF_max_gain (overall gain).
// Member of AF_shake_resistant_type.
//  Used to define the maximum gain allowed when the gain is boosted
//  under low light.
// Default value (calculated): 4 * Max Exposure Table Gain
// Data range: 4 to 10X max preview gain of exp table.
// Constraints: This value will limit the max_af_tradeoff_ratio applied.
// Effect: The bigger the value, the shorter the exposure time will be,
//         increasing the noise level.
//
// Variable name: AF_min_frame_luma.
// Member of AF_shake_resistant_type.
//  The minimum frame luma allowed below which shake-resistant AF will
//  be disabled.
// Default value: 0 (to be tuned later).
// Data range: 0~luma_target.
// Constraints: It should be greater than the value at which the frame is too dark
//              for AF to work successfully.
// Effect: The smaller this value is set, the darker lighting condition under which
//         shake-resistant AF will be turned off.
//
// Variable name: AF_tradeoff_ratio.
// Member of AF_shake_resistant_type.
//  Used to define how much the exposure time should be reduced.
// Default value: 4.
// Data range: 1~4.
// Constraints: The value should be greater than or equal to 1.
// Effect: The bigger the value is, the smaller the adjusted exposure time would be.
//
// Variable name: AF_shake_resistante_toggle_frame_skip.
// Member of AF_shake_resistant_type.
//  Sets number of frames to skip or drop from preview when this feature is called.
// Default value: 2.
// Data range: 0 to 4.
// Constraints: None.
// Effect: Will appear as preview is frozen for amount of frames set whenever
//         AF is started and finished.
//
/******************************************************************************/

      0, /* Enable */
      32.000000f, /* Max Gain */
      0, /* Min Frame Luma */
      4.000000f, /* AF Tradeoff Ratio */
      2, /* Toggle Frame Skip */
  },

/******************************************************************************/
// Feature name : AF scene change detection.
// applicable chipset: MSM7x01 and newer chipsets.
// applicable operation mode: View finder, snapshot and video
//
// variable name: AF_scene_change_detection_ratio
// Used to control how scene change should be detected for continuous AF
// 3A version: 1.5
// default value: 4
// data range: 1~8
// constraints: the gain should be not be smaller than 1.
// effect: The bigger the value is, the more sensitive the continuous AF
//         responds to the scene change.
//
// variable name: AF_peak_drop_down_factor
// Used to control the variation of current FV from peak FV
// 3A version: all.
// default value: 0.9
// data range: 0.8~0.9
// constraints: it should be smaller than 1 and greater than 0.
// effect: The bigger the value is, the smaller variation is allowed between the
//         current FV and the maximum FV.
/******************************************************************************/

    4, /* Scene Change Detection Ratio */
    0.800000f, /* Peak Drop Down Factor */
/* CAF */
    {

/******************************************************************************/
// Feature name : AF_CAF_type to aide AF configuring CAF behavior.
// Applicable chipset: MSM8x50 and newer chipsets.
// Applicable operation mode: Viewfinder and video.
//
// Variable name: af_cont_lux_index_change_threshold.
// exp_index delta or lux_idx > af_cont_lux_index_change_threshold then target change is
// considered to have occurred.
// 3A version: 3.0.
// Default value: 23.
// Data range: 0 to 100.
// Constraints: Nonen
// Effect: Smaller setting will cause CAF to trigger search easier.  May cause
// instability if set too small.  If value is set too high, lux_idx and/or
// exp_index change will not trigger a scene change, therefore may not trigger
// a new search.
//
// Variable name: af_scene_change_detection_ratio.
// FV change to trigger a target change, following with a new focus search.
// 3A version: 3.0
// Default value: 4.
// Data range: 0 to 60.
// Constraints: None.
// Effect: Higher value makes it easier to trigger a new search.  Smaller value
// makes it harder to trigger a search due to FV change.
//
// Variable name: af_panning_stable_fv_change_trigger.
// FV change vs. past frame FV to trigger to determine if scene is stable.
//           If ( |FV[i]-FV[i-1]|*t > FV[i-1]), not stable.
//
// 3A version: 3.0
// constraints: None.
// Effect: Higher value makes it harder for scene to be determined as stable.
//
// Variable name: af_panning_stable_fvavg_to_fv_change_trigger.
// FV change vs. average of 10 past frame's FV to trigger to determine if
// scene is stable.
//           If ( |FV[i]-FVavg|*t > Fvavg), not stable.
// 3A version: 3.0
// Constraints: None.
// Effect: Higher value makes it harder for scene to be determined as stable.
//
// Variable name: af_panning_unstable_trigger_cnt.
// How many panning unstable detections before triggering a scene change.
// Video mode to have different settings than camera.
// 3A version: 3.0
// Constraints: None.
// Effect: Higher value makes it harder for scene to be determined as stable.
//
// Variable name: af_Basedelay_normal.
// Number of frames to skip after every lens movement command.
// Suggest video mode to have different than camera mode.
// 3A version: 3.0
// Default value: 1.
// Data range: 0 to 10.
// Constraints: None.
// Effect: Higher values makes CAF search slower but tends to be more stable.
//
// Variable name: af_scene_change_trigger_cnt.
// Number of consecutive random scene change frames to determine need for
// new AF search.
// 3A version: 3.0
// Default value: 5.
// Data range: 0 to 150.
// Constraints: None.
// Effect: Higher values makes CAF harder to start a new search, for example,
// it is useful when a scene has movement but do not want to trigger
// a new CAF search.
//
// Variable name: af_downhill_allowance.
// Number of extra steps to search once peak FV is found
// new AF search.
// 3A version: 3.0
// Default value: 3.
// Data range: 0 to 10.
// Constraints: None.
// Effect: Higher value will cause focus search to go beyond peak this amount
// of frames then return.  Higher values is less prone to get AF stuck in local
// maximum but it takes longer and user experience is reduced.  Smaller values
// has better user experience and time but may cause AF to focus on local maximum.
/******************************************************************************/

      1, /* CAF Enable */
      6, /* Scene Change Detection Ratio */
      5, /* Panning Stable FV Change Trigger */
      12, /* Panning Stable FV Avg to FV Change Trigger */
      16000, /* Panning Unstable Trigger Count */
      5, /* Scene Change Trigger Count */
      2, /* Downhill Allowance */

/******************************************************************************/
// Feature name: Continuous AF.
// Variable name: af_cont_base_frame_delay.
// How many frames to wait after lens move.
// Applicale chipset(s): 7K and 8K.
// Applicable operation mode:  Continuous AF.
// Default value:  2.
// Data range: 2 ~ 5.
// Constraints: Integers.
// Effect: Bigger in value represents longer waiting time.
/******************************************************************************/

      0, /* Continuous Base Frame Delay */

/******************************************************************************/
// Feature name: Continuous AF.
// Variable name: af_cont_lux_index_change_threshold.
// Threshold above which the change of lux.
// Index will trigger the continuous AF search.
// Applicale chipset(s): 7K and 8K.
// Applicable operation mode:  Continuous AF.
// Default value:  10.
// Data range: > 0.
// Constraints: Integers.
// Effect: Refocusing is needed when exp change > threshold.
/******************************************************************************/

      23, /* Continuous Lux Index Change Threshold */

/******************************************************************************/
// Feature name: Continuous AF.
// Variable name: af_cont_threshold_in_noise.
// Determine if the variation in FV is
// caused by noise.
// Applicale chipset(s): 7K and 8K.
// Applicable operation mode:  Continuous AF.
// Default value:  0.05
// Data range: > 0.
// Constraints: Float.
// Effect: (FV1 - FV0)/FV1 (FV1>FV0), noise if this value < threshold,
//         otherwise, start FV search.
/******************************************************************************/

      0.050000f, /* Continous Threshold in Noise */
      2, /* Continuous Search Step Size */
      0, /* do full-sweep everytime or hill climb*/
    },

/******************************************************************************/
// Feature name : snapshot AF delay
// Applicable chipset: MSM8x50 and newer chipsets.
// Applicable operation mode: Applies to snapshot AF only.
//
// Variable name: basedelay_snapshot_AF.
// Number of frames to skip after lens move is complete during AF snapshot
// search.
// 3A version: 3.0
// Default value: 0.
// Data range: 0 to 10.
// Constraints: None.
// Effect: Set to 0 is fast exhaustive search method.  Setting to higher value
// is normal exhaustive search.
/******************************************************************************/

    0, /* Basedelay Snapshot AF */

/* AF Motion Sensor */
    {

/******************************************************************************/
// Feature name : AF_motion_sensor_type to aide AF in determining a change in
//           scene.
// Applicable chipset: MSM7x30, QSD8x60, and newer chipsets.
// Applicable operation mode: Viewfinder and video
//
// Variable name: af_gyro_trigger.
// Used to control how scene change should be detected for AEC.
// 3A version: 3.0
// Default value: 0.0
// Data range: -16000.0 to +16000.0
// Constraints: None.
// Effect: The bigger the value is, the less sensitive AEC response to
//         gyro output value.
//
// Variable name: af_accelerometer_trigger.
// Used to control how scene change should be detected for AEC.
// 3A version: 3.0
// Default value: 0.0
// Data range: -16000.0 to +16000.0
// Constraints: None.
// Effect: The bigger the value is, the less sensitive AEC response to
//         accelerometer change.
//
// Variable name: af_magnetometer_trigger.
// Used to control how scene change should be detected for AEC.
// 3A version: 3.0
// Default value: 0.0
// Data range: 0.0 to 360.0
// Constraints: None.
// Effect: The bigger the value is, the less sensitive AEC response to
//         magnetic field change.
//
// Variable name: af_DIS_motion_vector_trigger.
// Used to control how scene change should be detected for AEC.
// 3A version: 3.0
// Default value: 0.0
// Data range: -16000.0 to +16000.0
// Constraints: None.
// Effect: The bigger the value is, the less sensitive AEC response to
//         digital image stabilization movement/compensation value.
/******************************************************************************/

      0.000000f, /* Gyro Trigger */
      0.000000f, /* Accelerometer Trigger */
      0.000000f, /* Magnetometer Trigger */
      0.000000f, /* DIS Motion Vector Trigger */
    },

/******************************************************************************/
// Feature name: LED assisted AF.
// 
// Variable name: led_af_assist_enable.
// Enable or disable the LED assist for auto focus feature.
// Default value: 1.
// Data range: 1 or 0.
// Constraints: None.
// Effect: LED auto focus assist can is enable.
//
// Variable name: led_af_assist_trigger_idx.
// Lux Index at which LED assist for autofocus is enabled.
// Default value (calculated): wLED Trigger Index
// Data range: 0 to 1000.
// Constraints: None.
// Effect: Selects scene brightness level at which LED auto focus assist can be enabled.
/******************************************************************************/

    1, /* LED AF Assist Enable */
    367, /* LED AF Assist Trigger Idx */

    /* af_sp_tuned_params_t */
    {
      0.9, /* FV curve flat threshold */
      0.9, /* Threshold1 */
      1.1, /* Threshold2 */
      0.5, /* Threshold3 */
      3, /* Threshold4 */
      33, /* Lens position for object at 3m */
      29, /* Lens position for object at 70cm */
      25, /* Lens position for object at 30cm */
      21, /* Lens position for object at 20cm */
      13, /* Lens position for object at 10cm */
      7, /* Lens position for Macro */
      1, /* frame delay */
      2, /* Maximum consecutive downhill in first round */
      1, /* Maximum consecutive downhill in 2 or 3 round */
    },

    /* actuator_params_t */
    {
      /* i2c_addr */
      0xE4,
      /* i2c_data_type */
      MSM_ACTUATOR_BYTE_DATA,
      /* i2c_addr_type */
      MSM_ACTUATOR_BYTE_ADDR,
      /* act_type */
      ACTUATOR_PIEZO,
      /* data_size */
      8,
      /* af_restore_pos */
      0,
      /* msm_actuator_reg_tbl_t */
      {
        /* reg_tbl_size */
        1,
        /* msm_actuator_reg_params_t */
        {
          /* reg_write_type;hw_mask; reg_addr; hw_shift >>; data_shift << */
          {MSM_ACTUATOR_WRITE_DAC, 0x00000080, 0x0000, 0, 0},
        },
      },
      /* init_setting_size */
      8,
      /* init_settings */
      {
        {0x01, 0xA9},
        {0x02, 0xD2},
        {0x03, 0x0C},
        {0x04, 0x14},
        {0x05, 0xB6},
        {0x06, 0x4F},
        {0x00, 0x7F},
        {0x00, 0x7F},
      },
    },

    /* actuator_tuned_params_t */
    {
      /* scenario_size */
      {
        /* scenario_size[MOVE_NEAR] */
        1,
        /* scenario_size[MOVE_FAR] */
        1,
      },

      /* ringing_scenario */
      {
        /* ringing_scenario[MOVE_NEAR] */
        {
          41,
        },
        /* ringing_scenario[MOVE_FAR] */
        {
          41,
        },
      },

      /* intial_code */
      0x7F,
      /* region_size */
      1,

      /* region_params */
      {
        /* step_bound[0] - macro side boundary */
        /* step_bound[1] - infinity side boundary */
        /* Region 1 */
        {
          .step_bound = {41, 0},
          .code_per_step = 2,
        },
      },

      {
        /* damping */
        {
          /* damping[MOVE_NEAR] */
          {
            /* scenario 1 */
            {
              /* region 1 */
              {
                .damping_step = 0xFF,
                .damping_delay = 0,
                .hw_params = 0x80,
              },
            },
          },
        },

        {
          /* damping[MOVE_FAR] */
          {
            /* scenario 1 */
            {
              /* region 1 */
              {
                .damping_step = 0xFF,
                .damping_delay = 0,
                .hw_params = 0x00,
              },
            },
          },
        },
      },
    },
  },
},
