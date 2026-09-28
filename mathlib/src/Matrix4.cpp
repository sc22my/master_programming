//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include "Matrix4.h"
#include "Quaternion.h"
#include <limits>
#include <math.h>

// constructor - default to the zero matrix
Matrix4::Matrix4() {
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            coordinates[row][col] = 0.0;
}

Matrix4::Matrix4(const Matrix4 &other) {
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            coordinates[row][col] = other.coordinates[row][col];
}

// equality operator
bool Matrix4::operator ==(const Matrix4 &other) const {
    // loop through, testing for mismatches
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            if (abs(coordinates[row][col] - other.coordinates[row][col]) > std::numeric_limits<float>::epsilon())
                return false;
    // if no mismatches, matrices are the same
    return true;
}

// scalar operations
// multiplication operator (no division operator)
Matrix4 Matrix4::operator *(float factor) const {
    Matrix4 returnMatrix;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            returnMatrix[i][j] *= factor;
        }
    }

    // and return it
    return returnMatrix;
}

// vector operations on homogeneous coordinates
// multiplication is the only operator we use
Homogeneous4 Matrix4::operator *(const Homogeneous4 &vector) const {
    // get a zero-initialised vector
    Homogeneous4 productVector;

    for (int row = 0; row < 4; row++) {
        int sum = 0;

        for (int i = 0; i < 4; i++) {
            sum += vector[i] * coordinates[row][i];
        }

        productVector[row] = sum;
    }

    // return the result
    return productVector;
}

// and on Cartesian coordinates
Point3 Matrix4::operator *(const Vector3 &vector) const {
    // convert to Homogeneous coords and multiply
    Homogeneous4 productVector = (*this) * Homogeneous4(vector);

    // then divide back through
    return productVector.Point();
}

// matrix operations
// addition operator
Matrix4 Matrix4::operator +(const Matrix4 &other) const {
    // start with a zero matrix
    Matrix4 sumMatrix;

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            sumMatrix[r][c] = coordinates[r][c] + other.coordinates[r][c];
        }
    }

    // return the result
    return sumMatrix;
}

// subtraction operator
Matrix4 Matrix4::operator -(const Matrix4 &other) const {
    // start with a zero matrix
    Matrix4 differenceMatrix;

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            differenceMatrix[r][c] = coordinates[r][c] - other.coordinates[r][c];
        }
    }

    // return the result
    return differenceMatrix;
}

// multiplication operator
Matrix4 Matrix4::operator *(const Matrix4 &other) const {
    // start with a zero matrix
    Matrix4 productMatrix;
    // This function is provided to give the correct result ...
    // ... on the OpenGL render widget in the app UI
    // loop, adding products
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            for (int entry = 0; entry < 4; entry++)
                productMatrix.coordinates[row][col] += coordinates[row][entry] * other.coordinates[entry][col];

    // return the result
    return productMatrix;
}

// matrix transpose
Matrix4 Matrix4::transpose() const {
    // start with a zero matrix
    Matrix4 transposeMatrix;

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            transposeMatrix[c][r] = coordinates[r][c];
        }
    }

    // return the result
    return transposeMatrix;
}


void Matrix4::SetTranslation(const Vector3 &vector) {
    // start with an identity matrix
    SetIdentity();

    coordinates[3][0] = vector[0];
    coordinates[3][1] = vector[1];
    coordinates[3][2] = vector[2];
}

void Matrix4::SetScale(float xScale, float yScale, float zScale) {
    // start off with a zero matrix
    SetZero();

    coordinates[0][0] = xScale;
    coordinates[1][1] = yScale;
    coordinates[2][2] = zScale;
}

void Matrix4::SetRotation(const Vector3 &axis, float theta) {
    // This is derived from quaternions, so we invoke them
    Quaternion rotationQuaternion(axis.unit(), theta * 0.5);
    (*this) = rotationQuaternion.GetMatrix();
}


// scalar operations
// additional scalar multiplication operator
Matrix4 operator *(float factor, const Matrix4 &matrix) {
    // since this is commutative, call the other version
    return matrix * factor;
}

// factory methods that create specific matrices
// the zero matrix
void Matrix4::SetZero() {
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            coordinates[row][col] = 0.0;
}

// the identity matrix
void Matrix4::SetIdentity() {
    // start with a zero matrix
    SetZero();
    // fill in the diagonal with 1's
    for (int row = 0; row < 4; row++)
        coordinates[row][row] = 1.0;
}

// returns a column-major array of 16 values
// for use with OpenGL
columnMajorMatrix Matrix4::columnMajor() const {
    // start off with an unitialised array
    columnMajorMatrix returnArray;
    // loop to fill in
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            returnArray.coordinates[4 * col + row] = coordinates[row][col];
    // now return the array
    return returnArray;
}

// indexing - retrieves the beginning of a line
// array indexing will then retrieve an element
float * Matrix4::operator [](const int rowIndex) {
    // return the corresponding row
    return coordinates[rowIndex];
}

// similar routine for const pointers
const float * Matrix4::operator [](const int rowIndex) const {
    // return the corresponding row
    return coordinates[rowIndex];
}

// stream input
std::istream & operator >> (std::istream &inStream, Matrix4 &matrix) {
    // just loop, reading them in
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            inStream >> matrix.coordinates[row][col];   
    // and return the stream
    return inStream;
}

// stream output
std::ostream & operator << (std::ostream &outStream, const Matrix4 &matrix) {
    // just loop, reading them in
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            outStream << std::setprecision(4) << std::setw(8) << matrix.coordinates[row][col] << ((col == 3) ? "\n" : " "); 
    // and return the stream
    return outStream;
}
