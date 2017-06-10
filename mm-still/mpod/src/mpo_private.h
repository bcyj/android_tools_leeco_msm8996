/**
 *Copyright (C) 2011 Qualcomm Technologies, Inc.
 *All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
**/
/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
03/10/11   mingy   Created file.

========================================================================== */

#ifndef _MPO_PRIVATE_H
#define _MPO_PRIVATE_H

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "exif_private.h"
#include "mpo.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* MPO index IFD type */
typedef struct
{
    exif_tag_entry_ex_t *p_mp_f_version_first;
    exif_tag_entry_ex_t *p_number_of_images;
    exif_tag_entry_ex_t *p_mp_entry;
    exif_tag_entry_ex_t *p_image_uid_list;
    exif_tag_entry_ex_t *p_total_frames;

} mpo_index_ifd_t;

/* MPO attribute IFD type */
typedef struct
{
    exif_tag_entry_ex_t *p_mp_f_version;
    exif_tag_entry_ex_t *p_mp_individual_num;
    exif_tag_entry_ex_t *p_pan_orientation;
    exif_tag_entry_ex_t *p_pan_overlap_h;
    exif_tag_entry_ex_t *p_pan_overlap_v;
    exif_tag_entry_ex_t *p_base_viewpoint_num;
    exif_tag_entry_ex_t *p_convergence_angle;
    exif_tag_entry_ex_t *p_baseline_length;
    exif_tag_entry_ex_t *p_vertical_divergence;
    exif_tag_entry_ex_t *p_axis_distance_x;
    exif_tag_entry_ex_t *p_axis_distance_y;
    exif_tag_entry_ex_t *p_axis_distance_z;
    exif_tag_entry_ex_t *p_yaw_angle;
    exif_tag_entry_ex_t *p_pitch_angle;
    exif_tag_entry_ex_t *p_roll_angle;

} mpo_attribute_ifd_t;

#endif // #ifndef _MPO_PRIVATE_H

