#ifndef ABA_CORE_API_H
#define ABA_CORE_API_H
/*=================================================================================================

  File: aba_core_api.h

  DESCRIPTION
  This file contains interface specifications of the Adaptive Backlight
  Algorithm (ABA) feature for display enhancements on QUALCOMM MSM display
  solutions. It reduces display power consumption by adjusting pixel luminance
  and, in some cases, backlight. There are multiple
  algorithms implemented by ABA: CABL, FOSS...

     Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
     Qualcomm Technologies Proprietary and Confidential.
=================================================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "aba_type.h"


/*-------------------------------------------------------------------------------------------------
  Functions
---------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------

Function: AbaCreateSession

  This function creates the ABA session.
  Call only once. Use the handle returned to activate one or more features. Use handle for all APIs that
  require it.
Parameters:
  ppAbaSessionHandle          - [out]: Handle to the ABA session. The client will need use that handle in all
                             subsequent operations.

Return:
  AbaStatusType
-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaCreateSession(void **ppAbaSessionHandle);
/*-------------------------------------------------------------------------------------------------

Function: AbaGetDefaultParams

  Populates the default parameters. Call this function to request the default settings ( Available for CABL
  and SVI). The client can modify the parameters and apply those during a call to AbaInit.

Parameters:
  eConfigInfo       - [in]: Describes the feature and the Panel type.
  pAbaParameters    - [in]: Pointer to parameters that will be populated by ABA. The structure
                            passed in depends on the feature. For CABL use CablInitialConfigType
                            (TBD other algorithms).
  uParamSize        - [in]: Size of structure pointed to by pAbaParameters.

Return:
  AbaStatusType

-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaGetDefaultParams(AbaConfigInfoType eConfigInfo, void *pAbaParameters, uint32_t uParamSize);

/*-------------------------------------------------------------------------------------------------

Function: AbaInit

  This function initializes an ABA feature using the parameters passed in. Call this after AbaGetDefaultParams.
  Call this API for each feature that will be running in the ABA session. Call before attemping to process frames.


Parameters:
  eConfigInfo       - [in]:  Describes the feature and Panel type.
  pAbaOemParameters - [in]:  Pointer to OEM parameters from the client. For CABL, use CablInitialConfigType.
  uParamSize        - [in]:  Size of the structure pointed to by pAbaOemParameters.
  pHardwareInfo     - [in]:  Histogram configuration information.
  pHandle           - [in]:  Handle to the ABA context. The client will need use that handle in all
                             subsequent operations.

Return:
  AbaStatusType
-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaInit(
   AbaConfigInfoType    eConfigInfo,
   void                *pAbaOemParameters,
   uint32_t             uParamSize,
   AbaHardwareInfoType *pHardwareInfo,
   void                *pHandle);


/*-------------------------------------------------------------------------------------------------
Function: AbaDeinit

  Call when cleaning up the ABA session.
  Frees all memory allocated for the current ABA session. Do not use the handle after this call.
  Call only once for all features.

Parameters:
  pHandle         - [in]:   Handle to Aba module.

Return:
   None
-------------------------------------------------------------------------------------------------*/
void AbaDeinit(void *pHandle);

