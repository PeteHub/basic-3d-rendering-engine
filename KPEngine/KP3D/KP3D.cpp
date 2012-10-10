/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: KP.h
 *  Description: KPEngine Math Library implementation
 *				 - SSE Support check
 *				 - Some templates for finding max/min of 2 and 3 
 *
 *****************************************************************
*/

#include "KP3D.h"

// GLOBALS ////
bool g_bSSE = false;		// SSE Support, we don't need more for now

// IsSSESupported Function ////
///////////////////////////////
//
// This function checks whether we can use SSE SIMD instruction sets or not
// This could speed up our vector math significantly 
bool IsSSESupported(void)
{
	CPUINFO info;

	if ( ! GetCPUInfo(&info) )
		g_bSSE = false;

	else
	{
		if ( (info.Feature & CPU_FEATURE_SSE) && (info.OS_Support & CPU_FEATURE_SSE) )	// if SSE is supported by both OS & CPU
			g_bSSE = true;
		else
			g_bSSE = false;
	}

	return g_bSSE;
} // ! IsSSESupported()

// Min / Max ////
/////////////////

template <class T> const T KPMax( const T a, const T b )
{
	return (b<a)?a:b;
}

template <class T> const T KPMax( const T a, const T b, const T c )
{
	std::vector<T> sortMe;
	sortMe.resize(3);
	
	sortMe[0] = a;
	sortMe[1] = b;
	sortMe[2] = c;

	std::sort(sortMe.begin(), sortMe.end() );

	return sortMe[2];
}

template <class T> const T KPMin( const T a, const T b )
{
	return (b>a)?a:b;
}

template <class T> const T KPMin( const T a, const T b, const T c )
{
	std::vector<T> sortMe;
	sortMe.resize(3);
	
	sortMe[0] = a;
	sortMe[1] = b;
	sortMe[2] = c;

	std::sort(sortMe.begin(), sortMe.end() );

	return sortMe[0];
}

