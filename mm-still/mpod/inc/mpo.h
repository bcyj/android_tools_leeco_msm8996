/**
 *Copyright (C) 2011 Qualcomm Technologies, Inc.
 *All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
**/
/*=============================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     ------------------------------------------------------------
03/10/11   mingy   Created file.

============================================================================ */

#ifndef _MPO_H
#define _MPO_H

#include "exif.h"
#include "jpeg_common.h"

/* --------------------------------------------------------------------------
** Constant / Define Declarations
** ------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------
** Type Declarations
** ------------------------------------------------------------------------- */

/* MPO Index ifd (opaque definition) */
struct mpo_index_ifd_t;
typedef struct mpo_index_ifd_t *mpo_index_obj_t;

/* MPO attribute ifd (opaque definition) */
struct mpo_attritue_ifd_t;
typedef struct mpo_attribute_ifd_t *mpo_attribute_obj_t;


/* MPO Tag Entry Value*/
typedef struct
{
    /* The Data Type of the Tag *
     * Rational, etc */
    exif_tag_type_t type;

    /* Copy
     * This field is used when a user pass this structure to
     * be stored in an exif_info_t via the exif_set_tag method.
     * The routine would look like this field and decide whether
     * it is necessary to make a copy of the data pointed by this
     * structure (all string and array types).
     * If this field is set to false, only a pointer to the actual
     * data is retained and it is the caller's responsibility to
     * ensure the validity of the data before the exif_info_t object
     * is destroyed.
     */
    uint8_t copy;

    /* Data count
     * This indicates the number of elements of the data. For example, if
     * the type is EXIF_BYTE and the count is 1, that means the actual data
     * is one byte and is accessible by data._byte. If the type is EXIF_BYTE
     * and the count is more than one, the actual data is contained in an
     * array and is accessible by data._bytes. In case of EXIF_ASCII, it
     * indicates the string length and in case of EXIF_UNDEFINED, it indicates
     * the length of the array.
     */
    uint32_t count;

    /* Data
     * A union which covers all possible data types. The user should pick
     * the right field to use depending on the data type and the count.
     * See in-line comment below.
     */
    union
    {
        char      *_ascii;      // EXIF_ASCII (count indicates string length)
        uint8_t   *_bytes;      // EXIF_BYTE  (count > 1)
        uint8_t    _byte;       // EXIF_BYTE  (count = 1)
        uint16_t  *_shorts;     // EXIF_SHORT (count > 1)
        uint16_t   _short;      // EXIF_SHORT (count = 1)
        uint32_t  *_longs;      // EXIF_LONG  (count > 1)
        uint32_t   _long;       // EXIF_LONG  (count = 1)
        rat_t     *_rats;       // EXIF_RATIONAL  (count > 1)
        rat_t      _rat;        // EXIF_RATIONAL  (count = 1)
        uint8_t   *_undefined;  // EXIF_UNDEFINED (count indicates length)
        int32_t   *_slongs;     // EXIF_SLONG (count > 1)
        int32_t    _slong;      // EXIF_SLONG (count = 1)
        srat_t    *_srats;      // EXIF_SRATIONAL (count > 1)
        srat_t     _srat;       // EXIF_SRATIONAL (count = 1)

    } data;

} mpo_tag_entry_value_t;


/* MPO header structure */
typedef struct
{
    jpeg_frm_info_t      thumbnail;
    jpeg_frm_info_t      main;
    exif_info_obj_t      exif_info;
    mpo_index_obj_t      mpo_index_obj;
    mpo_attribute_obj_t  mpo_attribute_obj;

} mpo_hdr_t;

/* MPO Dependent Type */
typedef enum
{
    NON_DEPENDENT_IMAGE    = 0x00000000,   // Non dependent image
    DEPENDENT_CHILD_IMAGE  = 0x40000000,   // Dependent child image flag
    DEPENDENT_PARENT_IMAGE = 0x80000000,   // Dependent parent image flag
    DEPENDENT_MASK         = 0xc0000000,   // Dependent mask

    DEPENDENT_MAX,

} mpo_dependent_t;