/*-------------------------------------------------------------------------------------------------

Function: AbaDynamicConfigUpdate

This function Dynamically updates the configuration parameters during runtime. Can be called anytime after
AbaInit is successful and ABA is processing. Use the feature flag to indicate the feature for which the
update is requested.


Parameters:
pHandle           - [in]:  Handle to the ABA context.
pAbaOemParameters - [in]:  Pointer to OEM parameters from the client.
uParamSize        - [in]:  Size of the structure pointed to by pAbaOemParameters.
uFeature          - [in]:  Feature to apply the setting to.

Return:
AbaStatusType
-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaDynamicConfigUpdate(
  void                *pHandle,
  void                *pAbaOemParameters,
  uint32_t             uParamSize,
  uint32_t             uFeature);


/*-------------------------------------------------------------------------------------------------
Function: AbaProcess

  AbaProcess processes a histogram and calculates backlight levels (when applicable) and/or LUTs .

Parameters:
  pHandle     - [in]:   Handle to ABA module.
  pHistogram  - [in]:   Pointer to the histogram that will be processed.
  pLUT        - [out]:  Caller allocated pointer used by ABA to return
                        the LUT.
  pBacklight  - [out]:  Optional: caller pointer to return the calculated backlight value. Set to NULL when
                        backlight information is not needed (FOSS, SVI).

Return:
  AbaStatusType

-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaProcess(void *pHandle, uint32_t *pHistogram, uint32_t *pLUT, uint32_t *pBacklight);


/*-------------------------------------------------------------------------------------------------
Function: AbaSetAmbientLightThreshold

  Call to set the ambient light threshold for ABA. The threshold triggers a switch between CABL and SVI.

Parameters:
  pHandle     - [in]:   Handle to ABA module.
  uThreshold  - [in]:   LUX level used to switch between CABL and SVI. The range for uThreshold is 600-1200.
                        When the LUX is lower than uThreshold, CABL runs, otherwise SVI

Return:
  AbaStatusType

-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaSetAmbientLightThreshold(void *pHandle, uint32_t uThreshold);


/*-------------------------------------------------------------------------------------------------
Function: AbaGetState

  Returns the current state of ABA for the feature passed in as an input parameter.

Parameters:
  pHandle          - [in]:    Handle to Aba module.
  pState           - [out]:   Pointer to the current state of ABA.
  uFeature         - [in]:   Feature to apply the setting to.

Return:
  AbaStatusType

-------------------------------------------------------------------------------------------------*/
AbaStatusType  AbaGetState(void *pHandle, AbaStateType *pState, uint32_t eFeature);


