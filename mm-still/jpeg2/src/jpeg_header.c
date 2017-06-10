/*========================================================================

                 C o m m o n   D e f i n i t i o n

*//** @file jpeg_header.c

Copyright (C) 2008-2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
06/30/09   vma     Bug in releasing frame info (causing potential risk
                   in double freeing an allocated memory).
08/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_header.h"
#include "jpeglog.h"

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Function Declarations
** ======================================================================= */

void jpeg_scan_info_destroy(jpeg_scan_info_t *p_info)
{
    if (p_info)
    {
        JPEG_FREE(p_info->p_selectors);
        JPEG_FREE(p_info);
    }
}

void jpeg_frame_info_destroy(jpeg_frame_info_t *p_info)
{
    if (p_info)
    {
        uint8_t i;
        for (i = 0; i < 4; i++)
        {
            JPEG_FREE(p_info->p_qtables[i]);
        }
        if (p_info->pp_scan_infos)
        {
            for (i = 0; i < p_info->num_scans; i++)
            {
                jpeg_scan_info_destroy(p_info->pp_scan_infos[i]);
            }
            JPEG_FREE(p_info->pp_scan_infos);
        }
        if (p_info->p_comp_infos)
        {
            JPEG_FREE(p_info->p_comp_infos);
        }
        JPEG_FREE(p_info);
        p_info = NULL;
    }
}

void jpeg_header_destroy(jpeg_header_t *p_header)
{
    if (p_header)
    {
        exif_info_obj_t exif_info_obj = (exif_info_obj_t)p_header->p_exif_info;

        jpeg_frame_info_destroy(p_header->p_tn_frame_info);
        jpeg_frame_info_destroy(p_header->p_main_frame_info);
        exif_destroy(&exif_info_obj);

        STD_MEMSET(p_header, 0, sizeof(jpeg_header_t));
    }
}

jpeg_scan_info_t *jpeg_add_scan_info(jpeg_frame_info_t *p_frame_info)
{
    // Save the old array
    jpeg_scan_info_t **pp_old_scan_info = p_frame_info->pp_scan_infos;
    // Allocate a new scan info
    jpeg_scan_info_t *p_scan_info = (jpeg_scan_info_t *)JPEG_MALLOC(sizeof(jpeg_scan_info_t));
    if (!p_scan_info)
    {
        return NULL;
    }
    // Resize the scan info array
    p_frame_info->pp_scan_infos = (jpeg_scan_info_t **)JPEG_MALLOC((p_frame_info->num_scans + 1) *
                                                                   sizeof(jpeg_scan_info_t *));
    if (!p_frame_info->pp_scan_infos)
    {
        JPEG_FREE(p_scan_info);
        p_frame_info->pp_scan_infos = pp_old_scan_info;
        return NULL;
    }
    STD_MEMSET(p_scan_info, 0, sizeof(jpeg_scan_info_t));
    STD_MEMMOVE(p_frame_info->pp_scan_infos, pp_old_scan_info,
                p_frame_info->num_scans * sizeof(jpeg_scan_info_t *));

    p_frame_info->pp_scan_infos[p_frame_info->num_scans] = p_scan_info;
    p_frame_info->num_scans++;
    return p_scan_info;
}

#ifdef _DEBUG
void jpeg_dump_qtable(uint16_t *qtbl)
{
    uint32_t i;
    for (i = 0; i < 64; i++)
    {
        JPEG_DBG_LOW("%3d ", qtbl[i]);
        if (i % 16 == 15)
        {
            JPEG_DBG_LOW("\n");
        }
    }
}

void jpeg_dump_htable(jpeg_huff_table_t *htbl)
{
    uint32_t i;
    JPEG_DBG_LOW("Bits: ");
    for (i = 1; i < 17; i++)
    {
        JPEG_DBG_LOW("%2x ", htbl->bits[i]);
    }
    JPEG_DBG_LOW("\nValues:\n");
    for (i = 0; i < 256; i++)
    {
        JPEG_DBG_LOW("%2x ", htbl->values[i]);
        if (i % 32 == 31)
        {
            JPEG_DBG_LOW("\n");
        }
    }
    JPEG_DBG_LOW("\n");
}

