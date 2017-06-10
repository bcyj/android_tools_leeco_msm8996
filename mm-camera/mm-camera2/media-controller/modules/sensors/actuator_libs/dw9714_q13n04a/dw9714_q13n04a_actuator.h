/*============================================================================

Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
{
  /* actuator_params_t actuator_params */
  {
    /* char module_name[MAX_ACT_MOD_NAME_SIZE] */
    /* MAX_ACT_MOD_NAME_SIZE 32 */
    "SUNNY_Q13N04A",
    /* char actuator_name[MAX_ACT_NAME_SIZE] */
    /* MAX_ACT_NAME_SIZE 32 */
    "dw9714",
    /* uint32_t i2c_addr */
    0x18,
    /* enum msm_actuator_data_type i2c_data_type */
    MSM_ACTUATOR_BYTE_DATA,
    /* enum msm_actuator_addr_type i2c_addr_type */
    MSM_ACTUATOR_BYTE_ADDR,
    /* enum actuator_type act_type */
    ACTUATOR_VCM,
    /* uint16_t data_size */
    10,
    /* uint8_t af_restore_pos */
    0,
    /* struct msm_actuator_reg_tbl_t reg_tbl */
    {
      /* uint8_t reg_tbl_size */
      1,
      /* struct msm_actuator_reg_params_t reg_params[MAX_ACTUATOR_REG_TBL_SIZE] */
      /* MAX_ACTUATOR_REG_TBL_SIZE 8 */
      {
        /* reg_params[0] */
        {
          /* enum msm_actuator_write_type reg_write_type */
          /* uint32_t hw_mask */
          /* uint16_t reg_addr */
          /* uint16_t hw_shift *//* hw_shift >> */
          /* uint16_t data_shift *//* data_shift << */
          MSM_ACTUATOR_WRITE_DAC, 0x0000000F, 0xFFFF, 0, 4
        },
        /* reg_params[1] */
        /* reg_params[2] */
        /* reg_params[3] */
        /* reg_params[4] */
        /* reg_params[5] */
        /* reg_params[6] */
        /* reg_params[7] */
      },
    },

    /* uint16_t init_setting_size */
    4,

    /* struct reg_settings_t init_settings[MAX_ACTUATOR_INIT_SET] */
    /* MAX_ACTUATOR_INIT_SET 12 */
    {
      /* init_settings[0] */
      {
        /* uint16_t reg_addr */
        /* enum msm_actuator_addr_type addr_type */
        /* uint16_t reg_data */
        /* enum msm_actuator_data_type data_type */
        /* enum msm_actuator_i2c_operation i2c_operation */
        /* uint32_t delay */
        0xEC, MSM_ACTUATOR_BYTE_ADDR, 0xA3, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0
      },
      /* init_settings[1] */
      {
        /* uint16_t reg_addr */
        /* enum msm_actuator_addr_type addr_type */
        /* uint16_t reg_data */
        /* enum msm_actuator_data_type data_type */
        /* enum msm_actuator_i2c_operation i2c_operation */
        /* uint32_t delay */
        0xA1, MSM_ACTUATOR_BYTE_ADDR, 0x0d, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0
      },
      /* init_settings[2] */
      {
        /* uint16_t reg_addr */
        /* enum msm_actuator_addr_type addr_type */
        /* uint16_t reg_data */
        /* enum msm_actuator_data_type data_type */
        /* enum msm_actuator_i2c_operation i2c_operation */
        /* uint32_t delay */
        0xF2, MSM_ACTUATOR_BYTE_ADDR, 0x00, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0
      },
      /* init_settings[3] */
      {
        /* uint16_t reg_addr */
        /* enum msm_actuator_addr_type addr_type */
        /* uint16_t reg_data */
        /* enum msm_actuator_data_type data_type */
        /* enum msm_actuator_i2c_operation i2c_operation */
        /* uint32_t delay */
        0xDC, MSM_ACTUATOR_BYTE_ADDR, 0x51, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0
      },
      /* init_settings[4] */
      /* init_settings[5] */
      /* init_settings[6] */
      /* init_settings[7] */
      /* init_settings[8] */
      /* init_settings[9] */
      /* init_settings[10] */
      /* init_settings[11] */
    },
  },

  /* actuator_tuned_params_t actuator_tuned_params */
  {
    /* uint16_t scenario_size[NUM_ACTUATOR_DIR] */
    /* NUM_ACTUATOR_DIR 2 */
    {
      /* scenario_size[MOVE_NEAR] */
      4,
      /* scenario_size[MOVE_FAR] */
      4,
    },

    /* uint16_t ringing_scenario[NUM_ACTUATOR_DIR][MAX_ACTUATOR_SCENARIO] */
    /* NUM_ACTUATOR_DIR 2 */
    /* MAX_ACTUATOR_SCENARIO 8 */
    {
      /* ringing_scenario[MOVE_NEAR][MAX_ACTUATOR_SCENARIO] */
      {
        /* ringing_scenario[MOVE_NEAR][0] */
        9,
        /* ringing_scenario[MOVE_NEAR][1] */
        36,
        /* ringing_scenario[MOVE_NEAR][2] */
        226,
        /* ringing_scenario[MOVE_NEAR][3] */
        451,
        /* ringing_scenario[MOVE_NEAR][4] */
        /* ringing_scenario[MOVE_NEAR][5] */
        /* ringing_scenario[MOVE_NEAR][6] */
        /* ringing_scenario[MOVE_NEAR][7] */
      },
      /* ringing_scenario[MOVE_FAR][MAX_ACTUATOR_SCENARIO] */
      {
        /* ringing_scenario[MOVE_FAR][0] */
        9,
        /* ringing_scenario[MOVE_FAR][1] */
        36,
        /* ringing_scenario[MOVE_FAR][2] */
        226,
        /* ringing_scenario[MOVE_FAR][3] */
        451,
        /* ringing_scenario[MOVE_FAR][4] */
        /* ringing_scenario[MOVE_FAR][5] */
        /* ringing_scenario[MOVE_FAR][6] */
        /* ringing_scenario[MOVE_FAR][7] */
      },
    },

    /* int16_t initial_code */
    133,

    /* uint16_t region_size */
    1,

    /* struct region_params_t region_params[MAX_ACTUATOR_REGION] */
    /* MAX_ACTUATOR_REGION 5 */
    {
      /* region_params[0] */
      {
        /* [0] = ForwardDirection Macro boundary
           [1] = ReverseDirection Inf boundary
         */
        /* uint16_t step_bound[2] */
        .step_bound = {451, 0},
        /* uint16_t code_per_step */
        .code_per_step = 1,
      },
      /* region_params[1] */
      /* region_params[2] */
      /* region_params[3] */
      /* region_params[4] */
    },

    /* struct damping_t damping[NUM_ACTUATOR_DIR][MAX_ACTUATOR_SCENARIO] */
    /* NUM_ACTUATOR_DIR 2 */
    /* MAX_ACTUATOR_SCENARIO 8 */
    {
      /* damping[MOVE_NEAR][MAX_ACTUATOR_SCENARIO] */
      {
        /* damping[MOVE_NEAR][0] */
        {
          /* struct damping_params_t ringing_params[MAX_ACTUATOR_REGION] */
          /* MAX_ACTUATOR_REGION 5 */
          {
            /* ringing_params[0] */
            {
              .damping_step = 0x3FF,
              .damping_delay = 13000,
              .hw_params = 0x0,
            },
            /* ringing_params[1] */
            /* ringing_params[2] */
            /* ringing_params[3] */
            /* ringing_params[4] */
          },
        },
        /* damping[MOVE_NEAR][1] */
        {
          /* struct damping_params_t ringing_params[MAX_ACTUATOR_REGION] */
          /* MAX_ACTUATOR_REGION 5 */
          {
            /* ringing_params[0] */
            {
              .damping_step = 0x3FF,
              .damping_delay = 13000,
              .hw_params = 0x0,
            },
            /* ringing_params[1] */
            /* ringing_params[2] */
            /* ringing_params[3] */
            /* ringing_params[4] */
          },
        },
        /* damping[MOVE_NEAR][2] */
        {
          /* struct damping_params_t ringing_params[MAX_ACTUATOR_REGION] */
          /* MAX_ACTUATOR_REGION 5 */
          {
            /* ringing_params[0] */
            {
              .damping_step = 0x3FF,
              .damping_delay = 13000,
              .hw_params = 0x0,
            },
            /* ringing_params[1] */
            /* ringing_params[2] */
            /* ringing_params[3] */
            /* ringing_params[4] */
          },
        },
        /* damping[MOVE_NEAR][3] */
        {
          /* struct damping_params_t ringing_params[MAX_ACTUATOR_REGION] */
          /* MAX_ACTUATOR_REGION 5 */
          {
            /* ringing_params[0] */
            {
              .damping_step = 0x3FF,
              .damping_delay = 13000,
              .hw_params = 0x0,
            },
            /* ringing_params[1] */
            /* ringing_params[2] */
            /* ringing_params[3] */
            /* ringing_params[4] */
          },
        }
        /* damping[MOVE_NEAR][4] */
        /* damping[MOVE_NEAR][5] */
        /* damping[MOVE_NEAR][6] */
        /* damping[MOVE_NEAR][7] */
      },
      /* damping[MOVE_FAR][MAX_ACTUATOR_SCENARIO] */
      {
        /* damping[MOVE_FAR][0] */
        {
          /* struct damping_params_t ringing_params[MAX_ACTUATOR_REGION] */
          /* MAX_ACTUATOR_REGION 5 */
          {
            /* ringing_params[0] */
            {
              .damping_step = 0x3FF,
              .damping_delay = 13000,
              .hw_params = 0x0,
            },
            /* ringing_params[1] */
            /* ringing_params[2] */
            /* ringing_params[3] */
            /* ringing_params[4] */
          },
        },
        /* damping[MOVE_FAR][1] */
        {
          /* struct damping_params_t ringing_params[MAX_ACTUATOR_REGION] */
          /* MAX_ACTUATOR_REGION 5 */
          {
            /* ringing_params[0] */
            {
              .damping_step = 0x3FF,
              .damping_delay = 13000,
              .hw_params = 0x0,
            },
            /* ringing_params[1] */
            /* ringing_params[2] */
            /* ringing_params[3] */
            /* ringing_params[4] */
          },
        },
        /* damping[MOVE_FAR][2] */
        {
          /* struct damping_params_t ringing_params[MAX_ACTUATOR_REGION] */
          /* MAX_ACTUATOR_REGION 5 */
          {
            /* ringing_params[0] */
            {
              .damping_step = 0x3FF,
              .damping_delay = 13000,
              .hw_params = 0x0,
            },
            /* ringing_params[1] */
            /* ringing_params[2] */
            /* ringing_params[3] */
            /* ringing_params[4] */
          },
        },
        /* damping[MOVE_FAR][3] */
        {
          /* struct damping_params_t ringing_params[MAX_ACTUATOR_REGION] */
          /* MAX_ACTUATOR_REGION 5 */
          {
            /* ringing_params[0] */
            {
              .damping_step = 0x3FF,
              .damping_delay = 13000,
              .hw_params = 0x0,
            },
            /* ringing_params[1] */
            /* ringing_params[2] */
            /* ringing_params[3] */
            /* ringing_params[4] */
          },
        },
        /* damping[MOVE_FAR][4] */
        /* damping[MOVE_FAR][5] */
        /* damping[MOVE_FAR][6] */
        /* damping[MOVE_FAR][7] */
      },
    },
  },
},
