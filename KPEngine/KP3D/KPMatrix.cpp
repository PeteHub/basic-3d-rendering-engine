/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - August 2009
 *
 *  File: KP.h
 *  Description: KPEngine Matrix 4d Class implementation
 *
 *****************************************************************
*/

#include "KP3D.h"
#include <memory.h>

// KPMatrix::Identity ////
inline void KPMatrix::Identity(void)
{
	float *f = (float*)&this->_11;		// Create a pointer that points at the very first element of the matrix
	memset(f, 0, sizeof(KPMatrix));		// Fills the whole matrix with 0s.
	_11 = _22 = _33 = _44 = 1.0f;		// Set the values on the main diagonal to 1
}


// KPMatrix::RotateX ////
/////////////////////////
//
// Creates a rotation matrix around the X world axis
//
// NOTE: We use right handed system
//		 To convert it to left handed system change change signs of all the sines
inline void KPMatrix::RotateX(float angle)
{
	float fSine		= sinf(angle);
	float fCosine	= cosf(angle);

/*
	 X		 Y		 Z		 Tr
	1.0f	0.0f	0.0f	0.0f
	0.0f	Cos 	Sin		0.0f
	0.0f    -Sin	Cos		0.0f
	0.0f	0.0f	0.0f	1.0f
*/
	float *f = (float*)&this->_11;		// Create a pointer that points at the very first element of the matrix
	memset(f, 0, sizeof(KPMatrix));		// Fills the whole matrix with 0.0f.

	_11 = _44 = 1.0f;					// X does not change because we rotate around it.
										// The translation vector is irrelevant for the rotation, leave it as unit vector

	_22 = fCosine;						// Rotated Y coordinates
	_32 = -fSine;
	
	_23	= fSine;						// Rotated Z coordinates
	_33	= fCosine;

}


// KPMatrix::RotateY ////
/////////////////////////
//
// Creates a rotation matrix around the Y world axis
//
// NOTE: We use right handed system
//		 To convert it to left handed system change signs of all the sines
inline void KPMatrix::RotateY(float angle)
{
	float fSine		= sinf(angle);
	float fCosine	= cosf(angle);

/*
	 X		 Y		 Z		 Tr
	Cos		0.0f	-Sin	0.0f
	0.0f	1.0f 	0.0f	0.0f
	Sin		0.0f	Cos		0.0f
	0.0f	0.0f	0.0f	1.0f
*/
	float *f = (float*)&this->_11;		// Create a pointer that points at the very first element of the matrix
	memset(f, 0, sizeof(KPMatrix));		// Fills the whole matrix with 0.0f.

	_22 = _44 = 1.0f;					// Y does not change because we rotate around it.
										// The translation vector is irrelevant for the rotation, leave it as unit vector

	_11 = fCosine;						// Rotated X coordinates
	_31	= fSine;

	_13	= -fSine;						// Rotated Z coordinates
	_33 = fCosine;				
}


// KPMatrix::RotateY ////
/////////////////////////
//
// Creates a rotation matrix around the Z world axis
//
// NOTE: We use right handed system
//		 To convert it to left handed system change signs of all the sines
inline void KPMatrix::RotateZ(float angle)
{
	float fSine		= sinf(angle);
	float fCosine	= cosf(angle);

/*
	 X		 Y		 Z		 Tr
	Cos		Sin		0.0f	0.0f
	-Sin	Cos 	0.0f	0.0f
	0.0f	0.0f	1.0f	0.0f
	0.0f	0.0f	0.0f	1.0f
*/
	float *f = (float*)&this->_11;		// Create a pointer that points at the very first element of the matrix
	memset(f, 0, sizeof(KPMatrix));		// Fills the whole matrix with 0.0f.

	_33 = _44 = 1.0f;					// Z does not change because we rotate around it.
										// The translation vector is irrelevant for the rotation, leave it as unit vector

	_11 = fCosine;						// Rotated X coordinates
	_21	= -fSine;


	_12	= fSine;						// Rotated Y coordinates
	_22 = fCosine;				
}


