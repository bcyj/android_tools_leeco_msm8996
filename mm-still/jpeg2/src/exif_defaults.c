/*========================================================================


*//** @file exif_defaults.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
07/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpegerr.h"
#include "exif_private.h"

#undef JDBG
#undef LOG_TAG
#undef LOG_NIDEBUG
#define LOG_NIDEBUG 0
#define LOG_TAG "mm-still"
#include <utils/Log.h>

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
static uint8_t  default_exif_version[] = {0x30, 0x32, 0x32, 0x30};
static uint8_t  default_flash_pix_version[] = {0x30, 0x31, 0x30, 0x30};
static uint8_t  default_r98_version[] = {0x30, 0x31, 0x30, 0x30};
static uint8_t  default_components_config[] = {1, 2, 3, 0};
static uint16_t default_resolution_unit = 2;
static uint16_t default_compression = 6; // 1 for uncompressed, 6 for Jpeg compression
static uint16_t default_ycbcr_positioning = 1; // 1 for CENTER
static uint16_t default_color_space = 1; // 1 for SRGB
static rat_t  default_exif_resolution = {0x48, 1};

static exif_tag_entry_ex_t default_tag_make =
{
    {
        EXIF_ASCII,  // type
        0,           // copy
        8,           // count
        {"QCOM-AA"}, // data._ascii (initialization applies
                     // to first member of the union)
    }, // entry
    EXIFTAGID_MAKE,
};
static exif_tag_entry_ex_t default_tag_model =
{
    {
        EXIF_ASCII,   // type
        0,            // copy
        8,            // count
        {"QCAM-AA"},  // data._ascii (initialization applies
                      // to first member of the union)
    }, // entry
    EXIFTAGID_MODEL
};
static exif_tag_entry_ex_t default_tag_datetime_original =
{
    {
        EXIF_ASCII, // type
        0,          // copy
        20,         // count
        {"2002:12:08 12:00:00"},  // data._ascii (initialization applies
                                  // to first member of the union)
    }, // entry
    EXIFTAGID_EXIF_DATE_TIME_ORIGINAL
};
static exif_tag_entry_ex_t default_tag_datetime_digitized =
{
    {
        EXIF_ASCII, // type
        0,          // copy
        20,         // count
        {"2002:12:08 12:00:00"},  // data._ascii (initialization applies
                                  // to first member of the union)
    }, // entry
    EXIFTAGID_EXIF_DATE_TIME_DIGITIZED
};
exif_tag_entry_ex_t default_tag_interopindexstr =
{
    {
        EXIF_ASCII, // type
        0,          // copy
        4,          // count
        {"R98"},    // data._ascii (initialization applies
                    // to first member of the union)
    }, // entry
    CONSTRUCT_TAGID(EXIF_TAG_MAX_OFFSET, 0x0001)
};
static exif_tag_entry_ex_t default_tag_exif_version =
{
    {
        EXIF_UNDEFINED,          // type
        0,                       // copy
        4,                       // count
        {(char*)default_exif_version},  // data._ascii (initialization applies
                                 // to first member of the union)
    }, // entry
    EXIFTAGID_EXIF_VERSION
};
static exif_tag_entry_ex_t default_tag_flash_pix_version =
{
    {
        EXIF_UNDEFINED,              // type
        0,                           // copy
        4,                           // count
        {(char*)default_flash_pix_version}, // data._ascii (initialization applies
                                     // to first member of the union)
    }, // entry
    EXIFTAGID_EXIF_FLASHPIX_VERSION
};
exif_tag_entry_ex_t default_tag_r98_version =
{
    {
        EXIF_UNDEFINED,        // type
        0,                     // copy
        4,                     // count
        {(char*)default_r98_version}, // data._ascii (initialization applies
                               // to first member of the union)
    }, // entry
    CONSTRUCT_TAGID(EXIF_TAG_MAX_OFFSET, 0x0002)
};
static exif_tag_entry_ex_t default_tag_components_config =
{
    {
        EXIF_UNDEFINED,              // type
        0,                           // copy
        4,                           // count
        {(char*)default_components_config}, // data._ascii (initialization applies
                                     // to first member of the union)
    }, // entry
    EXIFTAGID_EXIF_COMPONENTS_CONFIG
};
static exif_tag_entry_ex_t default_tag_resolution_unit =
{
    {
        EXIF_SHORT,  // type
        0,           // copy
        1,           // count
        {(char*)0},  // data._ascii (initialization applies
                     // to first member of the union)
                     // needs to be properly initialized by code
    }, // entry
    EXIFTAGID_RESOLUTION_UNIT
};
static exif_tag_entry_ex_t default_tag_tn_resolution_unit =
{
    {
        EXIF_SHORT,  // type
        0,           // copy
        1,           // count
        {(char*)0},  // data._ascii (initialization applies
                     // to first member of the union)
                     // needs to be properly initialized by code
    }, // entry
    EXIFTAGID_TN_RESOLUTION_UNIT
};
static exif_tag_entry_ex_t default_tag_tn_compression =
{
    {
        EXIF_SHORT,  // type
        0,           // copy
        1,           // count
        {(char*)0},  // data._ascii (initialization applies
                     // to first member of the union)
                     // needs to be properly initialized by code
    }, // entry
    EXIFTAGID_TN_COMPRESSION
};
static exif_tag_entry_ex_t default_tag_exif_x_resolution =
{
    {
        EXIF_RATIONAL, // type
        0,             // copy
        1,             // count
        {(char*)0},    // data._ascii (initialization applies
                       // to first member of the union)
                       // needs to be properly initialized by code
    }, // entry
    EXIFTAGID_X_RESOLUTION
};
static exif_tag_entry_ex_t default_tag_exif_y_resolution =
{
    {
        EXIF_RATIONAL, // type
        0,             // copy
        1,             // count
        {(char*)0},    // data._ascii (initialization applies
                       // to first member of the union)
                       // needs to be properly initialized by code
    }, // entry
    EXIFTAGID_Y_RESOLUTION
};
static exif_tag_entry_ex_t default_tag_tn_exif_x_resolution =
{
    {
        EXIF_RATIONAL, // type
        0,             // copy
        1,             // count
        {(char*)0},    // data._ascii (initialization applies
                       // to first member of the union)
                       // needs to be properly initialized by code
    }, // entry
    EXIFTAGID_TN_X_RESOLUTION
};
static exif_tag_entry_ex_t default_tag_tn_exif_y_resolution =
{
    {
        EXIF_RATIONAL, // type
        0,             // copy
        1,             // count
        {(char*)0},    // data._ascii (initialization applies
                       // to first member of the union)
                       // needs to be properly initialized by code
    }, // entry
    EXIFTAGID_TN_Y_RESOLUTION
};
static exif_tag_entry_ex_t default_tag_ycbcr_positioning =
{
    {
        EXIF_SHORT,    // type
        0,             // copy
        1,             // count
        {(char*)0},    // data._ascii (initialization applies
                       // to first member of the union)
                       // needs to be properly initialized by code
    }, // entry
    EXIFTAGID_YCBCR_POSITIONING
};
static exif_tag_entry_ex_t default_tag_color_space =
{
    {
        EXIF_SHORT,    // type
        0,             // copy
        1,             // count
        {(char*)0},    // data._ascii (initialization applies
                       // to first member of the union)
                       // needs to be properly initialized by code
    }, // entry
    EXIFTAGID_EXIF_COLOR_SPACE
};

static exif_tag_entry_ex_t* default_tags[] =
{
    &default_tag_make,
    &default_tag_model,
    &default_tag_datetime_original,
    &default_tag_datetime_digitized,
    &default_tag_exif_version,
    &default_tag_flash_pix_version,
    &default_tag_components_config,
    &default_tag_resolution_unit,
    &default_tag_tn_resolution_unit,
    &default_tag_tn_compression,
    &default_tag_tn_exif_x_resolution,
    &default_tag_tn_exif_y_resolution,
    &default_tag_exif_x_resolution,
    &default_tag_exif_y_resolution,
    &default_tag_ycbcr_positioning,
    &default_tag_color_space
};

void exif_add_defaults(exif_info_obj_t obj)
{
    uint16_t i;
    default_tag_resolution_unit.entry.data._short = default_resolution_unit;
    default_tag_tn_resolution_unit.entry.data._short = default_resolution_unit;
    default_tag_tn_compression.entry.data._short = default_compression;
    default_tag_exif_x_resolution.entry.data._rat = default_exif_resolution;
    default_tag_exif_y_resolution.entry.data._rat = default_exif_resolution;
    default_tag_tn_exif_x_resolution.entry.data._rat = default_exif_resolution;
    default_tag_tn_exif_y_resolution.entry.data._rat = default_exif_resolution;
    default_tag_ycbcr_positioning.entry.data._short = default_ycbcr_positioning;
    default_tag_color_space.entry.data._short = default_color_space;
    for (i = 0; i < sizeof(default_tags)/sizeof(exif_tag_entry_ex_t*); i++)
    {
        exif_set_tag(obj, default_tags[i]->tag_id, &(default_tags[i]->entry));
    }
}
