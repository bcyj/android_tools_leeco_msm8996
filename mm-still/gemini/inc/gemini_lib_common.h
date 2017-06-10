
/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef GEMINI_HW_COMMON_H
#define GEMINI_HW_COMMON_H

#include <unistd.h>
#include <media/msm_gemini.h>

#define BOOL uint8_t

enum GEMINI_BURST_LENGTH_T
{
	GEMINI_BURST_LENGTH4 = 1,
	GEMINI_BURST_LENGTH8 = 2,
};


/*****************************************************************************************************/
/************************  JPEG Configuration ********************************************************/
/*****************************************************************************************************/

typedef struct
{
	uint32_t regionSize;		    /**< The number of MCU rows for each region. If there are N MCU rows per region,
                                               this register should be programmed as N-1. This implies that there is a minimum
                                               setting on 1 MCU row per region.*/
	uint8_t region0Budget;		    /**< Specifies the bit budget for region 0.   */
	uint8_t region1Budget;		    /**< Specifies the bit budget for region 1.   */
	uint8_t region2Budget;		    /**< Specifies the bit budget for region 2.   */
	uint8_t region3Budget;		    /**< Specifies the bit budget for region 3.   */
	uint8_t region4Budget;		    /**< Specifies the bit budget for region 4.   */
	uint8_t region5Budget;		    /**< Specifies the bit budget for region 5.   */
	uint8_t region6Budget;		    /**< Specifies the bit budget for region 6.   */
	uint8_t region7Budget;		    /**< Specifies the bit budget for region 7.   */
	uint8_t region8Budget;		    /**< Specifies the bit budget for region 8.   */
	uint8_t region9Budget;		    /**< Specifies the bit budget for region 9.   */
	uint8_t region10Budget;		    /**< Specifies the bit budget for region 10.  */
	uint8_t region11Budget;		    /**< Specifies the bit budget for region 11.  */
	uint8_t region12Budget;		    /**< Specifies the bit budget for region 12.  */
	uint8_t region13Budget;		    /**< Specifies the bit budget for region 13.  */
	uint8_t region14Budget;		    /**< Specifies the bit budget for region 14.  */
	uint8_t region15Budget;		    /**< Specifies the bit budget for region 15.  */
} gemini_cmds_jpeg_fsc_cfg;

/** Driver is to derive the table, and then send down to the
 *  hardware. */
typedef struct
{
	uint8_t huffcount[16];
	uint8_t huffval[162];
} gemini_huffmanAcTable;

typedef struct
{
	uint8_t huffcount[16];
	uint8_t huffval[12];
} gemini_huffmanDcTable;

/** Expect table in Q, driver is to do 1/Q, then program the
 *  hardware. */
typedef struct
{
	uint8_t qtbl[64];
} gemini_quantTable;

typedef struct
{
#if 0
	BOOL huffmanTblEnable;			/**< 0 = disabled.
                                                    \n 1= enabled. */
	BOOL statsEnable;			/**< 0 = disabled.
                                                      \n 1= enabled.  (Upper layer SW to program a value 1) */
	BOOL zigzagEnable;			/**< 0 = disabled.
                                                      \n 1= enabled.  (Upper layer SW to program a value 1)*/
	BOOL RLEEnable;				/**< 0 = disabled.
                                                      \n 1= enabled.  (Upper layer SW to program a value 1)*/
	BOOL dctEnable;				/**< 0 = disabled.
                                                      \n 1= enabled.  (Upper layer SW to program a value 1)*/
	BOOL quantizerEnable;			/**< 0 = disabled.
                                                      \n 1= enabled.  (SW to program a value 1)*/
#endif

	uint16_t restartInterval;		     /**< Number of MCU between restart marker insertion.
                                                    0 = disabled. A value of N MCU is programmed as N*/
	BOOL bCustomHuffmanTbl;
						    /**<  This field is only valid when huffmanTblEnable = 1.
                                                    \n 1 = customized Huffman table for JPEG processing.
                                                    \n 0 = default Huffman table for JPEG processing. */
	gemini_huffmanDcTable *huffmanTblYDcPtr;      /**< Only valid when customized huffman table is selected.  Pointer to y-dc bits/val table. */
	gemini_huffmanAcTable *huffmanTblYAcPtr;      /**< Only valid when customized huffman table is selected.  Pointer to y-ac bits/val table. */
	gemini_huffmanDcTable *huffmanTblCbcrDcPtr;   /**< Only valid when customized huffman table is selected.  Pointer to cbcr-dc bits/val table. */
	gemini_huffmanAcTable *huffmanTblCbcrAcPtr;   /**< Only valid when customized huffman table is selected.  Pointer to cbcr-ac bits/val table. */

	gemini_quantTable *quantTblY;		    /**< Q table for Y, 64 entries. Gemini Driver to convert to 1/Q,
                                                        then program the hardware.  */
	gemini_quantTable *quantTblChroma;	    /**< Q table for chroma, 64 entries. Gemini Driver to convert to 1/Q,
                                                        then program the hardware.  */
	BOOL bFSCEnable;			 /**< 0 = disabled.  (default)
                                                      \n 1= enabled. */
	gemini_cmds_jpeg_fsc_cfg sFileSizeControl;  /**< Valid when file size control is enabled.    */

} gemini_cmd_jpeg_encode_cfg;

