/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
  {
    /* actuator_params_t */
    {
      /* module_name */
      "sony",
      /* actuator_name */
      "iu074",
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
        {0x01, MSM_ACTUATOR_BYTE_ADDR, 0xA9, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0x02, MSM_ACTUATOR_BYTE_ADDR, 0xD2, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0x03, MSM_ACTUATOR_BYTE_ADDR, 0x0C, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0x04, MSM_ACTUATOR_BYTE_ADDR, 0x14, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0x05, MSM_ACTUATOR_BYTE_ADDR, 0xB6, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0x06, MSM_ACTUATOR_BYTE_ADDR, 0x4F, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0x00, MSM_ACTUATOR_BYTE_ADDR, 0x7F, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0x00, MSM_ACTUATOR_BYTE_ADDR, 0x7F, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
      },
    }, /* actuator_params_t */

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
                .damping_delay = 1500,
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
                .damping_delay = 1500,
                .hw_params = 0x00,
              },
            },
          },
        },
      },
    }, /* actuator_tuned_params_t */
  },
