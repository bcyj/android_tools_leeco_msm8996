/*===========================================================================
                           usf_coord_transformer.cpp

DESCRIPTION: This header file contains transformation functions and
operations.

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
/*-----------------------------------------------------------------------------
  Include Files
-----------------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include "usf_coord_transformer.h"
/*----------------------------------------------------------------------------
  Defines
----------------------------------------------------------------------------*/
/**
 * Number of "0.01 mm" in 1 mm
 */
#define CMM_PER_MM 100
/*-----------------------------------------------------------------------------
  Typedefs
-----------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/
/*==============================================================================
  FUNCTION:  get_zone()
==============================================================================*/
/**
 * Returns the zone of the point.
 *
 * @param point the point to check
 * @param trans_props the transformation properties
 *
 * @return zone_t
 */
static zone_t get_zone(const Vector &point,
                       transformation_properties trans_props);

/*==============================================================================
  FUNCTION:  rotate_point()
==============================================================================*/
/**
 * Rotate a point to get its coordinates in the rotated plane according to the
 * given rotated plane transformation matrix.
 *
 * @param point the point to rotate
 * @param rotated_origin the rotated plane origin - vector in input units from
 * input origin (0,0,0)
 * @param transformation_matrix the transformation matrix to perform the rotation with
 */
static void rotate_point(Vector &point,
                         const Vector &rotated_origin,
                         const Matrix &transformation_matrix);

/*==============================================================================
  FUNCTION:  rotate_tilt()
==============================================================================*/
/**
 * Rotate tilt according to the rotated plane transformation matrix
 *
 * @param tilt the pen tilt value to rotate
 * @param rotatedScreenOrigin the rotated plane origin
 * @param transformation_matrix the transformation matrix to perform the rotation with
 */
void rotate_tilt(Vector &tilt,
                 const Vector &rotatedPlaneOrigin,
                 const Matrix &transformation_matrix);

/*==============================================================================
  FUNCTION:  calc_sine_cosine()
==============================================================================*/
/**
 * Calculate sine and cosine of the angle between the two vectors A and B.
 * The angle is measured from A to B using the "right hand rule".
 * The order between the vectors is important and determines the order
 * between the vectors in vector product.
 *
 * @param vector_a the first vector
 * @param vector_b the second vector
 * @param normal the normal to the plane that contains A and B
 * @param res_sine_cosine the sine and cosine result
 */
void calc_sine_cosine(const Vector &vector_a,
                      const Vector &vector_b,
                      const Vector &normal,
                      angle_properties &res_sine_cosine);

/*==============================================================================
  FUNCTION:  calc_transformation_properties()
==============================================================================*/
/**
 * See function definition in header file.
 */
void calc_transformation_properties(plane_properties plane_props,
                                    Vector act_zone_area_border,
                                    transformation_properties &trans_props)
{
  // Calculate screen width and height in mm
  plane_props.point_end_x.vector_substract(plane_props.origin);
  double width_mm = plane_props.point_end_x.get_length();
  plane_props.point_end_y.vector_substract(plane_props.origin);
  double height_mm = plane_props.point_end_y.get_length();

  // Calculate scaling factors
  trans_props.scaling_factor.set_element(X,
                                         (TSC_LOGICAL_MAX_X / width_mm) / CMM_PER_MM);
  trans_props.scaling_factor.set_element(Y,
                                         (TSC_LOGICAL_MAX_Y / height_mm) / CMM_PER_MM);
  trans_props.scaling_factor.set_element(Z,
                                         -1.0);

  // Calculate active zone distances
  act_zone_area_border.vector_multiply_element_by_element(trans_props.scaling_factor);
  trans_props.act_zone_area_border = act_zone_area_border;
  trans_props.act_zone_area_border.set_element(Z, -act_zone_area_border.get_element(Z));

  // Store hover max range
  trans_props.hover_max_range = plane_props.hover_max_range;

  // Calculate plane origin in EPOS units as vector offset from EPOS origin (0,0,0)
  trans_props.origin = plane_props.origin;
  trans_props.origin.vector_mult_by_scalar(CMM_PER_MM);
}

