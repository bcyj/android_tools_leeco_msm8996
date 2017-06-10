/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef JPEGE_HW_COMMON_H
#define JPEGE_HW_COMMON_H

#include <unistd.h>
#include <media/msm_jpeg.h>

#define BOOL uint8_t

enum GEMINI_FE_BURST_LENGTH_T
{
  GEMINI_FE_BURST_LENGTH4 = 1,
  GEMINI_FE_BURST_LENGTH8 = 2,
};

enum GEMINI_WE_BURST_LENGTH_T
{
  GEMINI_WE_BURST_LENGTH4 = 1,
  GEMINI_WE_BURST_LENGTH8 = 2,
};

typedef enum {
  JPEG_SPEED_NORMAL = 1,
  JPEG_SPEED_HIGH = 2
} JPEG_SPEED_MODE_T;

/*****************************************************************************************************/
/************************  JPEG Configuration ********************************************************/
/*****************************************************************************************************/

typedef struct
{
  uint32_t regionSize;        /**< The number of MCU rows for each region. If there are N MCU rows per region,
                                               this register should be programmed as N-1. This implies that there is a minimum
                                               setting on 1 MCU row per region.*/
  uint8_t region0Budget;        /**< Specifies the bit budget for region 0.   */
  uint8_t region1Budget;        /**< Specifies the bit budget for region 1.   */
  uint8_t region2Budget;        /**< Specifies the bit budget for region 2.   */
  uint8_t region3Budget;        /**< Specifies the bit budget for region 3.   */
  uint8_t region4Budget;        /**< Specifies the bit budget for region 4.   */
  uint8_t region5Budget;        /**< Specifies the bit budget for region 5.   */
  uint8_t region6Budget;        /**< Specifies the bit budget for region 6.   */
  uint8_t region7Budget;        /**< Specifies the bit budget for region 7.   */
  uint8_t region8Budget;        /**< Specifies the bit budget for region 8.   */
  uint8_t region9Budget;        /**< Specifies the bit budget for region 9.   */
  uint8_t region10Budget;        /**< Specifies the bit budget for region 10.  */
  uint8_t region11Budget;        /**< Specifies the bit budget for region 11.  */
  uint8_t region12Budget;        /**< Specifies the bit budget for region 12.  */
  uint8_t region13Budget;        /**< Specifies the bit budget for region 13.  */
  uint8_t region14Budget;        /**< Specifies the bit budget for region 14.  */
  uint8_t region15Budget;        /**< Specifies the bit budget for region 15.  */
} gemini_cmds_jpeg_fsc_cfg;

/** Driver is to derive the table, and then send down to the
 *  hardware. */
typedef struct
{
  uint8_t huffcount[16];
  uint8_t huffval[162];
} jpege_huffmanAcTable;

typedef struct
{
  uint8_t huffcount[16];
  uint8_t huffval[12];
} jpege_huffmanDcTable;

/** Expect table in Q, driver is to do 1/Q, then program the
 *  hardware. */
typedef struct
{
  uint16_t qtbl[64];
} jpege_quantTable;

typedef struct
{

  uint16_t restartInterval;         /**< Number of MCU between restart marker insertion.
                                                    0 = disabled. A value of N MCU is programmed as N*/
  BOOL bCustomHuffmanTbl;
                /**<  This field is only valid when huffmanTblEnable = 1.
                                                    \n 1 = customized Huffman table for JPEG processing.
                                                    \n 0 = default Huffman table for JPEG processing. */
  jpege_huffmanDcTable *huffmanTblYDcPtr;      /**< Only valid when customized huffman table is selected.  Pointer to y-dc bits/val table. */
  jpege_huffmanAcTable *huffmanTblYAcPtr;      /**< Only valid when customized huffman table is selected.  Pointer to y-ac bits/val table. */
  jpege_huffmanDcTable *huffmanTblCbcrDcPtr;   /**< Only valid when customized huffman table is selected.  Pointer to cbcr-dc bits/val table. */
  jpege_huffmanAcTable *huffmanTblCbcrAcPtr;   /**< Only valid when customized huffman table is selected.  Pointer to cbcr-ac bits/val table. */

  jpege_quantTable *quantTblY;        /**< Q table for Y, 64 entries. Gemini Driver to convert to 1/Q,
                                                        then program the hardware.  */
  jpege_quantTable *quantTblChroma;      /**< Q table for chroma, 64 entries. Gemini Driver to convert to 1/Q,
                                                        then program the hardware.  */
  gemini_cmds_jpeg_fsc_cfg sFileSizeControl;
  JPEG_SPEED_MODE_T speed_mode;
} jpege_cmd_jpeg_encode_cfg;

/*****************************************************************************************************/
/************************  Fetch Engine and Write Engine *********************************************/
/*****************************************************************************************************/
typedef enum JPEGE_HW_INPUT_FORMAT
{
  JPEGE_INPUT_H1V1 = 0,
  JPEGE_INPUT_H1V2 = 1,         /**< only valid for offline encode mode. */
  JPEGE_INPUT_H2V1 = 2,
  JPEGE_INPUT_H2V2 = 3,
  JPEGE_INPUT_MONOCHROME = 4,
  JPEGE_INPUT_FORMAT_MAX =5,
} JPEGE_HW_INPUT_FORMAT;

typedef struct
{
  JPEGE_HW_INPUT_FORMAT inputFormat;
           /**< Use this information to derive others fields. */
  uint8_t input_cbcr_order;
  /**< 0 = cb pixel in LSB of the word. 1 = cr pixel in LSB of the word.  */
  uint32_t image_width;
  uint32_t image_height;
  uint16_t num_of_input_plns;
  uint32_t stride;
  uint32_t scanline;
  uint32_t hw_buf_size;
} jpege_cmd_input_cfg;

typedef struct
{
    uint8_t     scale_enable;
    uint32_t    scale_input_width;   // input width
    uint32_t    scale_input_height;  // input height
    uint32_t    h_offset;      // h_offset
    uint32_t    v_offset;      // v_offset
    uint32_t    output_width;  // output width
    uint32_t    output_height; // output height
    uint8_t     crop_enable;

}jpege_cmd_scale_cfg;


/*****************************************************************************************************/
/*********************************** Gemini Messages *************************************************/
/*****************************************************************************************************/

typedef enum
{
  JPEG_EVT_SESSION_DONE = MSM_JPEG_EVT_SESSION_DONE,
  JPEG_EVT_ERROR = MSM_JPEG_EVT_ERR,
  JPEG_EVT_BUS_ERROR = MSM_JPEG_EVT_ERR,
  JPEG_EVT_VIOLATION = MSM_JPEG_EVT_ERR,
  JPEG_EVT_MAX
} JPEG_EVT_ID;


#endif /*  JPEGE_HW_COMMON_H */
