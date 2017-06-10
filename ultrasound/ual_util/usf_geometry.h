/*===========================================================================
                           usf_geometry.h

DESCRIPTION: This file contains common definitions for vector and
matrix.

INITIALIZATION AND SEQUENCING REQUIREMENTS: None

Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#ifndef _USF_GEOMETRY_H_
#define _USF_GEOMETRY_H_

/*-----------------------------------------------------------------------------
  Include Files
-----------------------------------------------------------------------------*/
#include "usf_log.h"
#include <cutils/properties.h>
#include <ual_util.h>

// Math defines
#define _USE_MATH_DEFINES
#define DEG_TO_RAD (M_PI / 180)
#define RAD_TO_DEG (180 / M_PI)

/*-----------------------------------------------------------------------------
  Static Variable Definitions
-----------------------------------------------------------------------------*/
class Matrix;

/**
 * Contains enumeration for all valid dimentions.
 */
enum DIMENSIONS
{
  X,
  Y,
  Z,
  NUM_DIMENSIONS
};
/**
 * The number of transformation points given.
 */
static const int TRANSFORM_POINTS_NUM = 3;

/*------------------------------------------------------------------------------
  Function definitions
------------------------------------------------------------------------------*/
/*==============================================================================
  CLASS:  Vector()
==============================================================================*/
class Vector
{
public:
  /*==============================================================================
  FUNCTION:  Vector()
  ==============================================================================*/
  /**
   * Default Constructor of Vector
   */
  Vector()
  {
  }

  /*==============================================================================
  FUNCTION:  Vector()
  ==============================================================================*/
  /**
   * Constructor of Vector
   *
   * @param x coordinate
   * @param y coordinate
   * @param z coordinate
   */
  Vector(double x,
         double y,
         double z);

  /*==============================================================================
  FUNCTION:  operator=()
  ==============================================================================*/
  /**
   * This function creates a vector out of the given array
   *
   * @param array_3d the array to create
   *
   * @return Vector& the vector that is created of the input
   *         array.
   */
  Vector& operator=(char array_3d[FILE_PATH_MAX_LEN]);

  /*==============================================================================
  FUNCTION:  operator==()
  ==============================================================================*/
  /**
   * This function implements operator ==
   *
   * @param v the vector to compare
   *
   * @return bool true when this and the given vectors are equal,
   *         false otherwise.
   */
  bool operator==(Vector v) const;

  /*==============================================================================
  FUNCTION:  operator!=()
  ==============================================================================*/
  /**
   * This function implements operator !=
   *
   * @param v the vector to compare
   *
   * @return bool true when this and the given vectors are not
   *         equal, false otherwise.
   */
  bool operator!=(Vector v) const;

  /*==============================================================================
  FUNCTION:  get_element()
  ==============================================================================*/
  /**
   * Return the value of the given coordinate
   *
   * @param index the coordinate
   *
   * @return double value of the given coordinate
   */
  double get_element(int index) const;

  /*==============================================================================
  FUNCTION:  set_element()
  ==============================================================================*/
  /**
   * Set the given value to the given coordinate of this vector
   *
   * @param index the coordinate
   * @param value the new value
   */
  void set_element(int index,
                   double value);

  /*==============================================================================
  FUNCTION:  get_length()
  ==============================================================================*/
  /**
   * Return the absolute length of this vector
   *
   * @return double the absolute length
   */
  double get_length() const;

  /*==============================================================================
  FUNCTION:  vector_multiply_element_by_element()
  ==============================================================================*/
  /**
   * Multiply this vector in the elements of the given vector,
   * element by element
   *
   * @param otherVector the other vector
   */
  void vector_multiply_element_by_element(const Vector &otherVector);

  /*==============================================================================
  FUNCTION:  vector_substract()
  ==============================================================================*/
  /**
   * Substruct the value of the given vector from this vector,
   * element by element
   *
   * @param otherVector the other vector
   */
  void vector_substract(const Vector &otherVector);

  /*==============================================================================
  FUNCTION:  vector_add()
  ==============================================================================*/
  /**
   * Add the value of the given vector to this vector, element by
   * element
   *
   * @param otherVector the other vector
   */
  void vector_add(Vector otherVector);

  /*==============================================================================
  FUNCTION:  dot_product()
  ==============================================================================*/
  /**
   * Perform dot product between this vector to the given vector
   *
   * @param otherVector the other vector
   *
   * @return double the dot product result
   */
  double dot_product(const Vector &otherVector) const;

  /*==============================================================================
  FUNCTION:  vector_mult_by_scalar()
  ==============================================================================*/
  /**
   * Multiply this vector in a given scalar, element by element
   *
   * @param scalar the dot product result
   */
  void vector_mult_by_scalar(double scalar);

  /*==============================================================================
  FUNCTION:  calc_normal()
  ==============================================================================*/
  /**
   * Calculate the normal to the plane defined by firstVector and
   * secondVector from <firstVector X secondVector>
   *
   * @param firstVector the first vector
   * @param secondVector the second vector
   */
  void calc_normal(const Vector &firstVector,
                   const Vector &secondVector);

  /*==============================================================================
  FUNCTION:  vector_mult_by_matrix()
  ==============================================================================*/
  /**
   * Calculate the multiplication of this vector in a given
   * matrix.
   *
   * @param matrix the matrix to multily in this vector
   */
  void vector_mult_by_matrix(const Matrix &matrix);

  /*==============================================================================
  FUNCTION:  normalize()
  ==============================================================================*/
  /**
   * Normalizes this vector.
   */
  void normalize();