/*==============================================================================
  FUNCTION:  transform_point()
==============================================================================*/
/**
 * See function definition in header file.
 */
region_zone_t transform_point(usf_extended_epoint_t in_point,
                              usf_extended_epoint_t *out_point,
                              transformation_properties on_screen_trans_prop,
                              transformation_properties off_screen_trans_prop)
{
  /**
  * Algorithm description: rotates the raw point according to the
  * priority list below and checks whether the point is in the
  * drawing area of that plane. Then it rotates the tilt according
  * to the plane.
  *
  * This is the priority reporting for points in overlapping
  * areas:
  * 1. On screen zone
  * 2. Off screen zone
  * 3. On screen active zone border
  * 4. Off screen active zone border.
  * 5. Not on any active zone
  */
  Vector on_trans_point(in_point.X,
                        in_point.Y,
                        in_point.Z);
  Vector off_trans_point = on_trans_point;
  Vector tilt(in_point.TiltX,
              in_point.TiltY,
              in_point.TiltZ);
  tilt.vector_mult_by_scalar(1.0 / TILT_Q_FACTOR);

  region_zone_t result = NOT_IN_ANY_ACTIVE_ZONE;

  // Apply on-screen rotation on point and multiply by on screen scaling factors
  zone_t on_screen_zone = get_rotated_point(on_trans_point,
                                            on_screen_trans_prop);
  if (WORKING_ZONE == on_screen_zone)
  {
    result = ON_SCREEN_WORK_ZONE;
  }
  else
  {
    // Apply off-screen rotation on point and multiply by off screen scaling factors
    zone_t off_screen_zone = get_rotated_point(off_trans_point,
                                               off_screen_trans_prop);

    if (WORKING_ZONE == off_screen_zone)
    {
      result = OFF_SCREEN_WORK_ZONE;
    }
    else if (ACTIVE_ZONE_BORDER == on_screen_zone)
    {
      result = ON_SCREEN_ACTIVE_ZONE_BORDER;
    }
    else if (ACTIVE_ZONE_BORDER == off_screen_zone)
    {
      result = OFF_SCREEN_ACTIVE_ZONE_BORDER;
    }
    else
    {
      // Point is invalid
      result = NOT_IN_ANY_ACTIVE_ZONE;
    }
  }
  Vector result_point;
  // Apply suitable tilt rotation
  if (ON_SCREEN_WORK_ZONE == result)
  {
    rotate_tilt(tilt,
                on_screen_trans_prop.origin,
                on_screen_trans_prop.transformation_matrix);
    result_point = on_trans_point;
  }
  else
  {
    rotate_tilt(tilt,
                off_screen_trans_prop.origin,
                off_screen_trans_prop.transformation_matrix);
    result_point = off_trans_point;
  }

  out_point->X = result_point.get_element(X);
  out_point->Y = result_point.get_element(Y);
  out_point->Z = result_point.get_element(Z);
  out_point->TiltX = tilt.get_element(X);
  out_point->TiltY = tilt.get_element(Y);
  // When point is transformed, tiltz has no meaning. Therefore resetting it.
  out_point->TiltZ = 0;
  out_point->Type = in_point.Type;
  out_point->P = in_point.P;

  return result;
}

/*==============================================================================
  FUNCTION:  rotate_point_about_axis()
==============================================================================*/
/**
 * See function definition in header file.
 */
void rotate_point_about_axis(const double theta,
                             Vector axis_origin,
                             Vector axis_direction,
                             Vector &point)
{
  if (0 == theta)
  { // No rotation is required.
    return;
  }

  // Move point and axis to start from the same point.
  point.vector_substract(axis_origin);

  // Calculate the rotation matrix
  Matrix rotation_matrix;
  calculate_rotation_matrix(theta,
                            axis_direction,
                            rotation_matrix);

  // Rotate point by the rotation matrix
  point.vector_mult_by_matrix(rotation_matrix);

  // Return point to start from Epos origin again.
  point.vector_add(axis_origin);
}

