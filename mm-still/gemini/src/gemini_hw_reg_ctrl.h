
/* Copyright (c) 2010, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef GEMINI_HW_REG_CTRL_H
#define GEMINI_HW_REG_CTRL_H

#define JPEG_BUS_CMD_HALT_REQ                               0x00000001

#define JPEG_REALTIME_CMD_STOP_FB                           0x00000000
#define JPEG_REALTIME_CMD_STOP_IM                           0x00000003
#define JPEG_REALTIME_CMD_START                             0x00000001

#define JPEG_OFFLINE_CMD_START                              0x00000003

#define JPEG_DMI_CFG_DISABLE                                0x00000000
#define JPEG_DMI_CFG_QUANTIZATION                           0x00000005
#define JPEG_DMI_CFG_HUFFMAN                                0x00000006
#define JPEG_DMI_ADDR_START                                 0x00000000
#define JPEG_DMI_DATA_MAX                                   0x0000ffff

#define JPEG_FE_CMD_BUFFERRELOAD                            0x00000001

#define JPEG_WE_YUB_ENCODE                                  0x01ff0000
#define JPEG_WE_CMD_RESET                                   0x00000000
#define JPEG_WE_CMD_BUFFERRELOAD                            0x00000001
#define JPEG_WE_CFG_AREQP                                   0x00000002

#define JPEG_CFG_DEFAULT                                    0x000061fb	/*  0x000061e9 */

#define JPEG_RESET_DEFAULT                                  0x000cffff

#define JPEG_IRQ_DISABLE_ALL                                0x00000000
#define JPEG_IRQ_CLEAR_ALL                                  0xffffffff
#define JPEG_IRQ_ALLSOURCES_ENABLE                          0xffffffff

#define JPEG_IRQ_STATUS_FRAMEDONE_MASK                      0x00000001
#define JPEG_IRQ_STATUS_FRAMEDONE_SHIFT                     0x00000000

#define JPEG_IRQ_STATUS_FE_RD_DONE_MASK                     0x00000002
#define JPEG_IRQ_STATUS_FE_RD_DONE_SHIFT                    0x00000001

#define JPEG_IRQ_STATUS_FE_RTOVF_MASK                       0x00000004
#define JPEG_IRQ_STATUS_FE_RTOVF_SHIFT                      0x00000002

#define JPEG_IRQ_STATUS_FE_VFE_OVERFLOW_MASK                0x00000008
#define JPEG_IRQ_STATUS_FE_VFE_OVERFLOW_SHIFT               0x00000003

#define JPEG_IRQ_STATUS_WE_Y_PINGPONG_MASK                  0x00000010
#define JPEG_IRQ_STATUS_WE_Y_PINGPONG_SHIFT                 0x00000004

#define JPEG_IRQ_STATUS_WE_CBCR_PINGPONG_MASK               0x00000020
#define JPEG_IRQ_STATUS_WE_CBCR_PINGPONG_SHIFT              0x00000005

#define JPEG_IRQ_STATUS_WE_Y_BUFFER_OVERFLOW_MASK           0x00000040
#define JPEG_IRQ_STATUS_WE_Y_BUFFER_OVERFLOW_SHIFT          0x00000006

#define JPEG_IRQ_STATUS_WE_CBCR_BUFFER_OVERFLOW_MASK        0x00000080
#define JPEG_IRQ_STATUS_WE_CBCR_BUFFER_OVERFLOW_SHIFT       0x00000007

#define JPEG_IRQ_STATUS_WE_CH0_DATAFIFO_OVERFLOW_MASK       0x00000100
#define JPEG_IRQ_STATUS_WE_CH0_DATAFIFO_OVERFLOW_SHIFT      0x00000008

#define JPEG_IRQ_STATUS_WE_CH1_DATAFIFO_OVERFLOW_MASK       0x00000200
#define JPEG_IRQ_STATUS_WE_CH1_DATAFIFO_OVERFLOW_SHIFT      0x00000009

#define JPEG_IRQ_STATUS_RESET_ACK_MASK                      0x00000400
#define JPEG_IRQ_STATUS_RESET_ACK_SHIFT                     0x0000000a

#define JPEG_IRQ_STATUS_BUS_ERROR_MASK                      0x00000800
#define JPEG_IRQ_STATUS_BUS_ERROR_SHIFT                     0x0000000b

#define JPEG_IRQ_STATUS_VIOLATION_MASK                      0x00001000
#define JPEG_IRQ_STATUS_VIOLATION_SHIFT                     0x0000000c

#define JPEG_STATUS_BUSY_LOOP_MAXTRY                        0x0000ffff
#define JPEG_CONFIG_SINGLE_BUFFER                           1

typedef enum IGEMINI_FE_BURSTLENGTH
{
	IGEMINI_FE_BURST4 = 0,
	IGEMINI_FE_BURST8,
	IGEMINI_FE_BURST_MAX
} IGEMINI_FE_BURSTLENGTH;

typedef enum IGEMINI_FE_MASKREGS
{
	IGEMINI_FE_BURSTMASK = 0,
	IGEMINI_FE_YWRITEMASK0,
	IGEMINI_FE_YWRITEMASK1,
	IGEMINI_FE_YWRITEMASK2,
	IGEMINI_FE_YWRITEMASK3,
	IGEMINI_FE_CBCRWRITEMASK0,
	IGEMINI_FE_CBCRWRITEMASK1,
	IGEMINI_FE_CBCRWRITEMASK2,
	IGEMINI_FE_CBCRWRITEMASK3,
	IGEMINI_FE_MASKREGS_MAX
} IGEMINI_FE_MASKREGS;

typedef enum GEMINI_OPERATION_MODE
{
	GEMINI_REALTIME_ENCODE = 0,	       /**< Does not support rotation. */
	GEMINI_OFFLINE_ENCODE = 1,	    /**< Default mode.
                                                 \n Supports rotation, but does not support fragmented inputs. */
	GEMINI_REALTIME_ROTATION = 2,
	GEMINI_OFFLINE_ROTATION = 3,
	GEMINI_OPERATION_MODE_MAX
} GEMINI_OPERATION_MODE;

#endif /*  GEMINI_HW_REG_CTRL_H */
