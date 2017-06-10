/*========================================================================


*//** @file jpegd_engine_sw.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-09 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
12/10/09   vma     Removed the dependency on os_thread_detach by joining
                   the decode thread properly.
12/02/09   mingy   Added checking engine output handler return value.
11/18/09   mingy   Corrected typos.
10/22/09   vma     Export the engine 'profile' for easier engine picking.
10/15/09   mingy   Modified iDCT function signature.
09/21/09   mingy   Added region based decoding support.
08/03/09   vma     Switched to use the os abstraction layer (os_*)
07/20/09   zhiminl Added support for progressive decoding.
07/07/09   zhiminl Decoupled bit buffer from jpegd_engine_input_fetcher_t.
05/21/09   mingy   Modified the component starting index from 1 to 0.
03/24/09   mingy   Modified the way to compute the nResizeFactor variable.
                   The variable is computed based on both the output
                   width and height instead of using only output width.
10/27/08   vma     Fixed bug when is_active flag is not cleared in case of
                   any decode failure so destroying the engine causes the
                   thread joining to block forever.
09/15/08   vma     Added aborting.
07/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_buffer_private.h"
#include "jpeg_common.h"
#include "jpegd_engine_sw.h"
#include "jpegd.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include <stdlib.h>

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* Function prototypes of jpegd_engine_obj_t interface functions */
static void jpegd_engine_sw_create  (jpegd_engine_obj_t *p_obj,
                                     jpegd_obj_t decoder);
static int  jpegd_engine_sw_init    (jpegd_engine_obj_t*,
                                     jpegd_engine_event_handler_t,
                                     jpegd_engine_input_req_handler_t);
static int  jpegd_engine_sw_start   (jpegd_engine_obj_t*);
static int  jpegd_engine_sw_abort   (jpegd_engine_obj_t*);
static void jpegd_engine_sw_destroy (jpegd_engine_obj_t*);
static int  jpegd_engine_sw_configure (jpegd_engine_obj_t*,
                                       jpegd_engine_src_t*,
                                       jpegd_engine_dst_t*,
                                       jpegd_dst_t*,
                                       uint32_t*,
                                       uint32_t*);
/* Function prototypes of helper functions */
static int  jpegd_engine_sw_configure_baseline(jpegd_engine_sw_t *,
                                               uint32_t *,
                                               uint32_t *);
static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
             jpegd_engine_sw_decode_thread(OS_THREAD_FUNC_ARG_T arg);
static jpeg_event_t jpegd_engine_sw_decode_baseline(jpegd_engine_sw_t *);

/* Extern functions */
extern int  jpegd_engine_sw_check_qtable   (jpegd_engine_sw_t *);
extern int  jpegd_engine_sw_check_htable   (jpegd_engine_sw_t *);
extern void jpegd_engine_sw_output_gray_mcu(jpegd_engine_sw_t *);
extern void jpegd_engine_sw_output_h1v1_mcu(jpegd_engine_sw_t *);
extern void jpegd_engine_sw_output_h1v2_mcu(jpegd_engine_sw_t *);
extern void jpegd_engine_sw_output_h2v1_mcu(jpegd_engine_sw_t *);
extern void jpegd_engine_sw_output_h2v2_mcu(jpegd_engine_sw_t *);
extern void jpegd_engine_sw_idct_1x1(jpegd_coeff_t *,
                                     jpegd_sample_t *,
                                     int32_t);
extern void jpegd_engine_sw_idct_2x2(jpegd_coeff_t *,
                                     jpegd_sample_t *,
                                     int32_t);
extern void jpegd_engine_sw_idct_4x4(jpegd_coeff_t *,
                                     jpegd_sample_t *,
                                     int32_t);
extern void jpegd_engine_sw_idct_8x8(jpegd_coeff_t *,
                                     jpegd_sample_t *,
                                     int32_t);
extern uint32_t jpegd_engine_sw_get_padded_bits(jpegd_engine_sw_t *,
                                                int32_t);
extern void jpegd_engine_sw_skip_one_mcu(jpegd_engine_sw_t *);
extern int  jpegd_engine_sw_process_restart_marker(jpegd_engine_sw_t *,
                                                   int32_t *);
extern int  jpegd_engine_sw_decode_one_mcu(jpegd_engine_sw_t *);
extern int  jpegd_engine_sw_consume_one_mcu(jpegd_engine_sw_t *);
extern int  jpegd_engine_sw_init_bit_buffer(jpegd_engine_sw_t *);
extern int  jpegd_engine_sw_configure_progressive(jpegd_engine_sw_t *,
                                                  uint32_t *,
                                                  uint32_t *);
extern jpeg_event_t jpegd_engine_sw_decode_progressive(jpegd_engine_sw_t *);

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
#define INTERNAL_BUFFER_SIZE   8192 /* in bytes */
#define MAX_MCU_SIZE           384  /* in bytes */
static const char jpegd_engine_sw_name[] = "Jpeg Software Decode Engine";
jpegd_engine_profile_t jpegd_engine_sw_profile = {jpegd_engine_sw_create, // create_func
                                                         jpegd_engine_sw_name,   // engine_name
                                                         0};                     // need_pmem

/* =======================================================================
**                       Macro/Constant Definitions
** ======================================================================= */
#define BYTES_PER_FETCH   0x2000
/* =======================================================================
**                          Function Definitions
** ======================================================================= */

static void jpegd_engine_sw_create(jpegd_engine_obj_t *p_obj, jpegd_obj_t decoder)
{
    if (p_obj)
    {
        // Destroy previous engine if it exists
        if (p_obj->destroy)
        {
            p_obj->destroy(p_obj);
        }
        p_obj->create        = &jpegd_engine_sw_create;
        p_obj->init          = &jpegd_engine_sw_init;
        p_obj->configure     = &jpegd_engine_sw_configure;
        p_obj->start         = &jpegd_engine_sw_start;
        p_obj->abort         = &jpegd_engine_sw_abort;
        p_obj->destroy       = &jpegd_engine_sw_destroy;
        p_obj->p_engine      = NULL;
        p_obj->decoder       = decoder;
        p_obj->is_intialized = false;
        p_obj->skip_pp       = false;
    }
}