void jpeg_dump_entropy_selector(jpeg_comp_entropy_sel_t *p_sel)
{
    JPEG_DBG_LOW("\tcomp id:\t%d\n", p_sel->comp_id);
    JPEG_DBG_LOW("\tdc/ac sel:\t%d/%d\n", p_sel->dc_selector, p_sel->ac_selector);
}
void jpeg_dump_comp_info(jpeg_comp_info_t *p_info)
{
    JPEG_DBG_LOW("comp id:\t%d\n", p_info->comp_id);
    JPEG_DBG_LOW("sampling:\t(%d|%d)\n", p_info->sampling_h, p_info->sampling_v);
    JPEG_DBG_LOW("qtable sel:\t(%d)\n", p_info->qtable_sel);
}

void jpeg_dump_scan_info(jpeg_scan_info_t *p_info)
{
    uint32_t i;
    JPEG_DBG_LOW("offset:\t0x%x\n", p_info->offset);
    JPEG_DBG_LOW("# sel:\t%d\n", p_info->num_selectors);
    JPEG_DBG_LOW("spec start/end:\t(%d/%d)\n", p_info->spec_start, p_info->spec_end);
    JPEG_DBG_LOW("succ approx h/l:\t(%d/%d)\n", p_info->succ_approx_h, p_info->succ_approx_l);
    for (i = 0; i < p_info->num_selectors; i++)
    {
        jpeg_dump_entropy_selector(p_info->p_selectors + i);
    }
}

void jpeg_dump_frame_info(jpeg_frame_info_t *p_info)
{
    if (p_info)
    {
        uint32_t i;
        JPEG_DBG_LOW("Width:\t\t%d\n", p_info->width);
        JPEG_DBG_LOW("Height:\t\t%d\n", p_info->height);
        JPEG_DBG_LOW("Restart:\t%d\n", p_info->restart_interval);
        JPEG_DBG_LOW("Process:\t%d\n", p_info->process);
        JPEG_DBG_LOW("Precision:\t%d\n", p_info->precision);
        JPEG_DBG_LOW("Q precision:\t%d\n", p_info->quant_precision);
        JPEG_DBG_LOW("# comps:\t%d\n", p_info->num_comps);
        JPEG_DBG_LOW("# scans:\t%d\n", p_info->num_scans);
        JPEG_DBG_LOW("Q present flag:\t%x\n", p_info->qtable_present_flag);
        JPEG_DBG_LOW("H present flag:\t%x\n", p_info->htable_present_flag);
        for (i = 0; i < 4; i++)
        {
            if (p_info->qtable_present_flag & (1 << i))
            {
                JPEG_DBG_LOW("Q Table %d:\n", i+1);
                jpeg_dump_qtable(p_info->p_qtables[i]);
            }
        }
        for (i = 0; i < 8; i++)
        {
            if (p_info->htable_present_flag & (1 << i))
            {
                JPEG_DBG_LOW("H Table %d:\n", i+1);
                jpeg_dump_htable(&p_info->p_htables[i]);
            }
        }
        for (i = 0; i < p_info->num_comps; i++)
        {
            JPEG_DBG_LOW("Comp info %d:\n", i+1);
            jpeg_dump_comp_info(p_info->p_comp_infos + i);
        }
        for (i = 0; i < p_info->num_scans; i++)
        {
            JPEG_DBG_LOW("Scan info %d:\n", i+1);
            jpeg_dump_scan_info(p_info->pp_scan_infos[i]);
        }
    }
}
void jpeg_dump_header(jpeg_header_t *p_header)
{
    if (p_header)
    {
        JPEG_DBG_LOW("Dump thumbnail frame info:\n");
        jpeg_dump_frame_info(p_header->p_tn_frame_info);
        JPEG_DBG_LOW("Dump main frame info:\n");
        jpeg_dump_frame_info(p_header->p_main_frame_info);
    }
}
#endif // #ifdef _DEBUG