/* MPO Representative Type */
typedef enum
{
    NOT_REPRESENTATIVE_IMAGE = 0x00000000,   // Not a representative image
    REPRESENTATIVE_IMAGE     = 0x20000000,   // Representative image flag
    REPRESENTATIVE_MASK      = 0x20000000,   // Representative mask

    REPRESENTATIVE_MAX,

} mpo_representative_t;

/* MPO Image Data Format Type */
typedef enum
{
    JPEG                   = 0x00000000,   // Image is in JPEG format
    NON_JPEG               = 0x07000000,   // Image is not JPEG
    IMAGE_DATA_FORMAT_MASK = 0x07000000,   // Image mask

    IMAGE_DATA_FORMAT_MAX,

} mpo_image_data_format_t;

/* MPO Type */
typedef enum
{
    UNDEFINED              = 0x00000000,   // MP types undefined
    LARGE_TN_CLASS_1       = 0x00010001,   // Large thumbnail class 1 image
    LARGE_TN_CLASS_2       = 0x00010002,   // Large thumbnail class 2 image
    MULTI_VIEW_PANORAMA    = 0x00020001,   // Multi-view Panorama image
    MULTI_VIEW_DISPARITY   = 0x00020002,   // Multi-view Disparity image
    MULTI_VIEW_MULTI_ANGLE = 0x00020003,   // Multi-view Multi-angle image
    BASELINE_PRIMARY       = 0x00030000,   // Baseline MP Primary image
    TYPE_MASK              = 0x00ffffff,   // Type mask

    TYPE_MAX,

} mpo_type_t;


/* Individual image attribute type */
typedef struct
{
    mpo_dependent_t          dependent;       // Parent or child image flag
    mpo_representative_t     representative;  // Representative flag
    mpo_image_data_format_t  format;          // MPO image format
    mpo_type_t               type;            // MPO image type

} individual_image_attribute_t;


/* MPO MP entry type */
typedef struct
{
    // Individual image attribute (4 Bytes)
    individual_image_attribute_t  attribute;

    // Individual image size (4 Bytes)
    uint32_t                      image_size;

    // Individual image data offset (4 Bytes)
    uint32_t                      data_offset;

    // Dependent image x entry number (2 Bytes)
    uint16_t                      dep_image1_entry_num;
    uint16_t                      dep_image2_entry_num;

} mp_entry_t;

/* MPO individual image unique id type */
typedef struct
{
    // Individual image unique id (33 Bytes)
    uint8_t                        bytes[33];

} individual_image_unique_id_t;

#define DEPENDENT(f)          (f) & DEPENDENT_MASK
#define REPRESENTATIVE(f)     (f) & REPRESENTATIVE_MASK
#define JPEG(f)               (f) & IMAGE_DATA_FORMAT_MASK
#define TYPE(f)               (f) & TYPE_MASK

/* =======================================================================
**                          Function Definitions
** ======================================================================= */