/*-------------------------------------------------------------------------------------------------
Function: AbaSetOriginalBacklightLevel

  Communicates a user backlight level change. ABA will set the backlight to all features that support
  backlight input.

Parameters:
  pHandle         - [in]:   Handle to Aba module
  uOriginalLevel  - [in]:   Backlight level to apply to ABA

Return:
  AbaStatusType

-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaSetOriginalBacklightLevel(void *pHandle, uint32_t uOriginalLevel);


/*-------------------------------------------------------------------------------------------------
Function: AbaSetAmbientLightLevel

  Communicates the current ambient light level to the ABA core. Call only when SVI has been initialized
  either as a standallone feature or coexisting with CABL.

Parameters:
  pHandle                   - [in]:   Aba Handle
  uInputAmbientLightLevel   - [in]:   Ambient light  level to apply to ABA
  pOutputAmbientLightLevel: - [in/out]  New ambient light level calculated by algorithm.

Return:
  AbaStatusType

-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaSetAmbientLightLevel(void *pHandle, uint32_t uInputAmbientLightLevel, uint32_t *pOutputAmbientLightLevel);

/*-------------------------------------------------------------------------------------------------
Function: AbaSetFilterLevel

  Sets the filter speed level for ABA:
  LOW:     Slower speed
  MEDIUM:  Medium quality
  HIGH:    highest quality
  Currently availble for SVI only.

Parameters:
  pHandle           - [in]:   Handle to Aba module
  eFilterSpeed      - [in]:   Filter speed level
  uFeature          - [in]:   Feature to apply the setting to.

Return:
  AbaStatusType

-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaSetFilterLevel(void *pHandle, AbaFilterLevelType eFilterSpeed, uint32_t uFeature);

/*-------------------------------------------------------------------------------------------------
Function: AbaSetQualityLevel

  Sets the quality level for ABA:
  LOW:     Minimal quality
  MEDIUM:  Medium quality
  HIGH:    highest quality
  Currently availble for CABL only.

Parameters:
  pHandle           - [in]:   Handle to Aba module
  eABAQualityLevel  - [in]:   Quality level to apply to ABA

Return:
  AbaStatusType

-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaSetQualityLevel(void *pHandle, AbaQualityLevelType eABAQualityLevel);
/*-------------------------------------------------------------------------------------------------
Function: AbaActivate

  Activates the ABA feature passed in as an input parameter. Call activate on a feature after it has been
  initialized. When ABA is in the active state, it  processes histograms and calculates backlight and/or LUT.


Parameters:
  pHandle          - [in]:   Handle to Aba module
  uFeature         - [in]:   Feature to apply the setting to.

Return:
  AbaStatusType

-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaActivate(void *pHandle, uint32_t uFeature);

/*-------------------------------------------------------------------------------------------------
Function: AbaDeactivate

  Deactivates the ABA feature passed in as an input parameter. Call Deactivate on a feature after it has been
  initialized. Changes the ABA state to inactive. When in the inactive state, ABA will not perform any operation
  on the histograms. From the inactive state, ABA may move to the active state automatically if external parameters
  such as backlight, ambient light change. That process is internal and does not require any action from the client.


Parameters:
  pHandle          - [in]:   Handle to Aba module.
  uFeature         - [in]:   Feature to apply the setting to.
Return:
  AbaStatusType

-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaDeactivate(void *pHandle, uint32_t uFeature);

/*-------------------------------------------------------------------------------------------------
Function: AbaDisable

  Disables the ABA feature passed in as an input parameter. Call Disable on a feature after it has been
  initialized.
  Changes the ABA state to a disabled state. When disabled, ABA will not perform any operation on the histograms.
  The client will need to explicitly enable ABA by calling AbaActivate() in order to resume processing. Changing
  the external parameters such as backlight in this state will have no effect.


Parameters:
  pHandle          - [in]:   Handle to Aba module.
  uFeature         - [in]:   Feature to apply the setting to.
Return:
  AbaStatusType

-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaDisable(void *pHandle, uint32_t uFeature);

/*-------------------------------------------------------------------------------------------------
Function: AbaIsConverged

  Use AbaIsConverged to inquire if ABA has converged to stationary state. When ABA receives a histogram representing a
  new frame, it calculates the targeted backlight and/or LUT for that histogram. It process several frames and applies
  gradual changes on each of those frames. ABA reaches a stationary state when it is done applying
  the gradual changes.
  Resturns TRUE if all initialized features have converged.

Parameters:
  pHandle          - [in]:    Handle to Aba module
  pbConverged      - [out]:   ABA will set this pointer to TRUE when it is in a stationary mode, and FALSE when it is
                              still processing gradual changes.

Return:
  AbaStatusType

-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaIsConverged(void *pHandle, bool32 *pbConverged);

/*-------------------------------------------------------------------------------------------------
Function: AbaGetVersion

  Returns the version of the ABA code. API is optional.

Parameters:
  pVersion         - [in]:    Pointer to the version.
Return:
  AbaStatusType

-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaGetVersion(AbaVersionType *pVersion);


/*-------------------------------------------------------------------------------------------------
Function: AbaMapLut

  Use to change a one channel LUT into a 3 channel LUT.
  Use for CABL. Not needed for MDP5 and above. Do not use with SVI as it may create color shift.
  SVI is intended for targets which generate one channel histograms (MDP v5+).

Parameters:
  pHandle              - [in]:    Handle to Aba module.
  pInputLut            - [in]:    Input LUT.
  pOutputLut           - [out]:   Output LUT.
  uLength              - [in]:    Size of the LUT.

Return:
   AbaStatusType
-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaMapLut(void *pHandle, uint32_t *pInputLut, uint32_t *pOutputLut, uint32_t uLength);


/*-------------------------------------------------------------------------------------------------
Function: AbaPreprocessHistogram

  Use to change to change a 3 channel histogram to single channel histogram for MDP v4 and below.
  Use for CABL. Not needed for MDP5 and above. Do not use with SVI as it may create color shift.
  SVI is intended for targets which generate one channel histograms (MDP v5+).

Parameters:
  pHandle                    - [in]:    Handle to ABA module.
  pInputHistogram            - [in]:    Input histogram.
  pOutputHistogram           - [out]:   Output histogram.

Return:
   AbaStatusType
-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaPreprocessHistogram(
   void            *pHandle,
   uint32_t        *pInputHistogram,
   uint32_t        *pOutputHistogram);

/*-------------------------------------------------------------------------------------------------
Function: AbaGetProcessInfo

Returns processing  information.

Parameters:
pHandle               - [in]:    Handle to Aba module
pProcessingInfo       - [out]:   Pointer in which ABA writes the processing Info
uParamSize            - [in]:    Size of pProcessingInfo
uFeature              - [in]:    Feature to query info.

Return:
AbaStatusType

-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaGetProcessInfo(void *pHandle, void *pProcessingInfo, uint32_t uParamSize, uint32_t uFeature);

/*-------------------------------------------------------------------------------------------------
Function: AbaGetDebugInfo

  Returns CABL specific debug information.

Parameters:
  pHandle          - [in]:    Handle to Aba module
  pDebugInfo       - [out]:   Pointer in which ABA writes the debug info

Return:
  AbaStatusType

-------------------------------------------------------------------------------------------------*/
AbaStatusType AbaGetDebugInfo(void *pHandle, void *pDebugInfo);
#ifdef __cplusplus
}
#endif

#endif /* ABACORE_API_H */