/*==============================================================================
  FUNCTION:  calculate_rotation_matrix()
==============================================================================*/
/**
 * See function definition in header file.
 */
void calculate_rotation_matrix(const double theta,
                               Vector axis,
                               Matrix &rotation_matrix)
{
  Matrix unit(Vector(1.0, 0.0, 0.0),
              Vector(0.0, 1.0, 0.0),
              Vector(0.0, 0.0, 1.0));
  /** Calculate x rotation matrix */
  // Calculate sine cosine between the projection of the rotation axis
  // onto yz plane and the z axis.
  Vector axis_yz_proj(0.0,
                      axis.get_element(Y),
                      axis.get_element(Z));
  Vector z_axis(0.0, 0.0, 1.0);
  Matrix rx = unit, rx_1 = unit;
  if (axis_yz_proj.get_length())
  {
    Vector nx(1.0, 0.0, 0.0);
    angle_properties x_rotation;
    calc_sine_cosine(axis_yz_proj,
                     z_axis,
                     nx,
                     x_rotation);

    // X rotation matrix
    rx = Matrix(Vector(1.0, 0.0,               0.0),
                Vector(0.0, x_rotation.cosine, -x_rotation.sine),
                Vector(0.0, x_rotation.sine,   x_rotation.cosine));
    rx_1 = Matrix(Vector(1.0, 0.0,               0.0),
                  Vector(0.0, x_rotation.cosine, x_rotation.sine),
                  Vector(0.0,-x_rotation.sine,   x_rotation.cosine));
  }

  /** Calculate y rotation matrix */
  // Calculate sine cosine between rotation axis rotated about z axis,
  // and the z axis.
  axis.vector_mult_by_matrix(rx);
  Vector ny(0.0, 1.0, 0.0);
  angle_properties y_rotation;
  calc_sine_cosine(axis,
                   z_axis,
                   ny,
                   y_rotation);
  // Y rotation matrix
  Matrix ry(Vector(y_rotation.cosine, 0.0, y_rotation.sine),
            Vector(0.0,               1.0, 0.0),
            Vector(-y_rotation.sine,   0.0, y_rotation.cosine));
  Matrix ry_1 (Vector(y_rotation.cosine, 0.0, -y_rotation.sine),
               Vector(0.0,               1.0, 0.0),
               Vector(y_rotation.sine,   0.0, y_rotation.cosine));

  /** Calculate Z rotation matrix */
  // Z rotation matrix
  Matrix rz(Vector(cos(theta), -sin(theta), 0.0),
            Vector(sin(theta), cos(theta),  0.0),
            Vector(0.0,        0.0,         1.0));

  /** Calculate the complete rotation matrix */
  Matrix rotaion_matrix;
  // The complete matrix: rx_1 * ry_1 * rz * ry * rx
  Matrix temp_matrix, temp2_matrix, temp3_matrix;
  rx_1.matrix_by_matrix_multiply(ry_1, temp_matrix);
  temp_matrix.matrix_by_matrix_multiply(rz, temp2_matrix);
  temp2_matrix.matrix_by_matrix_multiply(ry, temp3_matrix);
  temp3_matrix.matrix_by_matrix_multiply(rx, rotation_matrix);
}

/*==============================================================================
  FUNCTION:  get_zone()
==============================================================================*/
/**
 * See function definition above.
 */
zone_t get_zone(const Vector &point,
                transformation_properties trans_props)
{
  if (point.get_element(X) >= 0.0 &&
      point.get_element(X) <= TSC_LOGICAL_MAX_X &&
      point.get_element(Y) >= 0.0 &&
      point.get_element(Y) <= TSC_LOGICAL_MAX_Y &&
      point.get_element(Z) <= trans_props.hover_max_range)
  {
    return WORKING_ZONE;
  }

  if (point.get_element(X) >=
      (0.0 - trans_props.act_zone_area_border.get_element(X)) &&
      point.get_element(X) <=
      (TSC_LOGICAL_MAX_X + trans_props.act_zone_area_border.get_element(X)) &&
      point.get_element(Y) >=
      (0.0 - trans_props.act_zone_area_border.get_element(Y)) &&
      point.get_element(Y) <=
      (TSC_LOGICAL_MAX_Y + trans_props.act_zone_area_border.get_element(Y)) &&
      point.get_element(Z) <=
      (trans_props.hover_max_range + trans_props.act_zone_area_border.get_element(Z)))
  {
    return ACTIVE_ZONE_BORDER;
  }

  return NOT_ACTIVE_ZONE;
}

