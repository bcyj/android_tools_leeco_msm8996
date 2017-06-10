/*============================================================================

Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
  {
    /* actuator_params_t */
    {
      /* module_name */
      "SUNNY-Q13V06K",
      /* actuator_name */
      "BU64297_Q13V06K",
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
      /* msm_actuator_reg_tbl_t */{
        /* reg_tbl_size */
        1,
        /* msm_actuator_reg_params_t */{
          /* reg_write_type;hw_mask; reg_addr; hw_shift >>; data_shift << */
           {MSM_ACTUATOR_WRITE_DAC, 0x0000c400, 0xFFFF, 0, 0},
        }
        ,
      }
      ,
      /* init_setting_size */
      24,
      /* init_settings */{
        /* first position->0mA */
        {0xc2, MSM_ACTUATOR_BYTE_ADDR, 0x00, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        /* Q-fact=7-9 */
        {0xE9, MSM_ACTUATOR_BYTE_ADDR, 0xB1, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE8, MSM_ACTUATOR_BYTE_ADDR, 0xC3, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE8, MSM_ACTUATOR_BYTE_ADDR, 0x8F, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE8, MSM_ACTUATOR_BYTE_ADDR, 0xC7, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE9, MSM_ACTUATOR_BYTE_ADDR, 0x16, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE9, MSM_ACTUATOR_BYTE_ADDR, 0x7A, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE8, MSM_ACTUATOR_BYTE_ADDR, 0x00, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE9, MSM_ACTUATOR_BYTE_ADDR, 0x19, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE9, MSM_ACTUATOR_BYTE_ADDR, 0x3F, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE8, MSM_ACTUATOR_BYTE_ADDR, 0xBD, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xEA, MSM_ACTUATOR_BYTE_ADDR, 0x63, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE9, MSM_ACTUATOR_BYTE_ADDR, 0x63, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE8, MSM_ACTUATOR_BYTE_ADDR, 0xE6, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE8, MSM_ACTUATOR_BYTE_ADDR, 0xF3, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE9, MSM_ACTUATOR_BYTE_ADDR, 0x2D, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE9, MSM_ACTUATOR_BYTE_ADDR, 0x69, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE9, MSM_ACTUATOR_BYTE_ADDR, 0x9B, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE9, MSM_ACTUATOR_BYTE_ADDR, 0xBF, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE9, MSM_ACTUATOR_BYTE_ADDR, 0xD7, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xE9, MSM_ACTUATOR_BYTE_ADDR, 0xE7, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0xEA, MSM_ACTUATOR_BYTE_ADDR, 0x00, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        /* 90Hz, Fo=150 X 0.4+30 */
        {0xd0, MSM_ACTUATOR_BYTE_ADDR, 0x96, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        /* step mode, isrc 0.8x mode(0x00 0.5x mode, 0x01 0.8x mode, 0x02 1.0x mode */
        {0xc8, MSM_ACTUATOR_BYTE_ADDR, 0x00, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
      }
      ,
    }
    , /* actuator_params_t */

    /* actuator_tuned_params_t */{
      /* scenario_size */
      {
        /* scenario_size[MOVE_NEAR] */
        4,
        /* scenario_size[MOVE_FAR] */
        4,
      }
      ,

      /* ringing_scenario */{
        /* ringing_scenario[MOVE_NEAR] */
        {
          9,
          18,
          36,
          350,
        }
        ,
        /* ringing_scenario[MOVE_FAR] */{
          9,
          18,
          36,
          350,
        }
        ,
      }
      ,

      /* intial_code */
      350,
      /* region_size */
      1,

      /* region_params */{
        /* step_bound[0] - macro side boundary */
        /* step_bound[1] - infinity side boundary */
        /* Region 1 */
        {
          .step_bound = {351, 0}
          ,
          .code_per_step = 1,
        }
        ,
      }
      ,
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
                .damping_delay = 15000,
                .hw_params = 0x0000E424,

              },
            },
          },
          {
            /* scenario 2 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 15000,
                .hw_params = 0x0000E424,
              },
            },
          },
          {
            /* scenario 3 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 15000,
                .hw_params = 0x0000E422,
              },
            },
          },
          {
            /* scenario 4 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 15000,
                .hw_params = 0x0000E422,
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
                .damping_step = 0x3FF,
                .damping_delay = 15000,
                .hw_params = 0x0000E424,
              },
            },
          },
          {
            /* scenario 2 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 2000,
                .hw_params = 0x0000E481,

              },
            },
          },
          {
            /* scenario 3 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 15000,
                .hw_params = 0x0000E422,
              },
            },
          },
          {
            /* scenario 4 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 15000,
                .hw_params = 0x0000E422,
              },
            },
          },
        },
      }
      ,
    }
    , /* actuator_tuned_params_t */
  },
