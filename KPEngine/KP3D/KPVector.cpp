/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: KP.h
 *  Description: KPEngine Vector 4d Class Implementation
 *
 *****************************************************************
*/

#include "KP3D.h"

// Globals ////
extern bool g_bSSE;

// KPVector::Set Method ////
inline void KPVector::Set(float _x, float _y, float _z, float _w)
{
	x = _x;
	y = _y;
	z = _z;
	w = _w;
}


// KPVector::GetSqaredLength ////
inline float KPVector::GetSqaredLength() const
{
	return (x*x, y*y, z*z);
}


// KPVector::Negate ////
inline void KPVector::Negate(void)
{
	x = -x;
	y = -y;
	z = -z;
}


// KPVector::Difference ////
inline void KPVector::Difference(const KPVector &v1, const KPVector &v2)
{
	x = v2.x - v1.x;
	y = v2.y - v1.y;
	z = v2.z - v1.z;
	w = 1.0f;
}


// KPVector::AngleWith ////
///////////////////////////
//
// Calculates the angle between two vectors
// Math: acos(v1.v2 / |v1| * |v2|) in rads
inline float KPVector::AngleWith(KPVector &v)
{
	return (float)acos( ( (*this) * v) / ( this->GetLength() * v.GetLength() ) );
}


// KPVector::GetLength ////
///////////////////////////
//
// Determining a Vectors magnitude(length) involves a square root operation
// which is considerably slow.
// 
// We will try to take advantage of SIMD instructions here, if possible.
//
// Math: v.Length = sqtr(x*x + y*y + x*x)
inline float KPVector::GetLength(void)
{
	float fLength;

	// If SSE is not available, do the math the slower way.
	if ( ! g_bSSE )
		fLength = sqrt(x*x + y*y + z*z);
	else
	{
		float *pLength = &fLength;
		w = 0.0f;	// Leaving it on 1.0f would make complications at addition

		// Our goal here is to end up with the squared length of the vector
		// in a single element of the register. We will do this by
		// getting the square of all the numbers, then shuffle it around
		// and add them together. At this point we will have the sum of 
		// two numbers in each element of the register.
		// Shuffle once more, add them together again and we will
		// have the same squared length of the vector in each four
		// elements of the register. This is actually faster than
		// only dealing with a single value due to SSE though.
		// After this we can use single instructions to get the square root
		// of the first element and retrieve it.
		__asm {
			mov		ecx,	pLength		// store fLength's address in ECX
			mov		esi,	this		// store our vector's address in ESI
			movups	xmm0,	[esi]		// Copies four packed single precision floats regardless
										// of alignment from memory to the xmm0 SSE register										
			mulps	xmm0,	xmm0		// x*x  y*y  z*z  w*w
			movaps	xmm1,	xmm0		// Copy the result into xmm1, now they are aligned

			shufps	xmm1,	xmm1,	4Eh	// 0100_1110b -> reorders: 3210 -> 1032
			addps	xmm0,	xmm1		// xx+zz yy+ww zz+xx ww+yy
			movaps	xmm1,	xmm0
			shufps	xmm1,	xmm1,	11h // 0001_0001b -> reorders: 3210 -> 0101
										// ww+yy zz+xx ww+yy zz+xx
			addps	xmm0,	xmm1		// xx+zz+ww+yy four times, perfect!
			sqrtss	xmm0,	xmm0		// single scalar square root of first element
			movss	[ecx],	xmm0		// Move the result of the square root to the address
										// ecx is pointing at, which is fLength.
		} // ! asm
		
		w = 1.0f;	// We can set it back to 1.0f now
	} // ! else

	return fLength;

} // ! KPVector::GetLength()


