/*========================================================================


*//** @file exif_private.h

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
07/14/08   vma     Created file.

========================================================================== */

#ifndef _EXIF_PRIVATE_H
#define _EXIF_PRIVATE_H

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "jpeg_common_private.h"
#include "exif.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef enum
{
    EXIF_BIG_ENDIAN,
    EXIF_LITTLE_ENDIAN,

} exif_endianness_t;

/* An extended version of exif_tag_entry_t to hold additional fields
 * that is private to the implementation */
typedef struct
{
    exif_tag_entry_t  entry;
    exif_tag_id_t     tag_id;

} exif_tag_entry_ex_t;

typedef struct
{
    exif_tag_entry_ex_t *p_version_id;
    exif_tag_entry_ex_t *p_latitude_ref;
    exif_tag_entry_ex_t *p_latitude;
    exif_tag_entry_ex_t *p_longitude_ref;
    exif_tag_entry_ex_t *p_longitude;
    exif_tag_entry_ex_t *p_altitude_ref;
    exif_tag_entry_ex_t *p_altitude;
    exif_tag_entry_ex_t *p_timestamp;
    exif_tag_entry_ex_t *p_satellites;
    exif_tag_entry_ex_t *p_status;
    exif_tag_entry_ex_t *p_measure_mode;
    exif_tag_entry_ex_t *p_dop;
    exif_tag_entry_ex_t *p_speed_ref;
    exif_tag_entry_ex_t *p_speed;
    exif_tag_entry_ex_t *p_track_ref;
    exif_tag_entry_ex_t *p_track;
    exif_tag_entry_ex_t *p_img_direction_ref;
    exif_tag_entry_ex_t *p_img_direction;
    exif_tag_entry_ex_t *p_map_datum;
    exif_tag_entry_ex_t *p_dest_latitude_ref;
    exif_tag_entry_ex_t *p_dest_latitude;
    exif_tag_entry_ex_t *p_dest_longitude_ref;
    exif_tag_entry_ex_t *p_dest_longitude;
    exif_tag_entry_ex_t *p_dest_bearing_ref;
    exif_tag_entry_ex_t *p_dest_bearing;
    exif_tag_entry_ex_t *p_dest_distance_ref;
    exif_tag_entry_ex_t *p_dest_distance;
    exif_tag_entry_ex_t *p_processing_method;
    exif_tag_entry_ex_t *p_area_information;
    exif_tag_entry_ex_t *p_datestamp;
    exif_tag_entry_ex_t *p_differential;

} exif_gps_ifd_t;