/******************************************************************************
 * Function: mpo_index_init
 * Description: Initializes the MPO Index object. Dynamic allocations take
 *              place during the call. One should always call mpo_index_destroy
 *              to clean up the MPO Index object after use.
 * Input parameters:
 *   p_obj - The pointer to the MPO Index object to be initialized.
 *
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EMALLOC
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpo_index_init(mpo_index_obj_t *p_mpo_index_obj);

/******************************************************************************
 * Function: mpo_attribute_init
 * Description: Initializes the MPO Attribute object. Dynamic allocations take
 *              place during the call. One should always call mpo_attribute_destroy
 *              to clean up the MPO Attribute object after use.
 * Input parameters:
 *   p_mpo_attribute_obj - The pointer to the MPO Attribute object to be initialized.
 *
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EMALLOC
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpo_attribute_init(mpo_attribute_obj_t *p_mpo_attribute_obj);

/******************************************************************************
 * Function: mpo_list_index_tagid
 * Description: List all the tags that are present in the MPO index object.
 * Input parameters:
 *   index_obj      - The MPO Info object where the MPO tags should be obtained.
 *   p_tag_ids      - An array to hold the MPO Index Tag IDs to be retrieved.
 *                    It should be allocated by the caller.
 *   len            - The length of the array (the number of elements)
 *   p_len_required - Pointer to the holder to get back the required number of
 *                    elements enough to hold all tag ids present. It can be set
 *                    to NULL if it is not needed.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpo_list_index_tagid(mpo_index_obj_t   index_obj,
                         exif_tag_id_t    *p_tag_ids,
                         uint32_t          len,
                         uint32_t         *p_len_required);

/******************************************************************************
 * Function: mpo_list_attribute_tagid
 * Description: List all the tags that are present in the MPO attribute object.
 * Input parameters:
 *   attribute_obj  - The MPO Info object where the MPO tags should be obtained.
 *   p_tag_ids      - An array to hold the MPO Attribute Tag IDs to be retrieved.
 *                    It should be allocated by the caller.
 *   len            - The length of the array (the number of elements)
 *   p_len_required - Pointer to the holder to get back the required number of
 *                    elements enough to hold all tag ids present. It can be set
 *                    to NULL if it is not needed.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpo_list_attribute_tagid(mpo_attribute_obj_t   attribute_obj,
                             exif_tag_id_t        *p_tag_ids,
                             uint32_t              len,
                             uint32_t             *p_len_required);

/******************************************************************************
 * Function: mpo_get_index_tag
 * Description: Queries the stored MPO tags in the MPO Index object. Typical
 *              use is to obtain an MPO Index object from the MPO Decoder
 *              object which parsed an MPO header then make multiple calls
 *              to this function to individually obtain each tag present in
 *              the header. Note that all memory allocations made to hold the
 *              exif tags are owned by the MPO Index object and they will be
 *              released when mpo_info_destroy is called on the MPO Index object.
 *              Any app needing to preserve those data for later use should
 *              make copies of the tag contents.
 *              If the requested TAG is not present from the file, an
 *              JPEGERR_TAGABSENT will be returned.
 * Input parameters:
 *   index_obj - The MPO Index object where the MPO tags should be obtained.
 *   tag_id    - The MPO Tag ID of the tag to be queried for.
 *   p_entry   - The pointer to the tag entry, filled by the function upon
 *               successful operation. Caller should not pre-allocate memory
 *               to hold the tag contents. Contents in the structure pointed
 *               by p_entry will be overwritten.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 *     JPEGERR_TAGABSENT
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpo_get_index_tag(mpo_index_obj_t    index_obj,
                      exif_tag_id_t      tag_id,
                      exif_tag_entry_t  *p_entry);

/******************************************************************************
 * Function: mpo_get_attribute_tag
 * Description: Queries the stored MPO tags in the MPO Attribute object. Typical
 *              use is to obtain an MPO Attribute object from the MPO Decoder
 *              object which parsed an MPO header then make multiple calls
 *              to this function to individually obtain each tag present in
 *              the header. Note that all memory allocations made to hold the
 *              exif tags are owned by the MPO Attribute object and they will be
 *              released when mpo_info_destroy is called on the MPO Attribute object.
 *              Any app needing to preserve those data for later use should
 *              make copies of the tag contents.
 *              If the requested TAG is not present from the file, an
 *              JPEGERR_TAGABSENT will be returned.
 * Input parameters:
 *   index_obj - The MPO Index object where the MPO tags should be obtained.
 *   tag_id    - The MPO Tag ID of the tag to be queried for.
 *   p_entry   - The pointer to the tag entry, filled by the function upon
 *               successful operation. Caller should not pre-allocate memory
 *               to hold the tag contents. Contents in the structure pointed
 *               by p_entry will be overwritten.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 *     JPEGERR_TAGABSENT
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpo_get_attribute_tag(mpo_attribute_obj_t  attribute_obj,
                          exif_tag_id_t        tag_id,
                          exif_tag_entry_t    *p_entry);

/******************************************************************************
 * Function: mpo_index_destroy
 * Description: Releases all allocated memory made over the lifetime of the
 *              MPO Index object. One should always call this function to clean
 *              up an 'mpo_index_init'-ed MPO Index object.
 * Input parameters:
 *   p_obj - The pointer to the Exif Info object to be destroyed.
 * Return values: None
 * Notes: none
 *****************************************************************************/
