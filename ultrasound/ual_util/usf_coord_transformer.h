/*===========================================================================
                           usf_coord_transformer.h

DESCRIPTION: This header file contains all shared structs, enums and defines
for usf_coord_transformer.cpp

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
#ifndef __USF_COORD_TRANSFORMER_
#define __USF_COORD_TRANSFORMER_

/*-----------------------------------------------------------------------------
  Include Files
-----------------------------------------------------------------------------*/
#include "usf_geometry.h"
#include "usf_epos_defs.h"

/*-----------------------------------------------------------------------------
  Typedefs and Defines
-----------------------------------------------------------------------------*/
/**
 * Contains angle information
 */
struct angle_properties
{
  double sine;
  double cosine;
};

/**
 * Contains information for the creation of the
 * transformation matrix.
 */
struct transformation_properties
{
  Vector origin;
  Matrix transformation_matrix;
  Vector scaling_factor;
  Vector act_zone_area_border;
  uint32_t hover_max_range;
};

/**
 * This enum indicates the current region zone.
 */
enum region_zone_t
{
  NOT_IN_ANY_ACTIVE_ZONE,
  ON_SCREEN_WORK_ZONE,
  OFF_SCREEN_WORK_ZONE,
  ON_SCREEN_ACTIVE_ZONE_BORDER,
  OFF_SCREEN_ACTIVE_ZONE_BORDER
};

/**
  Indicates the zone of a region.
 */
enum zone_t
{
  NOT_ACTIVE_ZONE = 0,
  WORKING_ZONE,
  ACTIVE_ZONE_BORDER
};

/**
  Indicates the side channel region.
 */
enum side_channel_region_t
{
  SIDE_CHANNEL_REGION_ALL = 0,
  SIDE_CHANNEL_REGION_ON_SCREEN,
  SIDE_CHANNEL_REGION_OFF_SCREEN
};

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/
/*==============================================================================
  FUNCTION:  transform_point()
==============================================================================*/
/**
 * Maps a point from rotated source plane in the input coordinate system to
 * screen coordinate system
 *
 * @param in_point the point to map
 * @param out_point the point after mapping.
 * @param on_screen_trans_prop on screen transformation properties needed for
 * the transformation
 * @param off_screen_trans_prop off screen transformation properties needed for
 * the transformation
 *
 * @return region_zone_t
 */
region_zone_t transform_point(usf_extended_epoint_t in_point,
                              usf_extended_epoint_t *out_point,
                              transformation_properties on_screen_trans_prop,
                              transformation_properties off_screen_trans_prop);

/*==============================================================================
  FUNCTION:  rotate_point_about_axis()
==============================================================================*/
/**
 * Rotates the given point in 3 dimensional space by theta about the given axis.
 *
 * @param theta[in] the angle to rotate the point by.
 * @param axis_origin[in] the origin of the rotation axis to rotate the point
 * about.
 * @param axis_direction[in] the direction of the rotation axis to rotate the point
 * about.
 * @param point[in,out] the point to rotate and the result.
 */
void rotate_point_about_axis(const double theta,
                             const Vector axis_origin,
                             const Vector axis_direction,
                             Vector &point);

/*==============================================================================
  FUNCTION:  get_rotated_point()
==============================================================================*/
/**
 * Rotate point to get its coordinate in the rotated plane according to the given
 * rotated plane transformation matrix.
 * Also multiply the point by scaling factor.
 *
 * @param point the point to rotate
 * @param trans_props the transformation properties
 */
zone_t get_rotated_point(Vector &point,
                         const transformation_properties &trans_props);

/*==============================================================================
  FUNCTION:  calculate_rotation_matrix()
==============================================================================*/
/**
 * Calculates the rotation matrix that rotates a point by theta about the given
 * axis.
 *
 * @param theta[in] the angle to rotate the point by.
 * @param axis[in] the axis to rotate he given point about.
 * @param rotation_matrix[out] the rotation matrix
 */
void calculate_rotation_matrix(const double theta,
                               const Vector axis,
                               Matrix &rotation_matrix);

/*==============================================================================
  FUNCTION:  create_transformation_matrix()
==============================================================================*/
/**
 * Builds a transformation matrix from input coordinate system to the rotated
 *plane coordinate system
 *
 * @param plane_properties the rotated plane properties (origin, the end point of the
 * x axis and y axis of the rotated plane - vectors offset in mm from input origin (0,0,0))
 * @param result_matrix output parameter for the transformation matrix.
 */
void create_transformation_matrix(const plane_properties &plane_properties,
                                  Matrix &result_matrix);

/*==============================================================================
  FUNCTION:  calc_transformation_properties()
==============================================================================*/
/**
 * Calculate scaling factors of the given plane and also return the origin of the given plane
 *
 * @param plane_props the rotated plane properties (origin, the end point of the x axis and
 * y axis of the rotated plane.
 * @param act_zone_area_border The vector of distances from the screen draw area which will be considered
 * as active zone border draw area.
 * @param trans_props[out] the transformation properties struct to be filled with suitable values
 */
void calc_transformation_properties(plane_properties plane_props,
                                    Vector act_zone_area_border,
                                    transformation_properties &trans_props);

/*==============================================================================
  FUNCTION:  are_two_planes_parallel()
==============================================================================*/
/**
 * Given three points on each plane, returns whether the two planes are parallel
 * to one another.
 *
 * @param first_plane_p1 A point on the first plane
 * @param first_plane_p2 A point on the first plane
 * @param first_plane_p3 A point on the first plane
 * @param second_plane_p1 A point on the second plane
 * @param second_plane_p2 A point on the second plane
 * @param second_plane_p3 A point on the second plane
 *
 * @return bool true - planes are parallel, false otherwise.
 */
bool are_two_planes_parallel(Vector first_plane_p1,
                             Vector first_plane_p2,
                             Vector first_plane_p3,
                             Vector second_plane_p1,
                             Vector second_plane_p2,
                             Vector second_plane_p3);

#endif // __USF_COORD_TRANSFORMER_
