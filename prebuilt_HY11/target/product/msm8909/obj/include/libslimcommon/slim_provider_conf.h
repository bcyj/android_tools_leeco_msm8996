#ifndef SLIM_PROVIDER_CONF_H
#define SLIM_PROVIDER_CONF_H
/*============================================================================
  @file slim_provider_conf.h

  SLIM processor specific provider configuration declaration.

               Copyright (c) 2014 QUALCOMM Atheros, Inc.
               All Rights Reserved.
               Qualcomm Atheros Confidential and Proprietary
============================================================================*/
/* $Header: //components/rel/gnss.mpss/6.0/gnss/slim/common/osal/inc/slim_provider_conf.h#2 $ */

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "slim_provider_data.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/
/**
@brief Initializes provider of given type and returns pointer to the provider.

Function initializes provider of given type and returns pointer to the provider.
If provider is not supported, NULL is returned.
Implementation of this function is processor specific.

@param e_Provider Provider type.
@return Pointer to the initialized provider. NULL if provider is not supported.
*/
slim_ServiceProviderInterfaceType *slim_ProviderConfInitProvider
(
  slim_ProviderEnumType e_Provider
);

/**
@brief Provides initial provider setting for preferred provider.

Function provides initial provider setting for preferred provider.
Implementation of this function is processor specific.

@return Preferred provider setting
*/
slim_ProviderSettingEnumType slim_ProviderConfGetInitialProviderSetting
(
  void
);

/**
@brief Returns available provider for the service requested by client.

Function returns available provider for the service requested by client.
Implementation of this function is processor specific.

@param q_AvailableProviders Available service providers.
@param q_AllowedProviders Providers allowed by client.
@param e_ProviderSetting Preferred provider setting.
@return Provider type
*/
slim_ProviderEnumType slim_ProviderConfGetProviderForClientService
(
  uint32 q_AvailableProviders,
  slimServiceProviderMaskT q_AllowedProviders,
  slim_ProviderSettingEnumType e_ProviderSetting
);

/**
@brief Returns mask of available providers for client.

Function returns mask of available providers for client.
Implementation of this function is processor specific.

@param q_AvailableProviders Available service providers.
@param e_ProviderSetting Preferred provider setting.
@return Mask of type slimServiceProviderMaskT of available providers to be sent
to SLIM client.
*/
slimServiceProviderMaskT slim_ProviderConfGetClientProviderMask
(
  uint32 q_AvailableProviders,
  slim_ProviderSettingEnumType e_ProviderSetting
);

/**
@brief Maps SLIM provider type to client service provider type.

Function maps SLIM provider type to client service provider type.
Implementation of this function is processor specific.

@param e_SlimProvider SLIM provider type.
@return Client provider type.
*/
slimServiceProviderEnumT slim_ProviderConfMapSlimProvider
(
  slim_ProviderEnumType e_SlimProvider
);

/**
@brief Applies provider setting to the given SLIM provider mask.

Function applies provider setting to the given SLIM provider mask.
Implementation of this function is processor specific.

@param q_AvailableProviders Available service providers.
@param e_ProviderSetting Preferred provider setting.
@return Mask of SLIM providers.
*/
uint32 slim_ProviderConfApplyProviderSetting
(
  uint32 q_AvailableProviders,
  slim_ProviderSettingEnumType e_ProviderSetting
);

#endif /* #ifndef SLIM_PROVIDER_CONF_H */