void mpo_index_destroy(mpo_index_obj_t *p_mpo_index_obj);

/******************************************************************************
 * Function: mpo_attribute_destroy
 * Description: Releases all allocated memory made over the lifetime of the
 *              MPO Attribute object. One should always call this function to clean
 *              up an 'mpo_attribute_init'-ed MPO Attribute object.
 * Input parameters:
 *   p_obj - The pointer to the Exif Info object to be destroyed.
 * Return values: None
 * Notes: none
 *****************************************************************************/
void mpo_attribute_destroy(mpo_attribute_obj_t *p_mpo_attribute_obj);

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
/* Enum defined to let compiler generate unique offset numbers for different
 * tags - ordering matters! NOT INTENDED to be used by any application. */
typedef enum
{
    // INDEX IFD
    MP_F_VERSION_FIRST = 0,
    NUMBER_OF_IMAGES,
    MP_ENTRY,
    IMAGE_UID_LIST,
    TOTAL_FRAMES,

    INDEX_TAG_MAX

} mpo_index_tag_offset_t;

typedef enum
{
    // ATTRIBUTE IFD
    MP_F_VERSION      = 0,
    MP_INDIVIDUAL_NUM,
    PAN_ORIENTATION,
    PAN_OVERLAP_H,
    PAN_OVERLAP_V,
    BASE_VIEWPOINT_NUM,
    CONVERGENCE_ANGLE,
    BASELINE_LENGTH,
    VERTICAL_DIVERGENCE,
    AXIS_DISTANCE_X,
    AXIS_DISTANCE_Y,
    AXIS_DISTANCE_Z,
    YAW_ANGLE,
    PITCH_ANGLE,
    ROLL_ANGLE,

    ATTRIBUTE_TAG_MAX

} mpo_attribute_tag_offset_t;


// MP Format Version
// Use MPOTAGTYPE_MP_F_VERSION as the exif_tag_type (EXIF_UNDEFINED)
// Count should be 4
#define _ID_MP_F_VERSION_FIRST           0xb000
#define MPOTAGID_MP_F_VERSION_FIRST      CONSTRUCT_TAGID(MP_F_VERSION_FIRST, _ID_MP_F_VERSION_FIRST)
#define MPOTAGTYPE_MP_F_VERSION_FIRST    EXIF_UNDEFINED

// Number of Images
// Use MPOTAGTYPE_NUMBER_OF_IMAGES as the exif_tag_type (EXIF_LONG)
// Count should be 1
#define _ID_NUMBER_OF_IMAGES             0xb001
#define MPOTAGID_NUMBER_OF_IMAGES        CONSTRUCT_TAGID(NUMBER_OF_IMAGES, _ID_NUMBER_OF_IMAGES)
#define MPOTAGTYPE_NUMBER_OF_IMAGES      EXIF_LONG

// MP Entry
// Use MPOTAGTYPE_MP_ENTRY as the exif_tag_type (EXIF_UNDEFINED)
// Count should be 16 x NumberOfImages
#define _ID_MP_ENTRY                     0xb002
#define MPOTAGID_MP_ENTRY                CONSTRUCT_TAGID(MP_ENTRY, _ID_MP_ENTRY)
#define MPOTAGTYPE_MP_ENTRY              EXIF_UNDEFINED

// Individual Image Unique ID List
// Use MPOTAGTYPE_IMAGE_UID_LIST as the exif_tag_type (EXIF_UNDEFINED)
// Count should be 33 x NumberOfImages
#define _ID_IMAGE_UID_LIST               0xb003
#define MPOTAGID_IMAGE_UID_LIST          CONSTRUCT_TAGID(IMAGE_UID_LIST, _ID_IMAGE_UID_LIST)
#define MPOTAGTYPE_IMAGE_UID_LIST        EXIF_UNDEFINED

