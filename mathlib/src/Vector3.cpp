//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5812M Foundations of Modelling & Rendering
//  User Interface for Coursework
////////////////////////////////////////////////////////////////////////

#include "Point3.h"
#include "Vector3.h"
#include "math.h"
#include <iomanip>
#include <limits>
// constructors
Vector3::Vector3()
    : x(0.0), y(0.0), z(0.0) 
    {}

Vector3::Vector3(float X, float Y, float Z)
    : x(X), y(Y), z(Z) 
    {}

Vector3::Vector3(const Vector3 &other)
    : x(other.x), y(other.y), z(other.z) 
    {}
    
// equality operator
bool Vector3::operator ==(const Vector3 &other) const
    { // Vector3::operator ==()
    return (abs(x - other.x) < std::numeric_limits<float>::epsilon() && abs(y - other.y) < std::numeric_limits<float>::epsilon() && abs(z - other.z) < std::numeric_limits<float>::epsilon());
    } // Vector3::operator ==()

// addition operator
Vector3 Vector3::operator +(const Vector3 &other) const {
    Vector3 returnVal;

    for (int i = 0; i < 3; i++) {
        returnVal[i] = (*this)[i] + other[i];
    }

    return returnVal;
}

// subtraction operator
Vector3 Vector3::operator -(const Vector3 &other) const {
    Vector3 returnVal;

    for (int i = 0; i < 3; i++) {
        returnVal[i] = (*this)[i] - other[i];
    }

    return returnVal;
}

// multiplication operator
Vector3 Vector3::operator *(float factor) const {
    Vector3 returnVal;

    for (int i = 0; i < 3; i++) {
        returnVal[i] = (*this)[i] * factor;
    }

    return returnVal;
}

// division operator. TODO: watch out for 0?
Vector3 Vector3::operator /(float factor) const {
    Vector3 returnVal;

    for (int i = 0; i < 3; i++) {
        returnVal[i] = (*this)[i] / factor;
    }

    return returnVal;
}

// dot product routine
float Vector3::dot(const Vector3 &other) const {
    return (x * other.x) + (y * other.y) + (z * other.z);
}

// cross product routine
Vector3 Vector3::cross(const Vector3 &other) const {
    Vector3 returnVal;

    for (int i = 0; i < 3; i++) {
        returnVal[i] = ((*this)[i] * other[(i + 1) % 3]) - ((*this)[(i + 1) % 3] * other[i]);
    }

    return returnVal;
}

// routine to find the length
float Vector3::length() const {
    float l2 = dot(*this);

    // TODO: something something fast sqroot idk. probaly good eough right?
    return std::sqrtf(l2);

    return 1.f;
}

// normalisation routine. TODO: watch out for 0 length vectors
Vector3 Vector3::unit() const {
    Vector3 returnVal;

    float l = length();
    return *this / l;
}

// operator that allows us to use array indexing instead of variable names
float &Vector3::operator [] (const int index)
    { // operator []
    // use default to catch out of range indices
    // we could throw an exception, but will just return the 0th element instead
    switch (index)
        { // switch on index
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        // actually the error case
        default:
            return x;       
        } // switch on index
    } // operator []

// operator that allows us to use array indexing instead of variable names
const float &Vector3::operator [] (const int index) const
    { // operator []
    // use default to catch out of range indices
    // we could throw an exception, but will just return the 0th element instead
    switch (index)
        { // switch on index
        case 0:
            return x;
        case 1:
            return y;
        case 2:
            return z;
        // actually the error case
        default:
            return x;       
        } // switch on index
    } // operator []

// addition operator (Point + Vector is valid, but not the other way round)
Point3 operator +(const Point3 &left, const Vector3 &right) {
    Point3 returnVal;

    for (int i = 0; i < 3; i++) {
        returnVal[i] = left[i] + right[i];
    }

    return returnVal;
}

// subtraction operator (can subtract a point from point, resulting a diff vector)
Vector3 operator -(const Point3 &left, const Point3 &right) {
    Vector3 returnVal;

    for (int i = 0; i < 3; i++) {
        returnVal[i] = left[i] + right[i];
    }

    return returnVal;
}

// multiplication operator by a scalar
Vector3 operator *(float factor, const Vector3 &right) {
    return right * factor;
}

// stream input
std::istream & operator >> (std::istream &inStream, Vector3 &value)
    { // stream output
    inStream >> value.x >> value.y >> value.z;
    return inStream;
    } // stream output
        
// stream output
std::ostream & operator << (std::ostream &outStream, const Vector3 &value)
    { // stream output
    outStream << std::setprecision(4) << value.x << " " << std::setprecision(4) << value.y << " " << std::setprecision(4) << value.z;
    return outStream;
    } // stream output

        