  /*==============================================================================
  FUNCTION:  get_tilt_angle_vector()
  ==============================================================================*/
  /**
   * Returns the x and y tilt angles of the given vector. z angle
   * is set to 0.
   */
  Vector get_tilt_angle_vector() const;

  /*==============================================================================
  FUNCTION:  print_vector()
  ==============================================================================*/
  /**
   * This function prints the given vector to the screen.
   * This function is for debugging.
   */
  void print_vector() const;

private:
  // The vectors coordinates
  double vector[NUM_DIMENSIONS];

  /*==============================================================================
  FUNCTION:  set_to_zero()
  ==============================================================================*/
  /**
   * Sets all components of the vector to 0.
   */
  void set_to_zero();
};



/*==============================================================================
  CLASS:  Matrix()
==============================================================================*/
class Matrix
{
public:
  /*==============================================================================
  FUNCTION:  Matrix()
  ==============================================================================*/
  /**
    * Default Constructor of Matrix
    */
  Matrix()
  {
  }

  /*==============================================================================
  FUNCTION:  Matrix()
  ==============================================================================*/
  /**
   * Constructor of Matrix
   *
   * @param xRow vector of the first row in the matrix
   * @param yRow vector of the second row in the matrix
   * @param zRow vector of the third row in the matrix
   */
  Matrix(Vector xRow,
         Vector yRow,
         Vector zRow);

  /*==============================================================================
  FUNCTION:  get_element()
  ==============================================================================*/
  /**
   * Find the element in the given row and column of this matrix
   *
   * @param row the row
   * @param col the col
   *
   * @return double the element
   */
  double get_element(int row,
                     int col) const;

  /*==============================================================================
  FUNCTION:  get_row_vector()
  ==============================================================================*/
  /**
   * Return the the given row of this matrix
   *
   * @param row the row
   *
   * @return const Vector& the the given row of this matrix
   */
  const Vector& get_row_vector(int row) const;

  /*==============================================================================
  FUNCTION:  matrix_by_matrix_multiply()
  ==============================================================================*/
  /**
   * Calculate the multiplication of two matrices - this matrix
   * and the given matrix
   *
   * @param otherMatrix the other matrix
   * @param resultMatrix the result matrix
   */
  void matrix_by_matrix_multiply(const Matrix &otherMatrix,
                                 Matrix &resultMatrix) const;

  /*==============================================================================
  FUNCTION:  print_matrix()
  ==============================================================================*/
  /**
   * This function prints the curernt matrix on the screen.
   * This function is for debugging.
   */
  void print_matrix() const;
private:

  /*==============================================================================
  FUNCTION:  set_element()
  ==============================================================================*/
  /**
   * Set the given value to the given cell of this matrix
   *
   * @param row the row of the cell
   * @param col the column of the cell
   * @param value the new value of the cell
   */
  void set_element(int row,
                   int col,
                   double value);

  // The matrix is built from vectors
  Vector matrix[NUM_DIMENSIONS];
};

/**
 * Contains information of the points that build the
 * transformation matrix.
 */
struct plane_properties
{
  Vector origin;
  Vector point_end_x;
  Vector point_end_y;
  uint32_t hover_max_range;
};

/*==============================================================================
  FUNCTION:  get_element()
==============================================================================*/
/**
 * See function definition above.
 */
inline double Vector::get_element(int index) const
{
  return vector[index];
}

/*==============================================================================
  FUNCTION:  set_element()
==============================================================================*/
/**
 * See function definition above.
 */
inline void Vector::set_element(int index,
                                double value)
{
  vector[index] = value;
}

/*==============================================================================
  FUNCTION:  vector_multiply_element_by_element()
==============================================================================*/
/**
 * See function definition above.
 */
inline void Vector::vector_multiply_element_by_element(const Vector &otherVector)
{
  for (int i = 0; i < NUM_DIMENSIONS; i++)
  {
    vector[i] *= otherVector.vector[i];
  }
}

/*==============================================================================
  FUNCTION:  vector_substract()
==============================================================================*/
/**
 * See function definition above.
 */
inline void Vector::vector_substract(const Vector &otherVector)
{
  for (int i = 0; i < NUM_DIMENSIONS; i++)
  {
    vector[i] -= otherVector.vector[i];
  }
}

/*==============================================================================
  FUNCTION:  vector_add()
==============================================================================*/
/**
 * See function definition above.
 */
inline void Vector::vector_add(Vector otherVector)
{
  otherVector.vector_mult_by_scalar(-1);
  vector_substract(otherVector);
}

/*==============================================================================
  FUNCTION:  dot_product()
==============================================================================*/
/**
 * See function definition above.
 */
inline double Vector::dot_product(const Vector &otherVector) const
{
  double dotProductRes = 0.0;
  for (int i = 0; i < NUM_DIMENSIONS; i++)
  {
    dotProductRes += (vector[i] * otherVector.vector[i]);
  }

  return dotProductRes;
}

/*==============================================================================
  FUNCTION:  vector_mult_by_matrix()
==============================================================================*/
/**
 * See function definition above.
 */
inline void Vector::vector_mult_by_matrix(const Matrix &matrix)
{
  Vector temp;
  // Copying this vector
  for (int i = 0; i < NUM_DIMENSIONS; i++)
  {
    temp.set_element(i,
                     vector[i]);
  }
  // Calculating multiplication
  for (int i = 0; i < NUM_DIMENSIONS; i++)
  {
    vector[i] = temp.dot_product(matrix.get_row_vector(i));
  }
}

/*==============================================================================
  FUNCTION:  get_row_vector()
==============================================================================*/
/**
 * See function definition above.
 */
inline const Vector& Matrix::get_row_vector(int row) const
{
  return matrix[row];
}

#endif // _USF_GEOMETRY_H_
