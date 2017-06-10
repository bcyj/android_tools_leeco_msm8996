/*============================================================================
  @file sns_sam_algo_gpio.c

  GPIO related functions for SAM algorithms

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#include "sns_sam.h"
#include "sns_debug_api.h"
#include "sns_sam_algo_gpio.h"
#include "sns_sam_pm.h"
#include "sns_debug_str.h"

#ifdef FEATURE_TEST_ALGO_GPIO

#include "DDITlmm.h"
#include "DALDeviceId.h"

#define SNS_SAM_TEST_GPIO_MAX_NUM 1

/* Array of GPIO pins available for sensor test */
uint16_t sns_sam_test_gpio_num_a[SNS_SAM_TEST_GPIO_MAX_NUM] = {77};

/* GPIO handle used to handle GPIOs */
static DalDeviceHandle *gpio_handle;

/**
 * @brief Configure and enable GPIOs
 *
 * @return None
 */
static void sns_sam_gpio_enable(uint16_t gpio_num)
{
  DALResult result = DAL_ERROR;
  DALGpioSignalType gpio_cfg;

  gpio_cfg = DAL_GPIO_CFG_OUT(gpio_num,
                              0,
                              DAL_GPIO_OUTPUT,
                              DAL_GPIO_NO_PULL,
                              DAL_GPIO_2MA,
                              DAL_GPIO_LOW_VALUE);


  result = DalTlmm_ConfigGpio(gpio_handle,
                              gpio_cfg,
                              DAL_TLMM_GPIO_ENABLE);
  if( result != DAL_SUCCESS )
  {
    SNS_PRINTF_STRING_ERROR_2(samModule, "Error enabling gpio: %d, result: %d", gpio_num, result);
  }
}

/**
 * Initialize GPIO.
 *
 * @return none
 */
void sns_sam_init_gpio( void )
{
  DALResult result = DAL_ERROR;
  uint32_t i;
  result = DAL_DeviceAttach(DALDEVICEID_TLMM, &gpio_handle);
  if( result != DAL_SUCCESS )
  {
    gpio_handle = NULL;
    SNS_PRINTF_STRING_ERROR_1(samModule, "Error initializing gpio handle, result: %d", result);
    return;
  }

  for( i=0; i<SNS_SAM_TEST_GPIO_MAX_NUM; i++ )
  {
    sns_sam_gpio_enable(sns_sam_test_gpio_num_a[i]);
  }
}


/**
 * Set GPIO state on a particular GPIO pin. This function can only be used by
 * one algorithm each time, since all sensors algos are sharing the same GPIO
 * pins for this test.
 *
 * @param[i] gpioIndex: index of GPIO array to indicate which GPIO is to be set
 * @param[i] setToHigh: whether set this GPIO to high
 *
 * @return None
*/
SNS_SAM_UIMAGE_CODE void sns_sam_algo_set_gpio(uint32_t gpioIndex, bool setToHigh)
{
  DALResult result = DAL_ERROR;
  int16_t gpio_num;
  DALGpioSignalType gpio_cfg;

  if(gpioIndex < SNS_SAM_TEST_GPIO_MAX_NUM && NULL != gpio_handle)
  {
    gpio_num = sns_sam_test_gpio_num_a[gpioIndex];
    gpio_cfg = DAL_GPIO_CFG_OUT(gpio_num,
                                0,
                                DAL_GPIO_OUTPUT,
                                DAL_GPIO_NO_PULL,
                                DAL_GPIO_2MA,
                                DAL_GPIO_LOW_VALUE);

    result = DalTlmm_GpioOut(gpio_handle,
                             gpio_cfg,
                             setToHigh ? DAL_GPIO_HIGH_VALUE : DAL_GPIO_LOW_VALUE);

    if( DAL_SUCCESS != result )
    {
      SNS_PRINTF_STRING_ERROR_2(samModule, "Error setting QMD GPIO: %d, err: %d", gpio_num, result);
    }
  }
  else
  {
    SNS_PRINTF_STRING_ERROR_1(samModule,
      "Error gpio index out of bound %d or gpio is not initialized correctly", gpioIndex);
  }
}
#else /* FEATURE_TEST_ALGO_GPIO */
/**
 * Initialize GPIO.
 *
 * @return none
 */
void sns_sam_init_gpio( void )
{
  //do nothing
}

/**
 * @brief Pulls the GPIO pin up/down depending on input.
 *
 */
SNS_SAM_UIMAGE_CODE void sns_sam_algo_set_gpio(uint32_t gpioIndex, bool setToHigh)
{
  //do nothing
}
#endif /* FEATURE_TEST_ALGO_GPIO */
