#ifndef CNE_CONFIGS_H
#define CNE_CONFIGS_H

/**----------------------------------------------------------------------------
  @file CneConfigs.h

         The CneConfigs reads the CneConfig file a text file with config
         elements and their values. ex:- ConfigType:ConfigValue
         It parses and stores them and provides an api for the clients
         to request for configuration value of a specific configuration.

-----------------------------------------------------------------------------*/


/*=============================================================================
               Copyright (c) 2009,2010 Qualcomm Technologies, Inc.
               All Rights Reserved.
               Qualcomm Technologies Confidential and Proprietary
=============================================================================*/

/*=============================================================================
  EDIT HISTORY FOR MODULE

  $Header: //depot/asic/sandbox/projects/cne/common/core/inc/CneConfigs.h#2 $
  $DateTime: 2009/11/20 14:15:13 $
  $Author: ggeldman $
  $Change: 1092306 $

  when        who     what, where, why
  ----------  ---     ---------------------------------------------------------
  2009-10-24  ysk     First revision.
=============================================================================*/


/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "CneDefs.h"
#include "cutils/properties.h"
#include <stdio.h>


/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Class Definitions
 * -------------------------------------------------------------------------*/

class CneConfigs
{
  public:
    /* Different configuration elments */
    typedef enum
    {
      CONFIG_BAT_LOW_TO_MED_TRIG,
      CONFIG_BAT_MED_TO_HIGH_TRIG,
      CONFIG_OP_POLICY_LOCATION,
      CONFIG_USER_POLICY_LOCATION,
      CONFIG_BW_BASED_RAT_SEL,
      CONFIG_SENSORS_IP_BASED_RAT_MGT,
      CONFIG_BAT_IP_BASED_RAT_MGT,
      CONFIG_RAT_ACQ_TIME_OUT,
      CONFIG_RAT_ACQ_RETRY_TIME_OUT,
      CONFIG_FMC_MODE,
      CONFIG_FMC_INIT_TIME_OUT,
      CONFIG_FMC_COMM_TIME_OUT,
      CONFIG_FMC_RETRY,
      CONFIG_RAT_WLAN_CHIPSET_OEM,
      CONFIG_NSRM_ENABLED,
      CONFIG_NSRM_BKG_EVT_FLAG

      /* other things that i can think of that can be configurable
       * what to substitute for RAT_ANY
       */

    } CneConfigProperties;

    typedef struct
    {
      CneConfigProperties configProperty;
      union
      {
        uint32_t batLowToMedTrig;
        uint32_t batMedToHighTrig;
        uint8_t opPolicyLocPtr[PROPERTY_VALUE_MAX];
        uint8_t userPolicyLocPtr[PROPERTY_VALUE_MAX];
        uint8_t bwBasedRatSelFlag;
        uint8_t sensorsIpBasedRatMgtFlag;
        uint8_t batIpBasedRatMgtFlag;
        uint32_t ratAcqTimeOut;
        uint32_t ratAcqRetryTimeOut;
        uint8_t  fmcModeFlag;       // turn on/off FMC feature
        uint32_t fmcInitTimeOut;    // timeout while creating socket
        uint32_t fmcCommTimeOut;    // timeout while sending enable/disable FMC
        uint8_t  fmcRetryFlag;      // turn on/off FMC retry feature
        uint8_t wlanChipsetOEM[PROPERTY_VALUE_MAX]; // OEM of wlan chipset
        uint8_t NsrmEnabledFlag; //turn on/off Nsrm feature
        uint16_t  NsrmBkgEvtFlag;       // turn on/off various Nsrm Background events
      } configValue;

    } CneConfigPropertyStruct;

    /**
    * @brief Returns an instance of the CneConfigs class.

      The user of this class will call this function to get an
      instance of the class. All other public functions will be
      called  on this instance

    * @param  None
    * @see    None
    * @return  An instance of the CneConfigs class is returned.
    */
    static CneConfigs* getInstance
    (
      void
    );

    /**
    * @brief Finds a static value of a configured property name, and
             stores the result in the configValue parameter.

    * The user of this function passes in the name of the property
    * whose configuration is unknown, and the value of the
    * configuration property is returned as its corresponding type,
    * which is defined in the CneConfigValues structure.

    * @param  configValue   Stores the name of the requested
    *                       property and, when the function
    *                       finishes, its configured value.
    * @see    None
    * @return Positive value if the function is successful, or
    *         negative value if the function fails. If the requested
    *         property is a pointer to a string, returns the length
    *         of that string.
    */
    CneRetType getConfigValue
    (
      CneConfigPropertyStruct& configValue
    );

    CneRetType setConfigValue
    (
      CneConfigs::CneConfigProperties configName,
      const char* propertyVal
    );

    private:
      /* constructor */
      CneConfigs();
      /* destructor */
      ~CneConfigs();

      static CneConfigs* instancePtr;
};


#endif /* CNE_CONFIGS_H*/
