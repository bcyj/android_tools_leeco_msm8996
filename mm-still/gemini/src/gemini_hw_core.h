
/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef GEMINI_HW_CORE_H
#define GEMINI_HW_CORE_H

#include "gemini_lib_common.h"

/*  according to Gemini HDD */
/*  Gemini core clock can run up to 160 MHz for Halcyon */
/*  to meet the requirement of 12MP@10fps */
/*  For BB/PT, it is 200MHz for 12MP@15fps */
#define GEMINI_JPEG_FREQUENCY_MAX           160

/*  queue length is set to 16 */
#define GEMINI_NUM_QUEUELENGTH              16

/*  index to indicate ping or pong buffer */
#define GEMINI_PING_BUFFER_INDEX            0
#define GEMINI_PONG_BUFFER_INDEX            1

/*  max number of MCUs for frame width and height */
/*  ref SWI section 2.1.1.5 */
#define GEMINI_FRAME_WIDTH_MCUS_MAX         512
#define GEMINI_FRAME_HEIGHT_MCUS_MAX        512
#define GEMINI_FRAME_WIDTH_MCUS_MIN         1
#define GEMINI_FRAME_HEIGHT_MCUS_MIN        1

/*  #define GEMINI_FE_BURST_LENGTH4             1 */
/*  #define GEMINI_FE_BURST_LENGTH8             2 */

#define GEMINI_FE_CRCB_ORDER                0
#define GEMINI_FE_CBCR_ORDER                1

/*  #define GEMINI_WE_BURST_LENGTH4             1 */
/*  #define GEMINI_WE_BURST_LENGTH8             2 */

#define GEMINI_BYTEORDERING_MIN             0
#define GEMINI_BYTEORDERING_MAX             7

#define GEMINI_DC_COMPONENT_INDEX           0
#define GEMINI_AC_COMPONENT_INDEX           1

#define GEMINI_NUM_QUANTIZATION_ENTRIES     64
#define GEMINI_NUM_HUFFMANDC_ENTRIES        12
#define GEMINI_NUM_HUFFMANAC_ENTRIES        176

typedef struct
{
	uint16_t codeLength;
	uint16_t codeWord;
} gemini_huff_lookup_table_type;

typedef struct
{
	uint8_t nUseMode;
	uint8_t nInputFormat;
	uint8_t nWEInputSel;
	uint8_t nJPEGInputSel;
	uint8_t nFEInputSel;
	uint8_t nVFEEnable;
	uint8_t nFSCEnable;
        uint8_t nImemFifoModeDisable;
} gemini_encode_pipeline_cfg_type;

typedef struct
{
	GEMINI_INPUT_FORMAT eInputFormat;
	uint32_t nReadBurstLength;
	uint32_t nFrameWidthMCUs;
	uint32_t nFrameHeightMCUs;
} gemini_fe_operation_cfg_type;

typedef struct
{
	BOOL bFrameDoneIrq;
	BOOL bFeReadDoneIrq;
	BOOL bFeRealtimeOvfIrq;
	BOOL bFeVfeOvfIrq;
	BOOL bWeYPingpongIrq;
	BOOL bWeCbcrPingpongIrq;
	BOOL bWeYBufferOvfIrq;
	BOOL bWeCbcrBufferOvfIrq;
	BOOL bWeCh0DataFifoOvfIrq;
	BOOL bWeCh1DataFifoOvfIrq;
	BOOL bResetAckIrq;
	BOOL bBusErrorIrq;
	BOOL bViolationIrq;
} gemini_interrupt_status_type;

typedef struct
{
	uint32_t nBufferAddr[GEMINI_NUM_QUEUELENGTH];	/*  bitstream buffer addresses circular queue */
	uint32_t nBufferLen[GEMINI_NUM_QUEUELENGTH];	/*  bitstream buffer length circular queue */
	uint8_t nPingpongIndex;	/*  ping or pong index to update the hardware */
	uint8_t nQHead;		/*  index of first valid buffer in queue */
	uint8_t nQTail;		/*  index of first valid empty buffer in queue */
	BOOL bFirstCfg;		/*  first write engine pp buffer configuration */
} gemini_we_pingpong_cfg_type;

typedef struct
{
	uint32_t nYBufferAddr[GEMINI_NUM_QUEUELENGTH];	/*  Y buffer addresses circular queue */
	uint32_t nCbcrBufferAddr[GEMINI_NUM_QUEUELENGTH];	/*  Cbcr buffer addresses circular queue */
	uint32_t nNumOfMCURows[GEMINI_NUM_QUEUELENGTH];	/*  MCU rows for each buffer enqueued */
	uint8_t nPingpongIndex;	/*  ping or pong index to update the hardware */
	uint8_t nQHead;		/*  index of first valid buffer in queue */
	uint8_t nQTail;		/*  index of first valid empty buffer in queue */
	BOOL bFirstCfg;		/*  first fetch engine pp buffer configuration */
} gemini_fe_pingpong_cfg_type;

#endif /*  GEMINI_HW_CORE_H */
