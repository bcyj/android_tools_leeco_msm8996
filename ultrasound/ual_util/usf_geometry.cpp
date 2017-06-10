/*===========================================================================
                           usf_geometry.cpp

DESCRIPTION: This file contains implementation of vector and matrix
common operations.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/
/*-----------------------------------------------------------------------------
  Include Files
-----------------------------------------------------------------------------*/
#include "usf_geometry.h"

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/

/*==============================================================================
  FUNCTION:  Vector()
==============================================================================*/
/**
 * See function definition in header file.
 */
Vector::Vector(double x,
               double y,
               double z)
{
  vector[X] = x;
  vector[Y] = y;
  vector[Z] = z;
}

/*==============================================================================
  FUNCTION:  operator=()
==============================================================================*/
/**
 * See function definition in header file.
 */
Vector& Vector::operator=(char array_3d[FILE_PATH_MAX_LEN])
{
  if (NULL == array_3d)
  {
    LOGW("%s: Null argument",
         __FUNCTION__);
    set_to_zero();
    return *this;
  }

  if ('\0' == *array_3d)
  {
    // Empty string, treat as zero vector
    set_to_zero();
    return *this;
  }

  double x, y, z;
  int ret = sscanf(array_3d,
                   "%lf ,%lf ,%lf",
                   &x,
                   &y,
                   &z);
  if (3 != ret)
  {
    LOGW("%s: 3d array should have three dims, ret from scanf=%d",
         __FUNCTION__,
         ret);
    set_to_zero();
    return *this;
  }

  this->set_element(X,x);
  this->set_element(Y,y);
  this->set_element(Z,z);
  return *this;
}

/*==============================================================================
  FUNCTION:  operator==()
==============================================================================*/
/**
 * See function definition in header file.
 */
bool Vector::operator==(Vector v) const
{
  if (v.get_element(X) == vector[X] &&
      v.get_element(Y) == vector[Y] &&
      v.get_element(Z) == vector[Z])
  {
    return true;
  }
  else
  {
    return false;
  }
}

/*==============================================================================
  FUNCTION:  operator!=()
==============================================================================*/
/**
 * See function definition in header file.
 */
bool Vector::operator!=(Vector v) const
{
  if (v.get_element(X) == vector[X] &&
      v.get_element(Y) == vector[Y] &&
      v.get_element(Z) == vector[Z])
  {
    return false;
  }
  else
  {
    return true;
  }
}

/*==============================================================================
  FUNCTION:  get_length()
==============================================================================*/
/**
 * See function definition in header file.
 */
double Vector::get_length() const
{
  return sqrt(pow(vector[X],2) + pow(vector[Y],2) + pow(vector[Z],2));
}

/*==============================================================================
  FUNCTION:  vector_mult_by_scalar()
==============================================================================*/
/**
 * See function definition in header file.
 */
void Vector::vector_mult_by_scalar(double scalar)
{
  for (int i = 0; i < NUM_DIMENSIONS; i++)
  {
    vector[i] *= scalar;
  }
}

/*==============================================================================
  FUNCTION:  calc_normal()
==============================================================================*/
/**
 * See function definition in header file.
 */
void Vector::calc_normal(const Vector &firstVector,
                         const Vector &secondVector)
{
  vector[X] =  firstVector.get_element(Y) * secondVector.get_element(Z) -
    firstVector.get_element(Z) * secondVector.get_element(Y);
  vector[Y] =  -firstVector.get_element(X) * secondVector.get_element(Z) +
    firstVector.get_element(Z) * secondVector.get_element(X);
  vector[Z] =  firstVector.get_element(X) * secondVector.get_element(Y) -
    firstVector.get_element(Y) * secondVector.get_element(X);
}

/*==============================================================================
  FUNCTION:  normalize()
==============================================================================*/
/**
 * See function definition in header file.
 */
void Vector::normalize()
{
  int length = get_length();
  for (int i = 0; i < NUM_DIMENSIONS; i++)
  {
    vector[i] /= length;
  }
}

/*==============================================================================
  FUNCTION:  get_tilt_angle_vector()
==============================================================================*/
/**
 * See function definition in header file.
 */
Vector Vector::get_tilt_angle_vector() const
{
  Vector result;
  if (0 == vector[Z])
  {
    result.set_element(X, 90);
    result.set_element(Y, 90);
  }
  else
  {
    result.set_element(X, atan(vector[X] / vector[Z]) * RAD_TO_DEG);
    result.set_element(Y, atan(vector[Y] / vector[Z]) * RAD_TO_DEG);
  }
  result.set_element(Z, 0);
  return result;
}

/*==============================================================================
  FUNCTION:  print_vector()
==============================================================================*/
/**
 * See function definition in header file.
 */
void Vector::print_vector() const
{
  LOGD("x: %lf, y: %lf, z: %lf",
       this->get_element(X),
       this->get_element(Y),
       this->get_element(Z));
}

/*==============================================================================
  FUNCTION:  set_to_zero()
==============================================================================*/
/**
 * See function definition in header file.
 */
void Vector::set_to_zero()
{
  this->set_element(X,0);
  this->set_element(Y,0);
  this->set_element(Z,0);
}


/*==============================================================================
  FUNCTION:  Matrix()
==============================================================================*/
/**
 * See function definition in header file.
 */
Matrix::Matrix(Vector xRow,
               Vector yRow,
               Vector zRow)
{
  matrix[X] = xRow;
  matrix[Y] = yRow;
  matrix[Z] = zRow;
}

/*==============================================================================
  FUNCTION:  get_element()
==============================================================================*/
/**
 * See function definition in header file.
 */
double Matrix::get_element(int row,
                           int col) const
{
  return matrix[row].get_element(col);
}

/*==============================================================================
  FUNCTION:  set_element()
==============================================================================*/
/**
 * See function definition in header file.
 */
void Matrix::set_element(int row,
                         int col,
                         double value)
{
  matrix[row].set_element(col,
                          value);
}

/*==============================================================================
  FUNCTION:  usf_dependencies_check_transformation()
==============================================================================*/
/**
 * See function definition in header file.
 */
void Matrix::matrix_by_matrix_multiply(const Matrix &otherMatrix,
                                       Matrix &resultMatrix) const
{
  double tempValue;
  for (int i = 0; i < NUM_DIMENSIONS; i++)
  {
    for (int j = 0; j < NUM_DIMENSIONS; j++)
    {
      tempValue = 0.0;
      for (int k = 0; k < NUM_DIMENSIONS; k++)
      {
        tempValue += get_element(i,
                                 k) * otherMatrix.get_element(k,
                                                              j);
      }
      resultMatrix.set_element(i,
                               j,
                               tempValue);
    }
  }
}

/*==============================================================================
  FUNCTION:  print_matrix()
==============================================================================*/
/**
 * See function definition in header file.
 */
void Matrix::print_matrix() const
{
  this->get_row_vector(0).print_vector();
  this->get_row_vector(1).print_vector();
  this->get_row_vector(2).print_vector();
}
