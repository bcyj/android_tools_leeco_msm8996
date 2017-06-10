#ifndef __ACDB_COMMAND_H__
#define __ACDB_COMMAND_H__

/*===========================================================================
    @file   acdb_command.h

    This file contains the implementation of the Acdb public interface.

    This file contains the implementation of the commands exposed by the
    Acdb public interface.

                    Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include "acdb_includes.h"
#include "acdb.h"

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
* Class Definitions
*--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Function Declarations and Documentation
 *--------------------------------------------------------------------------- */

int32_t AcdbCmdInitializeAcdb ();

int32_t AcdbCmdGetAcdbSwVersion (AcdbModuleVersionType *pOutput);

int32_t AcdbCmdGetTargetVersion (AcdbTargetVersionType *pOutput);

int32_t AcdbCmdGetDeviceInfo (AcdbDeviceInfoCmdType* pInput,
                              AcdbQueryResponseType* pOutput);

int32_t AcdbCmdGetDeviceCapabilities (AcdbDeviceCapabilitiesCmdType *pInput,
                                       AcdbQueryResponseType *pOutput);

int32_t AcdbCmdGetDevicePair (AcdbDevicePairType *pInput,
                              AcdbDevicePairingResponseType *pOutput);

int32_t AcdbCmdGetAudProcTable (AcdbAudProcTableCmdType *pInput,
                                 AcdbQueryResponseType *pOutput);

int32_t AcdbCmdGetAudProcData (AcdbAudProcCmdType *pInput,
                              AcdbQueryResponseType *pOutput);

int32_t AcdbCmdGetAudStrmTable (AcdbAudStrmTableCmdType *pInput,
                                 AcdbQueryResponseType *pOutput);

int32_t AcdbCmdGetAudStrmData (AcdbAudStrmCmdType *pInput,
                              AcdbQueryResponseType *pOutput);

int32_t AcdbCmdGetVocProcTable (AcdbVocProcTableCmdType *pInput,
                                 AcdbQueryResponseType *pOutput);

int32_t AcdbCmdGetVocProcData (AcdbVocProcCmdType *pInput,
                              AcdbQueryResponseType *pOutput);

int32_t AcdbCmdGetVocStrmTable (AcdbVocStrmTableCmdType *pInput,
                                 AcdbQueryResponseType *pOutput);

int32_t AcdbCmdGetVocStrmData (AcdbVocStrmCmdType *pInput,
                              AcdbQueryResponseType *pOutput);

int32_t AcdbCmdGetMemoryUsage (AcdbMemoryUsageType* pOutput);

int32_t AcdbCmdSetAudProcTable (AcdbAudProcTableCmdType *pInput);

int32_t AcdbCmdSetAudProcData (AcdbAudProcCmdType *pInput);

int32_t AcdbCmdSetAudStrmTable (AcdbAudStrmTableCmdType *pInput);

int32_t AcdbCmdSetAudStrmData (AcdbAudStrmCmdType *pInput);

int32_t AcdbCmdSetVocProcTable (AcdbVocProcTableCmdType *pInput);

int32_t AcdbCmdSetVocProcData (AcdbVocProcCmdType *pInput);

int32_t AcdbCmdSetVocStrmTable (AcdbVocStrmTableCmdType *pInput);

int32_t AcdbCmdSetVocStrmData (AcdbVocStrmCmdType *pInput);

int32_t AcdbCmdSystemReset(void);

int32_t AcdbCmdGetOEMInfo (AcdbGeneralInfoCmdType *pInput,
                           AcdbQueryResponseType *pOutput);

int32_t AcdbCmdSetOEMInfo (AcdbGeneralInfoCmdType *pInput);

int32_t AcdbCmdGetDateInfo (AcdbGeneralInfoCmdType *pInput,
                            AcdbQueryResponseType *pOutput);

int32_t AcdbCmdSetDateInfo (AcdbGeneralInfoCmdType *pInput);

int32_t AcdbCmdGetDevicePairList (AcdbGeneralInfoCmdType *pInput,
                                  AcdbQueryResponseType *pOutput);

int32_t AcdbCmdGetVocProcGainDepVolTable (AcdbVocProcVolTblCmdType *pInput,
                                          AcdbQueryResponseType *pOutput);

int32_t AcdbCmdGetAudProcGainDepVolTable (AcdbAudProcVolTblCmdType *pInput,
                                          AcdbAudProcGainDepVolTblRspType *pOutput);

int32_t AcdbCmdGetAudProcGainDepCoppTableStep (AcdbAudProcGainDepVolTblStepCmdType *pInput,
                                               AcdbQueryResponseType *pOutput);

int32_t AcdbCmdGetAudProcGainDepPoppTableStep (AcdbAudProcGainDepVolTblStepCmdType *pInput,
                                               AcdbQueryResponseType *pOutput);

int32_t AcdbCmdGetVolTableStepSize (AcdbVolTblStepSizeRspType *pOutput);

int32_t AcdbCmdSetVocProcGainDepData (AcdbVocProcVolTblDataCmdType *pInput);

int32_t AcdbCmdGetVocProcGainDepData (AcdbVocProcVolTblDataCmdType *pInput,
                                      AcdbQueryResponseType *pOutput);

int32_t AcdbCmdSetAudProcGainDepData (AcdbAudProcVolTblDataCmdType *pInput);