static int jpegd_engine_sw_init(
    jpegd_engine_obj_t              *p_obj,
    jpegd_engine_event_handler_t     p_event_handler,
    jpegd_engine_input_req_handler_t p_input_req_handler)
{
    jpegd_engine_sw_t *p_engine;

    // Validate input arguments
    if (!p_obj || !p_event_handler || !p_input_req_handler)
        return JPEGERR_ENULLPTR;

    // Allocate memory for the engine structure
    p_engine = (jpegd_engine_sw_t *)JPEG_MALLOC(sizeof(jpegd_engine_sw_t));
    if (!p_engine)
        return JPEGERR_EMALLOC;

    // Initialize the fields inside the engine structure below
    STD_MEMSET(p_engine, 0, sizeof(jpegd_engine_sw_t));  // Zero out the entire structure
    p_engine->p_wrapper               = p_obj;           // Initialize the pointer to the wrapper
    p_engine->input_fetcher.p_wrapper = p_obj;           // Initialize the pointer to the wrapper
    p_engine->p_event_handler         = p_event_handler; // Initialize the event handler
    p_engine->input_fetcher.p_input_req_handler = p_input_req_handler; // Initialize the input request handler
    p_engine->input_fetcher.p_source  = &p_engine->source; // Initialize source pointer of fetcher
    os_mutex_init(&(p_engine->mutex));                   // Initialize the mutex
    os_cond_init(&(p_engine->cond));                     // Initialize the condition variable

    // Assign allocated engine structure to p_obj
    p_obj->p_engine = (void *)p_engine;
    p_obj->is_intialized = true;
    return JPEGERR_SUCCESS;
}

static int jpegd_engine_sw_start(jpegd_engine_obj_t *p_obj)
{
    int rc;
    jpegd_engine_sw_t *p_engine;
    os_thread_t thread;

    // Validate input arguments
    if (!p_obj || !p_obj->p_engine)
        return JPEGERR_ENULLPTR;

    // Cast p_obj->obj as jpegd_engine_sw_t object
    p_engine = p_obj->p_engine;

    // Make sure no threads are running
    os_mutex_lock(&(p_engine->mutex));
    if (p_engine->is_active || !p_engine->is_configured)
    {
        os_mutex_unlock(&(p_engine->mutex));
        JPEG_DBG_HIGH("jpegd_engine_sw_start: bad state\n");
        return JPEGERR_EBADSTATE;
    }

    // Reset abort flag
    p_engine->abort_flag = false;
    // Set engine to be active
    p_engine->is_active = true;

    // Join previously created decode thread if needed
    if (p_engine->is_dthread_need_join)
    {
        (void)os_thread_join(&p_engine->decode_thread, NULL);
    }
    p_engine->is_dthread_need_join = true;

    // Start Jpeg Decoding as a new thread
    rc = os_thread_create(&p_engine->decode_thread, jpegd_engine_sw_decode_thread, (void *)p_engine);
    // If there is a failure in creating the thread, clean up and exit
    if (rc)
    {
        // Reset engine to be inactive
        p_engine->is_active = false;
        // Reset thread needs join flag
        p_engine->is_dthread_need_join = false;

        os_mutex_unlock(&p_engine->mutex);
        JPEG_DBG_HIGH("jpegd_engine_sw_start: os_thread_create() failed: %d\n", rc);
        return JPEGERR_EFAILED;
    }

    os_mutex_unlock(&(p_engine->mutex));

    return JPEGERR_SUCCESS;
}

static int jpegd_engine_sw_abort(
    jpegd_engine_obj_t *p_obj)
{
    jpegd_engine_sw_t *p_engine;

    // Validate input arguments
    if (!p_obj || !p_obj->p_engine)
        return JPEGERR_ENULLPTR;

    // Cast p_obj->obj as jpegd_engine_sw_t object
    p_engine = p_obj->p_engine;

    // Abort if engine is actively decoding
    os_mutex_lock(&(p_engine->mutex));
    if (p_engine->is_active)
    {
        p_engine->abort_flag = true;
        while (p_engine->abort_flag)
        {
            os_cond_wait(&(p_engine->cond), &(p_engine->mutex));
        }
    }
    os_mutex_unlock(&(p_engine->mutex));
    return JPEGERR_SUCCESS;
}

static void jpegd_engine_sw_destroy(
    jpegd_engine_obj_t *p_obj)
{
    if (p_obj)
    {
        uint32_t i;
        jpegd_engine_sw_t *p_engine = (jpegd_engine_sw_t *)(p_obj->p_engine);
        // Abort and wait until engine is done with current decoding
        // todo
        (void)jpegd_engine_sw_abort(p_obj);

        // Join decode thread if necessary
        if (p_engine->is_dthread_need_join)
            (void)os_thread_join(&p_engine->decode_thread, NULL);

        // Free pSampleMCUBuf
        for (i = 0; i < JPEGD_MAXBLOCKSPERMCU; i++)
        {
            JPEG_FREE(p_engine->pSampleMCUBuf[i]);
        }
        // Free pDerivedHuffTable
        for (i = 0; i < JPEGD_MAXHUFFTABLES; i++)
        {
            JPEG_FREE(p_engine->pDerivedHuffTable[i]);
        }
        // Free pDeQuantTable
        for (i = 0; i < JPEGD_MAXQUANTTABLES; i++)
        {
            JPEG_FREE(p_engine->pDeQuantTable[i]);
        }
        // Free pCompCoeffBuf
        for (i = 0; i < JPEGD_MAXCOMPONENTS; i++)
        {
            JPEG_FREE(p_engine->pCompCoeffBuf[i]);
        }
        // Release allocated memory
        if (p_engine)
        {
            os_mutex_destroy(&(p_engine->mutex));
            os_cond_destroy(&(p_engine->cond));
            JPEG_FREE(p_obj->p_engine);
            p_obj->p_engine = NULL;
        }
    }
}

/******************************************************************************
 * Helper functions below
 *****************************************************************************/
