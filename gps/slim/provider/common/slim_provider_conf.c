/*============================================================================
  FILE:         slim_provider_conf.c

  OVERVIEW:     SLIM processor specific provider configuration implementation.

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "slim_provider_conf.h"
#ifdef FEATURE_GSIFF_DSPS
#include "SlimSensor1ProviderWrapper.h"
#endif
#include "SlimNDKProviderWrapper.h"

#include "loc_cfg.h"
#include "SlimDaemonMsg.h"
#include "log_util.h"
/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/
#undef LOG_TAG
#define LOG_TAG "SlimProviderConf"

#ifdef FEATURE_GSIFF_DSPS
#define DEFAULT_SENSOR_PROVIDER SENSOR1_SENSOR_PROVIDER
#else
#define DEFAULT_SENSOR_PROVIDER ANDROID_NDK_SENSOR_PROVIDER
#endif

#define SAP_CONF_FILE "/etc/sap.conf"

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/
static uint32_t g_sensor_provider = MIN_SENSOR_PROVIDER;
static uint32_t g_sensor_usage = (uint32_t)0;

static loc_param_s_type conf_parameter_table[] =
{
  {"SENSOR_PROVIDER",  &g_sensor_provider, NULL,  'n'},
  {"SENSOR_USAGE",     &g_sensor_usage, NULL,     'n'},
};
/*----------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 * -------------------------------------------------------------------------*/
/**
@brief Returns configured sensor provider

Function returns configured sensor provider from SAP configuration file.

@return Configured sensor provider.
*/
static uint32_t GetConfiguredSensorProvider()
{
    uint32_t q_SensorProvider = DEFAULT_SENSOR_PROVIDER;
    /* Try to read the sensor provider if not read already. */
    if (g_sensor_provider <= MIN_SENSOR_PROVIDER ||
            g_sensor_provider >= MAX_SENSOR_PROVIDER)
    {
        /* Read logging configuration and sensor provider */
        UTIL_READ_CONF(SAP_CONF_FILE, conf_parameter_table);
        if (g_sensor_provider > MIN_SENSOR_PROVIDER &&
                g_sensor_provider < MAX_SENSOR_PROVIDER)
        {
            q_SensorProvider = g_sensor_provider;
        }

    }
    LOC_LOGI("%s:g_sensor_provider is %d", __func__, q_SensorProvider);
    return q_SensorProvider;
}

/*----------------------------------------------------------------------------
 * Externalized Function Definitions
 * -------------------------------------------------------------------------*/
/**
@brief Initializes provider of given type and returns pointer to the provider.

Function initializes provider of given type and returns pointer to the provider.
If provider is not supported, NULL is returned.
Implementation of this function is processor specific.

@param e_Provider: Pointer to the provider registry.
@return Pointer to the initialized provider. NULL if provider is not supported.
*/
slim_ServiceProviderInterfaceType *slim_ProviderConfInitProvider
(
    slim_ProviderEnumType e_Provider
)
{
    slim_ServiceProviderInterfaceType *pz_Provider = NULL;
    switch (e_Provider)
    {
    case SLIM_PROVIDER_SENSOR1:
#ifdef FEATURE_GSIFF_DSPS
        /* Initialize sensor1 only when it is configured */
        if (SENSOR1_SENSOR_PROVIDER == GetConfiguredSensorProvider())
        {
            slim_Sensor1Initialize(&pz_Provider);
        }
#else
        LOC_LOGI("%s: Sensor1 provider not present for this target", __func__);
#endif
        break;
    case SLIM_PROVIDER_NDK:
        slim_NDKInitialize(&pz_Provider);
        break;
    default:
        break;
    }
    return pz_Provider;
}

/**
@brief Provides initial provider setting for preferred provider.

Function provides initial provider setting for preferred provider.

@return Preferred provider setting
*/
slim_ProviderSettingEnumType slim_ProviderConfGetInitialProviderSetting
(
  void
)
{
    if (SENSOR1_SENSOR_PROVIDER == GetConfiguredSensorProvider())
    {
        return SLIM_PROVIDER_SETTING_SSC;
    }
    else
    {
        return SLIM_PROVIDER_SETTING_NATIVE;
    }
}

