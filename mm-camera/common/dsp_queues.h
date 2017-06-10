#ifndef DSP_QUEUES_H
#define DSP_QUEUES_H

#if AMSS_VERSION==6350

#define QDSP_lpmCommandQueue              0
#define QDSP_mpuAfeQueue                  1
#define QDSP_mpuGraphicsCmdQueue          2
#define QDSP_mpuModmathCmdQueue           3
#define QDSP_mpuVDecCmdQueue              4
#define QDSP_mpuVDecPktQueue              5
#define QDSP_mpuVEncCmdQueue              6
#define QDSP_rxMpuDecCmdQueue             7
#define QDSP_rxMpuDecPktQueue             8
#define QDSP_txMpuEncQueue                9
#define QDSP_uPAudPPCmd1Queue             10
#define QDSP_uPAudPPCmd2Queue             11
#define QDSP_uPAudPPCmd3Queue             12
#define QDSP_uPAudPlay0BitStreamCtrlQueue 13
#define QDSP_uPAudPlay1BitStreamCtrlQueue 14
#define QDSP_uPAudPlay2BitStreamCtrlQueue 15
#define QDSP_uPAudPlay3BitStreamCtrlQueue 16
#define QDSP_uPAudPlay4BitStreamCtrlQueue 17
#define QDSP_uPAudPreProcCmdQueue         18
#define QDSP_uPAudRecBitStreamQueue       19
#define QDSP_uPAudRecCmdQueue             20
#define QDSP_uPDiagQueue                  21
#define QDSP_uPJpegActionCmdQueue         22
#define QDSP_uPJpegCfgCmdQueue            23
#define QDSP_uPVocProcQueue               24
#define QDSP_vfeCommandQueue              25
#define QDSP_vfeCommandScaleQueue         26
#define QDSP_vfeCommandTableQueue         27
#define QDSP_MAX_NUM_QUEUES               28

#elif AMSS_VERSION==6056 || AMSS_VERSION==6066 || AMSS_VERSION==6074 || AMSS_VERSION==6220 || AMSS_VERSION==6225

#define QDSP_lpmCommandQueue              0
#define QDSP_mpuAfeQueue                  1
#define QDSP_mpuGraphicsCmdQueue          2
#define QDSP_mpuModmathCmdQueue           3
#define QDSP_mpuVDecCmdQueue              4
#define QDSP_mpuVDecPktQueue              5
#define QDSP_mpuVEncCmdQueue              6
#define QDSP_rxMpuDecCmdQueue             7
#define QDSP_rxMpuDecPktQueue             8
#define QDSP_txMpuEncQueue                9
#define QDSP_uPAudPPCmd1Queue             10
#define QDSP_uPAudPPCmd2Queue             11
#define QDSP_uPAudPPCmd3Queue             12
#define QDSP_uPAudPlay0BitStreamCtrlQueue 13
#define QDSP_uPAudPlay1BitStreamCtrlQueue 14
#define QDSP_uPAudPlay2BitStreamCtrlQueue 15
#define QDSP_uPAudPlay3BitStreamCtrlQueue 16
#define QDSP_uPAudPlay4BitStreamCtrlQueue 17
#define QDSP_uPAudPreProcCmdQueue         18
#define QDSP_uPAudRecBitStreamQueue       19
#define QDSP_uPAudRecCmdQueue             20
#define QDSP_uPJpegActionCmdQueue         21
#define QDSP_uPJpegCfgCmdQueue            22
#define QDSP_uPVocProcQueue               23
#define QDSP_vfeCommandQueue              24
#define QDSP_vfeCommandScaleQueue         25
#define QDSP_vfeCommandTableQueue         26

#elif AMSS_VERSION==6046

#define QDSP_lpmCommandQueue              0
#define QDSP_mpuAfeQueue                  1
#define QDSP_mpuGraphicsCmdQueue          2
#define QDSP_mpuModmathCmdQueue           3
#define QDSP_mpuVDecCmdQueue              4
#define QDSP_mpuVDecPktQueue              5
#define QDSP_mpuVEncCmdQueue              6
#define QDSP_rxMpuDecCmdQueue             7
#define QDSP_rxMpuDecPktQueue             8
#define QDSP_txMpuEncQueue                9
#define QDSP_uPAudPPCmd1Queue             10
#define QDSP_uPAudPPCmd2Queue             11
#define QDSP_uPAudPPCmd3Queue             12
#define QDSP_uPAudPlay0BitStreamCtrlQueue 13
#define QDSP_uPAudPlay1BitStreamCtrlQueue 14
#define QDSP_uPAudPlay2BitStreamCtrlQueue 15
#define QDSP_uPAudPlay3BitStreamCtrlQueue 16
#define QDSP_uPAudPlay4BitStreamCtrlQueue 17
#define QDSP_uPAudRecBitStreamQueue       18
#define QDSP_uPAudRecCmdQueue             19
#define QDSP_uPJpegActionCmdQueue         20
#define QDSP_uPJpegCfgCmdQueue            21
#define QDSP_uPVocProcQueue               22
#define QDSP_vfeCommandQueue              23
#define QDSP_vfeCommandScaleQueue         24
#define QDSP_vfeCommandTableQueue         25

#else

#error "Unknown value of AMSS_VERSION!"

#endif

#endif/*DSP_QUEUES_H*/