/*==============================================================================
  FUNCTION:  rotate_point()
==============================================================================*/
/**
 * See function definition above.
 */
void rotate_point(Vector &point,
                 const Vector &rotated_origin,
                 const Matrix &transformation_matrix)
{
  // Move the rotated coordinate system to have the same origin as input coordinate system
  point.vector_substract(rotated_origin);
  // Perform transformation from EPOS coordinate system to the rotated coordinate system
  point.vector_mult_by_matrix(transformation_matrix);
}

/*==============================================================================
  FUNCTION:  get_rotated_point()
==============================================================================*/
/**
 * See function definition above.
 */
zone_t get_rotated_point(Vector &point,
                         const transformation_properties &trans_props)
{
  // Rotate point according to plane rotation
  rotate_point(point,
               trans_props.origin,
               trans_props.transformation_matrix);
  // Multiply the point by plane scaling factors
  point.vector_multiply_element_by_element(trans_props.scaling_factor);

  return get_zone(point,
                  trans_props);
}

/*==============================================================================
  FUNCTION:  rotate_tilt()
==============================================================================*/
/**
 * See function definition above.
 */
void rotate_tilt(Vector &tilt,
                 const Vector &rotatedPlaneOrigin,
                 const Matrix &transformation_matrix)
{
  Vector tiltVector = tilt;
  Vector scaling_factor(1,1,-1);
  // Library refers to the tip of the pen as the tilt vector
  // direction. Therefore need to invert the direction.
  tiltVector.vector_mult_by_scalar(-1);
  // Rotation of the tilt vector
  tiltVector.vector_add(rotatedPlaneOrigin);
  rotate_point(tiltVector,
               rotatedPlaneOrigin,
               transformation_matrix);
  tiltVector.vector_multiply_element_by_element(scaling_factor);

  tilt = tiltVector.get_tilt_angle_vector();
}

/*==============================================================================
  FUNCTION:  create_transformation_matrix()
==============================================================================*/
/**
 * See function definition in header file.
 */
void create_transformation_matrix(const plane_properties &plane_properties,
                                  Matrix &result_matrix)
{
  // Move the rotated coordinate system to have the same origin as EPOS coordinate system
  Vector point_end_x = plane_properties.point_end_x;
  Vector point_end_y = plane_properties.point_end_y;

  point_end_x.vector_substract(plane_properties.origin);
  point_end_y.vector_substract(plane_properties.origin);

  // Find the normal (not normalized) of the rotated plane nr=(nrx,nry,nrz) from
  // (point_end_x x point_end_y)
  Vector nr;
  nr.calc_normal(point_end_x, point_end_y);

  // EPOS plane normal ne=(nex,ney,nez)
  Vector ne(0.0,
            0.0,
            1.0);

  // Find psi (third Eulerian angle) : this is the angle between point_end_x vector to the vector (1,?,0)
  // and y can be found from nrx*(x=1)+nry*(y=?)+nrz*(z=0)=0 => y=(-nrx-nrz*0)/nry
  Vector new_axis_x;
  if (0 == point_end_x.get_element(Z))
  {
    new_axis_x = point_end_x;
  }
  else
  {
    new_axis_x.set_element(X, 1.0);
    new_axis_x.set_element(Y, (-nr.get_element(X)-nr.get_element(Z)*0.0) / nr.get_element(Y));
    new_axis_x.set_element(Z, 0.0);
  }

  angle_properties psi;
  calc_sine_cosine(new_axis_x,
                   point_end_x,
                   nr,
                   psi);

  // Find teta (second Eulerian angel) : this is the angle between EPOS plane to the rotated plane
  angle_properties teta;
  calc_sine_cosine(ne,
                   nr,
                   new_axis_x,
                   teta);

  // Find phi (first Eulerian angle) : this is the angle between the new X axis to EPOS X axis
  Vector old_axis_x(1.0,0.0,0.0);
  angle_properties phi;
  calc_sine_cosine(old_axis_x,
                   new_axis_x,
                   ne,
                   phi);

  // First matrix (of psi) M(psi)
  Matrix psi_matrix(Vector(psi.cosine,
                          psi.sine,
                          0.0),
                   Vector(-psi.sine,
                          psi.cosine,
                          0.0),
                   Vector(0.0,
                          0.0,
                          1.0));

  // Second matrix (of teta) M(teta)
  Matrix teta_matrix(Vector(1.0,
                           0.0,
                           0.0),
                    Vector(0.0,
                           teta.cosine,
                           teta.sine),
                    Vector(0.0,
                           -teta.sine,
                           teta.cosine));

  // Third matrix (of phi) M(phi)
  Matrix phi_matrix(Vector(phi.cosine,
                          phi.sine,
                          0.0),
                   Vector(-phi.sine,
                          phi.cosine,
                          0.0),
                   Vector(0.0,
                          0.0,
                          1.0));

  // The complete matrix M(phi,teta,psi) : M(psi)*M(teta)*M(phi)
  Matrix temp_matrix;
  psi_matrix.matrix_by_matrix_multiply(teta_matrix,
                                   temp_matrix);
  temp_matrix.matrix_by_matrix_multiply(phi_matrix,
                                    result_matrix);
  LOGD("Transformation Matrix:");
  result_matrix.print_matrix();
}

