/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: KPD3D_main.cpp
 *  Description: Direct3D Render Device definitions
 *
 *****************************************************************
*/

#include "KP.h"
#include "KPD3D.h"

inline DWORD FToDW( float f )
{
	return *((DWORD*)&f);
}
void KPD3D::SetWorldTransform(const KPMatrix *mWorld)
{
	// Render all the vertices from the cache
	// before we change the World transform matrix
	m_pVertexMan->ForcedFlushAll();

	// Set the World Transform Matrix
	if ( !mWorld )
	{
		KPMatrix m;
		m.Identity();
		memcpy(&m_mWorld, &m, sizeof(KPMatrix));	// D3DMATRIX has the same size as KPMatrix
	}
	else
		memcpy(&m_mWorld, mWorld, sizeof(KPMatrix));

	// World matrix changed, recalculate affected matrices
	CalcWorldViewProjMatrix();

	m_pDevice->SetTransform(D3DTS_WORLD, &m_mWorld);

} // ! SetWorldTransform


// Get / Set Active Skin ////
/////////////////////////////
UINT KPD3D::GetActiveSkinID(void)
{
	return m_nActiveSkin;
}

void KPD3D::SetActiveSkinID(UINT nSkinID)
{
	m_nActiveSkin = nSkinID;
}


// SetBackfaceCulling ////
//////////////////////////
/*
	Defines how the renderer deals with backface culling.

	Params
		rs	: KPRENDERSTATE type value specifying the render state
*/
void KPD3D::SetBackfaceCulling(KPRENDERSTATE rs)
{
	// Clear the vertex cache before we change the rendering device
	m_pVertexMan->ForcedFlushAll();

	switch( rs )
	{
	case RS_CULL_CW:
		m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	case RS_CULL_CCW:
		m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	case RS_CULL_NONE:
		m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	default:
		m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	}

} // ! SetBackfaceCulling


// SetDepthBufferMode ////
//////////////////////////
/*
	Defines how the renderer deals with depth buffering (Z-Buffer).

	Params
		rs	: KPRENDERSTATE type value specifying the render state
*/
void KPD3D::SetDepthBufferMode(KPRENDERSTATE rs)
{
	// Clear the vertex cache before we change the rendering device
	m_pVertexMan->ForcedFlushAll();

	switch ( rs )
	{
	case RS_DEPTH_READWRITE:
		m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);	// Enables Z/Depth Buffering
		m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);	// Enable the application to write to the depth buffer
	case RS_DEPTH_READONLY:
		m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);	// Enables Z/Depth Buffering
		m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);	// Disable
	case RS_DEPTH_NONE:
		m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);	// Disable Z/Depth Buffering
		m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);	// Disable
	default:
		m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);	// Disable Z/Depth Buffering
		m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);	// Disable
	}

} // ! SetDepthBufferMode


// SetShadeMode ////
////////////////////
/*
	
*/
void KPD3D::SetShadeMode(KPRENDERSTATE rs, float f, const KPCOLOR *clrWireFrame)
{
	// Clear the vertex cache before we change the rendering device
	m_pVertexMan->ForcedFlushAll();

	// Set new wireframe color
	if ( clrWireFrame )
	{
		memcpy(&m_clrWireframe, clrWireFrame, sizeof(KPCOLOR));
		m_pVertexMan->InvalidateStates();
	}


	// If there is no mode change
	if ( rs == m_ShadeMode )
	{
		// But maybe the size of the points changed
		if ( rs == RS_SHADE_POINTS )
			m_pDevice->SetRenderState(D3DRS_POINTSIZE, FToDW(f) );	// Change point size

		return;

	} // if no mode change

	if ( rs == RS_SHADE_TRIWIRE )
	{
		// Enable wireframe mode
		m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		m_ShadeMode = rs;
	}
	else
	{
		// Enable solid fill mode
		m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		m_ShadeMode = rs;
	}

	if ( rs == RS_SHADE_POINTS )
	{
		// If the point size is > 0
		if ( f > 0.0 )
		{
			m_pDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
			m_pDevice->SetRenderState(D3DRS_POINTSCALEENABLE,  TRUE);		// Point size is in camera space not screen space (scaled)
			m_pDevice->SetRenderState(D3DRS_POINTSIZE,	   FToDW(f));		// Size of point
			m_pDevice->SetRenderState(D3DRS_POINTSIZE_MIN, FToDW(0.00f) );	// Minimum size of the point
			m_pDevice->SetRenderState(D3DRS_POINTSCALE_A, FToDW(0.00f)  );
			m_pDevice->SetRenderState(D3DRS_POINTSCALE_B, FToDW(0.00f)  );
			m_pDevice->SetRenderState(D3DRS_POINTSCALE_C, FToDW(1.00f)  );
		}
		else
		{
			m_pDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
			m_pDevice->SetRenderState(D3DRS_POINTSCALEENABLE,  FALSE);
		}
	}
	else
	{
		m_pDevice->SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
		m_pDevice->SetRenderState(D3DRS_POINTSCALEENABLE,  FALSE);
	}


	m_pVertexMan->InvalidateStates();

} // ! SetShadeMode


// GetShadeMode ////
////////////////////
//
// Retrieves the current render state / shading mode.
KPRENDERSTATE KPD3D::GetShadeMode(void)
{
	return m_ShadeMode;
}

// Get Wireframe Color ////
///////////////////////////
KPCOLOR KPD3D::GetWireColor(void)
{
	return m_clrWireframe;
}

void KPD3D::UseTextures(bool bUse)
{
	if ( m_bUseTextures == bUse)
		return;

	m_pVertexMan->ForcedFlushAll();
	m_pVertexMan->InvalidateStates();

	m_bUseTextures = bUse;

	if ( m_bUseTextures )
		Log("Textures enabled");
	else
		Log("Textures disabled");
}

bool KPD3D::UsesTextures(void)
{
	return m_bUseTextures;
}

KPSkinManager* KPD3D::GetSkinManager(void)
{
	return m_pSkinManager;
}

KPVertexCacheManager* KPD3D::GetVertexManager(void)
{
	return m_pVertexMan;
}