// Total Number of Camptured Frames
// Use MPOTAGTYPE_TOTAL_FRAMES as the exif_tag_type (EXIF_LONG)
// Count should be 1
#define _ID_TOTAL_FRAMES                 0xb004
#define MPOTAGID_TOTAL_FRAMES            CONSTRUCT_TAGID(TOTAL_FRAMES, _ID_TOTAL_FRAMES)
#define MPOTAGTYPE_TOTAL_FRAMES          EXIF_LONG




// MP Format Version
// Use MPOTAGTYPE_MP_F_VERSION as the exif_tag_type (EXIF_UNDEFINED)
// Count should be 4
#define _ID_MP_F_VERSION                 0xb000
#define MPOTAGID_MP_F_VERSION            CONSTRUCT_TAGID(MP_F_VERSION, _ID_MP_F_VERSION)
#define MPOTAGTYPE_MP_F_VERSION          EXIF_UNDEFINED

// MP Individual Image Number
// Use MPOTAGTYPE_MP_INDIVIDUAL_NUM as the exif_tag_type (EXIF_LONG)
// Count should be 1
#define _ID_MP_INDIVIDUAL_NUM            0xb101
#define MPOTAGID_MP_INDIVIDUAL_NUM       CONSTRUCT_TAGID(MP_INDIVIDUAL_NUM, _ID_MP_INDIVIDUAL_NUM)
#define MPOTAGTYPE_MP_INDIVIDUAL_NUM     EXIF_LONG

// Panorama Scaning Orientation
// Use MPOTAGTYPE_PAN_ORIENTATION as the exif_tag_type (EXIF_LONG)
// Count should be 1
#define _ID_PAN_ORIENTATION              0xb201
#define MPOTAGID_PAN_ORIENTATION         CONSTRUCT_TAGID(PAN_ORIENTATION, _ID_PAN_ORIENTATION)
#define MPOTAGTYPE_PAN_ORIENTATION       EXIF_LONG

// Horizontal Overlap Value
// Use MPOTAGTYPE_PAN_OVERLAP_H as the exif_tag_type (EXIF_RATIONAL)
// Count should be 1
#define _ID_PAN_OVERLAP_H                0xb202
#define MPOTAGID_PAN_OVERLAP_H           CONSTRUCT_TAGID(PAN_OVERLAP_H, _ID_PAN_OVERLAP_H)
#define MPOTAGTYPE_PAN_OVERLAP_H         EXIF_RATIONAL

// Vertical Overlap Value
// Use MPOTAGTYPE_PAN_OVERLAP_V as the exif_tag_type (EXIF_RATIONAL)
// Count should be 1
#define _ID_PAN_OVERLAP_V                0xb203
#define MPOTAGID_PAN_OVERLAP_V           CONSTRUCT_TAGID(PAN_OVERLAP_V, _ID_PAN_OVERLAP_V)
#define MPOTAGTYPE_PAN_OVERLAP_V         EXIF_RATIONAL

// Base Viewpoint Number
// Use MPOTAGTYPE_BASE_VIEWPOINT_NUM as the exif_tag_type (EXIF_LONG)
// Count should be 1
#define _ID_BASE_VIEWPOINT_NUM           0xb204
#define MPOTAGID_BASE_VIEWPOINT_NUM      CONSTRUCT_TAGID(BASE_VIEWPOINT_NUM, _ID_BASE_VIEWPOINT_NUM)
#define MPOTAGTYPE_BASE_VIEWPOINT_NUM    EXIF_LONG

// Convergence Angle
// Use MPOTAGTYPE_CONVERGENCE_ANGLE as the exif_tag_type (EXIF_SRATIONAL)
// Count should be 1
#define _ID_CONVERGENCE_ANGLE            0xb205
#define MPOTAGID_CONVERGENCE_ANGLE       CONSTRUCT_TAGID(CONVERGENCE_ANGLE, _ID_CONVERGENCE_ANGLE)
#define MPOTAGTYPE_CONVERGENCE_ANGLE     EXIF_SRATIONAL

