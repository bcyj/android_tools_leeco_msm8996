#ifndef __ACDB_PRIVATE_H__
#define __ACDB_PRIVATE_H__
/*===========================================================================
    @file   acdb_private.h

    This file contains the private interface to the Audio Calibration Database
    (ACDB) module.

    The private interface of the Audio Calibration Database (ACDB) module,
    providing calibration storage for the audio and the voice path. The ACDB
    module is coupled with Qualcomm Audio Calibration Tool (QACT) v2.x.x.
    Qualcomm recommends to always use the latest version of QACT for
    necessary fixes and compatibility.

                    Copyright (c) 2011-2012 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* $Header: //source/qcom/qct/multimedia2/Audio/audcal2/common/8k_PL/acdb/rel/4.2/src/acdb_private.h#1 $ */

/*===========================================================================

EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/22/11   ernanl  Introduced first version of 9615 ACDB Private API.
06/01/12   kpavan  Introduced internal interface to get Audio EC table.

===========================================================================*/

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */

#ifdef ACDB_SIM_ENV
    #include "mmdefs.h"
#endif 

/* ---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * Type Declarations
 *--------------------------------------------------------------------------- */

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_DEVICE_PAIRED_LIST Declarations and Documentation
 *--------------------------------------------------------------------------- */