// KPMatrix::Rotate ////
////////////////////////
//
// Creates a rotation matrix around an arbitrary axis
inline void KPMatrix::Rotate(KPVector aV, float angle)
{
/*

	// Graphics Gems (Glassner, Academic Press, 1990) ////

	Rotation Matrix about arbitrary axis (left handed)
	Change the signs of all sine functions for right handed

	c = cos(a)
	s = sin(a)
	t = 1-cos(a)
	<X,Y,Z> unit vector representing the arbitrary axis

	-------------------------------------
	tX^2+ c		tXY - sZ	tXZ + sY	0
	tXY + sZ	tY^2+ c		tYZ - sX	0
	tXZ - sY	tYZ + sX	tZ^2+ c		0
		0			0			0		1
	-------------------------------------
*/

	float fSine		= sinf(angle);
	float fCosine	= cosf(angle);
	float fSum		= 1.0f - fCosine;

	// Unit vectors magnitude should be 1. sqrt(1) = 1
	// This way we can save a square root operation
	// if the arbitrary axis is not a unit vector, normalize it!
	if ( aV.GetSqaredLength() != 1.0f )
		aV.Normalize();

	_11	=	fSum * ( aV.x * aV.x ) + fCosine;
	_12 =	fSum * ( aV.x * aV.y ) - ( fSine * aV.z );
	_13 =	fSum * ( aV.x * aV.z ) + ( fSine * aV.y );
	_14 =	0.0f;

	_21 =	fSum * ( aV.x * aV.y ) + ( fSine * aV.z );
	_22 =	fSum * ( aV.y * aV.y ) + fCosine;
	_23 =	fSum * ( aV.y * aV.z ) - ( fSine * aV.x );
	_24 =	0.0f;

	_31 =	fSum * ( aV.x * aV.z ) - ( fSine * aV.y );
	_32 =	fSum * ( aV.y * aV.z ) + ( fSine * aV.x );
	_33 =	fSum * ( aV.z * aV.z ) + fCosine;
	_34 =	0.0f;

	_41 = _42 = _43 = 0.0f;
	_44 = 1.0f;

} // ! KPMatrix::Rotate()


// KPMatrix::Translate ////
///////////////////////////
inline void KPMatrix::Translate(float distX, float distY, float distZ)
{
	_41	= distX;
	_42 = distY;
	_43 = distZ;
}


// KPMatrix::TransposeOf ////
/////////////////////////////
//
// It reflects the A matrix by its main diagonal (starts from the top left)
inline void KPMatrix::TransposeOf(const KPMatrix &m)
{
	_11 = m._11;
	_12 = m._21;
	_13 = m._31;
	_14 = m._41;

	_21 = m._12;
	_22 = m._22;
	_23 = m._32;
	_24 = m._42;

	_31 = m._13;
	_32 = m._23;
	_33 = m._33;
	_34 = m._43;

	_41 = m._14;
	_42 = m._24;
	_43 = m._34;
	_44 = m._44;
}


// KPMatrix Operator Overloads ////
///////////////////////////////////

KPMatrix KPMatrix::operator *(const KPMatrix &m) const
{
	KPMatrix result;

	// Pointers at the first float value of the matrices : _11
	float *pThis	= (float*)this;
	float *pM		= (float*)&m;
	float *pResult	= (float*)&result;

	memset(pResult, 0, sizeof(KPMatrix));		// Fill the whole result matrix with 0.0f

	for ( unsigned int i = 0; i < 4; ++i )		// Row Number (uchar for 1 byte size)
	{
		for ( unsigned int j = 0; j < 4; ++j )  // Column Number (uchar for 1 byte size)
		{
		//	 Row + Column        Row          Col
			pResult[4*i+j] += pThis[4*i]   * pM[j];		// First element of row * first element of column (NOTE: 4 is 4*floatsize ~= 4*4 = 16 bytes = 1 row)
			pResult[4*i+j] += pThis[4*i+1] * pM[j+4];   // Second element of row * second element of column
			pResult[4*i+j] += pThis[4*i+2] * pM[j+8];   // third element of row * third element of column
			pResult[4*i+j] += pThis[4*i+3] * pM[j+12];  // fourth element of row * fourth element of column
		}
	}

	return result;
}


KPVector KPMatrix::operator *(const KPVector &v) const
{
	KPVector result;

	result.x = v.x * _11 + v.y * _21 + v.z * _31 + _41;
	result.y = v.x * _12 + v.y * _22 + v.z * _32 + _42;
	result.z = v.x * _13 + v.y * _23 + v.z * _33 + _43;
	result.w = v.x * _14 + v.y * _24 + v.z * _34 + _44;

	// At this point vcReturn.w has some value, but we want it to be 1.0f
	// we have to scale the matrix down by vcReturn.w for this

	result.x /= result.w;
	result.y /= result.w;
	result.z /= result.w;
	result.w = 1.0f;

	return result;
}