static int jpegd_engine_sw_configure(jpegd_engine_obj_t *p_obj,
                                     jpegd_engine_src_t *p_source,
                                     jpegd_engine_dst_t *p_dest,
                                     jpegd_dst_t        *p_jpegd_dst,
                                     uint32_t           *p_output_chunk_width,
                                     uint32_t           *p_output_chunk_height)
{
    jpeg_frame_info_t *p_frame_info;
    jpegd_engine_sw_t *p_engine;
    int                rc;

    // Input validation
    if (!p_obj || !p_obj->p_engine ||
        !p_source || !p_dest || !p_source->p_frame_info || !p_source->p_input_req_handler ||
        !p_dest->p_luma_buffers[0] || !p_dest->p_chroma_buffers[0] ||
        !p_dest->p_luma_buffers[1] || !p_dest->p_chroma_buffers[1] ||
        !p_dest->width || !p_dest->height)
    {
        JPEG_DBG_ERROR("jpegd_engine_sw_configure: bad param\n");
        return JPEGERR_EBADPARM;
    }

    // Support baseline & progressive decoding
    p_frame_info = p_source->p_frame_info;
    if ((p_frame_info->process != JPEG_PROCESS_BASELINE) &&
        (p_frame_info->process != JPEG_PROCESS_PROGRESSIVE_HUFFMAN))
    {
        JPEG_DBG_ERROR("jpegd_engine_sw_configure: unsupported coding process %d\n",
                       p_frame_info->process);
        return JPEGERR_EUNSUPPORTED;
    }

    // Cast p_obj->p_engine as jpegd_engine_sw_t object
    p_engine = (jpegd_engine_sw_t *)p_obj->p_engine;

    // Initialize engine input
    p_engine->source = *p_source;
    jpeg_buffer_mark_empty(p_engine->source.p_buffers[0]);
    jpeg_buffer_mark_empty(p_engine->source.p_buffers[1]);
    p_engine->input_fetcher.buffer_in_use = 0;

    // Initialize engine output dimensions
    p_engine->dest = *p_dest;

    // Initialize engine output MCU line buffer pointers
    p_engine->pMCULineBufPtrY    = NULL;
    p_engine->pMCULineBufPtrCbCr = NULL;

    // Initialize engine internal data
    (void)STD_MEMSET(p_engine->MCUBlockCompList,     0, sizeof(p_engine->MCUBlockCompList));
    (void)STD_MEMSET(p_engine->compId,               0, sizeof(p_engine->compId));
    (void)STD_MEMSET(p_engine->quantId,              0, sizeof(p_engine->quantId));
    (void)STD_MEMSET(p_engine->compListInScan,       0, sizeof(p_engine->compListInScan));
    (void)STD_MEMSET(p_engine->dcHuffTableId,        0, sizeof(p_engine->dcHuffTableId));
    (void)STD_MEMSET(p_engine->acHuffTableId,        0, sizeof(p_engine->acHuffTableId));
    (void)STD_MEMSET(p_engine->hSample,              0, sizeof(p_engine->hSample));
    (void)STD_MEMSET(p_engine->vSample,              0, sizeof(p_engine->vSample));
    (void)STD_MEMSET(p_engine->compEntropySelectors, 0, sizeof(p_engine->compEntropySelectors));
    (void)STD_MEMSET(p_engine->compCoeffHBlocks,     0, sizeof(p_engine->compCoeffHBlocks));
    (void)STD_MEMSET(p_engine->compCoeffVBlocks,     0, sizeof(p_engine->compCoeffVBlocks));

    // Initialize restart interval
    p_engine->nRestartInterval = 0;

    // Initialize engine internal function pointers
    p_engine->idct_func = NULL;
    p_engine->mcu_output_func = NULL;

    if (p_frame_info->process == JPEG_PROCESS_BASELINE)
    {
        // Initialized engine in baseline mode
        p_engine->is_progressive = false;
        rc = jpegd_engine_sw_configure_baseline(
            p_engine,
            p_output_chunk_width,
            p_output_chunk_height);
        if (JPEG_FAILED(rc)) return rc;
    }
    else
    {
        // Initialized engine in progressive mode
        p_engine->is_progressive = true;
        rc = jpegd_engine_sw_configure_progressive(
            p_engine,
            p_output_chunk_width,
            p_output_chunk_height);
        if (JPEG_FAILED(rc)) return rc;
    }

    // Set engine output dimensions, may get changed by resize factor
    p_dest->width       = p_engine->nOutputWidth;
    p_dest->height      = p_engine->nOutputHeight;
    p_dest->subsampling = p_engine->jpegdFormat;

    // Set engine output MCU line buffer sizes
    *p_output_chunk_width  = p_engine->nOutputRowIncrY;
    *p_output_chunk_height = p_engine->nYLinesPerMCU;

    p_engine->is_configured = true;
    p_engine->fInputError = 0;
    return JPEGERR_SUCCESS;
}