int32_t AcdbCmdGetAudProcGainDepData (AcdbAudProcVolTblDataCmdType *pInput,
                                      AcdbQueryResponseType *pOutput);

int32_t AcdbCmdGetANCDevicePair (AcdbAncDevicePairCmdType *pInput,
                                 AcdbAncDevicePairRspType *pOutput);

int32_t AcdbCmdGetANCDevicePairList (AcdbGeneralInfoCmdType *pInput,
                                     AcdbQueryResponseType *pOutput);

int32_t AcdbCmdSetAdieProfileTable (AcdbAdiePathProfileCmdType *pInput
                                   );

int32_t AcdbCmdGetAdieProfileTable (AcdbAdiePathProfileCmdType *pInput,
                                    AcdbQueryResponseType *pOutput);

int32_t AcdbCmdSetAdieANCDataTable (AcdbANCSettingCmdType *pInput
                                    );

int32_t AcdbCmdGetAdieANCDataTable (AcdbANCSettingCmdType *pInput,
                                    AcdbQueryResponseType *pOutput
                                    );

int32_t AcdbCmdGetLookupTableSize (AcdbGetTableSizeCmdType *pInput,
                                   AcdbGetTableSizeRspType *pOutput
                                   );

int32_t AcdbCmdGetTableLookupIndex (AcdbQueriedTblIndexCmdType *pInput,
                                    AcdbQueryResponseType *pOutput
                                    );

int32_t AcdbCmdGetAudProcCmnTopId (AcdbGetAudProcTopIdCmdType *pInput,
                                   AcdbGetTopologyIdRspType *pOutput
                                   );

int32_t AcdbCmdGetAudProcStrmTopId (AcdbGetAudProcStrmTopIdCmdType *pInput,
                                    AcdbGetTopologyIdRspType *pOutput
                                    );

int32_t AcdbCmdGetAudProcCmnTopIdList (AcdbGeneralInfoCmdType *pInput,
                                       AcdbQueryResponseType *pOutput
                                       );

int32_t AcdbCmdGetAudProcStrmTopIdList (AcdbGeneralInfoCmdType *pInput,
                                        AcdbQueryResponseType *pOutput
                                        );

int32_t AcdbCmdGetAfeData (AcdbAfeDataCmdType *pInput,
                           AcdbQueryResponseType *pOutput
                           );

int32_t AcdbCmdSetAfeData (AcdbAfeDataCmdType *pInput
                           );

int32_t AcdbCmdGetGlobalTable (AcdbGblTblCmdType *pInput,
                               AcdbQueryResponseType *pOutput);

int32_t AcdbCmdSetGlobalTable (AcdbGblTblCmdType *pInput
                               );


int32_t AcdbCmdGetAfeCmnData (AcdbAfeCmnDataCmdType *pInput,
                           AcdbQueryResponseType *pOutput);
int32_t AcdbCmdSetAfeCmnData (AcdbAfeCmnDataCmdType *pInput
                           );
int32_t AcdbCmdGetAfeCmnTable (AcdbAfeCommonTableCmdType *pInput,
                                AcdbQueryResponseType *pOutput);
int32_t AcdbCmdSetAfeCmnTable (AcdbAfeCommonTableCmdType *pInput
                                );
int32_t AcdbCmdGetAfeTopIDList (AcdbGeneralInfoCmdType *pInput,
                                  AcdbQueryResponseType *pOutput);


/* ---------------------------------------------------------------------------
 * Externalized Function Declarations and Definitions
 *--------------------------------------------------------------------------- */

int32_t acdb_translate_sample_rate (const uint32_t nInput,
                                    uint32_t *pOutput
                                    );

int32_t acdb_translate_over_sample_rate (const uint32_t nInput,
                                         uint32_t *pOutput
                                         );

int32_t acdb_adieprofile_translation (const uint32_t nParamId,
                                      const uint32_t nDefaultParamId,
                                      uint8_t *pBuffer,
                                      uint8_t *pDataPtr,
                                      const uint32_t nDataSize
                                      );

int32_t acdb_deviceinfo_translation (const uint32_t nParamId,
                                     const uint32_t nDefaultParamId,
                                     uint8_t *pSrcData,
                                     uint8_t *pDstData,
                                     uint32_t nBufferSize,
                                     uint32_t *pRspSize
                                     );

int32_t acdb_deviceinfo_translationLev1 (const uint32_t nParamId,
                                         const uint32_t nDefaultParamId,
                                         uint8_t *pSrcData,
                                         uint8_t *pDstData,
                                         uint32_t nBufferSize
                                         );

int32_t acdb_deviceinfo_translationLev2 (const uint32_t nParamId,
                                         const uint32_t nDefaultParamId,
                                         uint8_t *pSrcData,
                                         uint8_t *pDstData,
                                         uint32_t nBufferSize,
                                         uint32_t *pRspSize
                                         );

int32_t acdb_validate_PID (uint32_t cmdPID,
                           uint32_t* pParamId,
                           uint32_t nDefaultParamId
                           );

int32_t acdb_map_command_PID (const uint32_t nCmdPID,
                              uint32_t *pDataPID
                              );

int32_t acdb_validate_data_to_copy (const uint32_t nCmdPID,
                                    const uint32_t nDataLenToCopy,
                                    uint32_t *pRsp
                                    );

#endif /* __ACDB_COMMAND_H__ */