// KPMatrix::InverseOf ////
///////////////////////////
inline void KPMatrix::InverseOf(const KPMatrix &m)
{
	/*
	How to calculate inverse of NxN square matrix
	---------------------------------------------
	1) Find the matrix of minors
		- Minor for any ij element is the determinant of a matrix without the i row and j column
		- For a 4x4 square matrix, have to calculate 16 3x3 matrices and 3 2x2 matrices for each 3x3 matrix
	2) Turn the matrix of minors into matrix of cofactors
		- Cofactor of any ij_minor is (-1)^(i+j) * ij_minor
	3) Find the adjacent matrix by transposing the matrix of cofactors
	4) Divide the adjacent matrix by the determinant of the matrix
		- Pick a row or column (if it has one or more zeros, that can be used for optimization)
		- Get the minors of the elements of that row or column (have to expand until we have 2x2 matrices)
		- Turn these minors into cofactors
		- Use Laplace expansion to get the determinant

	Formula: inverse(A) = 1/det(A) * adj(A), where adj(A) = transpose(C) and Cij = (-1)^(i+j) * Mij,
			 where Mij is the minor of the ij element

	This function is based on a largly optimized version from:
		3D Game Engine Design: A Practical Approach to Real-Time Computer Graphics, 2nd edition
		http://geometrictools.com/
	*/

	KPMatrix mTrans;	// Transposed matrix
	float fTemp[12];	// Cofactors
	float fDet;			// Determinant

	mTrans.TransposeOf(m);		// Get the transposed matrix of m

	// Pairs for the first 8 cofactors
	fTemp[ 0]  = mTrans._33 * mTrans._44;
	fTemp[ 1]  = mTrans._34 * mTrans._43;
	fTemp[ 2]  = mTrans._32 * mTrans._44;
	fTemp[ 3]  = mTrans._34 * mTrans._42;
	fTemp[ 4]  = mTrans._32 * mTrans._43;
	fTemp[ 5]  = mTrans._33 * mTrans._42;
	fTemp[ 6]  = mTrans._31 * mTrans._44;
	fTemp[ 7]  = mTrans._34 * mTrans._41;
	fTemp[ 8]  = mTrans._31 * mTrans._43;
	fTemp[ 9]  = mTrans._33 * mTrans._41;
	fTemp[10]  = mTrans._31 * mTrans._42;
	fTemp[11]  = mTrans._32 * mTrans._41;

	// Calculate the first 8 cofactors
	// (Pick an element, get it's cofactor, multiply the two)
	this->_11  = fTemp[0]*mTrans._22 + fTemp[3]*mTrans._23 + fTemp[4] *mTrans._24;
	this->_11 -= fTemp[1]*mTrans._22 + fTemp[2]*mTrans._23 + fTemp[5] *mTrans._24;
	this->_12  = fTemp[1]*mTrans._21 + fTemp[6]*mTrans._23 + fTemp[9] *mTrans._24;
	this->_12 -= fTemp[0]*mTrans._21 + fTemp[7]*mTrans._23 + fTemp[8] *mTrans._24;
	this->_13  = fTemp[2]*mTrans._21 + fTemp[7]*mTrans._22 + fTemp[10]*mTrans._24;
	this->_13 -= fTemp[3]*mTrans._21 + fTemp[6]*mTrans._22 + fTemp[11]*mTrans._24;
	this->_14  = fTemp[5]*mTrans._21 + fTemp[8]*mTrans._22 + fTemp[11]*mTrans._23;
	this->_14 -= fTemp[4]*mTrans._21 + fTemp[9]*mTrans._22 + fTemp[10]*mTrans._23;
	this->_21  = fTemp[1]*mTrans._12 + fTemp[2]*mTrans._13 + fTemp[5] *mTrans._14;
	this->_21 -= fTemp[0]*mTrans._12 + fTemp[3]*mTrans._13 + fTemp[4] *mTrans._14;
	this->_22  = fTemp[0]*mTrans._11 + fTemp[7]*mTrans._13 + fTemp[8] *mTrans._14;
	this->_22 -= fTemp[1]*mTrans._11 + fTemp[6]*mTrans._13 + fTemp[9] *mTrans._14;
	this->_23  = fTemp[3]*mTrans._11 + fTemp[6]*mTrans._12 + fTemp[11]*mTrans._14;
	this->_23 -= fTemp[2]*mTrans._11 + fTemp[7]*mTrans._12 + fTemp[10]*mTrans._14;
	this->_24  = fTemp[4]*mTrans._11 + fTemp[9]*mTrans._12 + fTemp[10]*mTrans._13;
	this->_24 -= fTemp[5]*mTrans._11 + fTemp[8]*mTrans._12 + fTemp[11]*mTrans._13;

	// Pairs for the second 8 cofactors
	fTemp[ 0]  = mTrans._13 * mTrans._24;
	fTemp[ 1]  = mTrans._14 * mTrans._23;
	fTemp[ 2]  = mTrans._12 * mTrans._24;
	fTemp[ 3]  = mTrans._14 * mTrans._22;
	fTemp[ 4]  = mTrans._12 * mTrans._23;
	fTemp[ 5]  = mTrans._13 * mTrans._22;
	fTemp[ 6]  = mTrans._11 * mTrans._24;
	fTemp[ 7]  = mTrans._14 * mTrans._21;
	fTemp[ 8]  = mTrans._11 * mTrans._23;
	fTemp[ 9]  = mTrans._13 * mTrans._21;
	fTemp[10]  = mTrans._11 * mTrans._22;
	fTemp[11]  = mTrans._12 * mTrans._21;

	// Calculate the second 8 cofactors
	this->_31  = fTemp[0] *mTrans._42 + fTemp[3] *mTrans._43 + fTemp[4] *mTrans._44;
	this->_31 -= fTemp[1] *mTrans._42 + fTemp[2] *mTrans._43 + fTemp[5] *mTrans._44;
	this->_32  = fTemp[1] *mTrans._41 + fTemp[6] *mTrans._43 + fTemp[9] *mTrans._44;
	this->_32 -= fTemp[0] *mTrans._41 + fTemp[7] *mTrans._43 + fTemp[8] *mTrans._44;
	this->_33  = fTemp[2] *mTrans._41 + fTemp[7] *mTrans._42 + fTemp[10]*mTrans._44;
	this->_33 -= fTemp[3] *mTrans._41 + fTemp[6] *mTrans._42 + fTemp[11]*mTrans._44;
	this->_34  = fTemp[5] *mTrans._41 + fTemp[8] *mTrans._42 + fTemp[11]*mTrans._43;
	this->_34 -= fTemp[4] *mTrans._41 + fTemp[9] *mTrans._42 + fTemp[10]*mTrans._43;
	this->_41  = fTemp[2] *mTrans._33 + fTemp[5] *mTrans._34 + fTemp[1] *mTrans._32;
	this->_41 -= fTemp[4] *mTrans._34 + fTemp[0] *mTrans._32 + fTemp[3] *mTrans._33;
	this->_42  = fTemp[8] *mTrans._34 + fTemp[0] *mTrans._31 + fTemp[7] *mTrans._33;
	this->_42 -= fTemp[6] *mTrans._33 + fTemp[9] *mTrans._34 + fTemp[1] *mTrans._31;
	this->_43  = fTemp[6] *mTrans._32 + fTemp[11]*mTrans._34 + fTemp[3] *mTrans._31;
	this->_43 -= fTemp[10]*mTrans._34 + fTemp[2] *mTrans._31 + fTemp[7] *mTrans._32;
	this->_44  = fTemp[10]*mTrans._33 + fTemp[4] *mTrans._31 + fTemp[9] *mTrans._32;
	this->_44 -= fTemp[8] *mTrans._32 + fTemp[11]*mTrans._33 + fTemp[5] *mTrans._31;

	// Calcualte the determinant with Laplace expansion using the first row
	fDet = mTrans._11*this->_11 + mTrans._12*this->_12 + mTrans._13*this->_13 + mTrans._14*this->_14;

	// Get the inverse of the determinant and divide the matrix with it to get the inverse
	fDet = 1/fDet;

	this->_11 *= fDet;  
	this->_12 *= fDet;  
	this->_13 *= fDet;  
	this->_14 *= fDet;

	this->_21 *= fDet;  
	this->_22 *= fDet;  
	this->_23 *= fDet;  
	this->_24 *= fDet;

	this->_31 *= fDet;  
	this->_32 *= fDet;  
	this->_33 *= fDet;  
	this->_34 *= fDet;

	this->_41 *= fDet;  
	this->_42 *= fDet;  
	this->_43 *= fDet;  
	this->_44 *= fDet;

} // ! KPMatrix::InverseOf()