static int jpegd_engine_sw_configure_baseline
(
    jpegd_engine_sw_t *p_engine,
    uint32_t          *p_output_chunk_width,
    uint32_t          *p_output_chunk_height
)
{
    uint8_t  j;
    uint32_t i;
    uint8_t  componentId;
    int32_t  blockCount;
    uint32_t numOfBlocks;

    uint8_t  hSample[JPEGD_MAXCOMPONENTS];
    uint8_t  vSample[JPEGD_MAXCOMPONENTS];

    uint32_t numOfHBlocksInComp[JPEGD_MAXCOMPONENTS];
    uint32_t numOfVBlocksInComp[JPEGD_MAXCOMPONENTS];

    jpeg_rectangle_t region;

    jpeg_frame_info_t *p_frame_info = p_engine->source.p_frame_info;
    jpeg_scan_info_t  *p_scan_info  = p_frame_info->pp_scan_infos[0];

    uint8_t  maxHSample  = 1;
    uint8_t  maxVSample  = 1;

    uint32_t nRowsPerMCU = 0;
    uint32_t nColsPerMCU = 0;

    /**********************************************************************
     * First convert jpeg header related info in the p_engine->decodeSpec into
     * local variables
     * 1. image size
     * 2. restart interval
     * 3. components in frame and scan (for now only 1 scan is allowed)
     * 4. set components info (id, sampling, quant table) in sof marker
     * 5. set components info (huff table) in sos (scan 1 only)
     *    as well as spectrum selection.
     *********************************************************************/
    p_engine->nImageWidth               = p_frame_info->width;
    p_engine->nImageHeight              = p_frame_info->height;

    p_engine->nOutputWidth              = p_engine->dest.width;
    p_engine->nOutputHeight             = p_engine->dest.height;

    p_engine->nRestartInterval          = p_frame_info->restart_interval;
    p_engine->nNumberOfComponentsInScan = p_scan_info->num_selectors;

    // component id, sampling, quant info
    for (j = 0; j < p_frame_info->num_comps; j++)
    {
        p_engine->compId[j]  = p_frame_info->p_comp_infos[j].comp_id;
        hSample[j]           = p_frame_info->p_comp_infos[j].sampling_h;
        vSample[j]           = p_frame_info->p_comp_infos[j].sampling_v;
        p_engine->quantId[j] = p_frame_info->p_comp_infos[j].qtable_sel;
    }

    // scan info, for baseline it is only 1 scan.
    p_engine->nNumberOfComponentsInScan = p_scan_info->num_selectors;
    p_engine->currentScanProgressiveInfo.ss = (uint32_t)(p_scan_info->spec_start);
    p_engine->currentScanProgressiveInfo.se = (uint32_t)(p_scan_info->spec_end);

    // component huffman info
    for (j = 0; j < p_engine->nNumberOfComponentsInScan; j++)
    {
        // get component id
        componentId = p_scan_info->p_selectors[j].comp_id;
        // dc table index
        p_engine->dcHuffTableId[componentId] = p_scan_info->p_selectors[j].dc_selector;

        // since only one set of huff table index is used,
        // ac table is appened to dc table with index starting from
        // JPEGD_MAXHUFFTABLES/2.
        p_engine->acHuffTableId[componentId] = p_scan_info->p_selectors[j].ac_selector +
            (uint8_t)(JPEGD_MAXHUFFTABLES >> 1);
        // get components list in a MCU in one scan
        p_engine->compListInScan[j] = componentId;
    }

    /**********************************************************************
    *  If no region information is specified, then set the region to      *
    *  the whole image.                                                   *
    **********************************************************************/
    region = p_engine->dest.region;
    if (region.right == 0 && region.bottom == 0)
    {
        region.right  = p_engine->nImageWidth  - 1;
        region.bottom = p_engine->nImageHeight - 1;
    }

    /**********************************************************************
    * Validate the region:
    * 0 <= left < right  < Input Image Width
    * 0 <= top  < bottom < Input Image Height
    *********************************************************************/
    if (0 > region.left || region.left >= region.right  || region.right  >= (int32_t)(p_engine->nImageWidth) ||
        0 > region.top  || region.top  >= region.bottom || region.bottom >= (int32_t)(p_engine->nImageHeight) )
    {
        return JPEGERR_EFAILED;
    }


    /**********************************************************************
    * Here we check the color format. Also set up MCU related settings
    * 1. color format
    * 2. numbers of blocks in a MCU
    * 3. numbers of pixels in a row in one MCU
    * 4. numbers of pixels in a column in one MCU
    **********************************************************************/
    if (p_frame_info->num_comps == 1)
    {
        /******************************************************************
        * grey scale, only 1 (Y) 8x8 block per MCU
        ******************************************************************/
        p_engine->jpegdFormat   = JPEG_GRAYSCALE;
        p_engine->nBlocksPerMCU = 1;
        nRowsPerMCU     = 8;
        nColsPerMCU     = 8;

        /**************************************************************************
        * Compute the # of MCUs outside of the Region.
        *
        * left_mcu_num   is the # of MCU before the Region MCU Row.
        * top_mcu_num    is the # of MCU above  the Region MCU Row.
        * right_mcu_num  is the # of MCU after  the Region MCU Row.
        * bottom_mcu_num is the # of MCU below  the Region MCU Row.
        *
        * Right shift by 3 is equivalent to integer divide by 8.
        *************************************************************************/
        p_engine->left_mcu_num   = region.left >> 3;
        p_engine->top_mcu_num    = region.top  >> 3;
        p_engine->right_mcu_num  = (ROUND_TO_8(p_engine->nImageWidth)  - (uint32_t)(region.right)  - 1) >> 3;
        p_engine->bottom_mcu_num = (ROUND_TO_8(p_engine->nImageHeight) - (uint32_t)(region.bottom) - 1) >> 3;
    }
    else if (p_frame_info->num_comps == 3)
    {
        if (hSample[1]!=1 || vSample[1]!=1 ||
            hSample[2]!=1 || vSample[2]!=1)
        {
            return JPEGERR_EUNSUPPORTED;
        }
        if (hSample[0] == 1 && vSample[0] == 1)
        {
            /**************************************************************
             * H1V1, 3 (Y, Cb, Cr) blocks per MCU
             *************************************************************/
            p_engine->jpegdFormat   = JPEG_H1V1;
            p_engine->nBlocksPerMCU = 3;
            nRowsPerMCU     = 8;
            nColsPerMCU     = 8;

            /**************************************************************************
            * Compute the # of MCUs outside of the Region.
            *************************************************************************/
            p_engine->left_mcu_num   = region.left >> 3;
            p_engine->top_mcu_num    = region.top  >> 3;
            p_engine->right_mcu_num  = (ROUND_TO_8(p_engine->nImageWidth)  - (uint32_t)(region.right)  - 1) >> 3;
            p_engine->bottom_mcu_num = (ROUND_TO_8(p_engine->nImageHeight) - (uint32_t)(region.bottom) - 1) >> 3;
        }
        else if (hSample[0] == 2 && vSample[0] == 1)
        {
            /**************************************************************
             * H2V1, 4 (Yx2, Cb, Cr) blocks per MCU
             *************************************************************/
            p_engine->jpegdFormat   = JPEG_H2V1;
            p_engine->nBlocksPerMCU = 4;
            nRowsPerMCU     = 8;
            nColsPerMCU     = 16;

            /**************************************************************************
            * Compute the # of MCUs outside of the Region.
            *************************************************************************/
            p_engine->left_mcu_num   = region.left >> 4;
            p_engine->top_mcu_num    = region.top  >> 3;
            p_engine->right_mcu_num  = (ROUND_TO_16(p_engine->nImageWidth) - (uint32_t)(region.right)  - 1) >> 4;
            p_engine->bottom_mcu_num = (ROUND_TO_8(p_engine->nImageHeight) - (uint32_t)(region.bottom) - 1) >> 3;
        }
        else if (hSample[0] == 1 && vSample[0] == 2)
        {
            /**************************************************************
             * H1V2, 4 (Yx2, Cb, Cr) blocks per MCU
             *************************************************************/
            p_engine->jpegdFormat   = JPEG_H1V2;
            p_engine->nBlocksPerMCU = 4;
            nRowsPerMCU     = 16;
            nColsPerMCU     = 8;

            /**************************************************************************
            * Compute the # of MCUs outside of the Region.
            *************************************************************************/
            p_engine->left_mcu_num   = region.left >> 3;
            p_engine->top_mcu_num    = region.top  >> 4;
            p_engine->right_mcu_num  = (ROUND_TO_8(p_engine->nImageWidth)   - (uint32_t)(region.right)  - 1) >> 3;
            p_engine->bottom_mcu_num = (ROUND_TO_16(p_engine->nImageHeight) - (uint32_t)(region.bottom) - 1) >> 4;
        }
        else if (hSample[0] == 2 && vSample[0] == 2)
        {
            /**************************************************************
             * H2V2, 6 (Yx4, Cb, Cr) blocks per MCU
             *************************************************************/
            p_engine->jpegdFormat   = JPEG_H2V2;
            p_engine->nBlocksPerMCU = 6;
            nRowsPerMCU     = 16;
            nColsPerMCU     = 16;

            /**************************************************************************
            * Compute the # of MCUs outside of the Region.
            *************************************************************************/
            p_engine->left_mcu_num   = region.left >> 4;
            p_engine->top_mcu_num    = region.top  >> 4;
            p_engine->right_mcu_num  = (ROUND_TO_16(p_engine->nImageWidth)  - (uint32_t)(region.right)  - 1) >> 4;
            p_engine->bottom_mcu_num = (ROUND_TO_16(p_engine->nImageHeight) - (uint32_t)(region.bottom) - 1) >> 4;
        }
        else
        {
            /**************************************************************
             * if none of these format, we can not decode this, never
             * should go here.
             *************************************************************/
            return JPEGERR_EUNSUPPORTED;
        } // end if ( hSample[0] == 1 &&  vSample[0] == 1 )
    }
    else
    {
        return JPEGERR_EUNSUPPORTED;
    } // end if ( p_frame_info->num_comps == 1 )

    /**********************************************************************
    * here we find out how many lines and coloumns of MCUs in a component
    * 1. find out the maximum sampling factor for all components
    * 2. calculate number of blocks in verticle and horizontal direction
    *    for each component.
    **********************************************************************/
    // first figure out the maximum sample for rols and cols
    for (j = 0; j < p_frame_info->num_comps; j++)
    {
        if (hSample[j] > maxHSample)
        {
            maxHSample = hSample[j];
        }
        if (vSample[j] > maxVSample)
        {
            maxVSample = vSample[j];
        }
    }
    // then use the maximum sample to determine the largest block per line
    // and block per col.
    for (j = 0; j < p_frame_info->num_comps; j++)
    {
        numOfHBlocksInComp[j] = (uint32_t) ( (7+ (p_engine->nImageWidth * maxHSample
                                               + maxHSample-1) /maxHSample) /8);
        numOfVBlocksInComp[j] = (uint32_t) ( (7+ (p_engine->nImageHeight * maxVSample
                                               + maxVSample-1) / maxVSample) /8);
    }
    /**********************************************************************
     * also need to calculate the number of MCUs per row and col
     * This is determind by the color format and number of blocks in image
     * For each mcu, the type of block need to be labeled also
     *********************************************************************/
    if (p_engine->nNumberOfComponentsInScan == 1)
    {
        // grey image
        p_engine->nMCUPerRow = numOfHBlocksInComp[p_engine->compListInScan[0]];
        p_engine->nMCUPerCol = numOfVBlocksInComp[p_engine->compListInScan[0]];
        p_engine->MCUBlockCompList[0] = p_engine->compListInScan[0];
    }
    else
    {
        //color image.
        p_engine->nMCUPerRow = ( (p_engine->nImageWidth  + 7) / 8 + maxHSample - 1) / maxHSample;
        p_engine->nMCUPerCol = ( (p_engine->nImageHeight + 7) / 8 + maxVSample - 1) / maxVSample;

        blockCount = 0;
        for (j = 0; j < p_engine->nNumberOfComponentsInScan; j++)
        {
            // using color format information, we can get number of blocks
            // per MCU. we can assume that blocks in MCU is Y first
            // followed by cb and cr.
            componentId = p_engine->compListInScan[j];
            numOfBlocks = hSample[componentId] * vSample[componentId];

            while (numOfBlocks--)
            {
                p_engine->MCUBlockCompList[blockCount++] = componentId;
            }
        }
    }

    /**************************************************************************
    *  Calculate the Region Start MCU index and Region End MCU index.         *
    **************************************************************************/
    p_engine->region_start_mcu_id =
        p_engine->top_mcu_num * p_engine->nMCUPerRow + p_engine->left_mcu_num;

    p_engine->region_end_mcu_id   =
        (p_engine->nMCUPerCol - p_engine->bottom_mcu_num) * p_engine->nMCUPerRow -
        p_engine->right_mcu_num;

    // Adjust the # of MCUs in a MCU Row for Region decoding
    p_engine->nMCUPerRow -= (p_engine->right_mcu_num + p_engine->left_mcu_num);

    // Adjust the image width/height to be the Region dimensions.
    p_engine->nImageWidth  = region.right  - region.left + 1;
    p_engine->nImageHeight = region.bottom - region.top  + 1;

    /**********************************************************************
     * Calculate the Resize factor for IDCT according to input/output size
     * ratio
     *********************************************************************/
    p_engine->nResizeFactor = 3;
    while ( (p_engine->nResizeFactor >= 0) &&
            ((p_engine->nOutputWidth > (p_engine->nImageWidth  >> p_engine->nResizeFactor)) ||
            (p_engine->nOutputHeight > (p_engine->nImageHeight >> p_engine->nResizeFactor))))
    {
        p_engine->nResizeFactor--;
    }
    if (p_engine->nResizeFactor < 0)
    {
        return JPEGERR_EFAILED;
    }
    nRowsPerMCU >>= p_engine->nResizeFactor;
    nColsPerMCU >>= p_engine->nResizeFactor;
    // set idct method
    switch (p_engine->nResizeFactor)
    {
    case 0:
        {
            p_engine->idct_func = &jpegd_engine_sw_idct_8x8;
            p_engine->numOfValidCoeff = 64;
            break;
        }
    case 1:
        {
            p_engine->idct_func = &jpegd_engine_sw_idct_4x4;
            p_engine->numOfValidCoeff = 25;
            break;
        }
    case 2:
        {
            p_engine->idct_func = &jpegd_engine_sw_idct_2x2;
            p_engine->numOfValidCoeff = 5;
            break;
        }
    case 3:
        {
            p_engine->idct_func = &jpegd_engine_sw_idct_1x1;
            p_engine->numOfValidCoeff = 1;
            break;
        }
    default:
        {
            return JPEGERR_EFAILED;
        }
    }

    if (p_engine->nRestartInterval != 0)
    {
        p_engine->nRestartLeft    = p_engine->nRestartInterval;
        p_engine->nNextRestartNum = 0xD0;
    }

    /**********************************************************************
     * check/make quantization table, huffman table
     *********************************************************************/
    if (JPEG_FAILED(jpegd_engine_sw_check_qtable(p_engine)))
    {
        return JPEGERR_EFAILED;
    }

    if (JPEG_FAILED(jpegd_engine_sw_check_htable(p_engine)))
    {
        return JPEGERR_EFAILED;
    }

    /**************************************************************************
    * Paddings are added to Jpeg image to round it up to multiple of blocks,
    * but actual image size be less than the round up.
    *
    * Calculate the Row Increment for the MCU Row buffer that holds the
    * rounded up lines.
     *
    * Also set which JpegdOutputOneMCU method to use according to format.
     *
     * The output size is changed proportionally to the input size, unlike
    * the output buffer size, which is rounded to multiple of cols per MCU.
     *
    * This will be used for postprocess to unpad the image.
    *************************************************************************/
    if (p_engine->jpegdFormat == JPEG_GRAYSCALE)
    {
        // gray-scale
        p_engine->nYLinesPerMCU      = nRowsPerMCU;
        p_engine->nCbCrLinesPerMCU   = 0;
        p_engine->nOutputRowIncrY    = nColsPerMCU * p_engine->nMCUPerRow;
        p_engine->nOutputRowIncrCbCr = 0;
        p_engine->mcu_output_func    = &jpegd_engine_sw_output_gray_mcu;
    }
    else if (p_engine->jpegdFormat == JPEG_H1V1)
    {
        // H1V1
        p_engine->nYLinesPerMCU      = nRowsPerMCU;
        p_engine->nCbCrLinesPerMCU   = nRowsPerMCU;
        p_engine->nOutputRowIncrY    = nColsPerMCU * p_engine->nMCUPerRow;
        p_engine->nOutputRowIncrCbCr = p_engine->nOutputRowIncrY*2;
        p_engine->mcu_output_func    = &jpegd_engine_sw_output_h1v1_mcu;
    }
    else if (p_engine->jpegdFormat == JPEG_H2V1)
    {
        // H2V1
        p_engine->nYLinesPerMCU      = nRowsPerMCU;
        p_engine->nCbCrLinesPerMCU   = nRowsPerMCU;
        p_engine->nOutputRowIncrY    = nColsPerMCU * p_engine->nMCUPerRow;
        p_engine->nOutputRowIncrCbCr = p_engine->nOutputRowIncrY;
        p_engine->mcu_output_func    = &jpegd_engine_sw_output_h2v1_mcu;
    }
    else if (p_engine->jpegdFormat == JPEG_H1V2)
    {
        // H1V2
        p_engine->nYLinesPerMCU      = nRowsPerMCU;
        p_engine->nCbCrLinesPerMCU   = nRowsPerMCU >> 1;
        p_engine->nOutputRowIncrY    = nColsPerMCU * p_engine->nMCUPerRow;
        p_engine->nOutputRowIncrCbCr = p_engine->nOutputRowIncrY*2;
        p_engine->mcu_output_func    = &jpegd_engine_sw_output_h1v2_mcu;
    }
    else if (p_engine->jpegdFormat == JPEG_H2V2)
    {
        // H2V2
        p_engine->nYLinesPerMCU      = nRowsPerMCU;
        p_engine->nCbCrLinesPerMCU   = nRowsPerMCU >> 1;
        p_engine->nOutputRowIncrY    = nColsPerMCU * p_engine->nMCUPerRow;
        p_engine->nOutputRowIncrCbCr = p_engine->nOutputRowIncrY;
        p_engine->mcu_output_func    = &jpegd_engine_sw_output_h2v2_mcu;
    }

    // Set output dimension
    p_engine->nOutputWidth  = (uint32_t)(p_engine->nImageWidth >> p_engine->nResizeFactor);
    p_engine->nOutputHeight = (uint32_t)(p_engine->nImageHeight >> p_engine->nResizeFactor);

    p_engine->nOutputBufferSizeY     = p_engine->nOutputRowIncrY * p_engine->nYLinesPerMCU;
    p_engine->nOutputBufferSizeCbCr  = p_engine->nOutputRowIncrCbCr * p_engine->nCbCrLinesPerMCU;

    /**********************************************************************
     *  Allocate buffer for use.
     *  No need for coefficent buff, it is decleared array
     *  sample buffer arry after idct
     *  allocated sample buffer with in a MCU
     *********************************************************************/
    for (i = 0; i < p_engine->nBlocksPerMCU; i++)
    {
        if (!p_engine->pSampleMCUBuf[i])
        {
            p_engine->pSampleMCUBuf[i] = (jpegd_sample_t *)JPEG_MALLOC(
                (JPEGD_DCTSIZE2) * sizeof(jpegd_sample_t));
            if (!p_engine->pSampleMCUBuf[i])
            {
                return JPEGERR_EMALLOC;
            }
        }
    }

    return JPEGERR_SUCCESS;
}