/*==============================================================================
  FUNCTION:  print_matrix()
==============================================================================*/
/**
 * See function definition in header file.
 */
void print_matrix (Matrix mat)
{
  for (int i = 0; i < NUM_DIMENSIONS; i++)
  {
    for (int j = 0; j < TRANSFORM_POINTS_NUM; j++)
    {
      LOGD("%s: (%d,%d) = %lf",
           __FUNCTION__,
           i,
           j,
           mat.get_element(j,
                           i));
    }
  }
}

/*==============================================================================
  FUNCTION:  calc_sine_cosine()
==============================================================================*/
/**
 * See function definition above.
 */
void calc_sine_cosine(const Vector &vector_a,
                      const Vector &vector_b,
                      const Vector &normal,
                      angle_properties &res_sine_cosine)
{
  Vector v3;
  v3.calc_normal(normal, vector_a);

  double cosine = vector_a.dot_product(vector_b) / (vector_a.get_length() * vector_b.get_length());
  double dot_product = v3.dot_product(vector_b);

  double angle = acos(cosine);

  // Find angle sign : equals to the sign of ((normal x va) * vb) and normal is the
  // normal to the plane that contains va and vb
  if (0.0 > dot_product)
  {
    angle = -angle;
  }

  res_sine_cosine.cosine = cos(angle);
  res_sine_cosine.sine = sin(angle);
}

/*==============================================================================
  FUNCTION:  are_two_planes_parallel()
==============================================================================*/
/**
 * See function definition in header file.
 */
bool are_two_planes_parallel(Vector first_plane_p1,
                             Vector first_plane_p2,
                             Vector first_plane_p3,
                             Vector second_plane_p1,
                             Vector second_plane_p2,
                             Vector second_plane_p3)
{
  first_plane_p2.vector_substract(first_plane_p1);
  first_plane_p3.vector_substract(first_plane_p1);

  Vector first_plane_normal;
  first_plane_normal.calc_normal(first_plane_p2,
                                 first_plane_p3);

  second_plane_p2.vector_substract(second_plane_p1);
  second_plane_p3.vector_substract(second_plane_p1);

  Vector second_plane_normal;
  second_plane_normal.calc_normal(second_plane_p2,
                                  second_plane_p3);

  Vector both_planes_normal;
  both_planes_normal.calc_normal(first_plane_normal,
                                 second_plane_normal);

  return (0 == both_planes_normal.get_length()) ? true : false;
}