/*****************************************************************************************************/
/************************  Fetch Engine and Write Engine *********************************************/
/*****************************************************************************************************/
typedef enum GEMINI_INPUT_FORMAT
{
	GEMINI_INPUT_INVALID = 0,
	GEMINI_INPUT_H1V2 = 1,	       /**< only valid for offline encode mode. */
	GEMINI_INPUT_H2V1 = 2,
	GEMINI_INPUT_H2V2 = 3,
	GEMINI_INPUT_FORMAT_MAX
} GEMINI_INPUT_FORMAT;

typedef struct
{
	GEMINI_INPUT_FORMAT inputFormat;
				   /**< Use this information to derive others fields. */
	uint8_t input_cbcr_order;
				   /**< 0 = cb pixel in LSB of the word.
                                     \n 1 = cr pixel in LSB of the word.  */
	uint8_t fe_burst_length;
				   /**< 0x1 = burst length 4,  0x2 = burst length 8. */
	uint8_t byte_ordering;
				   /**< Given 64 bit word, byte 7,6,5,4,3,2,1,0,  here are the options:
                                        \n  bit 0: byte level swapping.
                                        \n  bit 1: 16 bit level swapping.
                                        \n  bit 2: 32 bit level swapping.
                                        \n Therefore:
                                        \n  b'000   7,6,5,4,3,2,1,0
                                        \n  b'001   6,7,4,5,2,3,0,1
                                        \n  b'010   5,4,7,6,1,0,3,2
                                        \n  b'011   4,5,6,7,0,1,2,3
                                        \n  b'100   3,2,1,0,7,6,5,4
                                        \n  b'101   2,3,0,1,6,7,4,5
                                        \n  b'110   1,0,3,2,5,4,7,6
                                        \n  b'111   0,1,2,3,4,5,6,7
                                   */
	uint32_t frame_height_mcus;
				   /**<  Total number of MCU vertical.   */
	uint32_t frame_width_mcus;
				   /**<  Total number of MCU horizontal. */
} gemini_cmd_input_cfg;

typedef struct
{
	uint8_t we_burst_length;	   /**< 0x1 = burst length 4,  0x2 = burst length 8. */

	uint8_t byte_ordering;		   /**< Given 64 bit word, byte 7,6,5,4,3,2,1,0,  here are the options:
                                           \n  bit 0: byte level swapping.
                                           \n  bit 1: 16 bit level swapping.
                                           \n  bit 2: 32 bit level swapping.
                                           \n Therefore:
                                           \n  b'000   7,6,5,4,3,2,1,0
                                           \n  b'001   6,7,4,5,2,3,0,1
                                           \n  b'010   5,4,7,6,1,0,3,2
                                           \n  b'011   4,5,6,7,0,1,2,3
                                           \n  b'100   3,2,1,0,7,6,5,4
                                           \n  b'101   2,3,0,1,6,7,4,5
                                           \n  b'110   1,0,3,2,5,4,7,6
                                           \n  b'111   0,1,2,3,4,5,6,7
                                            */
} gemini_cmd_output_cfg;

typedef enum GEMINI_ROTATE_TYPE
{
	GEMINI_ROTATE_NONE = 0,
	GEMINI_ROTATE_90,		    /**< only valid for offline encode mode. */
	GEMINI_ROTATE_180,
	GEMINI_ROTATE_270,
	GEMINI_ROTATE_MAX
} GEMINI_ROTATE_TYPE;

