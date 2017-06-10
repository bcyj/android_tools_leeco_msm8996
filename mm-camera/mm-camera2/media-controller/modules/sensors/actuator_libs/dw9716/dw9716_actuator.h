/*============================================================================

Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
  {
    {
      /* module_name */
      "abico",
      /* actuator_name */
      "dw9716",
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
          {MSM_ACTUATOR_WRITE_DAC, 0x0000000F, 0xFFFF, 0, 4}
          ,
        }
        ,
      }
      ,
      /* init_setting_size */
      0,
      /* init_settings */{
        {0x00, MSM_ACTUATOR_BYTE_ADDR, 0x00, MSM_ACTUATOR_BYTE_DATA, MSM_ACT_WRITE, 0},
      }
      ,
    }
    , /* actuator_params_t */

    /* actuator_tuned_params_t */{
      /* scenario_size */
      {
        /* scenario_size[MOVE_NEAR] */
        2,
        /* scenario_size[MOVE_FAR] */
        3,
      }
      ,

      /* ringing_scenario */{
        /* ringing_scenario[MOVE_NEAR] */
        {
          40,
          370,
        }
        ,
        /* ringing_scenario[MOVE_FAR] */{
          80,
          220,
          370,
        }
        ,
      }
      ,

      /* intial_code */
      88,
      /* region_size */
      1,

      /* region_params */{
        /* step_bound[0] - macro side boundary */
        /* step_bound[1] - infinity side boundary */
        /* Region 1 */
        {
          .step_bound = {373, 0}
          ,
          .code_per_step = 1,
        }
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
                .damping_step = 0x1FF,
                .damping_delay = 4500,
                .hw_params = 0xF,
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
                .damping_step = 0x1FF,
                .hw_params = 0xB,
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
                .damping_step = 0x1FF,
                .damping_delay = 4500,
                .hw_params = 0xF,
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
                .damping_step = 0x1FF,
                .damping_delay = 4500,
                .hw_params = 0xB,
              }
              ,
            }
            ,
          }
          ,{
            /* scenario 3 */
            {
              /* region 1 */
              {
                .damping_step = 0x1FF,
                .damping_delay = 15000,
                .hw_params = 0xB,
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