/**
@brief Returns available provider for the service requested by client.

Function returns available provider for the service requested by client.
Implementation of this function is processor specific.

@param q_AvailableProviders: Available service providers.
@param q_AllowedProviders: Providers allowed by client.
@param e_ProviderSetting: Preferred provider setting.
@return Provider type
*/
slim_ProviderEnumType slim_ProviderConfGetProviderForClientService
(
  uint32 q_AvailableProviders,
  slimServiceProviderMaskT q_AllowedProviders,
  slim_ProviderSettingEnumType e_ProviderSetting
)
{
    slim_ProviderEnumType e_Provider = SLIM_PROVIDER_NONE;

    /* SSC should be used only when it is enabled by provider setting */
    if (SLIM_PROVIDER_SETTING_NATIVE != e_ProviderSetting)
    {
        if (SLIM_MASK_IS_SET(q_AvailableProviders, SLIM_PROVIDER_SENSOR1) &&
            SLIM_MASK_IS_SET(q_AllowedProviders, eSLIM_SERVICE_PROVIDER_SSC))
        {
            e_Provider = SLIM_PROVIDER_SENSOR1;
        }
    }

    /* Check other providers if SSC was not used */
    if (SLIM_PROVIDER_NONE == e_Provider)
    {
        /* Native providers */
        if (SLIM_MASK_IS_SET(q_AllowedProviders, eSLIM_SERVICE_PROVIDER_NATIVE))
        {
            if (SLIM_MASK_IS_SET(q_AvailableProviders, SLIM_PROVIDER_NDK))
            {
                e_Provider = SLIM_PROVIDER_NDK;
            }
        }
    }

    return e_Provider;
}

/**
@brief Returns mask of available providers for client.

Function returns mask of available providers for client.
Implementation of this function is processor specific.

@param q_AvailableProviders: Available service providers.
@param e_ProviderSetting: Preferred provider setting.
@return Mask of type slimServiceProviderMaskT of available providers to be sent
to SLIM client.
*/
slimServiceProviderMaskT slim_ProviderConfGetClientProviderMask
(
  uint32 q_AvailableProviders,
  slim_ProviderSettingEnumType e_ProviderSetting
)
{
    slimServiceProviderMaskT q_ClientMask = 0;

    if (SLIM_PROVIDER_SETTING_NATIVE != e_ProviderSetting &&
        SLIM_MASK_IS_SET(q_AvailableProviders, SLIM_PROVIDER_SENSOR1))
    {
        SLIM_MASK_SET(q_ClientMask, eSLIM_SERVICE_PROVIDER_SSC);
    }
    if (SLIM_MASK_IS_SET(q_AvailableProviders, SLIM_PROVIDER_NDK))
    {
        SLIM_MASK_SET(q_ClientMask, eSLIM_SERVICE_PROVIDER_NATIVE);
    }

    return q_ClientMask;
}

/**
@brief Maps SLIM provider type to client service provider type.

Function maps SLIM provider type to client service provider type.
Implementation of this function is processor specific.

@param e_SlimProvider: SLIM provider type.
@return Client provider type.
*/
slimServiceProviderEnumT slim_ProviderConfMapSlimProvider
(
  slim_ProviderEnumType e_SlimProvider
)
{
    switch (e_SlimProvider)
    {
    case SLIM_PROVIDER_SENSOR1:
        return eSLIM_SERVICE_PROVIDER_SSC;
    case SLIM_PROVIDER_NDK:
        return eSLIM_SERVICE_PROVIDER_NATIVE;
    default:
        break;
    }
    return eSLIM_SERVICE_PROVIDER_DEFAULT;
}

/**
@brief Applies provider setting to the given SLIM provider mask.

Function applies provider setting to the given SLIM provider mask.
Implementation of this function is processor specific.

@param q_AvailableProviders: Available service providers.
@param e_ProviderSetting: Preferred provider setting.
@return Mask of SLIM providers.
*/
uint32 slim_ProviderConfApplyProviderSetting
(
  uint32 q_AvailableProviders,
  slim_ProviderSettingEnumType e_ProviderSetting
)
{
    if (SLIM_PROVIDER_SETTING_NATIVE == e_ProviderSetting)
    {
        SLIM_MASK_CLEAR(q_AvailableProviders, SLIM_PROVIDER_SENSOR1);
    }
    return q_AvailableProviders;
}