static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
    jpegd_engine_sw_decode_thread(OS_THREAD_FUNC_ARG_T arg)
{
    // Cast input argument as the engine object
    jpegd_engine_sw_t *p_engine = (jpegd_engine_sw_t *)arg;
    jpeg_event_t       event;

    if (!p_engine->is_progressive)
    {
        event = jpegd_engine_sw_decode_baseline(p_engine);
    }
    else
    {
        event = jpegd_engine_sw_decode_progressive(p_engine);
    }

    // Set the active flag to inactive
    os_mutex_lock(&(p_engine->mutex));
    p_engine->is_active = false;
    if (p_engine->abort_flag)
    {
        p_engine->abort_flag = false;
        os_cond_signal(&p_engine->cond);
    }
    os_mutex_unlock(&(p_engine->mutex));

    // Notify event
    p_engine->p_event_handler(p_engine->p_wrapper, event, NULL);
    return 0;
}

static jpeg_event_t jpegd_engine_sw_decode_baseline(jpegd_engine_sw_t *p_engine)
{
    int32_t  nSkippedMCUs;
    uint32_t nNumberOfMCUsInRow;
    uint32_t num_mcu_to_process;
    uint32_t num_region_mcu_to_skip;
    int      rc;

    /**********************************************************************
     * first is to make sure input and output pointer are valid and ready
     *********************************************************************/
    p_engine->input_fetcher.next_byte_to_fetch_offset =
        p_engine->source.p_frame_info->pp_scan_infos[0]->offset;

    /**********************************************************************
     * then load up the input buffer and bit buffer
     *********************************************************************/
    rc = jpegd_engine_sw_init_bit_buffer(p_engine);
    if (JPEG_FAILED(rc)) return JPEG_EVENT_ERROR;

    // set output - MCU line buffers
    p_engine->pOutputBufferY = p_engine->dest.p_luma_buffers[0]->ptr;
    p_engine->pOutputBufferCbCr = p_engine->dest.p_chroma_buffers[0]->ptr;
    p_engine->curr_dest_buf_index = 0;

    p_engine->pMCULineBufPtrY    = p_engine->pOutputBufferY;
    p_engine->pMCULineBufPtrCbCr = p_engine->pOutputBufferCbCr;

    /**********************************************************************
     * Now start decode bitstream.
     *********************************************************************/
    nSkippedMCUs       = 0;
    nNumberOfMCUsInRow = 0;

    // Compute the # of MCUs to Process in between the Region starting MCU
    // and the Region ending MCU.
    num_mcu_to_process = p_engine->region_end_mcu_id - p_engine->region_start_mcu_id;

    // first make sure dc value is cleared
    (void)STD_MEMSET(p_engine->lastDcVal, 0, JPEGD_MAXBLOCKSPERMCU * sizeof(uint32_t));

    /**********************************************************************
    * The first while loop is used to skip the MCUs before the
    * Region starts.
    *********************************************************************/
    while (p_engine->region_start_mcu_id > 0)
    {
        /* Check abort flag */
        os_mutex_lock(&(p_engine->mutex));
        if (p_engine->abort_flag)
        {
            p_engine->is_active = false;
            p_engine->abort_flag = false;
            os_cond_signal(&(p_engine->cond));
            os_mutex_unlock(&(p_engine->mutex));
            return JPEG_EVENT_ABORTED;
        }
        os_mutex_unlock(&(p_engine->mutex));

        if (nSkippedMCUs!=0)
        {
            jpegd_engine_sw_skip_one_mcu(p_engine);

            nSkippedMCUs--;
            p_engine->region_start_mcu_id--;
        }
        else /* if no regions is to be skipped */
        {
            // for each MCU, check if restart marker is expected.
            if ( (p_engine->nRestartLeft == 0) && (p_engine->nRestartInterval != 0))
            {
                 // process restart marker
                if (JPEG_FAILED(jpegd_engine_sw_process_restart_marker(p_engine,
                                                                       &nSkippedMCUs)))
                {
                    // break out of while loop and exit
                    return JPEG_EVENT_ERROR;
                }

                (void)STD_MEMSET(p_engine->lastDcVal, 0, JPEGD_MAXBLOCKSPERMCU * sizeof (uint32_t));
                p_engine->nRestartLeft = p_engine->nRestartInterval;
            }
            else
            {
                (void)jpegd_engine_sw_consume_one_mcu(p_engine);

                p_engine->nRestartLeft--;
                p_engine->region_start_mcu_id--;
            }
        }
    }

    /**********************************************************************
    * The second while loop decodes the MCUs until the end of the region.
    *********************************************************************/
    while (num_mcu_to_process > 0)
    {
        /* Check abort flag */
        os_mutex_lock(&(p_engine->mutex));
        if (p_engine->abort_flag)
        {
            p_engine->is_active = false;
            p_engine->abort_flag = false;
            os_cond_signal(&(p_engine->cond));
            os_mutex_unlock(&(p_engine->mutex));
            return JPEG_EVENT_ABORTED;
        }
        os_mutex_unlock(&(p_engine->mutex));

        if (nSkippedMCUs != 0)
        {
            /**************************************************************
             * if here, some MCUs need to be skipped
             * each time we skip one MCU until it is all skipped
             * when MCU is skipped we do:
             * 1. jpegd_skip_one_mcu: which writes 0 to that MCU buffer
             * 2. reduce skip MCU count
             * 3. reduce number of MCUs to be decoded count
             * 4. increase number of MCUs in current line count
             * 5. output one mcu: this can be replaced by moving the output
             *    pointer around.
             *************************************************************/
            jpegd_engine_sw_skip_one_mcu(p_engine);

            nSkippedMCUs--;
            num_mcu_to_process--;
            nNumberOfMCUsInRow++;

            p_engine->mcu_output_func(p_engine);
        }
        else /* if no MCUs are to be skipped */
        {
            // for each MCU, check if restart marker is expected.
            if ( (p_engine->nRestartLeft == 0) && (p_engine->nRestartInterval != 0))
            {
                /**********************************************************
                 * If restart marker is expected,
                 * process restart marker first
                 * 1. call process_restart_marker: this function will
                 *    search for restart marker
                 * 1.a if correct restart marker is found, then do nothing
                 *     and skipped MCU count will be 0.
                 * 1.b if correct restart marker is not found, calculate
                 *     how many MCUs need to be skipped.
                 * 2. clear dc residual
                 * 3. reset restart left to restart interval, start a fresh
                 *    restart interval.
                 *********************************************************/
                if (JPEG_FAILED(jpegd_engine_sw_process_restart_marker(p_engine,
                                                                       &nSkippedMCUs)))
                {
                    // break out of while loop and exit
                    return JPEG_EVENT_ERROR;
                }

                (void)STD_MEMSET(p_engine->lastDcVal, 0, JPEGD_MAXBLOCKSPERMCU * sizeof (uint32_t));
                p_engine->nRestartLeft = p_engine->nRestartInterval;
            }
            else
            {
                /**********************************************************
                 * if this is not a restarted marker location
                 * we do normal decode of 1 MCU
                 * 1. call decode_one_mcu: this will decode 1 MCU
                 * 2. reduce restart counter by 1.
                 * 3. reduce total MCU to decode count by 1.
                 * 4. increase current row MCU count by 1.
                 * 5. output_one_mcu: write one mcu to output buffer
                 *    (internal)
                 *********************************************************/
                // Regardless if there is something wrong or not in decode one MCU,
                // continue until all MCUs are done before the next restart marker.
                (void)jpegd_engine_sw_decode_one_mcu(p_engine);

                p_engine->nRestartLeft--;
                num_mcu_to_process--;
                nNumberOfMCUsInRow++;

                p_engine->mcu_output_func(p_engine);
            }
        }

        // now check if one MCU Row is been decoded
        // if yes, write out that MCU Row to external buffer.
        if (nNumberOfMCUsInRow == p_engine->nMCUPerRow)
        {
            //Number of MCUs to skip in between MCU Rows for region decoding
            num_region_mcu_to_skip = p_engine->right_mcu_num + p_engine->left_mcu_num;

            // send output to engine user
            rc = p_engine->dest.p_output_handler(p_engine->p_wrapper,
                                            p_engine->dest.p_luma_buffers[p_engine->curr_dest_buf_index],
                                            p_engine->dest.p_chroma_buffers[p_engine->curr_dest_buf_index]);
            // if something goes wrong when sending the output, bail out from the loop and return event error.
            if (JPEG_FAILED(rc))
            {
                return JPEG_EVENT_ERROR;
            }

            if (p_engine->fInputError == 1) return JPEG_EVENT_ERROR;

            // Switch to use the next set of buffer
            p_engine->curr_dest_buf_index = (p_engine->curr_dest_buf_index + 1) % 2;

            // Wait for the now current set of buffer to be ready
            jpeg_buffer_wait_until_empty(p_engine->dest.p_luma_buffers[p_engine->curr_dest_buf_index]);
            jpeg_buffer_wait_until_empty(p_engine->dest.p_chroma_buffers[p_engine->curr_dest_buf_index]);

            // Reload the new buffer addresses
            p_engine->pOutputBufferY    = p_engine->dest.p_luma_buffers[p_engine->curr_dest_buf_index]->ptr;
            p_engine->pOutputBufferCbCr = p_engine->dest.p_chroma_buffers[p_engine->curr_dest_buf_index]->ptr;

            // reset MCU row buffer to beginning of output buffer
            p_engine->pMCULineBufPtrY    = p_engine->pOutputBufferY;
            p_engine->pMCULineBufPtrCbCr = p_engine->pOutputBufferCbCr;

            // reset MCU Row count
            nNumberOfMCUsInRow = 0;

            /**********************************************************************
            * This while loop skips the MCUs in-between 2 consecutive MCU Rows
            * but are outside of the Region
            ***********************************************************************/
            while (num_region_mcu_to_skip > 0 && num_mcu_to_process != 0)
            {
                /* Check abort flag */
                os_mutex_lock(&(p_engine->mutex));
                if (p_engine->abort_flag)
                {
                    p_engine->is_active = false;
                    p_engine->abort_flag = false;
                    os_cond_signal(&(p_engine->cond));
                    os_mutex_unlock(&(p_engine->mutex));
                    return JPEG_EVENT_ABORTED;
                }
                os_mutex_unlock(&(p_engine->mutex));

                if (nSkippedMCUs!=0)
                {
                    jpegd_engine_sw_skip_one_mcu(p_engine);

                    nSkippedMCUs--;
                    num_mcu_to_process--;
                    num_region_mcu_to_skip--;
                }
                else /* if no MCUs are to be skipped */
                {
                    // for each MCU, check if restart marker is expected.
                    if ( (p_engine->nRestartLeft == 0) && (p_engine->nRestartInterval != 0))
                    {
                         // process restart marker
                        if (JPEG_FAILED(jpegd_engine_sw_process_restart_marker(p_engine,
                                                                               &nSkippedMCUs)))
                        {
                            // break out of while loop and exit
                            return JPEG_EVENT_ERROR;
                        }

                        (void)STD_MEMSET(p_engine->lastDcVal, 0, JPEGD_MAXBLOCKSPERMCU * sizeof (uint32_t));
                        p_engine->nRestartLeft = p_engine->nRestartInterval;
                    }
                    else
                    {
                        (void)jpegd_engine_sw_consume_one_mcu(p_engine);

                        p_engine->nRestartLeft--;
                        num_mcu_to_process--;
                        num_region_mcu_to_skip--;
                    }
                }
            } // End the inner while loop
        }
    }

    // If this point is reached, decoding is done successfully
    return JPEG_EVENT_DONE;
}

