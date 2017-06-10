/*============================================================================

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
  {
    /* af_header_info_t */
    {
      /* header_version */
      0x301,
      /* module_name */
      "SUNNY_13P1BA",
      /* actuator_name */
      "dw9714_13p1ba",
      /* af_camera_name */
      0,
    }, /* af_header_info_t */

    /* af_tuning_algo_t */
    {
      /* Variable name: af_process_type.
       * Defines which AF algorithm to use -
       *   Exhaustive/Slope-predictive/Continuous.
       * 3A version:
       * Default value: AF_EXHAUSTIVE_SEARCH.
       * Data range: based on af_algo_type
       */
      AF_EXHAUSTIVE_SEARCH,

      /* Variable name: position_near_end.
       * Used to control how far lens can move away from mechanical stop.  It
       * is defined in term of indices, where position_far_end >
       * position_near_end.
       * 3A version:
       * Default value: 0.
       * Data range: 0 to (position_far_end - 1).
       * Constraints: Less than position_far_end. Total steps for AF lens to
       *              travel = position_far_end - position_near_end.  For
       *              sanity check, it should be more than 20 steps.
       * Effect: Non-zero means we are limiting AF travel range even more than
       *         the values obtained from AF tuning.  For example, if AF lens
       *         on the final production modules move 8 steps beyond the
       *         necessary MACRO focused distance, we can reduce travel range
       *         by setting position_near_end to 8 (or less to account for
       *         module-to-module variation).
       */
      0,

      /* Variable name: position_default_in_macro.
       * Gives default rest position of lens when focus mode is Macro.
       * 3A version:
       * Default value: 0.
       * Data range: 0 to position_far_end.
       */
      140,

      /* Variable name: position_boundary.
       * Used to control how far lens can move away from mechanical stop in
       * NORMAL search mode.
       * 3A version:
       * Default value: 0.
       * Data range: 0 to (position_far_end - 1).
       * Constraints: Less than position_far_end.
       * Effect: The closer it is to position_far_end, the less steps AF lens is allowed
       * to travel in NORMAL search mode.
       */
      200,

      /* Variable name: position_default_in_normal.
       * Gives default rest position of lens when focus mode is Normal/Auto.
       * mode.
       * 3A version:
       * Default value: position_far_end.
       * Data range: 0 to position_far_end.
       */
      270,

      /* Variable name: position_far_end.
       * Used to control how far lens can move away from mechanical stop.  It is
       * defined in term of indices, where position_far_end > position_near_end.
       * 3A version:
       * Default value: actuator infinity position
       * Data range: 1 to infinty
       * Constraints:
       * Effect: Non-zero means we are limiting AF travel range even more than the
       *         values obtained from AF tuning.
       */
      335,

       /* Variable name: position_normal_hyperfocal.
        * Gives default position of lens when focus mode is Normal/Auto and
        * focus fails.
        * 3A version:
        * Default value: position_far_end.
        * Data range: 0 to position_far_end.
        */
      280,

      /* Variable name: position_macro_rgn.
       * Starting lens position of macro region.
       * 3A version:
       * Default value: tunable..
       * Data range: 0 to position_far_end.
       */
      170,
      /* Variable name: undershoot_protect.
       * Boolean flag to enable/disable the feature
       * 3A version:
       * Default value: 0 (disable)
       * Data range: 0 (enable) or 1 (disable)
       * Constraints: the degree of protection from undershoot will be depends
       *              on undershoot_adjust variable
       * Effect: If this feature is enabled, lens will move more in one
       *         direction over the other direction. This is needed when
       *         it is determined that AF actuator has severe hysteresis on
       *         its movement during characterization process. The feature
       *         compensate hysteresis by moving the lens more
       *         in either forward or backward direction.
     */
      0,

      /* Variable name: undershoot_adjust.
       * Used when undershoot protection is enabled.
       * 3A version:
       * Default value: 0
       * Data range: 0 to (coarse step size - 1)
       * Constraints: As noted above, number greater than or equal to coarse
       *              step size is not recommended.
       * Effect: When feature is turned on, the feature will compensate the
       *         undershoot movement of lens (mainly due to severe hysteresis)
       *         by moving extra step specified in this variable.
     */
      0,

      /* Variable name: min_max_ratio_th.
       * If focus value drops below this much of maximum fv, search forward
       * is stopped.
       * 3A version:
       * Default value: 0.5
       * Data range: 0 to less than 1
       * Constraints: Value less than 0.75 is recommended.
       * Effect: Increasing this value makes it easier to stop the search
       * forward.
     */
      0.97,

      /* Variable name: lef_af_assist_enable.
       * Enable or disable the LED assist for auto focus feature.
       * 3A version:
       * Default value: 0.5
       * Data range: 1 or 0.
       * Constraints: None
       * Effect: LED auto focus assist is enabled.
       */
      1,

      /* Variable name: led_af_assist_trigger_idx.
       * Lux Index at which LED assist for autofocus is enabled.
       * 3A version:
       * Default value: wLED Trigger Index (calculated)
       * Data range: 0 to 1000
       * Constraints: None
       * Effect: Selects scene brightness level at which LED auto focus assist
       * can be enabled.
       */
      367,

      /* Variable name: lens_reset_frame_skip_cnt
       * How many frames to skip after resetting the lens
       * 3A version:
       * Default value: 2
       * Data range: 2 - 6
       * Constraints: Integers
       * Effect: Bigger in value represents longer waiting time.
       */
       2,

      /* Variable name: low_light_gain_th
       * When the aec gain is above this threshold, we assume it's low light condition.
       * 3A version:
       * Default value: 10
       * Data range:
       * Constraints:
       * Effect:
       */
      12,

      /* Variable name: base_delay_adj_th
       * Threshold to check while adjusting base delay for CAF. When fps drops
       * we'll need to reduce the base delay.
       * 3A version:
       * Default value: 0.034  (note this value has to be larger than 0.033 which matches 30 fps)
       * Data range:
       * Constraints:
       * Effect:
       */
      0.034f,

      /* af_tuning_continuous_t */
      {
        /* Variable name: enable
         * Enable/disable continuous autofocus.
         * 3A version:
         * Default value: 1
         * Data range: 0 or 1
         * Constraints: None
         * Effect: Continuous AF is enabled if set to 1.
       */
        1,

        /* Variable name: scene_change_detection_ratio
         * FV change to trigger a target change, following with a new focus search.
         * 3A version:
         * Default value: 4
         * Data range: 0 to 60
         * Constraints: None
         * Effect: Higher value makes it easier to trigger a new search.
         *         Smaller value makes it harder to trigger a search due
         *         to FV change.
         */
        4,

        /* Variable name: panning_stable_fv_change_trigger
         * FV change vs. past frame FV to trigger to determine if scene
         * is stable.
         *   If ( |FV[i]-FV[i-1]|*t > FV[i-1]), not stable.
         * 3A version:
         * Constraints: None
         * Effect: Higher value makes it harder for scene to be determined
         *         as stable.
         */
        0.0f,

        /* Variable name: panning_stable_fvavg_to_fv_change_trigger
         * FV change vs. average of 10 past frame's FV to trigger to determine
         * if scene is stable.
         *   If ( |FV[i]-FVavg|*t > Fvavg), not stable.
         * 3A version:
         * Constraints: None
         * Effect: Higher value makes it harder for scene to be determined
         *         as stable.
         */
        0.02f,

        /* Variable name: panning_unstable_trigger_cnt
         * How many panning unstable detections before triggering a
         * scene change.
         * 3A version:
         * Constraints: None
         * Effect: Higher value makes it harder for scene to be determined
         * as stable.
         */
        16000,

        /* Variable name: panning_stable_trigger_cnt
         * Number of consecutive stable frames after panning to required to trigger new search
         * 3A version:
         * Default value: 5
         * Data range: 0 to 150
         * Constraints: None
         * Effect: Higher values makes CAF harder to start a new search, for
         * example, it is useful when a scene has movement but do not want to
         * trigger a new CAF search.
         */
        3,

        /* Variable name: downhill_allowance
        * Number of extra steps to search once peak FV is found
        * 3A version:
        * Default value: 3
        * Data range: 0 to 10
        * Constraints: None
        * Effect: Higher value will cause focus search to go beyond peak this
        *         amount of frames then return.  Higher values is less prone to
        *         get AF stuck in local maximum but it takes longer and user
        *         experience is reduced.  Smaller values has better user
        *         experience and time but may cause AF to focus on local
        *         maximum.
        */
        3,

        /* Variable name: uphill_allowance
         * Number of steps we move if FV keeps increasing.
         * 3A version:
         * Default value: 3
         * Data range: 0 to 10
         * Constraints: None
         */
        3,

        /* Variable name: base_frame_delay
        * How many frames to wait after lens move.
        * 3A version:
        * Default value: 2
        * Data range: 2 - 6
        * Constraints: Integers
        * Effect: Bigger in value represents longer waiting time.
        */
        0,

        /* Variable name: scene_change_luma_threshold
         * Threshold above which the change of lux will trigger the
         * continuous AF search.
         * 3A version:
         * Default value: 10
         * Data range: > 0
         * Constraints: None
         * Effect: Refocusing is needed when exp change > threshold.
         */
        24,

                /* Variable name: luma_settled_threshold
         * AF calculates AEC settled condition as follows
         * if abs(Prev AF Luma - Current AF Luma) < luma_settled_threshold
         * then CAF can begin focus search without waiting for AEC to
         * completely settle
         * 3A version:
         * Default value: 10
         * Data range: > 0
         * Constraints: None
         * Effect: Smaller value represents longer wait time for AEC to settle
         */
        25,

        /* Variable name: noise_level_th
         * Determine if the variation in FV is caused by noise.
         * 3A version:
         * Default value: 0.05
         * Data range: > 0
         * Constraints: Float
         * Effect: (FV1 - FV0)/FV1 (FV1>FV0), noise if this value < threshold,
         * otherwise, start FV search.
         */
        0.05f,

        /* Variable name: search_step_size
         * Single step size while moving lens in continuous AF.
         * 3A version:
         * Default value: 2
         * Data range: 2 to 6
         * Constraints: Integer
         * Effect: Larger value would move lens faster but can cause jerkiness.
         */
        1,

        /* Variable name: init_search_type
         * When continuous af starts we run this algorithm to keep lens in
         * known position and enter monitor mode.
         * 3A version:
         * Default value: AF_EXHAUSTIVE_SEARCH
         * Data range: NA
         * Constraints: Should be valid algo type.
         */
        AF_EXHAUSTIVE_SEARCH,

        /* Variable name: search_type
         * When scene change is detected, we use this algorithm to find maximum
         * fv position.
         * 3A version:
         * Default value: AF_CONTINUOUS_SEARCH
         * Data range: NA
         * Constraints: Should be valid algo type.
         */
        AF_CONTINUOUS_SEARCH,

        /* Variable name: low_light_wait
         * How many extra frames to skip under low light condition
         * 3A version:
         * Default value: 0
         * Data range: 0 to 6
         * Constraints:
         */
        0,

        /* Variable name: max_indecision_cnt
         * maximum number of times to stay in make decision state while trying
         * to determine which direction to start new search in.
         * 3A version:
         * Default value: 1
         * Data range: 0 to 6
         * Constraints:
         * Effect: Higher value might give better result; trade-off is
         *   performance.
         */
        1,

        /* Variable name: flat_fv_confidence_level
         * Used for flat field detection. Determine how confidence we are that we
         * don't have flat FV curve by comparing min and max FV.
         * 3A version:
         * Default value: 0.95
         * Data range:
         * Constraints:
         * Effect:
         */
        0.94,

        /* af_tuning_sad_t */
        {
          /* Variable name: enable
           * enable/disable SAD scene-detection mechanism.
           * 3A version:
           * Default value: 1
           * Data range: 0 or 1
           * Constraints:
           * Effect:
           */
          1,

          /* Variable name: gain_min
           * minimum gain
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          2.0,

          /* Variable name: gain_max
           * maximum gain
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          30,

          /* Variable name: ref_gain_min
           * minimum reference gain
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          2.0,

          /* Variable name: ref_gain_max
           * minimum reference gain
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          30,

          /* Variable name: threshold_min
           * threshold when current gain is less than min gain
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          2,

          /* Variable name: threshold_max
           * threshold when current gain is more than max gain
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          3,

          /* Variable name: ref_threshold_min
           * threshold when current gain is less than min reference gain
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          3,

          /* Variable name: ref_threshold_max
           * threshold when current gain is more than max reference gain
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          6,

          /* Variable name: frames_to_wait
           * frames to wait before storing reference luma
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          3,
        }
        , /* af_tuning_sad_t */

        /* af_tuning_gyro_t */
        {
          /* Variable name: enable
           * enable/disable gyro assisted CAF.
           * 3A version:
           * Default value: 1
           * Data range: 0 or 1
           * Constraints:
           * Effect:
           */
          0,

          /* Variable name: min_movement_threshold
           * threshold above this means device is moving.
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          0.33,

          /* Variable name: stable_detected_threshold
           * device is be stable if above this threshold after panning.
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          0.015,

          /* Variable name: unstable_count_th
           * number of frames device was above movement threshold
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          3,

          /* Variable name: stable_count_th
           * number of frames we need to be stable after panning.
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          3,
          /* Variable name: fast_pan_threshold
           * Threshold to be consider as fast panning, comparing to
           *   gyro_data->sqr
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          0.1,
          /* Variable name: slow_pan_threshold
           * Threshold to be consider as slow panning, comparing to
           * gyro_data->sqr
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          0.04,
          /* Variable name: fast_pan_count_threshold
           * Threshold of fast panning cnt to trigger refocusing
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          8,
          /* Variable name: sum_return_to_orig_pos_threshold
           * apart from fast panning count, if the gyro data sum is
           * pass the threshold, it will trigger refocus
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          15,
          /* Variable name: stable_count_delay
           * Number of stable frame from gyro to be consider stable
           * long enough
           * 3A version:
           * Default value:
           * Data range:
           * Constraints:
           * Effect:
           */
          6,
        }
        , /* af_tuning_gyro_t */
      }
      , /* af_tuning_continuous_t */

      /* af_tuning_exhaustive_t */
      {
        /* Variable name: num_gross_steps_between_stat_points
         * Used to control how rough initial AF search (coarse search) is.
         * 3A version:
         * Default value: 4
         * Data range: 3 to 6
         * Constraints: None
         * Effect: Larger value means more displacement between initial sampled
         *   points.  This would require more num_fine_search_points during
         *   subsequent search (fine search) to locate optimal ending AF lens
         *   position.
         */
        36,

        /* Variable name: num_fine_steps_between_stat_points
         * Used to control how precise subsequent AF search (fine search) is.
         * 3A version:
         * Default value: 1
         * Data range: 1 o 2
         * Constraints: Less than num_gross_steps_between_stat_points
         * Effect: The bigger the value is, the less likely AF lens ends in
         *   optimal position.
         */
        9,

        /* Variable name: num_fine_search_points
         * Used to control how many search points to be gather in fine search
         * 3A version:
         * Default value: 8
         * Data range: Fixed
         * Constraints: It is set to 2 * num_gross_steps_between_stat_points to
         *  cover entire range of coarse search's neighboring sampled points.
         * Effect: If it is less than 2*num_gross_steps_between_stat_points, AF
         *   precision maybe lost.
         */
        8,

        /* Variable name: downhill_allowance
         * Number of extra steps to search once peak FV is found
         * 3A version:
         * Default value: 3
         * Data range: 0 to 10
         * Constraints: None
         * Effect: Higher value will cause focus search to go beyond peak this
         *   amount of frames then return.  Higher values is less prone to get
         *   AF stuck in local maximum but it takes longer and user experience
         *   is reduced. Smaller valueshas better user experience and time but
         *   may cause AF to focus on local maximum..
         */
        1,

                /* Variable name: uphill_allowance
         * Number of steps we move if FV keeps increasing.
         * 3A version:
         * Default value: 3
         * Data range: 0 to 10
         * Constraints: None
         */
        3,

                /* Variable name: base_frame_delay
         * Number of frames to skip after lens move is complete
         * 3A version:
         * Default value: 3
         * Data range: 0 to 10
         * Constraints: None
         * Effect:  Lower value gives faster response but jerkier. Higher value
         *  gives smooth response.
         */
        1,

        /* Variable name: coarse_frame_delay
         * Number of frames to skip after lens move is complete in coarse search
         * 3A version:
         * Default value: 0
         * Data range: 0 to 10
         * Constraints: None
         * Effect:  Lower value gives faster response but jerkier. Higher value
         *  gives smooth response.
         */
        1,

        /* Variable name: fine_frame_delay
         * Number of frames to skip after lens move is complete in fine search
         * 3A version:
         * Default value: 0
         * Data range: 0 to 10
         * Constraints: None
         * Effect:  Lower value gives faster response but jerkier. Higher value
         *  gives smooth response.
         */
        1,

        /* Variable name: coarse_to_fine_frame_delay
         * Number of frames to skip after lens move is complete in coarse search
         * and before starting the fine search
         * 3A version:
         * Default value: 1
         * Data range: 0 to 10
         * Constraints: None
         * Effect:  Lower value gives faster response but jerkier. Higher value
         *  gives smooth response.
         */
        1,

        /* Variable name: noise_level_th
         * Variation between last and current FV should be above this threshold,
         * otherwise the variation is considered to be due to noise.
         * 3A version:
         * Default value: 0.02
         * Data range:
         * Constraints:
         * Effect:
         */
        0.015f,

        /* Variable name: flat_fv_confidence_level
         * Used for flat field detection. Determine how confidence we are that we
         * don't have flat FV curve by comparing min and max FV.
         * 3A version:
         * Default value: 0.95
         * Data range:
         * Constraints:
         * Effect:
         */
        0.97f,

        /* Variable name: climb_ratio_th
         * Used for flat field detection. Cumulative focus curve inflections
         * less than this threshold denotes flat fv curve.
         * 3A version:
         * Default value: greater than 1
         * Data range:
         * Constraints:
         * Effect:
         */
        1.12f,

        /* Variable name: low_light_luma_th
         * Used for flat field detection. When the luma gets below this threshold, we
         * assume it's too dark to focus and report failure.
         * 3A version:
         * Default value: 4
         * Data range:
         * Constraints:
         * Effect:
         */
        4,
        /* Variable name: enable_multiwindow
         * Enable Flag for using Multi window or Single window AF stats.
         * 3A version:
         * Default value: 4
         * Data range:
         * Constraints:
         * Effect: 0 = Single Window, 1 = MultiWindow
         */
        0,
        /* Variable name: gain_thresh
         * Gain Threshold for triggering multi window AF stats
         * 3A version:
         * Default value: 4
         * Data range:
         * Constraints:
         * Effect: 0 = Single Window, 1 = MultiWindow
         */
        0,
      }
      , /* af_tuning_exhaustive_t */

      /* af_tuning_fullsweep_t */{
        /* Variable name: num_steps_between_stat_points
         * Used to control how many steps to move the lens at a time during
         * search.
         * 3A version:
         * Default value: 1
         * Data range: 1 to max steps
         * Constraints: None
         * Effect: Should always be 1, but for some tests could be more.
         */
        4,

        /* Variable name: frame_delay_inf
         * Number of frames to skip after lens move to initial (inf) position.
         * 3A version:
         * Default value: 2
         * Data range: 0 to 10
         * Constraints: None
         * Effect: Bigger value will give more time for the lens to settle
         *   after going into the inf. position.
         */
        2,

        /* Variable name: frame_delay_norm
         * Number of frames to skip after lens move to the next position.
         * 3A version:
         * Default value: 2
         * Data range: 0 to 10
         * Constraints: None
         * Effect: Bigger value will give more time for the lens to settle
         *   between steps.
         */
        1,

        /* Variable name: frame_delay_final
         * Number of frames to skip after lens move to its final position where
         * the maximum FV is registered.
         * 3A version:
         * Default value: 2
         * Data range: 0 to 10
         * Constraints: None
         * Effect: Bigger value will give more time for the lens to settle
         *   after going into the final position, so the FV can be observed in
         *   the logs.
         */
        2,
      }
      , /* af_tuning_fullsweep_t */

      /* af_tuning_sp_t */
      {
        /* Variable name: fv_curve_flat_threshold
         * threshold to determine if FV curve is flat
         * 3A version:
         * Default value: 0.9
         * Data range: 0 to 1
         * Constraints: None
         * Effect:
         */
        0.9,

        /* Variable name: slope_threshold1
         * sp threshold 1
         * 3A version:
         * Default value: 0.9
         * Data range: 0 to 1
         * Constraints: None
         * Effect:
         */
        0.9,

        /* Variable name: slope_threshold2
         * sp threshold 2
         * 3A version:
         * Default value: 1.1
         * Data range:
         * Constraints: None
         * Effect:
         */
        1.1,

        /* Variable name: slope_threshold3
         * sp threshold 3
         * 3A version:
         * Default value: 0.5
         * Data range:
         * Constraints: None
         * Effect:
         */
        0.5,

        /* Variable name: slope_threshold4
         * sp threshold 4
         * 3A version:
         * Default value: 3
         * Data range:
         * Constraints: None
         * Effect:
         */
        3,

        /* Variable name: lens_pos_0
         * Lens poisiton when the object is at 3m
         * 3A version:
         * Default value: Calculated
         * Data range:
         * Constraints: None
         * Effect:
         */
        340,

        /* Variable name: lens_pos_1
         * Lens poisiton when the object is at 70cm
         * 3A version:
         * Default value: Calculated
         * Data range:
         * Constraints: less than lens_pos_0
         * Effect:
         */
        304,

        /* Variable name: lens_pos_2
        * Lens poisiton when the object is at 30cm
        * 3A version:
        * Default value: Calculated
        * Data range:
        * Constraints: less than lens_pos_1
        * Effect:
        */
        232,

        /* Variable name: lens_pos_3
         * Lens poisiton when the object is at 20cm
         * 3A version:
         * Default value: Calculated
         * Data range:
         * Constraints: less than lens_pos_2
         * Effect:
         */
        124,

        /* Variable name: lens_pos_4
         * Lens poisiton when the object is at 10cm
         * 3A version:
         * Default value: Calculated
         * Data range:
         * Constraints: less than lens_pos_3
         * Effect:
         */
        88,

        /* Variable name: lens_pos_5
         * Lens poisiton when the object is at macro
         * 3A version:
         * Default value: Calculated
         * Data range:
         * Constraints: less than lens_pos_3
         * Effect:
         */
        52,

        /* Variable name: frame_delay
         * Number of frames to skip after lens move is complete
         * 3A version:
         * Default value: calculated
         * Data range:
         * Constraints: less than lens_pos_4
         * Effect:  Lower value gives faster response but jerkier. Higher value
         *  gives smooth response.
         */
        1,

        /* Variable name: downhill_allowance
         * max number of consecutive downhill in the first 4 or 6 samples.
         * 3A version:
         * Default value: 2
         * Data range: 0 to 10
         * Constraints: None
         * Effect: Higher value will cause focus search to go beyond peak this
         *   amount of frames then return.  Higher values is less prone to get
         *   AF stuck in local maximum but it takes longer and user experience
         *   is reduced. Smaller valueshas better user experience and time but
         *   may cause AF to focus on local maximum..
         */
        2,

        /* Variable name: downhill_allowance_1
         * max number of consecutive downhill in the first 2nd or 3rd round.
         * 3A version:
         * Default value: 1
         * Data range: 0 to 10
         * Constraints: None
         * Effect: Higher value will cause focus search to go beyond peak this
         *   amount of frames then return.  Higher values is less prone to get
         *   AF stuck in local maximum but it takes longer and user experience
         *   is reduced. Smaller valueshas better user experience and time but
         *   may cause AF to focus on local maximum..
         */
        1,
      }, /* af_tuning_sp_t */

      /* af_tuning_single_t */
      {
        /* Variable name: single_index_t
         *  Description: Index mapping from physical distance to lens position
         *  3A version: 4.1
         *  Default value: Measured Data
         *  Data range: 0 to far end lens index
         *  Constraints: None
         *  Effect: Determines boundaries of decision in Single AF algo
         */
        {
          10,  /* SINGLE_NEAR_LIMIT_IDX */
          1,   /* SINGLE_7CM_IDX        */
          79,  /* SINGLE_10CM_IDX       */
          131, /* SINGLE_14CM_IDX       */
          175, /* SINGLE_20CM_IDX       */
          203, /* SINGLE_30CM_IDX       */
          227, /* SINGLE_40CM_IDX       */
          235, /* SINGLE_50CM_IDX       */
          247, /* SINGLE_60CM_IDX       */
          267, /* SINGLE_120CM_IDX      */
          280, /* SINGLE_HYP_F_IDX      */
          335, /* SINGLE_INF_LIMIT_IDX  */
        },

        /* Variable name: actuator_type
         *  Description: Type of Actuator used in camera module
         *  3A version: 4.1
         *  Default value: ACT_TYPE_OPENLOOP
         *  Data range: ACT_TYPE_OPENLOOP or ACT_TYPE_CLOSELOOP
         *  Constraints: Valid Type
         *  Effect: Affects behaviors in algorithm
         */
        ACT_TYPE_OPENLOOP,

        /* Variable name: is_hys_comp_needed
         *  Description: Flag to indicate if needed hysteresis,
         *    depending on actuator
         *  3A version: 4.1
         *  Default value: FALSE (0)
         *  Data range:  0 or 1
         *  Constraints: Valid Type
         *  Effect: Affects behaviors in algorithm
         */
        0,

        /* Variable name: step_index_per_um
         *  Description: Number of step index per micro-meter
         *  3A version: 4.1
         *  Default value: 1
         *  Data range: positive integer
         *  Constraints: None
         *  Effect:
         */
        2,

        /* Variable name: TAF_step_table
         *  Description: PreScan and fine search step Size of different
         *    region from far end to near end of the region specified in
         *    single_optic_t srch region structure.
         *    entry 1 is from far end to hyperfocal
         *    entry 2 is from hyperfocal to rgn 1 boundary
         *    entry 2 is from rgn 1 to rgn 2 boundary
         *    entry 2 is from rgn 2 to rgn 3 boundary
         *    entry 2 is from rgn 3 to near end boundary
         *  3A version: 4.1
         *  Default value:
         *  Data range: 0 to far end lens position
         *  Constraints: setting too high of scan step will have adverse effect
         *    on actuator movement, result in bad stats.
         *  Effect: Higher number increase speed of search, but lower accuracy
         */
        {
          /* Variable name: Prescan_normal_light (step_size_t) */
          {15, 15, 15, 16, 16,},

          /* Variable name: Prescan_low_light (step_size_t) */
          {18, 18, 18, 18, 20,},

          /* Variable name: Finescan_normal_light (step_size_t) */
          {6, 6, 6, 6, 6,},

          /* Variable name: Finescan_low_light (step_size_t) */
          {8, 8, 8, 8, 8,},
        },

        /* Variable name: CAF_step_table
         *  Description: PreScan and fine search step Size of different
         *    region from far end to near end of the region specified in
         *    single_optic_t srch region structure.
         *    entry 1 is from far end to hyperfocal
         *    entry 2 is from hyperfocal to rgn 1 boundary
         *    entry 2 is from rgn 1 to rgn 2 boundary
         *    entry 2 is from rgn 2 to rgn 3 boundary
         *    entry 2 is from rgn 3 to near end boundary
         *  3A version: 4.1
         *  Default value:
         *  Data range: 0 to far end lens position
         *  Constraints: setting too high of scan step will have adverse effect
         *    on actuator movement, result in bad stats.
         *  Effect: Higher number increase speed of search, but lower accuracy
         */
        {
          /* Variable name: Prescan_normal_light (step_size_t) */
          {15, 15, 15, 16, 16,},

          /* Variable name: Prescan_low_light (step_size_t) */
          {18, 18, 18, 18, 20,},

          /* Variable name: Finescan_normal_light (step_size_t) */
          {6, 6, 6, 6, 6,},

          /* Variable name: Finescan_low_light (step_size_t) */
          {8, 8, 8, 8, 8,},
        },

        /* Variable name: PAAF_enable
         *  Description: Enable flag to use SW stats
         *    It is up to Algo to decide whether to enable sw stats in the end.
         *    For some modes in Video use case we cannot enable SW stats
         *  3A version: 4.1
         *  Default value: 1
         *  Data range: 0 or 1
         *  Constraints: None
         *  Effect: setting 0 will use HW stats.
         */
        1,

        /* Variable name: sw
         *  Description: Set of algorithm Thresholds to find peak len position
         *    flat threshold is to detect flat curve(increase and decrease)
         *    Macro threshold is to detect a peak in the macro area
         *    Drop threshold is to add confidence peak is present but focus
         *    curve is bad
         *    hist_thres: Threshold for Histogram value
         *
         *    reset of the sets of thresholds are to detect peak under
         *    different lighting condition
         *  3A version: 4.1
         *  Default value: 0.97
         *  Data range: 1 - 1.1 for flat_inc_thres , 0.5 - 1 for rest
         *  Constraints: None
         *  Effect: Lower value makes detection harder.
         */
        {
          /* Variable name: flat_inc_thres */
          1.015f,

          /* Variable name: flat_dec_thres */
          0.985f,

          /* Variable name: macro_thres */
          0.989f,

          /* Variable name: drop_thres */
          0.75f,

          /* Variable name: hist_dec_dec_thres */
          500,

          /* Variable name: hist_inc_dec_thres */
          500,

          /* Variable name: dec_dec_3frame */
          {0.97f, 0.97f, 0.97f, 0.95f, 0.94f, 0.94f, 0.94f, 0.80f,},

          /* Variable name: inc_dec_3frame */
          {0.97f, 0.97f, 0.97f, 0.97f, 0.96f, 0.96f, 0.96f, 0.96f,},

          /* Variable name: dec_dec */
          {0.97f, 0.97f, 0.97f, 0.96f, 0.96f, 0.95f, 0.95f, 0.75f,},

          /* Variable name: dec_dec_noise */
          {0.86f, 0.86f, 0.86f, 0.86f, 0.85f, 0.84f, 0.84f, 0.75f,},

          /* Variable name: inc_dec */
          {0.94f, 0.94f, 0.94f, 0.92f, 0.91f, 0.90f, 0.90f, 0.90f,},

          /* Variable name: inc_dec_noise */
          {0.94f, 0.94f, 0.94f, 0.92f, 0.91f, 0.90f, 0.90f, 0.90f,},

          /* Variable name: flat_threshold (BV_threshold_t) */
          {0.97f, 0.97f, 0.97f, 0.97f, 0.97f, 0.97f, 0.96f, 0.96f,},
        },

        /* Variable name: HW
         *  Description: Set of algorithm Thresholds to find peak len position
         *    flat threshold is to detect flat curve(increase and decrease)
         *    Macro threshold is to detect a peak in the macro area
         *    Drop threshold is to add confidence peak is present but focus
         *    curve is bad
         *    reset of the sets of thresholds are to detect peak under
         *    different lighting condition with only 3 samples, increase trend,
         *    decrease trend and trend with noise
         *  3A version: 4.1
         *  Default value: 0.97
         *  Data range: 1 - 1.1 for flat_inc_thres , 0.5 - 1 for rest
         *  Constraints: None
         *  Effect: Lower value makes detection harder.
         */
        {
          /* Variable name: flat_inc_thres */
          1.02f,

          /* Variable name: flat_dec_thres */
          0.986f,

          /* Variable name: macro_thres */
          0.989f,

          /* Variable name: drop_thres */
          0.75f,

          /* Variable name: hist_dec_dec_thres */
          500,

          /* Variable name: hist_inc_dec_thres */
          500,

          /* Variable name: dec_dec_3frame */
          {0.97f, 0.97f, 0.97f, 0.95f, 0.94f, 0.94f, 0.94f, 0.80f,},

          /* Variable name: inc_dec_3frame */
          {0.97f, 0.97f, 0.97f, 0.97f, 0.96f, 0.96f, 0.96f, 0.96f,},

          /* Variable name: dec_dec */
          {0.96f, 0.96f, 0.96f, 0.92f, 0.91f, 0.91f, 0.91f, 0.75f,},

          /* Variable name: dec_dec_noise */
          {0.86f, 0.86f, 0.86f, 0.86f, 0.85f, 0.84f, 0.84f, 0.75f,},

          /* Variable name: inc_dec */
          {0.94f, 0.94f, 0.94f, 0.92f, 0.91f, 0.90f, 0.90f, 0.90f,},

          /* Variable name: inc_dec_noise */
          {0.94f, 0.94f, 0.94f, 0.92f, 0.91f, 0.90f, 0.90f, 0.90f,},

          /* Variable name: flat_threshold (BV_threshold_t) */
          {0.97f, 0.97f, 0.97f, 0.97f, 0.97f, 0.97f, 0.96f, 0.96f,},
        },

        /* Variable name: BV_gain[6] (CUR_BV_INFO)
         *  Description: Gain thresholds to determine Lux level
         *  3A version: 4.1
         *  Default value: float
         *  Data range: float
         *  Constraints: Do not modify unless it causes issues
         *  Effect: changes Lux Level decision
         */
        {-30.0f, -2.00f, -0.50f, 0.60f, 2.50f, 3.20f, 6.00f, 9.00f,},

        /* Variable name: single_optic_t
         *  Description: All the optic information for setting up
         *    boundary for search regions in terms of lens position index measured
         *    CAF_far_end: far end search limit for CAF
         *    CAF_near_end: near end search limit for CAF
         *    TAF_far_end: far end search limit for TAF
         *    TAF_near_end: near end search limit for TAF
         *    srch_rgn_1: region boundary for non linear region for far end
         *    srch_rgn_2: region boundary for linear region
         *    srch_rgn_3: region boundary for non linear region for near end
         *    fine_srch_rgn: Boundary to use fine search from far end
         *    far_zone: zone boundary to consider far end for starting location
         *    near_zone: zone boundary to consider near end for starting location
         *    mid_zone: Boundary to consider middle zone (middle of far and near)
         *    far_start_pos: search start position when lens is at far zone
         *    near_start_pos: search start position when lens is at near zone
         *    init_pos: intial search poisiton for the first
         *  3A version: 4.1
         *  Default value:
         *    CAF_far_end:    SINGLE_INF_LIMIT_IDX
         *    CAF_near_end:   SINGLE_NEAR_LIMIT_IDX
         *    TAF_far_end:    SINGLE_INF_LIMIT_IDX
         *    TAF_near_end:   SINGLE_NEAR_LIMIT_IDX
         *    srch_rgn_1:     SINGLE_50CM_IDX
         *    srch_rgn_2:     SINGLE_20CM_IDX
         *    srch_rgn_3:     SINGLE_14CM_IDX
         *    fine_srch_rgn:  SINGLE_50CM_IDX
         *    far_zone:       SINGLE_INF_LIMIT_IDX
         *    near_zone:      SINGLE_10CM_IDX
         *    mid_zone:       SINGLE_60CM_IDX
         *  Data range:       SINGLE_120CM_IDX
         *  Constraints: Logically place these indexes
         *  Effect: check description
         */
        {
          SINGLE_INF_LIMIT_IDX,   /* Variable name: CAF_far_end     */
          SINGLE_NEAR_LIMIT_IDX,  /* Variable name: CAF_near_end    */
          SINGLE_INF_LIMIT_IDX,   /* Variable name: TAF_far_end     */
          SINGLE_NEAR_LIMIT_IDX,  /* Variable name: TAF_near_end    */
          SINGLE_50CM_IDX,        /* Variable name: srch_rgn_1      */
          SINGLE_20CM_IDX,        /* Variable name: srch_rgn_2      */
          SINGLE_14CM_IDX,        /* Variable name: srch_rgn_3      */
          SINGLE_50CM_IDX,        /* Variable name: fine_srch_rgn   */
          SINGLE_INF_LIMIT_IDX,   /* Variable name: far_zone        */
          SINGLE_10CM_IDX,        /* Variable name: near_zone       */
          SINGLE_30CM_IDX,        /* Variable name: mid_zone        */
          SINGLE_HYP_F_IDX,       /* Variable name: far_start_pos   */
          SINGLE_NEAR_LIMIT_IDX,  /* Variable name: near_start_pos  */
          SINGLE_120CM_IDX,       /* Variable name: init_pos        */
        },
      }, /* af_tuning_single_t */

      /* af_shake_resistant_t */
      {
        /* Variable name: enable
         * Enables and disables the feature.
         * 3A version:
         * Default value: 1
         * Data range: 0 or 1
         * Constraints: None
         * Effect:  Enables or disables the featue.
         */
        0,

        /* Variable name: max_gain
         * Used to define the maximum gain allowed when the gain is boosted
         *   under low light.
         * 3A version:
         * Default value: 4 * Max Exposure Table Gain (Calculated)
         * Data range: 4 to 10X max preview gain of exp table.
         * Constraints: This value will limit the max_af_tradeoff_ratio applied.
         * Effect:  The bigger the value, the shorter the exposure time will be,
         *   increasing the noise level.
         */
        4.000000f,

        /* Variable name: min_frame_luma
         * The minimum frame luma allowed below which shake-resistant AF will
         *   be disabled.
         * 3A version:
         * Default value: 0 (to be tuned later).
         * Data range: 0~luma_target.
         * Constraints: It should be greater than the value at which the frame
         *   is too dark for AF to work successfully.
         * Effect:  The smaller this value is set, the darker lighting
         *   condition under which shake-resistant AF will be turned off.
         */
        0,

        /* Variable name: tradeoff_ratio
         * Used to define how much the exposure time should be reduced.
         * 3A version:
         * Default value: 4
         * Data range: 1~4
         * Constraints: The value should be greater than or equal to 1.
         * Effect:  The bigger the value is, the smaller the adjusted exposure
         *   time would be.
         */
        4.000000f,

        /* Variable name: toggle_frame_skip
         * Sets number of frames to skip or drop from preview when this
         *   feature is called.
         * 3A version:
         * Default value: 2
         * Data range: 0 to 4
         * Constraints: None
         * Effect:  Will appear as preview is frozen for amount of frames
         *    set whenever AF is started and finished.
         */
        2,
      }, /* af_shake_resistant_t */

      /* af_motion_sensor_t */
      {
        /* Variable name: af_gyro_trigger
         * Used to control how scene change should be detected for AF.
         * 3A version:
         * Default value: 0.0
         * Data range: -16000.0 to +16000.0
         * Constraints: None
         * Effect:  The bigger the value is, the less sensitive AF response to
         *    gyro output value.
         */
        0.000000f,

        /* Variable name: af_accelerometer_trigger
         * Used to control how scene change should be detected for AF.
         * 3A version:
         * Default value: 0.0
         * Data range: -16000.0 to +16000.0
         * Constraints: None
         * Effect:  The bigger the value is, the less sensitive AF response to
         *    gyro output value.
         */
        0.000000f,

        /* Variable name: af_magnetometer_trigger
         * Used to control how scene change should be detected for AF.
         * 3A version:
         * Default value: 0.0
         * Data range: -16000.0 to +16000.0
         * Constraints: None
         * Effect:  The bigger the value is, the less sensitive AF response to
         *    gyro output value.
         */
        0.000000f,

        /* Variable name: af_dis_motion_vector_trigger
         * Used to control how scene change should be detected for AF.
         * 3A version:
         * Default value: 0.0
         * Data range: -16000.0 to +16000.0
         * Constraints: None
         * Effect:  The bigger the value is, the less sensitive AF response to
         *    gyro output value.
         */
        0.000000f,
      }, /* af_motion_sensor_t */

      /* af_fd_priority_caf_t */
      {
        /* Variable name: pos_change_th
         * Controls when to reconfigure ROI when position has changed
         * with respect to last stable ROI.
         * 3A version:
         * Default value:
         * Data range:
         * Constraints: None
         * Effect:  The bigger the value is, the less sensitive AF to
         * face position change
         */
        3.0,

        /* Variable name: pos_stable_th_hi
         * percentage differnce between last and current position above
         * this indicate face is moving and not stable to trigger new search.
         * 3A version:
         * Default value: 0.0
         * Data range:
         * Constraints: None
         * Effect:
         */
        0.5f,

        /* Variable name: pos_stable_th_low
         * position is deemed stable only after face position change
         * is less than this threshold.
         * 3A version:
         * Default value:
         * Data range:
         * Constraints: None
         * Effect:
         */
        0.4f,

        /* Variable name: size_change_th
         * threshold to check if size change has decreased enough to be
         * considered stable.
         * 3A version:
         * Default value:
         * Data range:
         * Constraints: None
         * Effect:
         */
        4.0f,

        /* Variable name: old_new_size_diff_th
         * percentage difference between last biggest face and current
         * biggest face to check if we should start focusing on new face.
         * 3A version:
         * Default value:
         * Data range:
         * Constraints: None
         * Effect:
         */
        2.0f,

        /* Variable name: stable_count_size
         * number of frames face size should remain stable to trigger
         * new search.
         * 3A version:
         * Default value:
         * Data range:
         * Constraints: None
         * Effect:
         */
        3,

        /* Variable name: stable_count_pos
         * number of frames face position should remain stable to trigger
         * new search.
         * 3A version:
         * Default value:
         * Data range:
         * Constraints: None
         * Effect:
         */
        3,

        /* Variable name: no_face_wait_th
         * number of frames to wait to reset default ROI once face disappears.
         * 3A version:
         * Default value:
         * Data range:
         * Constraints: None
         * Effect:
         */
        3,

        /* Variable name: fps_adjustment_th
         * if current fps falls below this threshold we'll adjust stability counts.
         * 3A version:
         * Default value: 15
         * Data range:
         * Constraints: None
         * Effect:
         */
        15,
      }, /* af_fd_priority_caf_t */
    }, /* af_tuning_algo_t */

    /* af_tuning_vfe_t */
    {
      /* Variable name: fv_metric
       * Focus value metric - 0 means sum and 1 means Max.
       * 3A version:
       * Default value: 0
       * Data range:
       * Constraints:
       * Effect:
       */
      0,

      /* af_vfe_config_t */
      {
        /* Variable name: fv_min
         * Minimum focus value for each pixel below which it'll be ignored.
         * Required for AF stats hardware configuration.
         * 3A version:
         * Default value: tunable
         * Data range:
         * Constraints:
         * Effect:
         */
        31,

        /* Variable name: max_h_num
         * maximum number of horizontal grids configurable in each ROI.
         * 3A version:
         * Default value: 18
         * Data range:
         * Constraints:
         * Effect:
         */
        5,

        /* Variable name: max_v_num
         * maximum number of vertical grids configurable in each ROI.
         * 3A version:
         * Default value: 14
         * Data range:
         * Constraints:
         * Effect:
         */
        5,

        /* Variable name: max_block_width
         * maximum width of each block in the grids.
         * 3A version:
         * Default value: to be read from datasheet
         * Data range:
         * Constraints:
         * Effect:
         */
        336,

        /* Variable name: max_block_height
         * maximum height of each block in the grids.
         * 3A version:
         * Default value: to be read from datasheet
         * Data range:
         * Constraints:
         * Effect:
         */
        252,

        /* Variable name: min_block_width
         * minimum width of each block in the grids.
         * 3A version:
         * Default value: to be read from datasheet
         * Data range:
         * Constraints:
         * Effect:
         */
        64,

        /* Variable name: min_block_height
         * minimum height of each block in the grids.
         * 3A version:
         * Default value: to be read from datasheet
         * Data range:
         * Constraints:
         * Effect:
         */
        48,

        /* Variable name: h_offset_ratio_normal_light
         * Horizontal location of first pixel in terms of the ratio to the
         * whole frame size. For example, image width is 1000, we want to use
         * the middle 500 as AF window. Horizontal offset ratio is
         * 250/1000=0.25.
         * 3A version:
         * Default value: 0.25
         * Data range:
         * Constraints:
         * Effect:
         */
        0.42f,

        /* Variable name: v_offset_ratio_normal_light
         * Similar to Horizontal Offset Ratio, but this is in the veritcal direction.
         * whole frame size.
         * 3A version:
         * Default value: 0.25
         * Data range:
         * Constraints:
         * Effect:
         */
        0.42f,

        /* Variable name: h_clip_ratio_normal_light
         * AF window horizontal size in terms of ratio to the whole image. For the
         * same example above, Horizontal Clip Ratio is 500/1000=0.5.
         * 3A version:
         * Default value: 0.5
         * Data range:
         * Constraints:
         * Effect:
         */
        0.16f,

        /* Variable name: v_clip_ratio_normal_light
         * AF window veritical size in terms of ratio to the whole image. For the
         * same example above, Vertical Clip Ratio is 500/1000=0.5.
         * 3A version:
         * Default value: 0.5
         * Data range:
         * Constraints:
         * Effect:
         */
        0.16f,

        /* Variable name: h_offset_ratio_low_light
         * Horizontal location of first pixel in terms of the ratio to the
         * whole frame size. For example, image width is 1000, we want to use
         * the middle 500 as AF window. Horizontal offset ratio is
         * 250/1000=0.25.
         * 3A version:
         * Default value: 0.25
         * Data range:
         * Constraints:
         * Effect:
         */
        0.375f,

        /* Variable name: v_offset_ratio_low_light
         * Similar to Horizontal Offset Ratio, but this is in the veritcal direction.
         * whole frame size.
         * 3A version:
         * Default value: 0.25
         * Data range:
         * Constraints:
         * Effect:
         */
        0.375f,

        /* Variable name: h_clip_ratio_low_light
         * AF window horizontal size in terms of ratio to the whole image. For the
         * same example above, Horizontal Clip Ratio is 500/1000=0.5.
         * 3A version:
         * Default value: 0.5
         * Data range:
         * Constraints:
         * Effect:
         */
         0.25f,

        /* Variable name: v_clip_ratio_low_light
         * AF window veritical size in terms of ratio to the whole image. For the
         * same example above, Vertical Clip Ratio is 500/1000=0.5.
         * 3A version:
         * Default value: 0.5
         * Data range:
         * Constraints:
         * Effect:
         */
         0.25f,

        /* Variable name: touch_scaling_factor_ normal_light
         * Factor by how much we will reduce the touch AF roi in addition to
         * clip ratio.
         * 3A version:
         * Default value: 0.5
         * Data range:
         * Constraints:
         * Effect:
         */
        0.75f,

        /* Variable name: touch_scaling_factor_ low_light
         * Factor by how much we will reduce the touch AF roi in addition to
         * clip ratio.
         * 3A version:
         * Default value: 1.0
         * Data range:
         * Constraints:
         * Effect:
         */
        0.75f,
      }
      , /* af_vfe_config_t */

      /* af_vfe_hpf_t default */
      {
        /* af_vfe_legacy_hpf_t */
        {
          /* Variable name:
           * Highpass filter coeffs used for focus value calculation. For 3x5
           * kernel. Only 8 parameters are configurable.
           * 3A version:
           * Default value:
           * Data range: -16 to 15
           * Constraints:
           * Effect:
           */
          -4,   /* a00 */
          -2,   /* a02 */
          -4,   /* a04 */
          -1,   /* a20 */
          -1,    /* a21 */
          14,    /* a22 */
          -1,    /* a23 */
          -1,  /* a24 */
        }, /* af_vfe_legacy_hpf_t */

        /* bf_hpf_2x5  */
        {
          /* Variable name:
           * Highpass filter coeffs used for focus value calculation. For 2x5
           * kernel, all 10 parameters are configurable.
           * 3A version:
           * Default value:
           * Data range: -16 to 15
           * Constraints:
           * Effect:
           */
          -4,    /* a00 */
          0,     /* a01 */
          -2,    /* a02 */
          0,     /* a03 */
          -4,    /* a04 */
          -1,    /* a10 */
          -1,    /* a11 */
          14,    /* a12 */
          -1,    /* a13 */
          -1,    /* a14 */
        }, /* bf_hpf_2x5 */
#ifdef AF_2X13_FILTER_SUPPORT
        /* bf_hpf_2x13 */
        {
         /* Variable name:
                * Highpass filter coeffs for 2x13 kernel used for focus value
           * calculation.
           * 3A version:
           * Default value:
           * Data range: -16 to 15
           * Constraints:
           * Effect:
           */
           0,   /* a00 */
           0,   /* a01 */
           0,    /* a02 */
           1,    /* a03 */
           2,    /* a04 */
           3,    /* a05 */
           4,    /* a06 */
           3,    /* a07 */
           2,    /* a08 */
           1,   /* a09 */
           0,   /* a010 */
           0,   /* a10 */
           0,   /* a11 */
           -4,    /* a12 */
           -4,    /* a13 */
           0,    /* a14 */
           0,    /* a15 */
           0,    /* a16 */
           0,    /* a17 */
           0,    /* a18 */
           0,   /* a19 */
           0,   /* a110 */
           0,   /* a111 */
           0,   /* a112 */
           -4,   /* a113 */
           -4,   /* a114 */
        }, /* bf_hpf_2x13 */
#endif
      }, /* af_vfe_hpf_t default*/
      /* af_vfe_hpf_t face*/
      {
        /* af_vfe_legacy_hpf_t */
        {
          /* Variable name:
           * Highpass filter coeffs used for focus value calculation. For 3x5
           * kernel. Only 8 parameters are configurable.
           * 3A version:
           * Default value:
           * Data range: -16 to 15
           * Constraints:
           * Effect:
           */
          -3,   /* a00 */
          0,    /* a02 */
          -2,   /* a04 */
          15,   /* a20 */
          0,    /* a21 */
          0,    /* a22 */
          0,    /* a23 */
          -10,  /* a24 */
        }
        , /* af_vfe_legacy_hpf_t */

        /* bf_hpf_2x5 */{
          /* Variable name:
           * Highpass filter coeffs used for focus value calculation. For 2x5
           * kernel, all 10 parameters are configurable.
           * 3A version:
           * Default value:
           * Data range: -16 to 15
           * Constraints:
           * Effect:
           */
          -3,   /* a00 */
          0,    /* a01 */
          0,    /* a02 */
          0,    /* a03 */
          -2,   /* a04 */
          15,   /* a10 */
          0,    /* a11 */
          0,    /* a12 */
          0,    /* a13 */
          -10,  /* a14 */
        }, /* bf_hpf_2x5 */
#ifdef AF_2X13_FILTER_SUPPORT
        /* bf_hpf_2x13 */
        {
         /* Variable name:
                * Highpass filter coeffs for 2x13 kernel used for focus value
           * calculation.
           * 3A version:
           * Default value:
           * Data range: -16 to 15
           * Constraints:
           * Effect:
           */
           0,   /* a00 */
           0,   /* a01 */
           0,    /* a02 */
           1,    /* a03 */
           2,    /* a04 */
           3,    /* a05 */
           4,    /* a06 */
           3,    /* a07 */
           2,    /* a08 */
           1,   /* a09 */
           0,   /* a010 */
           0,   /* a10 */
           0,   /* a11 */
           -4,    /* a12 */
           -4,    /* a13 */
           0,    /* a14 */
           0,    /* a15 */
           0,    /* a16 */
           0,    /* a17 */
           0,    /* a18 */
           0,   /* a19 */
           0,   /* a110 */
           0,   /* a111 */
           0,   /* a112 */
           -4,   /* a113 */
           -4,   /* a114 */
        }, /* bf_hpf_2x13 */
#endif
      }, /* af_vfe_hpf_t */
      /* af_vfe_hpf_t lowlight*/
      {
        /* af_vfe_legacy_hpf_t */
        {
          /* Variable name:
           * Highpass filter coeffs used for focus value calculation. For 3x5
           * kernel. Only 8 parameters are configurable.
           * 3A version:
           * Default value:
           * Data range: -16 to 15
           * Constraints:
           * Effect:
           */
          -3,   /* a00 */
          0,    /* a02 */
          -2,   /* a04 */
          15,   /* a20 */
          0,    /* a21 */
          0,    /* a22 */
          0,    /* a23 */
          -10,  /* a24 */
        }
        , /* af_vfe_legacy_hpf_t */

        /* bf_hpf_2x5 */{
          /* Variable name:
           * Highpass filter coeffs used for focus value calculation. For 2x5
           * kernel, all 10 parameters are configurable.
           * 3A version:
           * Default value:
           * Data range: -16 to 15
           * Constraints:
           * Effect:
           */
          -4,    /* a00 */
          0,     /* a01 */
          -2,    /* a02 */
          0,     /* a03 */
          -4,    /* a04 */
          -1,    /* a10 */
          -1,    /* a11 */
          14,    /* a12 */
          -1,    /* a13 */
          -1,    /* a14 */
        }, /* bf_hpf_2x5 */
#ifdef AF_2X13_FILTER_SUPPORT
        /* bf_hpf_2x13 */
        {
         /* Variable name:
                * Highpass filter coeffs for 2x13 kernel used for focus value
           * calculation.
           * 3A version:
           * Default value:
           * Data range: -16 to 15
           * Constraints:
           * Effect:
           */
           0,   /* a00 */
           0,   /* a01 */
           0,    /* a02 */
           1,    /* a03 */
           2,    /* a04 */
           3,    /* a05 */
           4,    /* a06 */
           3,    /* a07 */
           2,    /* a08 */
           1,   /* a09 */
           0,   /* a010 */
           0,   /* a10 */
           0,   /* a11 */
           -4,    /* a12 */
           -4,    /* a13 */
           0,    /* a14 */
           0,    /* a15 */
           0,    /* a16 */
           0,    /* a17 */
           0,    /* a18 */
           0,   /* a19 */
           0,   /* a110 */
           0,   /* a111 */
           0,   /* a112 */
           -4,   /* a113 */
           -4,   /* a114 */
        }, /* bf_hpf_2x13 */
#endif
      }, /* af_vfe_hpf_t lowlight*/

      /* af_vfe_sw_hpf_t sw_hpf_default*/
      {
        /* Variable name: filter_type
         * Specify Choice of filter used for SW Stats
         * 3A version: 4.1
         * Default value: AFS_ON_IIR
         * Data range: AFS_ON_IIR ( 1 ), AFS_ON_FIR ( 2 )
         * Constraints: DO NOT USE AFS_OFF
         * Effect: Choose different set of fitler below
         */
        1,

        /* af_vfe_sw_fir_hpf_t fir */
        {
          /* Variable name: a
           * Highpass FIR filter coeffs used for focus value calculation. For 1x11
           * kernel. Only 11 parameters are configurable.
           * 3A version: 4.1
           * Default value:
           * Data range: int -16 to 15
           * Constraints:
           * Effect:
           */
          {
          -4,    /* a00 */
          -4,    /* a01 */
          1,     /* a02 */
          2,     /* a03 */
          3,     /* a04 */
          4,     /* a05 */
          3,     /* a06 */
          2,     /* a07 */
          1,     /* a08 */
          -4,    /* a09 */
          -4,    /* a10 */
          },

          /* Variable name: fv_min_hi
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

          /* Variable name: fv_min_lo
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

          /* Variable name: coeff_length
           * Length of the filter coefficients used
           * 3A version: 4.1
           * Default value: FILTER_SW_LENGTH_FIR (11)
           * Data range: 1 to 11
           * Constraints: Should not modify randomly, Should
           *   match the filter entries used.
           * Effect: for different camera use case, different kernel is used.
           */
          FILTER_SW_LENGTH_FIR,
        }, /* af_vfe_sw_fir_hpf_t */

        /* af_vfe_sw_iir_hpf_t iir */
        {
          /* Variable name: a, b
           * Highpass IIR filter coeffs used for focus value calculation. For 2x6
           * kernel, all 6 parameters are configurable.
           * 3A version: 4.1
           * Default value:
           * Data range: double -16 to 15
           * Constraints:
           * Effect:
           */
          /* a */
          {
            1.00000000f,   /* a00 */
            -1.0772000f,   /* a01 */
            0.61280000f,   /* a02 */
            0.00000000f,   /* a03 */
            0.00000000f,   /* a04 */
            0.00000000f,   /* a05 */
          },
          /* b */
          {
            0.19360000f,   /* a00 */
            0.00000000f,   /* a01 */
            -0.1936000f,   /* a02 */
            0.00000000f,   /* a03 */
            0.00000000f,   /* a04 */
            0.00000000f,   /* a05 */
          },
          /* Variable name: fv_min_hi
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

           /* Variable name: fv_min_lo
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

          /* Variable name: coeff_length
           * Length of the filter coefficients used
           * 3A version: 4.1
           * Default value: FILTER_SW_LENGTH_IIR (6)
           * Data range: 3 or 6
           * Constraints: Should not modify randomly, Should
           *   match the filter entries used.
           * Effect: for different camera use case, different kernel is used.
           */
          3,
        }, /* af_vfe_sw_iir_hpf_t */

        /* af_vfe_sw_fir_hpf_t fir_low_end */
        {
          /* Variable name: a
           * Highpass FIR filter coeffs used for focus value calculation. For 1x11
           * kernel. Only 11 parameters are configurable.
           * 3A version: 4.1
           * Default value:
           * Data range: int -16 to 15
           * Constraints:
           * Effect:
           */
          {
          -4,    /* a00 */
          -4,    /* a01 */
          1,     /* a02 */
          2,     /* a03 */
          3,     /* a04 */
          4,     /* a05 */
          3,     /* a06 */
          2,     /* a07 */
          1,     /* a08 */
          -4,    /* a09 */
          -4,    /* a10 */
          },

          /* Variable name: fv_min_hi
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

          /* Variable name: fv_min_lo
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

          /* Variable name: coeff_length
           * Length of the filter coefficients used
           * 3A version: 4.1
           * Default value: FILTER_SW_LENGTH_FIR (11)
           * Data range: 1 to 11
           * Constraints: Should not modify randomly, Should
           *   match the filter entries used.
           * Effect: for different camera use case, different kernel is used.
           */
          FILTER_SW_LENGTH_FIR,
        }, /* af_vfe_sw_fir_hpf_t */

        /* af_vfe_sw_iir_hpf_t iir_low_end */
        {
          /* Variable name: a, b
           * Highpass IIR filter coeffs used for focus value calculation. For 2x6
           * kernel, all 6 parameters are configurable.
           * 3A version: 4.1
           * Default value:
           * Data range: double -16 to 15
           * Constraints:
           * Effect:
           */
          /* a */
          {
            1.00000000f,   /* a00 */
            -1.0772000f,   /* a01 */
            0.61280000f,   /* a02 */
            0.00000000f,   /* a03 */
            0.00000000f,   /* a04 */
            0.00000000f,   /* a05 */
          },
          /* b */
          {
            0.19360000f,   /* a00 */
            0.00000000f,   /* a01 */
            -0.1936000f,   /* a02 */
            0.00000000f,   /* a03 */
            0.00000000f,   /* a04 */
            0.00000000f,   /* a05 */
          },
          /* Variable name: fv_min_hi
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

           /* Variable name: fv_min_lo
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

          /* Variable name: coeff_length
           * Length of the filter coefficients used
           * 3A version: 4.1
           * Default value: 3
           * Data range: 3 or 6
           * Constraints: Should not modify randomly, Should
           *   match the filter entries used.
           * Effect: for different camera use case, different kernel is used.
           */
          3,
        }, /* af_vfe_sw_iir_hpf_t */
      }, /* af_vfe_sw_hpf_t sw_hpf_default*/

      /* af_vfe_sw_hpf_t sw_hpf_face*/
      {
        /* Variable name: filter_type
         * Specify Choice of filter used for SW Stats
         * 3A version: 4.1
         * Default value: AFS_ON_IIR
         * Data range: AFS_ON_IIR ( 1 ), AFS_ON_FIR ( 2 )
         * Constraints: DO NOT USE AFS_OFF
         * Effect: Choose different set of fitler below
         */
        1,

        /* af_vfe_sw_fir_hpf_t fir */
        {
          /* Variable name: a
           * Highpass FIR filter coeffs used for focus value calculation. For 1x11
           * kernel. Only 11 parameters are configurable.
           * 3A version: 4.1
           * Default value:
           * Data range: int -16 to 15
           * Constraints:
           * Effect:
           */
          {
          -4,    /* a00 */
          -4,    /* a01 */
          1,     /* a02 */
          2,     /* a03 */
          3,     /* a04 */
          4,     /* a05 */
          3,     /* a06 */
          2,     /* a07 */
          1,     /* a08 */
          -4,    /* a09 */
          -4,    /* a10 */
          },

          /* Variable name: fv_min_hi
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

           /* Variable name: fv_min_lo
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

          /* Variable name: coeff_length
           * Length of the filter coefficients used
           * 3A version: 4.1
           * Default value: FILTER_SW_LENGTH_FIR (11)
           * Data range: 1 to 11
           * Constraints: Should not modify randomly, Should
           *   match the filter entries used.
           * Effect: for different camera use case, different kernel is used.
           */
          FILTER_SW_LENGTH_FIR,
        }, /* af_vfe_sw_fir_hpf_t */

        /* af_vfe_sw_iir_hpf_t */
        {
          /* Variable name: a, b
           * Highpass IIR filter coeffs used for focus value calculation. For 2x6
           * kernel, all 6 parameters are configurable.
           * 3A version: 4.1
           * Default value:
           * Data range: double -16 to 15
           * Constraints:
           * Effect:
           */
          /* a */
          {
            1.00000000f,   /* a00 */
            -1.0772000f,   /* a01 */
            0.61280000f,   /* a02 */
            0.00000000f,   /* a03 */
            0.00000000f,   /* a04 */
            0.00000000f,   /* a05 */
          },
          /* b */
          {
            0.19360000f,   /* a00 */
            0.00000000f,   /* a01 */
            -0.1936000f,   /* a02 */
            0.00000000f,   /* a03 */
            0.00000000f,   /* a04 */
            0.00000000f,   /* a05 */
          },
          /* Variable name: fv_min_hi
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

           /* Variable name: fv_min_lo
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

          /* Variable name: coeff_length
           * Length of the filter coefficients used
           * 3A version: 4.1
           * Default value: FILTER_SW_LENGTH_IIR (6)
           * Data range: 3 or 6
           * Constraints: Should not modify randomly, Should
           *   match the filter entries used.
           * Effect: for different camera use case, different kernel is used.
           */
          3,
        }, /* af_vfe_sw_iir_hpf_t */
        /* af_vfe_sw_fir_hpf_t fir_low_end */
        {
          /* Variable name: a
           * Highpass FIR filter coeffs used for focus value calculation. For 1x11
           * kernel. Only 11 parameters are configurable.
           * 3A version: 4.1
           * Default value:
           * Data range: int -16 to 15
           * Constraints:
           * Effect:
           */
          {
          -4,    /* a00 */
          -4,    /* a01 */
          1,     /* a02 */
          2,     /* a03 */
          3,     /* a04 */
          4,     /* a05 */
          3,     /* a06 */
          2,     /* a07 */
          1,     /* a08 */
          -4,    /* a09 */
          -4,    /* a10 */
          },

          /* Variable name: fv_min_hi
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

           /* Variable name: fv_min_lo
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

          /* Variable name: coeff_length
           * Length of the filter coefficients used
           * 3A version: 4.1
           * Default value: FILTER_SW_LENGTH_FIR (11)
           * Data range: 1 to 11
           * Constraints: Should not modify randomly, Should
           *   match the filter entries used.
           * Effect: for different camera use case, different kernel is used.
           */
          FILTER_SW_LENGTH_FIR,
        }, /* af_vfe_sw_fir_hpf_t */

        /* af_vfe_sw_iir_hpf_t iir_low_end */
        {
          /* Variable name: a, b
           * Highpass IIR filter coeffs used for focus value calculation. For 2x6
           * kernel, all 6 parameters are configurable.
           * 3A version: 4.1
           * Default value:
           * Data range: double -16 to 15
           * Constraints:
           * Effect:
           */
          /* a */
          {
            1.00000000f,   /* a00 */
            -1.0772000f,   /* a01 */
            0.61280000f,   /* a02 */
            0.00000000f,   /* a03 */
            0.00000000f,   /* a04 */
            0.00000000f,   /* a05 */
          },
          /* b */
          {
            0.19360000f,   /* a00 */
            0.00000000f,   /* a01 */
            -0.1936000f,   /* a02 */
            0.00000000f,   /* a03 */
            0.00000000f,   /* a04 */
            0.00000000f,   /* a05 */
          },
          /* Variable name: fv_min_hi
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

           /* Variable name: fv_min_lo
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

          /* Variable name: coeff_length
           * Length of the filter coefficients used
           * 3A version: 4.1
           * Default value: 3
           * Data range: 3 or 6
           * Constraints: Should not modify randomly, Should
           *   match the filter entries used.
           * Effect: for different camera use case, different kernel is used.
           */
          3,
        }, /* af_vfe_sw_iir_hpf_t */
      }, /* af_vfe_sw_hpf_t sw_hpf_face*/

      /* af_vfe_sw_hpf_t sw_hpf_lowlight*/
      {
        /* Variable name: filter_type
         * Specify Choice of filter used for SW Stats
         * 3A version: 4.1
         * Default value: AFS_ON_IIR
         * Data range: AFS_ON_IIR ( 1 ), AFS_ON_FIR ( 2 )
         * Constraints: DO NOT USE AFS_OFF
         * Effect: Choose different set of fitler below
         */
        1,

        /* af_vfe_sw_fir_hpf_t fir */
        {
          /* Variable name: a
           * Highpass FIR filter coeffs used for focus value calculation. For 1x11
           * kernel. Only 11 parameters are configurable.
           * 3A version: 4.1
           * Default value:
           * Data range: int -16 to 15
           * Constraints:
           * Effect:
           */
          {
          -4,    /* a00 */
          -4,    /* a01 */
          1,     /* a02 */
          2,     /* a03 */
          3,     /* a04 */
          4,     /* a05 */
          3,     /* a06 */
          2,     /* a07 */
          1,     /* a08 */
          -4,    /* a09 */
          -4,    /* a10 */
          },

          /* Variable name: fv_min_hi
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,


           /* Variable name: fv_min_lo
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

          /* Variable name: coeff_length
           * Length of the filter coefficients used
           * 3A version: 4.1
           * Default value: FILTER_SW_LENGTH_FIR (11)
           * Data range: 1 to 11
           * Constraints: Should not modify randomly, Should
           *   match the filter entries used.
           * Effect: for different camera use case, different kernel is used.
           */
          FILTER_SW_LENGTH_FIR,
        }, /* af_vfe_sw_fir_hpf_t */

        /* af_vfe_sw_iir_hpf_t iir */
        {
          /* Variable name: a, b
           * Highpass IIR filter coeffs used for focus value calculation. For 2x6
           * kernel, all 6 parameters are configurable.
           * 3A version: 4.1
           * Default value:
           * Data range: double -16 to 15
           * Constraints:
           * Effect:
           */
          /* a */
          {
            1.00000000f,   /* a00 */
            -1.0772000f,   /* a01 */
            0.61280000f,   /* a02 */
            0.00000000f,   /* a03 */
            0.00000000f,   /* a04 */
            0.00000000f,   /* a05 */
          },
          /* b */
          {
            0.19360000f,   /* a00 */
            0.00000000f,   /* a01 */
            -0.1936000f,   /* a02 */
            0.00000000f,   /* a03 */
            0.00000000f,   /* a04 */
            0.00000000f,   /* a05 */
          },
          /* Variable name: fv_min_hi
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

           /* Variable name: fv_min_lo
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

          /* Variable name: coeff_length
           * Length of the filter coefficients used
           * 3A version: 4.1
           * Default value: FILTER_SW_LENGTH_IIR (6)
           * Data range: 3 or 6
           * Constraints: Should not modify randomly, Should
           *   match the filter entries used.
           * Effect: for different camera use case, different kernel is used.
           */
          3,
        }, /* af_vfe_sw_iir_hpf_t */
        /* af_vfe_sw_fir_hpf_t fir_low_end */
        {
          /* Variable name: a
           * Highpass FIR filter coeffs used for focus value calculation. For 1x11
           * kernel. Only 11 parameters are configurable.
           * 3A version: 4.1
           * Default value:
           * Data range: int -16 to 15
           * Constraints:
           * Effect:
           */
          {
          -4,    /* a00 */
          -4,    /* a01 */
          1,     /* a02 */
          2,     /* a03 */
          3,     /* a04 */
          4,     /* a05 */
          3,     /* a06 */
          2,     /* a07 */
          1,     /* a08 */
          -4,    /* a09 */
          -4,    /* a10 */
          },

          /* Variable name: fv_min_hi
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,


           /* Variable name: fv_min_lo
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

          /* Variable name: coeff_length
           * Length of the filter coefficients used
           * 3A version: 4.1
           * Default value: FILTER_SW_LENGTH_FIR (11)
           * Data range: 1 to 11
           * Constraints: Should not modify randomly, Should
           *   match the filter entries used.
           * Effect: for different camera use case, different kernel is used.
           */
          FILTER_SW_LENGTH_FIR,
        }, /* af_vfe_sw_fir_hpf_t */

        /* af_vfe_sw_iir_hpf_t iir_low_end */
        {
          /* Variable name: a, b
           * Highpass IIR filter coeffs used for focus value calculation. For 2x6
           * kernel, all 6 parameters are configurable.
           * 3A version: 4.1
           * Default value:
           * Data range: double -16 to 15
           * Constraints:
           * Effect:
           */
          /* a */
          {
            1.00000000f,   /* a00 */
            -1.0772000f,   /* a01 */
            0.61280000f,   /* a02 */
            0.00000000f,   /* a03 */
            0.00000000f,   /* a04 */
            0.00000000f,   /* a05 */
          },
          /* b */
          {
            0.19360000f,   /* a00 */
            0.00000000f,   /* a01 */
            -0.1936000f,   /* a02 */
            0.00000000f,   /* a03 */
            0.00000000f,   /* a04 */
            0.00000000f,   /* a05 */
          },
          /* Variable name: fv_min_hi
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

           /* Variable name: fv_min_lo
           * minimum FV value to be accumulated wrt to the filter
           * 3A version: 4.1
           * Default value: 0.5
           * Data range: double 0.0f to 5.0f (float)
           * Constraints:
           * Effect: Higher value will result in a sharper FV curve but
           *   it might filter out low contrast context, or missing peak.
           *   Lower Value will have flatter curve due to noise.
           */
          0.1f,

          /* Variable name: coeff_length
           * Length of the filter coefficients used
           * 3A version: 4.1
           * Default value: 3
           * Data range: 3 or 6
           * Constraints: Should not modify randomly, Should
           *   match the filter entries used.
           * Effect: for different camera use case, different kernel is used.
           */
          3,
        }, /* af_vfe_sw_iir_hpf_t */
      }, /* af_vfe_sw_hpf_t sw_hpf_lowlight*/

      /* Variable name: sw_fv_min_lux_trig_hi
       * Lux index to trigger full NORMAL LIGHT fv min.
       * kernel. Only 11 parameters are configurable.
       * 3A version: 4.1
       * Default value: 310 (120 lux corresponding index)
       * Data range: 0 to 500
       * Constraints: should be tuned wrt to Exposure table in chromatix
       * Effect: Decrease value will activate normal light fv_min
       *   at a brighter condition
       */
      310,

      /* Variable name: sw_fv_min_lux_trig_lo
       * Lux index to trigger full LOW LIGHT fv min.
       * kernel. Only 11 parameters are configurable.
       * 3A version: 4.1
       * Default value: 450 (20 lux corresponding index)
       * Data range: 0 to 500
       * Constraints: should be tuned wrt to Exposure table in chromatix
       * Effect: Decrease value will activate normal light fv_min
       *   at a brighter condition
       */
      450,
    }, /*af_tuning_vfe_t */
  },
