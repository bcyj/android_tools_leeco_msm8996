/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
  {
    /* actuator_params_t */
    {
      /* module_name */
      "OV5648_q5v22e",
      /* actuator_name */
      "a3907",
      /* i2c_addr */
      0x18,
      /* i2c_data_type */
      MSM_ACTUATOR_BYTE_DATA,
      /* i2c_addr_type */
      MSM_ACTUATOR_BYTE_ADDR,
      /* act_type */
      ACTUATOR_VCM,
      /* data_size */
      10,
      /* af_restore_pos */
      0,
      /* msm_actuator_reg_tbl_t */
      {
        /* reg_tbl_size */
        1,
        /* msm_actuator_reg_params_t */
        {
          /* reg_write_type;hw_mask; reg_addr; hw_shift >>; data_shift << */
          {MSM_ACTUATOR_WRITE_DAC, 0x0000000F, 0xFFFF, 0, 4},
        },
      },
      /* init_setting_size */
      0,
      /* init_settings */
      {
			  {0x00, MSM_ACTUATOR_BYTE_ADDR, 0x00, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},

      },
    }, /* actuator_params_t */

    /* actuator_tuned_params_t */
    {
      /* scenario_size */
      {
        /* scenario_size[MOVE_NEAR] */
        4,
        /* scenario_size[MOVE_FAR] */
        4,
      },

      /* ringing_scenario */
      {
        /* ringing_scenario[MOVE_NEAR] */
        {
          60,
          120,
          240,
          460,
        },
        /* ringing_scenario[MOVE_FAR] */
        {
          60,
          120,
          240,
          460,
        },
      },

      /* intial_code */
      180,
      /* region_size */
      1,

      /* region_params */
      {
        /* step_bound[0] - macro side boundary */
        /* step_bound[1] - infinity side boundary */
        /* Region 1 */
        {
          .step_bound = {461, 0},
          .code_per_step = 1,
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
                .damping_step = 0x3FF,
                .damping_delay = 12000,
                .hw_params = 0x6,
              }
              ,
            }
            ,
          }
          ,{
            /* scenario 2 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 12000,
                .hw_params = 0x5,
              },
            },
          },
          {
            /* scenario 3 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 12000,
                .hw_params = 0x4,
              }
              ,
            }
            ,
          },
          {
            /* scenario 4 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 12000,
                .hw_params = 0x3,
              }
              ,
            }
            ,
          },
        },

        {
          /* damping[MOVE_FAR] */
          {
            /* scenario 1 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 12000,
                .hw_params = 0x6,
              }
              ,
            }
            ,
          }
          ,{
            /* scenario 2 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 12000,
                .hw_params = 0x5,
              },
            },
          },
          {
            /* scenario 3 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 12000,
                .hw_params = 0x4,
              }
              ,
            }
            ,
          },
          {
            /* scenario 4 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 12000,
                .hw_params = 0x3,
              }
              ,
            }
            ,
          },
        },
      },
    }, /* actuator_tuned_params_t */
  },