/**
   @fn   acdb_ioctl (ACDB_CMD_GET_DEVICE_CHANNEL_TYPE_LIST, ...)
   @brief API to query for a list of device pairs.

   This command will query for all possible device channel types.

   return structure will look like
   struct DeviceChannelTypeList {
      uint32_t nCount;
      struct DeviceChannelTypeItem {
         uint32_t nDeviceID;
         uint32_t nDeviceChannelType[8]; //Channel mappings: L, R...
      } DeviceChannelTypeItem[nCount];
   };

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_DEVICE_CHANNEL_TYPE_LIST.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGeneralInfoCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGeneralInfoCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbGeneralInfoCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_DEVICE_CHANNEL_TYPE_LIST 0x00011338
/**
   @struct   AcdbGeneralInfoCmdType
   @brief This is a query return result structure for query DevicePairList/ANCDevicePairList
          OEM/Date information.

   @param nBufferPointer: pointer to query buffer
   @param nBufferLength: The buffer length

   This is a query command structure for OEM/Date information
*/
typedef struct _AcdbGeneralInfoCmdType {
   uint32_t nBufferLength;
   uint8_t* nBufferPointer;
} AcdbGeneralInfoCmdType;

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_DEVICE_PAIR_LIST, ...)
   @brief API to query for a list of device pairs.

   This command will query for all possible device pairs.

   return structure will look like
   struct DevicePairList {
      uint32_t nPairs;
      struct DevicePair {
         uint32_t nTxDevice;
         uint32_t nRxDevice;
      } DevicePair[nCount];
   };

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_DEVICE_PAIRED.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGeneralInfoCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGeneralInfoCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbGeneralInfoCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_DEVICE_PAIR_LIST                   0x000111DB

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_OEM_INFO Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDB_CMD_GET_OEM_INFO, ...)
   @brief API to query for the OEM Information

   This command will obtain the OEM information queried for.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_OEM_INFO.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGeneralInfoCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGeneralInfoCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[out] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbGeneralInfoCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_OEM_INFO                             0x000111DC

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_DATE_INFO Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDB_CMD_GET_DATE_INFO, ...)
   @brief API to query for the OEM Information

   This command will obtain the Date information queried for.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_DATE_INFO.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGeneralInfoCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGeneralInfoCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[out] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbGeneralInfoCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_DATE_INFO                             0x000111DD

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_ANC_DEVICE_PAIRED_LIST Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_ANC_DEVICE_PAIR_LIST, ...)
   @brief API to query for a list of ANC device pairs.

   This command will query for all possible ANC device pairs.

   return structure will look like
   struct ANCDevicePairList {
      uint32_t nPairs;
      struct ANCDevicePair {
         uint32_t nANCTxDevice;
         uint32_t nANCRxDevice;
      } ANCDevicePair[nCount];
   };

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_ANC_DEVICE_PAIR_LIST.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGeneralInfoCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGeneralInfoCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbGeneralInfoCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_ANC_DEVICE_PAIR_LIST                   0x000111DF

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_TABLE_SIZE Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_TABLE_SIZE, ...)
   @brief This API is to query for table size

   This command will query for all available table size such as vocproc common,
                vocproc stream, vocproc gain-dependent calibration, audproc 
                common, audproc stream, audproc gain-dependent calibration, adie
                profile calibration, ANC data setting tables.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_TABLE_SIZE.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGetTableSizeCmdType
   @param[in] nCommandStructLength:
         This should equal to sizeof (AcdbGetTableSizeCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbGetTableSizeRspType.
   @param[in] nResponseStructLength:
         This should equal to sizeof (AcdbGetTableSizeRspType).

   @see  acdb_ioctl
   @see  AcdbGetTableSizeCmdType
   @see  AcdbGetTableSizeRspType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_BADSTATE: When ACDB is in a bad state.
*/
#define ACDB_CMD_GET_LOOKUP_TABLE_SIZE                    0x00011210

/**
   @struct   AcdbGetTableSizeCmdType
   @brief This is a query command structure to understand the size of specific table
          by nParamId

   @param nParamId: Parameter ID associate with vocproc common, audproc common, 
                    vocproc stream, audproc stream, vocproc gain-dependent, audproc 
                    gain-dependent, adie codec path profile and ANC config data tables.

   This is a query command structure to understand the size of specific table by 
   nParamId.
*/
typedef struct _AcdbGetTableSizeCmdStruct {
   uint32_t nParamId;
} AcdbGetTableSizeCmdType;

/**
   @struct   AcdbGetTableSizeRspType
   @brief This is a query response command structure to understand the how many entries
          contain in the table

   @param nEntries: number of entries.

   This is a query response command structure to understand the how many entries contain
   in the table.
*/
typedef struct _AcdbTableSizeRspStruct {
   uint32_t nEntries;
} AcdbGetTableSizeRspType;

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_TABLE_INDEX_COMBINATION Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_TABLE_INDEX_COMBINATION, ...)
   @brief This API is to query for all available index keys for vocproc common,
          audproc common, vocproc stream, audproc stream, vocproc gain-dependent,
          audproc gain-dependent, adie codec path profile and ANC config data tables 
          from default database.

   This command will query for all available index keys for vocproc common,
          audproc common, vocproc stream, audproc stream, vocproc gain-dependent,
          audproc gain-dependent, adie codec path profile and ANC config data tables 
          from default database.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_TABLE_INDEX_COMBINATION.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbQueriedTblIndexCmdType.
   @param[in] nCommandStructLength:
         This should equal to size of (AcdbQueriedTblIndexCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbQueriedTblIndexCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
         ACDB_BADSTATE: When ACDB is in a bad state.
*/
#define ACDB_CMD_GET_TABLE_INDEX_COMBINATION                   0x00011211

/**
   @struct  AcdbQueriedTblIndexCmdType
   @brief This is a query command structure for query the lookup index mask
          of vocproc common, audproc common, vocproc stream, audproc stream, 
          vocproc gain-dependent, audproc gain-dependent, adie codec path profile 
          and ANC config data tables.
   
   @param nParamId: Parameter ID associate with vocproc common, audproc common, 
                    vocproc stream, audproc stream, vocproc gain-dependent, audproc 
                    gain-dependent, adie codec path profile and ANC config data tables.
   @param nTblKeyIndexStart: The table Index start to copy.
   @param nDataLenToCopy: The acutal specify data length client for query.
   @param nBufferLength: The length of the calibration buffer. This should be
                         large enough to hold the parameters identified by the
                         nParamId identifier.
   @param nBufferPointer: A virtual memory pointer pointing to a memory region
                          containing the payload (or receiving the payload) 
                          identified by nParamId.

   This is a query command structure for query all available index keys for vocproc 
           common, audproc common, vocproc stream, audproc stream, vocproc gain-dependent,
           audproc gain-dependent, adie codec path profile and ANC config data tables from
           default database.
*/
typedef struct _AcdbQueriedTblIndexCmdStruct {
   uint32_t nParamId;
   uint32_t nTblKeyIndexStart;
   uint32_t nDataLenToCopy;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbQueriedTblIndexCmdType;

/**
   @def ACDB_PID_VOCPROC_COMMON_TABLE
*/
#define ACDB_PID_VOCPROC_COMMON_TABLE                        0x00011218
/**
   This paramId is to use for query VocProc common table lookup index and
   number of VocProc common table entries
   
   Returned VocProc common table entries buffer structure format
   struct VocProcCmnLookupIndex{
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
   uint32_t nTxDeviceSampleRateId;
   uint32_t nRxDeviceSampleRateId;
   uint32_t nNetworkId;
   uint32_t nVocProcSampleRateId;
   }

   struct VocProcCmnIndexList {
   VocProcCmmLookupIndex [number of vocProc common table entries];
   }
*/

/**
   @def ACDB_PID_VOCPROC_STREAM_TABLE
*/
#define ACDB_PID_VOCPROC_STREAM_TABLE                        0x00011219
/**
   This paramId is to use for query VocProc stream table lookup index and
   number of VocProc stream table entries

   Returned VocProc stream table entries buffer structure format
   struct VocProcStrmLookupIndex{
   uint32_t nNetworkId;
   uint32_t nVocProcSampleRateId;
   }

   struct VocProcStrmIndexList {
   VocProcCmmLookupIndex [number of vocProc stream table entries];
   }
*/

/**
   @def ACDB_PID_VOCPROC_VOLUME_TABLE
*/
#define ACDB_PID_VOCPROC_VOLUME_TABLE                        0x0001121A
/**
   This paramId is to use for query VocProc volume gain-dependent table lookup
   index and number of VocProc volume table entries

   Returned VocProc volume table entries buffer structure format
   struct VocProcVolLookupIndex{
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
   uint32_t nNetworkId;
   uint32_t nVocProcSampleRateId;
   }

   struct VocProcVolIndexList {
   VocProcCmmLookupIndex [number of vocProc volume table entries];
   }
   */

/**
   @def ACDB_PID_AUDPROC_COMMON_TABLE
*/
#define ACDB_PID_AUDPROC_COMMON_TABLE                       0x0001121B
/**
   This paramId is to use for query AudProc common table lookup index and 
   number of AudProc common table entries

   Returned AudProc common table entries buffer structure format
   struct AudProcCmnLookupIndex{
   uint32_t nDeviceId;
   uint32_t nDeviceSampleRateId;
   uint32_t nApplicationType;
   }

   struct AudProcCmnIndexList {
   AudProcCmnLookupIndex [number of audproc common table entries];
   }
*/

/**
   @def ACDB_PID_AUDPROC_STREAM_TABLE
*/
#define ACDB_PID_AUDPROC_STREAM_TABLE                        0x0001121C
/**
   This paramId is to use for query AudProc stream table lookup index and 
   number of AudProc stream table entries

   Returned AudProc stream table entries buffer structure format
   struct AudProcStrmLookupIndex{
   uint32_t nDeviceId;
   uint32_t nApplicationTypeId;
   }

   struct AudProcStrmIndexList {
   AudProcStrmLookupIndex [number of audproc stream table entries];
   }
*/

/**
   @def ACDB_PID_AUDPROC_VOLUME_TABLE
*/
#define ACDB_PID_AUDPROC_VOLUME_TABLE                        0x0001121D
/**
   This paramId is to use for query AudProc volume gain-dependent table lookup
   index and number of AudProc volume table entries

   Returned AudProc volume table entries buffer structure format
   struct AudProcVolLookupIndex{
   uint32_t nDeviceId;
   uint32_t nApplicationType
   }

   struct AudProcVolIndexList {
   AudProcVolLookupIndex [number of audproc volume table entries];
   }
*/

/**
   @def ACDB_PID_ADIE_TABLE
*/
#define ACDB_PID_ADIE_PROFILE_TABLE                          0x0001121E
/**
   This paramId is to use for query Adie codec path prfile table lookup index
   and number of Adie codec path profile table entries

   Returned Adie codec path profile table entries buffer structure format
   struct AdieProfileLookupIndex{
   uint32_t ulCodecPathId;
   uint32_t nFrequencyId;
   uint32_t nOversamplerateId;
   }

   struct AdieProfileIndexList {
   AdieProfileLookupIndex [number of Adie codec path profile table entries];
   }
*/

/**
   @def ACDB_PID_ANC_CONFIG_DATA_TABLE
*/
#define ACDB_PID_ANC_CONFIG_DATA_TABLE                        0x0001121F
/**
   This paramId is to use for query ANC config data table lookup index and number
   of ANC config data table entries
   
   Returned ANC config data table entries buffer structure format
   struct ANCConfigDataLookupIndex{
   uint32_t nRxDeviceId;
   uint32_t nFrequencyId;
   uint32_t nOversamplerateId;
   }

   struct ANCConfigDataIndexList {
   ANCConfigDataLookupIndex [number of ANC config data table entries];
   }
*/

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID_LIST Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDB_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID_LIST, ...)
   @brief API to query for audio COPP topology ID list

   This command will obtain audproc common COPP topology ID list queried for.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AUDPROC_TOPOLOGY_ID_LIST.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGeneralInfoCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof(AcdbGeneralInfoCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[out] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbGeneralInfoCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.

   This command will obtain audproc common COPP topology ID list queried for.
   struct TopologyList {
         uint32_t nCount;
         struct _acdb_Topologylist {
            uint32_t nDeviceId;
            uint32_t nApplicationTypeId;
            uint32_t nTopologyId;
         } acdb_Topologylist [nCount];
      };
*/
#define ACDB_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID_LIST            0x0001122F

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID_LIST Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID_LIST, ...)
   @brief API to query for audproc stream POPP topology ID list

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID_LIST.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGeneralInfoCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof(AcdbGeneralInfoCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[out] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbGeneralInfoCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.

   This command will obtain audproc stream POPP topology ID list queried for.

   struct TopologyList {
         uint32_t nCount;
         struct _acdb_Topologylist {
            uint32_t nApplicationTypeId
            uint32_t nTopologyId;
         } acdb_Topologylist [nCount];
      };
*/
#define ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID_LIST            0x00011230

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_VOCPROC_COMMON_TOPOLOGY_ID_LIST Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDB_CMD_GET_VOCPROC_COMMON_TOPOLOGY_ID_LIST, ...)
   @brief API to query for vocproc common COPP topology ID list

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_VOCPROC_COMMON_TOPOLOGY_ID_LIST.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGeneralInfoCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof(AcdbGeneralInfoCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[out] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbGeneralInfoCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.

   This command will obtain vocproc common COPP topology ID list queried for.

   struct TopologyList {
         uint32_t nCount;
         struct _acdb_Topologylist {
            uint32_t nApplicationTypeId
            uint32_t nTopologyId;
         } acdb_Topologylist [nCount];
      };
*/
#define ACDB_CMD_GET_VOCPROC_COMMON_TOPOLOGY_ID_LIST            0x000112B6

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_QACT_INFO Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   AcdbDataIoctl (ACDB_CMD_GET_QACT_INFO, ...)
   @brief API to query for the QACT Information

   This command will obtain the QACT information queried for.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_QACT_INFO.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGeneralInfoCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGeneralInfoCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[out] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbGeneralInfoCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_QACT_INFO                             0x000112B7


/**
   This paramId is to use for query Afe common table lookup index and 
   number of Afe common table entries

   Returned Afe common table entries buffer structure format
   struct AfeCmnLookupIndex{
   uint32_t nDeviceId;
   uint32_t nAfeSampleRateId;
   }

   struct AfeCmnIndexList {
   AfeCmnLookupIndex [number of afe common table entries];
   }
*/
#define ACDB_PID_AFE_COMMON_TABLE                       0x000112FF



/**
   @fn   acdb_ioctl (ACDB_CMD_SET_GLBTBL_DATA, ...)
   @brief API to change individual calibration data in database.

   This command will allow the overriding global table calibration 
   data stored in the database.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_GLBTBL_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGblTblCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGblTblCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbGblTblCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_GLBDEFS_DATA                                     0x0001132A

/**
   @struct   AcdbGblTblCmdType
   @brief This is a query command structure allowing for individual get/set
          for calibration data in the Global cal tables.

   @param nModuleId: The Module ID
   @param nParamId: The Parameter ID of the Module to query for.
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the parameters identified by the
                    nParamId identifier.
   @param nBufferPointer: A virtual memory pointer pointing to a memory region
                          containing the payload (or receiving the payload) 
                          identified by nParamId.
  
   This is a query command structure allowing for multiple get/set of
   calibration data in the global calibration table. The format of the data is described
   in below.

   The return buffer formatted is determined by the parameter ID as one of key to access
   to ACDB.
   //Existing Pause/resume volume stepping structure is formatted as
   struct VolumeStepStruct {
      uint32_t nEnableFlag; //Enable flag
      uint32_t nPeriod; //duration
      uint32_t nStep;   //number of step
      uint32_t ramping_curve; //Ramping curve
   }
*/
typedef struct _AcdbGblDefsCmdType {
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbGblDefsCmdType;


/**
   @fn   acdb_ioctl (ACDB_CMD_GET_AUD_VOL_POPP_MODULE_LIST, ...)
   @brief API to change individual calibration data in database.

   This command will allow the overriding global table calibration 
   data stored in the database.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_GLBTBL_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGblTblCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGblTblCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbGblTblCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AUD_VOL_POPP_MODULE_LIST                           0x0001132B

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_AUDIO_RECORD_PAIR_LIST, ...)
   @brief API to query for a list of device pairs.

   This command will query for all possible device pairs.

   return structure will look like
   struct AudioRecordPairList {
      uint32_t nBytesCount;
      struct AudioRecordItem {
         uint32_t nDeviceID;
         uint32_t nAfeTopologyID;
      } AudioRecordPair[]; // AudioRecordPair size = nBytesCount/ sizeof(AudioRecordItem)
   };

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_AUDIO_RECORD_PAIR_LIST.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbGeneralInfoCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbGeneralInfoCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbGeneralInfoCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_SUCCESS: When the command executes without problem.
         ACDB_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_AUDIO_RECORD_PAIR_LIST                   0x00011385 

/**
   This paramId is to use for query Adie Sidetone table lookup index and 
   number of Adie Sidetone table entries

   Returned AdieSidetone table entries buffer structure format
   struct AdieSidetoneLookupIndex{
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
   }

   struct AdieSidetoneIndexList {
   AdieSidetoneLookupIndex [number of adie sidetone table entries];
   }
*/
#define ACDB_PID_ADIE_SIDETONE_TABLE                        0x00012A10

/* ---------------------------------------------------------------------------
 * ACDB_CMD_GET_ADIE_SIDETONE_DATA and ACDB_CMD_SET_AFE_DATA
 * Declarations and Documentation
 *--------------------------------------------------------------------------- */

/**
   @fn   acdb_ioctl (ACDB_CMD_GET_ADIE_SIDETONE_DATA, ...)
   @brief API to query for individual calibration data.

   This command will obtain the specific AdieSidetone calibration data 
   queried for.

   @param[in] nCommandId:
         The command id is ACDB_CMD_GET_ADIE_SIDETONE_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAdieSidetoneDataCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAdieSidetoneDataCmdType).
   @param[out] pResponseStruct:
         This should be a pointer to AcdbQueryResponseType.
   @param[in] nResponseStructLength:
         This should equal sizeof (AcdbQueryResponseType).

   @see  acdb_ioctl
   @see  AcdbAdieSidetoneDataCmdType
   @see  AcdbQueryResponseType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_GET_ADIE_SIDETONE_DATA                                     0x00012A11

/**
   @fn   acdb_ioctl (ACDB_CMD_SET_ADIE_SIDETONE_DATA, ...)
   @brief API to change individual calibration data in database.

   This command will allow the overriding of AdieSidetone calibration 
   data stored in the database.

   @param[in] nCommandId:
         The command id is ACDB_CMD_SET_ADIE_SIDETONE_DATA.
   @param[in] pCommandStruct:
         This should be a pointer to AcdbAdieSidetoneDataCmdType.
   @param[in] nCommandStructLength:
         This should equal sizeof (AcdbAdieSidetoneDataCmdType).
   @param[out] pResponseStruct:
         There is no output structure so this should be NULL.
   @param[in] nResponseStructLength:
         There is no output structure so this should be 0.

   @see  acdb_ioctl
   @see  AcdbAdieSidetoneDataCmdType

   @return  The return values of this function are:
         ACDB_RES_SUCCESS: When the command executes without problem.
         ACDB_RES_BADPARM: When any parameter is not as specified as above.
*/
#define ACDB_CMD_SET_ADIE_SIDETONE_DATA                                      0x00012A12

/**
   @struct   AcdbAdieSidetoneDataCmdType
   @brief This is a query command structure allowing for individual get/set
          for calibration data in the AdieSidetone data tables.

   @param nTxDeviceId: The Tx path Device ID
   @param nRxDeviceId: The Rx path Device ID
   @param nModuleId: The Module ID
   @param nParamId: The Parameter ID of the Module to query for.
   @param nBufferLength: The length of the calibration buffer. This should be
                    large enough to hold the parameters identified by the
                    nParamId identifier.
   @param nBufferPointer: A virtual memory pointer pointing to a memory region
                          containing the payload (or receiving the payload) 
                          identified by nParamId.
  
   This is a query command structure allowing for multiple get/set of
   calibration data in the AdieSidetone data tables. The format of the data is described
   in below.
      
   struct AdieSideToneStruct {
      uint16_t nEnableFlag;
      uint16_t npreGain;        --//regular integer format in dB  (max=40dB min=-84dB)
      uint32_t iir[5*5]			--//Q2.28 (including sign bit)   b0, b1, b2, a1, a2
   }
*/
typedef struct _AcdbAdieSidetoneDataCmdType {
   uint32_t nTxDeviceId;
   uint32_t nRxDeviceId;
   uint32_t nModuleId;
   uint32_t nParamId;
   uint32_t nBufferLength;
   uint8_t *nBufferPointer;
} AcdbAdieSidetoneDataCmdType;



/* ---------------------------------------------------------------------------
 * Function Declarations and Documentation
 *--------------------------------------------------------------------------- */

/*===========================================================================

FUNCTION acdb_ioctl

DESCRIPTION
   Main entry function to the ACDB. This entry function will take any
   supported ACDB IOCTL and provide the appropriate response.

   @param[in] nCommandId
         Command id to execute on the Audio Calibration Database. The
         pCommandStruct and the pResponseStruct should match the expected
         structures for that command. If not, the command will fail.
   @param[in] pCommandStruct
         Pointer to the command structure.
   @param[in] nCommandStructLength
         The size of command structure.
   @param[out] pResponseStruct
         Pointer to the response structure.
   @param[in] nResponseStructLength
         The size of the response structure.

   Supported commands are:
      ACDB_CMD_GET_DEVICE_PAIR_LIST
      ACDB_CMD_GET_DATE_INFO
      ACDB_CMD_GET_OEM_INFO
      ACDB_CMD_GET_ANC_DEVICE_PAIR_LIST
      ACDB_CMD_GET_LOOKUP_TABLE_SIZE
      ACDB_CMD_GET_TABLE_INDEX_COMBINATION
      ACDB_CMD_GET_AUDPROC_COMMON_TOPOLOGY_ID_LIST
      ACDB_CMD_GET_AUDPROC_STREAM_TOPOLOGY_ID_LIST
      ACDB_CMD_GET_VOCPROC_COMMON_TOPOLOGY_ID_LIST
      ACDB_CMD_GET_QACT_INFO

   Please see individual command documentation for more details on the
   expectations of the command.

DEPENDENCIES
   None

RETURN VALUE
   The result of the call as defined by the command.

SIDE EFFECTS
   None.

===========================================================================*/

#endif /* __ACDB_PRIVATE_H__ */


