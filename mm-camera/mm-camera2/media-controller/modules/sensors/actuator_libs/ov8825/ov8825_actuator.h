/*============================================================================

Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
  {
    /* actuator_params_t */
    {
      /* module_name */
      "turly-cm7700",
      /* actuator_name */
      "ov8825",
      /* i2c_addr */
      0x6C,
      /* i2c_data_type */
      MSM_ACTUATOR_BYTE_DATA,
      /* i2c_addr_type */
      MSM_ACTUATOR_WORD_ADDR,
      /* act_type */
      ACTUATOR_VCM,
      /* data_size */
      10,
      /* af_restore_pos */
      0,
      /* msm_actuator_reg_tbl_t */{
        /* reg_tbl_size */
        2,
        /* msm_actuator_reg_params_t */{
          /* reg_write_type;hw_mask; reg_addr; hw_shift >>; data_shift << */
           {MSM_ACTUATOR_WRITE_DAC, 0x0000000F, 0x3618, 0, 4},
           {MSM_ACTUATOR_WRITE_DAC, 0x0000000F, 0x3619, 0, 4},
        }
        ,
      }
      ,
      /* init_setting_size */
      3,
      /* init_settings */{
        {0x361A, MSM_ACTUATOR_WORD_ADDR, 0xB0, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0x361B, MSM_ACTUATOR_WORD_ADDR, 0x04, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
        {0x361C, MSM_ACTUATOR_WORD_ADDR, 0x07, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
      }
      ,
    }
    , /* actuator_params_t */

    /* actuator_tuned_params_t */{
      /* scenario_size */
      {
        /* scenario_size[MOVE_NEAR] */
        7,
        /* scenario_size[MOVE_FAR] */
        7,
      }
      ,

      /* ringing_scenario */{
        /* ringing_scenario[MOVE_NEAR] */
        {
          10,
          20,
          40,
          90,
          160,
          260,
          350,
        }
        ,
        /* ringing_scenario[MOVE_FAR] */
        {
          10,
          20,
          40,
          90,
          160,
          260,
          350,
        }
        ,
      }
      ,

      /* intial_code */
      112,
      /* region_size */
      1,

      /* region_params */
      {
        /* step_bound[0] - macro side boundary */
        /* step_bound[1] - infinity side boundary */
        /* Region 1 */
        {
          .step_bound = {359, 0},
          .code_per_step = 1,
        }
        ,
      }
      ,{
        /* damping */
        {
          /* damping[MOVE_NEAR] */
          {
            /* scenario 1 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 18000,
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
                .damping_delay = 22000,
                .hw_params = 0x5,
              }
              ,
            }
            ,
          }
          ,
          {
            /* scenario 3 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 22000,
                .hw_params = 0x5,
              }
              ,
            }
            ,
          }
          ,
          {
            /* scenario 4 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 23000,
                .hw_params = 0x3,
              }
              ,
            }
            ,
          }
          ,
          {
            /* scenario 5 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 22000,
                .hw_params = 0x2,
              }
              ,
            }
            ,
          }
          ,
          {
            /* scenario 6 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 25000,
                .hw_params = 0x2,
              }
              ,
            }
            ,
          }
          ,
          {
            /* scenario 7 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 23000,
                .hw_params = 0x1,
              }
              ,
            }
            ,
          }
          ,
        }
        ,{
          /* damping[MOVE_FAR] */
          {
            /* scenario 1 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 18000,
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
                .damping_delay = 22000,
                .hw_params = 0x5,
              }
              ,
            }
            ,
          }
          ,
          {
            /* scenario 3 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 22000,
                .hw_params = 0x5,
              }
              ,
            }
            ,
          }
          ,
          {
            /* scenario 4 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 23000,
                .hw_params = 0x3,
              }
              ,
            }
            ,
          }
          ,
          {
            /* scenario 5 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 22000,
                .hw_params = 0x2,
              }
              ,
            }
            ,
          }
          ,
          {
            /* scenario 6 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 25000,
                .hw_params = 0x2,
              }
              ,
            }
            ,
          }
          ,
          {
            /* scenario 7 */
            {
              /* region 1 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 23000,
                .hw_params = 0x1,
              }
              ,
            }
            ,
          }
          ,
        }
        ,
      }
      ,
    }
    , /* actuator_tuned_params_t */
  },
