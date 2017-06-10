#ifndef _MMI_FILEMUX_EXTENSIONS_H_
#define _MMI_FILEMUX_EXTENSIONS_H_

/*==============================================================================
*        @file omx_filemux_extensions.h
*
*  @par DESCRIPTION:
*       This file defines all extensions to openmax used in filemux.
*
*
*  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

 $Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/omx/mm-mux/omxmux/src/omx_filemux_extensions.h#1 $



================================================================================
*/


#include "QOMX_AudioExtensions.h"



/**-----------------------------------------------------------------------------
Temporarily add the EVRC extns here. need to move appr. place once decided.
--------------------------------------------------------------------------------
*/



#define OMX_QCOM_INDEX_PARAM_EVRCB        "OMX.Qualcomm.index.audio.evrcb"
#define OMX_QCOM_INDEX_PARAM_EVRCWB       "OMX.Qualcomm.index.audio.evrcwb"
#define OMX_QCOM_INDEX_PARAM_AMRWBPLUS    "OMX.Qualcomm.index.audio.amrwbplus"
#define OMX_QCOM_INDEX_PARAM_VID_SYNTXHDR "OMX.QCOM.index.param.video.SyntaxHdr"
#define OMX_QCOM_INDEX_CONFIG_MEDIA_INFO  "OMX.QCOM.index.config.mediainfo"
#define OMX_QCOM_INDEX_PARM_CONTENTINTERFACE_ISTREAMPORT "OMX.QCOM.index.param.contentinterface.istreamport"
#define OMX_QCOM_INDEX_CONFIG_RECORDINGSTATISTICS_INTERVAL "OMX.Qualcomm.index.config.RecordingStatisticsInterval"
#define OMX_QCOM_INDEX_CONFIG_RECORDINGSTATISTICS_STATUS   "OMX.Qualcomm.index.config.RecordingStatisticsStatus"
#define OMX_QCOM_INDEX_CONFIG_ENCRYPT_TYPE   "OMX.Qualcomm.index.config.EncryptType"

#define QOMX_FILEMUX_INDEX_BASE              0x7F1FF000



#define QOMX_FilemuxIndexParamAudioEvrcb     QOMX_FILEMUX_INDEX_BASE + 1
                                       /**< "OMX.Qualcomm.index.audio.evrcb"  */
#define QOMX_FilemuxIndexParamAudioEvrcwb    QOMX_FILEMUX_INDEX_BASE + 2
                                       /**< "OMX.Qualcomm.index.audio.evrcwb" */
#define QOMX_FilemuxIndexParamAudioAmrWbPlus QOMX_FILEMUX_INDEX_BASE + 3
                                    /**< "OMX.Qualcomm.index.audio.amrwbplus" */
#define QOMX_FilemuxIndexParamVideoSyntaxHdr QOMX_FILEMUX_INDEX_BASE + 4
                                  /**< "OMX.QCOM.index.param.video.SyntaxHdr" */
#define QOMX_FilemuxIndexConfigMediaInfo     QOMX_FILEMUX_INDEX_BASE + 5
                                       /**< "OMX.QCOM.index.config.mediainfo" */
#define QOMX_FilemuxIndexParamIStreamPort    QOMX_FILEMUX_INDEX_BASE + 6
                                      /**< "OMX.QCOM.index.param.contentinterface.ixstream"*/

#define QOMX_FilemuxIndexConfigRecordingStatisticsInterval \
                        QOMX_FILEMUX_INDEX_BASE + 7
                        /**<OMX.Qualcomm.index.config.RecordingStatisticsInterval*/

#define QOMX_FilemuxIndexConfigRecordingStatisticsStatus  \
                       QOMX_FILEMUX_INDEX_BASE + 8
                       /**<OMX.Qualcomm.index.config.RecordingStatisticsStatus*/


#define QOMX_FilemuxIndexParamEncryptType    QOMX_FILEMUX_INDEX_BASE + 9
                                      /**< "OMX.QCOM.index.param.contentinterface.ixstream"*/

#define QOMX_FilemuxIndexParamContainerInfo  0x7F000009


#ifndef QOMX_ErrorStorageLimitReached

/* In some scenarios there may be a possibilty to run out of the storage space
 * and components may want to notify this error to IL client to take appropriate
 * action by the IL client.
 *
 * For example, In recording scenario, MUX component can know the available
 * space in the recording media and can compute peridically to accommodate the
 * meta data before we reach to a stage where we end up no space to write even
 * the meta data. When the space limit reached in recording media, MUX component
 * would like to notify the IL client with  QOMX_ErrorSpaceLimitReached.
 * After this error all the buffers that are returned will have nFilledLen
 * unchanges i.e not consumed.
 */
#define QOMX_ErrorStorageLimitReached (OMX_ErrorVendorStartUnused + 2)

#endif

#endif //_MMI_FILEMUX_EXTENSIONS_H_
