/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: KP3D.h
 *  Description: KPEngine Math Library Declarations
 *				 - Vector 4D
 *				 - Matrix 4D
 *				 - Plane
 *
 *****************************************************************
*/

#ifndef KP3D_H
#define KP3D_H

// Geometric Tools for Computer Graphics By Philip J. Schneider, David H. Eberly


#include <math.h>
#include <vector>
#include <algorithm>
#include "KPCPU.h"

// Forward Declarations ////
bool IsSSESupported(void);	//!< Initializes the CPU and checks SIMD support.

class KPVector;
class KPMatrix;
class KPPlane;
class KPPolygon;

// Epsilon value for Tri/Ray intersection checking 
// to circumvent poor floating point precision
// comparisons to 0
#define EPSILON 0.00001	


// Object States
#define KPFRONT		0
#define KPBACK		1
#define KPPLANAR	2
#define KPCLIPPED	3
#define KPCULLED	4
#define KPVISIBLE	5

// Maximum/Minimum Search ////
//////////////////////////////

template <class T> const T KPMax( const T a, const T b );
template <class T> const T KPMax( const T a, const T b, const T c );
template <class T> const T KPMin( const T a, const T b );
template <class T> const T KPMin( const T a, const T b, const T c );

//! 4 Dimensional Vector Class

//! The 4th dimension is mostly only needed
//! for compatibilty with 4x4 matrices.
class __declspec( dllexport ) KPVector
{
public:
	float x, y, z, w;							// Vector coordinates

	//! Default constructor
	KPVector(void) { x=0.0f, y=0.0f, z=0.0f, w=1.0f; }

	//! Constructor that takes three floating point coordinates
	/*!
		\param [in] _x floating point value specifying the X coordinate of the vector
		\param [in] _y floating point value specifying the Y coordinate of the vector
		\param [in] _z floating point value specifying the Z coordinate of the vector
	*/
	KPVector(float _x, float _y, float _z) { x = _x, y = _y, z = _z, w = 1.0f; }

	////
	// Member Functions
	////

	//! Assigns new floating point values to the vector's coordinates
	/*!
		\param [in] _x floating point value specifying the X coordinate of the vector
		\param [in] _y floating point value specifying the Y coordinate of the vector
		\param [in] _z floating point value specifying the Z coordinate of the vector
		\param [in] _w floating point value specifying the W coordinate of the vector. This is 1.0f by default.
	*/
	inline void		Set(float _x, float _y, float _z, float _w = 1.0f);

	//! Negates the vector.
	inline void		Negate(void);

	//! Normalizes the vector
	inline void		Normalize(void);

	//! Calculates the difference of two vectors
	/*!
		\param [in] v1 KPVector object specifying the fist vector
		\param [in] v2 KPVector object specifying the second vector
	*/
	inline void		Difference(const KPVector &v1, const KPVector &v2);

	//! Calculates the cross product of two vectors
	/*!
		\param [in] v1 KPVector object specifying the fist vector
		\param [in] v2 KPVector object specifying the second vector
	*/
	inline void		Cross(const KPVector &v1, const KPVector &v2);

	//! Calculates the length of the vector
	inline float	GetLength(void);

	//! Calculates the squared length of the vector
	inline float	GetSqaredLength(void) const;

	//! Calculates the angle between two vectors.
	/*!
		\param [in] v KPVector object specifying the vector
		\return floating point value specifying the angle in radian.
	*/
	inline float	AngleWith(KPVector &v);
	
	// Operator Overloads ////
	KPVector operator  + (const KPVector &v) const;	//!< Vector addition
	void	 operator += (const KPVector &v);		//!< Vector addition

	KPVector operator  - (const KPVector &v) const;	//!< Vector subtraction
	void	 operator -= (const KPVector &v);		//!< Vector subtraction

	KPVector operator  * (const float f)	 const;	//!< Vector scaling 
	void	 operator *= (const float f);			//!< Vector scaling 
	void	 operator /= (const float f);			//!< Vector scaling 

	float	 operator  * (const KPVector &v) const; //!< Dot Product of Vector
	KPVector operator  * (const KPMatrix &m) const;	//!< Vector * Matrix product

}; // ! KPVector class