typedef struct
{
    exif_tag_entry_ex_t *p_new_subfile_type;
    exif_tag_entry_ex_t *p_subfile_type;
    exif_tag_entry_ex_t *p_image_width;
    exif_tag_entry_ex_t *p_image_length;
    exif_tag_entry_ex_t *p_bits_per_sample;
    exif_tag_entry_ex_t *p_compression;
    exif_tag_entry_ex_t *p_photometric_interpretation;
    exif_tag_entry_ex_t *p_thresholding;
    exif_tag_entry_ex_t *p_cellwidth;
    exif_tag_entry_ex_t *p_cellheight;
    exif_tag_entry_ex_t *p_fillorder;
    exif_tag_entry_ex_t *p_document_name;
    exif_tag_entry_ex_t *p_image_description;
    exif_tag_entry_ex_t *p_make;
    exif_tag_entry_ex_t *p_model;
    exif_tag_entry_ex_t *p_strip_offsets;
    exif_tag_entry_ex_t *p_orientation;
    exif_tag_entry_ex_t *p_samples_per_pixel;
    exif_tag_entry_ex_t *p_rows_per_strip;
    exif_tag_entry_ex_t *p_strip_byte_counts;
    exif_tag_entry_ex_t *p_min_sample_value;
    exif_tag_entry_ex_t *p_max_sample_value;
    exif_tag_entry_ex_t *p_x_resolution;
    exif_tag_entry_ex_t *p_y_resolution;
    exif_tag_entry_ex_t *p_planar_configuration;
    exif_tag_entry_ex_t *p_page_name;
    exif_tag_entry_ex_t *p_x_position;
    exif_tag_entry_ex_t *p_y_position;
    exif_tag_entry_ex_t *p_free_offset;
    exif_tag_entry_ex_t *p_free_byte_counts;
    exif_tag_entry_ex_t *p_gray_response_unit;
    exif_tag_entry_ex_t *p_gray_response_curve;
    exif_tag_entry_ex_t *p_t4_option;
    exif_tag_entry_ex_t *p_t6_option;
    exif_tag_entry_ex_t *p_resolution_unit;
    exif_tag_entry_ex_t *p_page_number;
    exif_tag_entry_ex_t *p_transfer_function;
    exif_tag_entry_ex_t *p_software;
    exif_tag_entry_ex_t *p_date_time;
    exif_tag_entry_ex_t *p_artist;
    exif_tag_entry_ex_t *p_host_computer;
    exif_tag_entry_ex_t *p_predictor;
    exif_tag_entry_ex_t *p_white_point;
    exif_tag_entry_ex_t *p_primary_chromaticities;
    exif_tag_entry_ex_t *p_color_map;
    exif_tag_entry_ex_t *p_half_tone_hints;
    exif_tag_entry_ex_t *p_tile_width;
    exif_tag_entry_ex_t *p_tile_length;
    exif_tag_entry_ex_t *p_tile_offset;
    exif_tag_entry_ex_t *p_tile_byte_counts;
    exif_tag_entry_ex_t *p_inkset;
    exif_tag_entry_ex_t *p_inknames;
    exif_tag_entry_ex_t *p_num_of_inks;
    exif_tag_entry_ex_t *p_dot_range;
    exif_tag_entry_ex_t *p_target_printer;
    exif_tag_entry_ex_t *p_extra_samples;
    exif_tag_entry_ex_t *p_sample_format;
    exif_tag_entry_ex_t *p_transfer_range;
    exif_tag_entry_ex_t *p_jpeg_proc;
    exif_tag_entry_ex_t *p_jpeg_interchange_format;
    exif_tag_entry_ex_t *p_jpeg_interchange_format_length;
    exif_tag_entry_ex_t *p_jpeg_restart_interval;
    exif_tag_entry_ex_t *p_jpeg_lossless_pred;
    exif_tag_entry_ex_t *p_jpeg_point_transforms;
    exif_tag_entry_ex_t *p_jpeg_q_tables;
    exif_tag_entry_ex_t *p_jpeg_dc_tables;
    exif_tag_entry_ex_t *p_jpeg_ac_tables;
    exif_tag_entry_ex_t *p_ycbcr_coefficients;
    exif_tag_entry_ex_t *p_ycbcr_sub_sampling;
    exif_tag_entry_ex_t *p_ycbcr_positioning;
    exif_tag_entry_ex_t *p_reference_black_white;
    exif_tag_entry_ex_t *p_gamma;
    exif_tag_entry_ex_t *p_icc_profile_desc;
    exif_tag_entry_ex_t *p_srgb_render;
    exif_tag_entry_ex_t *p_image_title;
    exif_tag_entry_ex_t *p_copyright;
    exif_tag_entry_ex_t *p_exif_ifd;
    exif_tag_entry_ex_t *p_icc_profile;
    exif_tag_entry_ex_t *p_gps_ifd;
} exif_tiff_ifd_t;

typedef struct
{
    exif_tag_entry_ex_t *p_image_width;
    exif_tag_entry_ex_t *p_image_length;
    exif_tag_entry_ex_t *p_bits_per_sample;
    exif_tag_entry_ex_t *p_compression;
    exif_tag_entry_ex_t *p_photometric_interpretation;
    exif_tag_entry_ex_t *p_image_description;
    exif_tag_entry_ex_t *p_make;
    exif_tag_entry_ex_t *p_model;
    exif_tag_entry_ex_t *p_strip_offsets;
    exif_tag_entry_ex_t *p_orientation;
    exif_tag_entry_ex_t *p_samples_per_pixel;
    exif_tag_entry_ex_t *p_rows_per_strip;
    exif_tag_entry_ex_t *p_strip_byte_counts;
    exif_tag_entry_ex_t *p_x_resolution;
    exif_tag_entry_ex_t *p_y_resolution;
    exif_tag_entry_ex_t *p_planar_configuration;
    exif_tag_entry_ex_t *p_resolution_unit;
    exif_tag_entry_ex_t *p_transfer_function;
    exif_tag_entry_ex_t *p_software;
    exif_tag_entry_ex_t *p_date_time;
    exif_tag_entry_ex_t *p_artist;
    exif_tag_entry_ex_t *p_white_point;
    exif_tag_entry_ex_t *p_primary_chromaticities;
    exif_tag_entry_ex_t *p_interchange_format;
    exif_tag_entry_ex_t *p_interchange_format_l;
    exif_tag_entry_ex_t *p_ycbcr_coefficients;
    exif_tag_entry_ex_t *p_ycbcr_sub_sampling;
    exif_tag_entry_ex_t *p_ycbcr_positioning;
    exif_tag_entry_ex_t *p_reference_black_white;
    exif_tag_entry_ex_t *p_copyright;

} exif_thumbnail_ifd_t;