/**
 * Should we seperate this command into 3 commands?
 */
typedef struct
{
	uint8_t useMode;
					      /**< MSM_GEMINI_CTRL_CMD_RESET_OFFLINE_ENCODE  */
					      /**< MSM_GEMINI_CTRL_CMD_RESET_REALTIME_ENCODE */
					      /**< MSM_GEMINI_CTRL_CMD_RESET_ROTATION */
	GEMINI_ROTATE_TYPE rotationDegree;
					      /**<  Driver to determine if FE or WE engine does rotation.
                                                \n  1.  When it is offline JPEG mode, FE does rotation.
                                                \n  2.  When it is real time rotation mode, WE does rotation.
                                                \n  3.  Does not support real time JPEG with rotation.
                                                 */
	enum msm_gmn_out_mode outputMode;     /** flag to indicate whether the fragmentation needs
	                                          to be done by the client or driver */
} gemini_cmd_operation_cfg;

/*****************************************************************************************************/
/*********************************** Gemini Messages *************************************************/
/*****************************************************************************************************/

typedef enum
{
	GEMINI_EVT_FRAMEDONE                 = MSM_GEMINI_EVT_FRAMEDONE,
	/* GEMINI_EVT_FE_RD_DONE, */
	GEMINI_EVT_FE_REALTIME_OVERFLOW      = MSM_GEMINI_EVT_ERR,
	GEMINI_EVT_FE_VFE_OVERFLOW           = MSM_GEMINI_EVT_ERR,
	/* GEMINI_EVT_WE_Y_PINGPONG, */
	/* GEMINI_EVT_WE_CBCR_PINGPONG, */
	GEMINI_EVT_WE_Y_BUFFER_OVERFLOW      = MSM_GEMINI_EVT_ERR,
	GEMINI_EVT_WE_CBCR_BUFFER_OVERFLOW   = MSM_GEMINI_EVT_ERR,
	GEMINI_EVT_WE_CH0_DATA_FIFO_OVERFLOW = MSM_GEMINI_EVT_ERR,
	GEMINI_EVT_WE_CH1_DATA_FIFO_OVERFLOW = MSM_GEMINI_EVT_ERR,
	/* GEMINI_EVT_RESET_ACK, */
	GEMINI_EVT_BUS_ERROR                 = MSM_GEMINI_EVT_ERR,
	GEMINI_EVT_VIOLATION                 = MSM_GEMINI_EVT_ERR,
	GEMINI_EVT_RESET                     = MSM_GEMINI_EVT_RESET,
	GEMINI_EVT_MAX
} GEMINI_EVT_ID;

#if 0
/*****************************************************************************************************/
/***********************************  Input buffer enqueue********************************************/
/*****************************************************************************************************/
/**
 * Driver to program the corresponding 'fragment dimension' as well, in this case, it is one MCU row.
 */
typedef struct
{
	uint32_t pingY;
	uint32_t pongY;
	uint32_t pingCbcr;
	uint32_t pongCbcr;
} gemini_cmd_set_imem_addr;

/**
 * The buffers must worth the same number of MCU rows for Y and
 * cbcr.  Upper layer must have the size correctly set based
 * upon image format.
 */
typedef struct
{
	uint8_t frameIndex;	      /**< Specifies which frame this fragment belongs to.
                                     A note to current design:  this field will always = 0.
                                     This is because in offline mode, the upper layer SW itself is limited
                                     to encode one frame at a time. However the driver shall not limit the
                                     hardware capability at API. This field is reserved for future where
                                     the input is not limited to one buffer at a time. */
	uint32_t yBufferAddr;
	uint32_t cbcrBufferAddr;
	uint32_t numberOfMCURows;
} gemini_cmds_input_buf;

/**
 * This command is used for offline JPEG encoder.  The image may
 * be fragmented.  Driver simply schedules the hardware to read
 * the data, if there are addresses available to the driver.
 * Each fragment could have different size, in terms of number
 * of MCU rows.
 *
 * \n Note that the hardware actually does not support the
 * automatic ping/pong buffer switch.  It will generate the
 * rd_irq once ping buffer is read.  Driver is then to check if
 * the buffer queue has more addresses, then schedule another
 * read as needed.
 *
 * \n Note that hardware does not support offline JPEG encoder
 * with rotation, if the input image is fragmented!!!  It is the
 * responsibility of the upper layer SW to make sure that the
 * input image for offline JPEG with rotation must NOT be
 * fragmented.
 *
 * \n Driver is to maintain the buffer queue for offline
 * processing.  Queue length is now set at 16, that is 16
 * addresses pairs total. Upper layer can send this command
 * multiple times, as needed.
 */
