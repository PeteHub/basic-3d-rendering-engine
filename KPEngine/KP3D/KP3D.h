/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: KP.h
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
bool IsSSESupported(void);

class KPVector;
class KPMatrix;
class KPPlane;
class KPPolygon;

// Epsilon value for Tri/Ray intersection checking 
// to circumvent poor floating point precision
// comparisons to 0
//#define EPSILON 0.000001
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

// KPVector 4 Dimensional Vector Class ////
///////////////////////////////////////////
//
// The 4th dimension is mostly only needed
// for compatibilty with 4x4 matrices.
class __declspec( dllexport ) KPVector
{
public:
	float x, y, z, w;							// Coordinates

	// Constructors /////
	KPVector(void) { x=0.0f, y=0.0f, z=0.0f, w=1.0f; }
	KPVector(float _x, float _y, float _z) { x = _x, y = _y, z = _z, w = 1.0f; }

	// Member Functions ////
	inline void		Set(float _x, float _y, float _z, float _w = 1.0f);
	inline void		Negate(void);
	inline void		Normalize(void);
	inline void		Difference(const KPVector &v1, const KPVector &v2);
	inline void		Cross(const KPVector &v1, const KPVector &v2);
	inline float	GetLength(void);
	inline float	GetSqaredLength(void) const;
	inline float	AngleWith(KPVector &v);
	
	// Operator Overloads ////
	KPVector operator  + (const KPVector &v) const;	// Vector addition
	void	 operator += (const KPVector &v);		

	KPVector operator  - (const KPVector &v) const;	// Vector subtraction
	void	 operator -= (const KPVector &v);		

	KPVector operator  * (const float f)	 const;	// Vector scaling 
	void	 operator *= (const float f);					
	void	 operator /= (const float f);	

	float	 operator  * (const KPVector &v) const; // Dot Product of Vector
	KPVector operator  * (const KPMatrix &m) const;	// Vector * Matrix product

}; // ! KPVector class


// KPMatrix 4x4 Matrix Class ////
/////////////////////////////////
class __declspec( dllexport ) KPMatrix
{
public:
	float _11, _12, _13, _14;	// Rows of the Matrix
	float _21, _22, _23, _24;
	float _31, _32, _33, _34;
	float _41, _42, _43, _44;

	KPMatrix(void) { }

	inline void Identity(void);
	inline void RotateX(float angle);									// Rotation matrix around X axis
	inline void RotateY(float angle);									// Rotation matrix around Y axis
	inline void RotateZ(float angle);									// Rotation matrix around Z axis
	inline void Rotate(KPVector aV, float angle);						// Rotation matrix around arbitrary axis
	inline void Translate(float distX, float distY, float distZ);		// Translation matrix (movement)

	inline void TransposeOf(const KPMatrix &m);							// Transposition of the matrix
	inline void InverseOf(const KPMatrix &m);							// Inverse of matrix

	KPMatrix operator * (const KPMatrix &m) const;
	KPVector operator * (const KPVector &v) const;

}; // ! KPMatrix class


// KPPlane Class ////
/////////////////////
class _declspec( dllexport ) KPPlane
{
	/*
	Plane: V * N + d = 0,	 where V is a vector to a point on the plane,
							 N is the normal of the plane and
							 d is distance from world origin.
	*/

public:
	KPVector m_vcNormal;	// Normal vector of the plane
	KPVector m_vcPoint;		// A point on the plane
	float	 m_fDistance;	// Shortest distance from world origin to the plane


	KPPlane(void) { };

	inline void Set(const KPVector &vcNormal, const KPVector &vcPoint);						// Calculate the distance
	inline void Set(const KPVector &vcNormal, const KPVector &vcPoint, float fDistance);	// Specify the distance
	inline void Set(const KPVector &v0, const KPVector &v1, const KPVector &v2);			// Define the plane with 3 vectors

}; // ! KPPlane Class

#endif // ! KP3D_H