//! 4x4 Matrix Class
class __declspec( dllexport ) KPMatrix
{
public:
	// Elements of the matrix: _RC, where R= Row and C= Column
	float _11, _12, _13, _14;	
	float _21, _22, _23, _24;
	float _31, _32, _33, _34;
	float _41, _42, _43, _44;


	//! Constructor
	KPMatrix(void) { }

	//! Makes an identity matrix from the matrix
	inline void Identity(void);

	//! Creates a rotation matrix around the X axis
	/*!
		\param [in] angle floating point value specifying the angle of rotation in radian.
	*/
	inline void RotateX(float angle);									// Rotation matrix around X axis

	//! Creates a rotation matrix around the Y axis
	/*!
		\param [in] angle floating point value specifying the angle of rotation in radian
	*/
	inline void RotateY(float angle);									// Rotation matrix around Y axis

	//! Creates a rotation matrix around the Z axis
	/*!
		\param [in] angle floating point value specifying the angle of rotation in radian
	*/
	inline void RotateZ(float angle);									// Rotation matrix around Z axis

	//! Creates a rotation matrix around an arbitrary axis
	/*!
		\param [in] aV KPVector object specifying the axis vector we want to rotate around.
		\param [in] angle floating point value specifying the angle of rotation in radian
	*/
	inline void Rotate(KPVector aV, float angle);						// Rotation matrix around arbitrary axis

	//! Creates a translation matrix. It represents movement in space.
	/*!
		\param [in] distX floating point value specifying the amount of movement on the X axis
		\param [in] distY floating point value specifying the amount of movement on the Y axis
		\param [in] distZ floating point value specifying the amount of movement on the Z axis
	*/
	inline void Translate(float distX, float distY, float distZ);		// Translation matrix (movement)

	//! Transpositioin of a matrix
	/*!
		\param [in] m KPMatrix object we want to calculate the transposition of.
	*/
	inline void TransposeOf(const KPMatrix &m);							// Transposition of the matrix

	//! Inverse of a matrix
	/*!
		\param [in] m KPMatrix object we want to calcualte the inverse of.
	*/
	inline void InverseOf(const KPMatrix &m);							// Inverse of matrix

	//! Matrix multiplication by another matrix
	KPMatrix operator * (const KPMatrix &m) const;

	//! Matrix multiplication by vector
	KPVector operator * (const KPVector &v) const;

}; // ! KPMatrix class


//! Plane Class
/*!
	Plane: V * N + d = 0,	 where V is a vector to a point on the plane, 
							 N is the normal of the plane and
							 d is distance from world origin.
*/
class __declspec( dllexport ) KPPlane
{

public:
	KPVector m_vcNormal;	//!< Normal vector of the plane
	KPVector m_vcPoint;		//!< A point on the plane
	float	 m_fDistance;	//!< Shortest distance from world origin to the plane


	//! Constructor
	KPPlane(void) { };

	//! Specify the plane using a normal vector and a point on the plane
	/*!
		\param [in] vcNormal KPVector object specifying the normal vector of the plane.
		\param [in] vcPoint KPVector object specifying a point on the plane.
	*/
	inline void Set(const KPVector &vcNormal, const KPVector &vcPoint);						// Calculate the distance

	//! Specify the plane using a normal vector, a point on the plane and the distance from the origin
	/*!
		\param [in] vcNormal KPVector object specifying the normal vector of the plane.
		\param [in] vcPoint KPVector object specifying a point on the plane.
		\param [in] fDistance floating point value specifying the distance from the origin.
	*/
	inline void Set(const KPVector &vcNormal, const KPVector &vcPoint, float fDistance);	// Specify the distance

	//! Specify the plane using three vectors
	/*!
		\param [in] v0 KPVector object specifying the first vector.
		\param [in] v1 KPVector object specifying the second vector.
		\param [in] v2 KPVector object specifying the third vector.
	*/
	inline void Set(const KPVector &v0, const KPVector &v1, const KPVector &v2);			// Define the plane with 3 vectors

}; // ! KPPlane Class

#endif // ! KP3D_H