typedef struct
{
    exif_tag_entry_ex_t *p_exposure_time;
    exif_tag_entry_ex_t *p_f_number;
    exif_tag_entry_ex_t *p_exposure_program;
    exif_tag_entry_ex_t *p_spectral_sensitivity;
    exif_tag_entry_ex_t *p_iso_speed_rating;
    exif_tag_entry_ex_t *p_oecf;
    exif_tag_entry_ex_t *p_exif_version;
    exif_tag_entry_ex_t *p_exif_date_time_original;
    exif_tag_entry_ex_t *p_exif_date_time_digitized;
    exif_tag_entry_ex_t *p_exif_components_config;
    exif_tag_entry_ex_t *p_compressed_bits_per_pixel;
    exif_tag_entry_ex_t *p_shutter_speed;
    exif_tag_entry_ex_t *p_aperture;
    exif_tag_entry_ex_t *p_brightness;
    exif_tag_entry_ex_t *p_exposure_bias_value;
    exif_tag_entry_ex_t *p_max_aperture;
    exif_tag_entry_ex_t *p_subject_distance;
    exif_tag_entry_ex_t *p_metering_mode;
    exif_tag_entry_ex_t *p_light_source;
    exif_tag_entry_ex_t *p_flash;
    exif_tag_entry_ex_t *p_focal_length;
    exif_tag_entry_ex_t *p_subject_area;
    exif_tag_entry_ex_t *p_exif_maker_note;
    exif_tag_entry_ex_t *p_exif_user_comment;
    exif_tag_entry_ex_t *p_subsec_time;
    exif_tag_entry_ex_t *p_subsec_time_original;
    exif_tag_entry_ex_t *p_subsec_time_digitized;
    exif_tag_entry_ex_t *p_exif_flashpix_version;
    exif_tag_entry_ex_t *p_exif_color_space;
    exif_tag_entry_ex_t *p_exif_pixel_x_dimension;
    exif_tag_entry_ex_t *p_exif_pixel_y_dimension;
    exif_tag_entry_ex_t *p_related_sound_file;
    exif_tag_entry_ex_t *p_interop;
    exif_tag_entry_ex_t *p_flash_energy;
    exif_tag_entry_ex_t *p_spatial_freq_response;
    exif_tag_entry_ex_t *p_focal_plane_x_resolution;
    exif_tag_entry_ex_t *p_focal_plane_y_resolution;
    exif_tag_entry_ex_t *p_focal_plane_resolution_unit;
    exif_tag_entry_ex_t *p_subject_location;
    exif_tag_entry_ex_t *p_exposure_index;
    exif_tag_entry_ex_t *p_sensing_method;
    exif_tag_entry_ex_t *p_file_source;
    exif_tag_entry_ex_t *p_scene_type;
    exif_tag_entry_ex_t *p_cfa_pattern;
    exif_tag_entry_ex_t *p_custom_rendered;
    exif_tag_entry_ex_t *p_exposure_mode;
    exif_tag_entry_ex_t *p_white_balance;
    exif_tag_entry_ex_t *p_digital_zoom_ratio;
    exif_tag_entry_ex_t *p_focal_length_35mm;
    exif_tag_entry_ex_t *p_scene_capture_type;
    exif_tag_entry_ex_t *p_gain_control;
    exif_tag_entry_ex_t *p_contrast;
    exif_tag_entry_ex_t *p_saturation;
    exif_tag_entry_ex_t *p_sharpness;
    exif_tag_entry_ex_t *p_device_settings_description;
    exif_tag_entry_ex_t *p_subject_distance_range;
    exif_tag_entry_ex_t *p_image_uid;
    exif_tag_entry_ex_t *p_pim;

} exif_exif_ifd_t;

/* The private declaration of the Exif Info structure */
typedef struct exif_info_t
{
    exif_gps_ifd_t        gps_ifd;
    exif_tiff_ifd_t       tiff_ifd;
    exif_thumbnail_ifd_t  thumbnail_ifd;
    exif_exif_ifd_t       exif_ifd;

} exif_info_t;

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
exif_tag_entry_ex_t *exif_create_tag_entry(void);
void exif_destroy_tag_entry(exif_tag_entry_ex_t *);
void exif_add_defaults(exif_info_obj_t obj);

#endif /* #ifndef _EXIF_PRIVATE_H */