// Baseline Length
// Use MPOTAGTYPE_BASELINE_LENGTH as the exif_tag_type (EXIF_RATIONAL)
// Count should be 1
#define _ID_BASELINE_LENGTH              0xb206
#define MPOTAGID_BASELINE_LENGTH         CONSTRUCT_TAGID(BASELINE_LENGTH, _ID_BASELINE_LENGTH)
#define MPOTAGTYPE_BASELINE_LENGTH       EXIF_RATIONAL

// Divergence Angle
// Use MPOTAGTYPE_VERTICAL_DIVERGENCE as the exif_tag_type (EXIF_SRATIONAL)
// Count should be 1
#define _ID_VERTICAL_DIVERGENCE          0xb207
#define MPOTAGID_VERTICAL_DIVERGENCE     CONSTRUCT_TAGID(VERTICAL_DIVERGENCE, _ID_VERTICAL_DIVERGENCE)
#define MPOTAGTYPE_VERTICAL_DIVERGENCE   EXIF_SRATIONAL

// Horizontal Axis Distance
// Use MPOTAGTYPE_AXIS_DISTANCE_X as the exif_tag_type (EXIF_SRATIONAL)
// Count should be 1
#define _ID_AXIS_DISTANCE_X              0xb208
#define MPOTAGID_AXIS_DISTANCE_X         CONSTRUCT_TAGID(AXIS_DISTANCE_X, _ID_AXIS_DISTANCE_X)
#define MPOTAGTYPE_AXIS_DISTANCE_X       EXIF_SRATIONAL

// Vertical Axis Distance
// Use MPOTAGTYPE_AXIS_DISTANCE_Y as the exif_tag_type (EXIF_SRATIONAL)
// Count should be 1
#define _ID_AXIS_DISTANCE_Y              0xb209
#define MPOTAGID_AXIS_DISTANCE_Y         CONSTRUCT_TAGID(AXIS_DISTANCE_Y, _ID_AXIS_DISTANCE_Y)
#define MPOTAGTYPE_AXIS_DISTANCE_Y       EXIF_SRATIONAL

// Collimation Axis Distance
// Use MPOTAGTYPE_AXIS_DISTANCE_Z as the exif_tag_type (EXIF_SRATIONAL)
// Count should be 1
#define _ID_AXIS_DISTANCE_Z              0xb20a
#define MPOTAGID_AXIS_DISTANCE_Z         CONSTRUCT_TAGID(AXIS_DISTANCE_Z, _ID_AXIS_DISTANCE_Z)
#define MPOTAGTYPE_AXIS_DISTANCE_Z       EXIF_SRATIONAL

// Yaw Angle
// Use MPOTAGTYPE_YAW_ANGLE as the exif_tag_type (EXIF_SRATIONAL)
// Count should be 1
#define _ID_YAW_ANGLE                    0xb20b
#define MPOTAGID_YAW_ANGLE               CONSTRUCT_TAGID(YAW_ANGLE, _ID_YAW_ANGLE)
#define MPOTAGTYPE_YAW_ANGLE             EXIF_SRATIONAL

// Pitch Angle
// Use MPOTAGTYPE_PITCH_ANGLE as the exif_tag_type (EXIF_SRATIONAL)
// Count should be 1
#define _ID_PITCH_ANGLE                  0xb20c
#define MPOTAGID_PITCH_ANGLE             CONSTRUCT_TAGID(PITCH_ANGLE, _ID_PITCH_ANGLE)
#define MPOTAGTYPE_PITCH_ANGLE           EXIF_SRATIONAL

// Roll Angle
// Use MPOTAGTYPE_ROLL_ANGLE as the exif_tag_type (EXIF_SRATIONAL)
// Count should be 1
#define _ID_ROLL_ANGLE                    0xb20d
#define MPOTAGID_ROLL_ANGLE               CONSTRUCT_TAGID(ROLL_ANGLE, _ID_ROLL_ANGLE)
#define MPOTAGTYPE_ROLL_ANGLE             EXIF_SRATIONAL

#endif // #ifndef _MPO_H