typedef struct
{
	uint8_t numberOfInputBuffers;
	gemini_cmds_input_buf *inputPtr;
} gemini_cmd_input_buf_enqueue;

/*****************************************************************************************************/
/*********************************** Output buffer enqueue********************************************/
/*****************************************************************************************************/

/*********************************** Output buffer enqueue for JPEG encode modes. *******************/
typedef struct
{
	uint32_t bufferAddr;	     /**<  Must be 64 bits aligned.  (Multiple of 8 bytes.)       */
	uint32_t bufferLength;	     /**<  Buffer length is in number of bytes.                   */
} gemini_cmds_jpeg_buf;
/**
* \n An important note:  The frame done irq won't trigger the
* hardware to do ping/pong switch automatically.
*
* \n Driver to do WE ping/pong reload once the encode session is
* done. In between the frames within one encode session, driver
* does not do ping/pong reload.
*
* \n Driver does not limit the API to encode one frame only. In
* fact, it supports to encode multiple frames.
*/

typedef struct
{
	uint8_t numberOfBuffers;		/**< number of valid buffers in the array, for output bit stream. */
	gemini_cmds_jpeg_buf *outputPtr;      /**< It is the responsibility of upper layer SW to make sure
                                                   that the array contains N number of buffers specifies in the field above */
} gemini_cmd_jpeg_output_buf_enqueue;

/*********************************** Output buffer enqueue for real-time rotation mode. *******************/
#define GEMINI_MAX_NUM_FRAGMENTS_PER_FRAME 4
typedef struct
{
	uint32_t yBuf[GEMINI_MAX_NUM_FRAGMENTS_PER_FRAME];   /**<  Up to 4 fragments. */
	uint32_t cbcrBuf[GEMINI_MAX_NUM_FRAGMENTS_PER_FRAME];/**<  Up to 4 fragments. */
} gemini_cmds_rotated_yuv_buf;

/*
 * This command contains the output buffers for real time frame
 * rotation.  Upper layer SW to specify the number of the buffers
 * in the array.  Driver to put them into the queue.
 *
 * \n When in real-time frame rotation mode, VFE operates in
 * continuous mode.  It's the upper layer's responsibility to
 * supply output buffers in time.  If failed, the driver will
 * drop the current buffer. This scheme is the same as the view
 * finder use case.
 *
 * \n Driver to maintain an address queue up to 16 frames.
 */
typedef struct
{
	uint8_t numberOfFrames;
	gemini_cmds_rotated_yuv_buf *pBuf;
} gemini_cmd_rotated_yuv_buf_enqueue;

/**
 * This data structure is the payload of "GEMINI_MSG_JPEG_DONE".
 * When each frame is encoded, driver is to read out the encode
 * status out from the hardware register and report it to the
 * upper layer.
 */
typedef struct
{
	uint32_t frameCount;		/**< nth frame in one session.  */
	uint32_t jpegOutputSize;	/**< Includes all markers and EOI, in the unit of bytes. */
	uint32_t statsDcBitsY;
	uint32_t statsDcBitsCbcr;
	uint32_t statsAcBitsY;
	uint32_t statsAcBitsCbcr;
	uint32_t statsNonzeroAcY;
	uint32_t statsNonzeroAcCbcr;
} gemini_msg_jpeg_done;

/**
 * Driver returns this data structure to upper layer.
 */
typedef struct
{
	uint8_t hw_core_version;
	uint8_t hw_major_version;
	uint8_t hw_minor_version;
} gemini_cmd_hw_version;

/**
 * Encode command payload.  Used only for encode modes.
 */
typedef struct
{
	uint8_t numberOfFramesToEncode;	/**< Specifies for this encode session, the number of frames to encode.*/
} gemini_cmd_encode;

typedef struct
{
	uint32_t frameCount;		/**<  nth frame in one session.  */
	uint32_t yBufAddr;		/**<  Y buffer address  */
	uint32_t cbcrBufAddr;		/**<  cbcr buffer address  */
} gemini_msg_rotated_yuv_done;
#endif

#endif /*  GEMINI_HW_COMMON_H */