// KPVector::Normalize ////
///////////////////////////
//
// Normalized vectors are vectors in the same direction
// but with the legth 1.
// 
// Math: norm(x) = x / magnitude(x)
inline void KPVector::Normalize(void)
{
	// if SSE is not available, do the math the slower way
	if ( ! g_bSSE )
	{
		float fLength = sqrt(x*x + y*y + z*z);

		// Check for NULL vector
		if ( fLength != 0.0f )
		{
			x /= fLength;
			y /= fLength;
			z /= fLength;
		}
	} // ! SSE Check
	else
	{
		w = 0.0f;	// Set this to 0 to avoid problems at additions

		// To get the normal of the vector, we have to first calculate
		// it's length the same way we did for GetLength.
		// After that we have to scale the vector down with it to unit length.
		// Normal(X) = X / sqrt(x*x+y*y+z*z)
		// Notice that X/SQRT = X * 1/SQRT. Because of this, we can use the
		// RSQRTPS - reciprocal square root SSE instruction
		__asm {
			mov		esi,	this		// Store our vector's address in ESI register
			movups	xmm0,	[esi]		// Copy the vector data to XMM0 register
			movaps	xmm2,	xmm0		// Make a copy of the data to XMM2 for later use
			mulps	xmm0,	xmm0		// x*x y*y z*z w*w
			movaps	xmm1,	xmm0		// Copy it to XMM1
			shufps	xmm1,	xmm1,	4Eh	// Shuffle it around using 0100_0111b
			addps	xmm0,	xmm1		// xx+zz yy+ww zz+xx ww+yy
			movaps	xmm1,	xmm0		// Copy the result into XMM1
			shufps	xmm1,	xmm1,	11h	// Shuffle it again using 0001_0001b
			addps	xmm0,	xmm1		// All four floats are xx+zz+ww+yy now

			rsqrtps	xmm0,	xmm0		// Reciprocal square root of the vector
			mulps	xmm2,	xmm0		// vector * 1/vector_length
			movups	[esi],	xmm2		// replace our vector with the normalized form
		} // ! asm

		w = 1.0f;	// Restore it's value after normalization

	} // !else

} // ! KPVector::Normalize()


// KPVector::Cross ////
///////////////////////
//
// Calculates the cross product from two vectors
// Math: Too long to explain :P See the non SSE implementation
inline void KPVector::Cross(const KPVector &v1, const KPVector &v2)
{
	// If there is no SSE support
	if ( ! g_bSSE )
	{
		x = v1.y * v2.z - v1.z * v2.y;
		y = v1.z * v2.x - v1.x * v2.z;
		z = v1.x * v2.y - v1.y * v2.x;
		w = 1.0f;	// Set w to 1 regardless of v1.w and v2.w
	}
	else
	{
		__asm
		{
			mov		esi,	v1			// Store v1's address in ESI
			mov		edi,	v2			// Store v2's address in EDI

			movups	xmm0,	[esi]		// Copy v1's data to XMM0
			movups	xmm1,	[edi]		// Copy v2's data to XMM1
			movaps	xmm2,	xmm0		// Make a copy of v1 to XMM2 
			movaps	xmm3,	xmm1		// Make a copy of v2 to XMM3

			// Calculating the left side of the subtraction
			shufps	xmm0,	xmm0,	0xD8// 1101_1000b, swaps y and z -> wyzx
			shufps	xmm1,	xmm1,	0xE1// 1110_0001b, swaps x and y -> wzxy
			mulps	xmm0,	xmm1		// <1y*2z,1z*2x,1x*2y,1w*2w>

			// Calculating the right side of the subtraction (inverse of left side)
			shufps	xmm2,	xmm2,	0xE1
			shufps	xmm3,	xmm3,	0xD8
			mulps	xmm2,	xmm3		// <1z*2y,1x*2z,1y*2x,1w*2w>

			subps	xmm0,	xmm2		// Subtract

			mov		esi,	this		// Store the address of out vector in ESI
			movups	[esi],	xmm0		// Copy the cross product into our vector
		} // ! asm
		w = 1.0f;	// Set w to 1 regardless of v1.w and v2.w

	} // ! else
} // ! KPVector::Cross()


// KPVector Operator Overloads ////
///////////////////////////////////

KPVector KPVector::operator +(const KPVector &v) const
{
	return KPVector(x+v.x, y+v.y, z+v.z);
}

void KPVector::operator +=(const KPVector &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
}

KPVector KPVector::operator -(const KPVector &v) const
{
	return KPVector(x-v.x, y-v.y, z-v.z);
}

void KPVector::operator -=(const KPVector &v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

KPVector KPVector::operator *(const float f) const
{
	return KPVector(x*f, y*f, z*f);
}

void KPVector::operator *=(const float f)
{
	x *= f;
	y *= f;
	z *= f;
}

void KPVector::operator /=(const float f)
{
	x /= f;
	y /= f;
	z /= f;
}

float KPVector::operator *(const KPVector &v) const
{
	return (x*v.x + y*v.y + z*v.z);
}

KPVector KPVector::operator *(const KPMatrix &m) const
{
	KPVector vcReturn;

	vcReturn.x = x*m._11 + y*m._21 + z*m._31 + m._41;
	vcReturn.y = x*m._12 + y*m._22 + z*m._32 + m._42;
	vcReturn.z = x*m._13 + y*m._23 + z*m._33 + m._43;
	vcReturn.w = x*m._14 + y*m._24 + z*m._34 + m._44;

	// At this point vcReturn.w has some value, but we want it to be 1.0f
	// we have to scale the matrix down by vcReturn.w for this

	vcReturn.x /= vcReturn.w;
	vcReturn.y /= vcReturn.w;
	vcReturn.z /= vcReturn.w;
	vcReturn.w  = 1.0f;

	return vcReturn;
}

