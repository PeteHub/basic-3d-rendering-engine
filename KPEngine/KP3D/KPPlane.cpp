/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - August 2009
 *
 *  File: KP.h
 *  Description: KPEngine Plane Class implementation
 *
 *****************************************************************
*/

#include "KP3D.h"

inline void KPPlane::Set(const KPVector &vcNormal, const KPVector &vcPoint)
{
	m_fDistance = - ( vcNormal * vcPoint );
	m_vcNormal	= vcNormal;
	m_vcPoint	= vcPoint;
}

inline void KPPlane::Set(const KPVector &vcNormal, const KPVector &vcPoint, float fDistance)
{
	m_fDistance = fDistance;
	m_vcNormal	= vcNormal;
	m_vcPoint	= vcPoint;
}

inline void KPPlane::Set(const KPVector &v0, const KPVector &v1, const KPVector &v2)
{
	KPVector vcEdge1 = v1 - v0;
	KPVector vcEdge2 = v2 - v0;

	m_vcNormal.Cross(vcEdge1, vcEdge2);
	m_vcNormal.Normalize();

	// Any point would do, we pick v0; distance = - Normal * Point

	m_fDistance = - ( m_vcNormal * v0 );

	m_vcPoint	= v0;
}
