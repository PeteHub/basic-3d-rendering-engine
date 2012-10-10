/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - August 2009
 *
 *  File: KPD3D_vcm.cpp
 *  Description: Direct3D Vertex Cache Manager definition
 *
 *****************************************************************
*/

#include "KPD3D_vcache.h"


// KPD3DVertexManager ////
//////////////////////////
/*
	Sets up the initial class attributes and creates two dynamic buffers
	for rendering primitive lists.

	Params:
		pSkinManager	: Pointer to a KPD3DSkinManager object
		pDevice			: Pointer to a Direct3D device
		pKPD3D			: Pointer to a Direct3D interface
		numMaxVertices	: UINT Type value specifying the maximum amount of vertices in the dynamic buffers
		numMaxVertices	: UINT Type value specifying the maximum amount of indices in the dynamic buffers
		pLog			: Pointer to a FILE object that can be used for logging data
*/
KPD3DVertexCacheManager::KPD3DVertexCacheManager(KPD3DSkinManager *pSkinManager, LPDIRECT3DDEVICE9 pDevice, KPD3D *pKPD3D,
												 UINT numMaxVertices, UINT numMaxIndices, FILE *pLog)
{
	DWORD dwID = 0;	// Vertex cache ID

	m_pSB			= NULL;
	m_numSB			= 0;
	m_pLog			= pLog;
	m_pDevice		= pDevice;
	m_pKPD3D		= pKPD3D;
	m_pSkinManager	= pSkinManager;
	m_dwActiveCache	= KPNOTEXTURE;	// 65535, this is the maximum a dword can hold
	m_dwActiveSB	= KPNOTEXTURE;	// 65535, this is the maximum a dword can hold

	// Create the buffers
	for ( int i = 0; i < KPNUMCACHES; ++i )
	{
		// Untransformed, Unlit vertex buffer
		m_CacheUU[i] = new KPD3DVertexCache(numMaxVertices, numMaxIndices, sizeof(VERTEX), pSkinManager,
											pDevice, this, ++dwID, FVF_VERTEX, pLog);

		// Untransformed, lit vertex buffer
		m_CacheUL[i] = new KPD3DVertexCache(numMaxVertices, numMaxIndices, sizeof(LVERTEX), pSkinManager,
											pDevice, this, ++dwID, FVF_LVERTEX, pLog);

	}

	Log("successfully initialized.");
	
} // ! Constructor

KPD3DVertexCacheManager::~KPD3DVertexCacheManager(void)
{
	UINT n;

	// Free up the static buffers if there is any
	if ( m_pSB )
	{
		for ( n = 0; n < m_numSB; ++n )
		{
			// Release the D3D vertex buffer
			if ( m_pSB[n].pVB )
			{
				m_pSB[n].pVB->Release();
				m_pSB[n].pVB = NULL;
			}
			
			// Release the D3D index buffer
			if ( m_pSB[n].pIB )
			{
				m_pSB[n].pIB->Release();
				m_pSB[n].pIB = NULL;
			}

		} // ! for n

		free( m_pSB );
		m_pSB = NULL;

	} // ! if SB

	// Release the dynamic vertex caches
	for ( int i = 0; i < KPNUMCACHES; ++i )
	{
		if ( m_CacheUU[i] )
		{
			delete m_CacheUU[i];
			m_CacheUU[i] = NULL;
		}

		if ( m_CacheUL[i] )
		{
			delete m_CacheUL[i];
			m_CacheUL[i] = NULL;
		}

	} // ! for i

	Log("successfully released.");

} // ! Destructor


// RENDER ////
//////////////
/*
	Caches the vertex and index lists. NOTE: The supplised data
	will not be rendered to the screen immediately, only after
	enough data is cached. You can force immediate rendering
	by calling the ForcedFlush, ForceFlushAll functions.

	Parameters:
		VertexID	: KPVERTEXID type object specifying the type of the vertex data
		nSkinID		: UINT type value specifying the ID of the Skin the vertices are using
		nVertices	: UINT type value specifying the amount of vertices
		nIndices	: Uint Type value specifying the amount of indices
		pVertices	: Pointer to an array of vertex data
		pIndices	: Pointer to an array of index data

	Returns:
		KP_OK upon success

*/
HRESULT KPD3DVertexCacheManager::Render(KPVERTEXID VertexID, UINT nSkinID, UINT nVertices, UINT nIndices, 
										const void *pVertices, const WORD *pIndices)
{
	KPD3DVertexCache	**pCache = NULL,		// Pointer to a pointer pointing at a vertex cache
						*pEmptyCache = NULL,	// Pointer to the last empty cache
						*pFullestCache = NULL;  // Pointer to the cache that is the most full

	int	nEmptyVC	= -1;
	int nFullestVC	=  0;

	////
	//  1) Determine the vertex type 
	////

	switch ( VertexID )
	{
	case VID_UU:
		pCache = m_CacheUU;
		break;
	case VID_UL:
		pCache = m_CacheUL;
		break;
	default:
		Log("Render: Invalid vertex type!");
		return KP_INVALIDID;
	}

	pFullestCache = pCache[0];		// Set the fullest cache to the first one.
	m_dwActiveSB  = KPNOTEXTURE;	// Invalidate the currently active static buffer.

	////
	//	2) Search for the most appropriate cache
	////

	// First we check whether any other cache uses the same skin.
	// In that case, we can just try to add our vertices to it fast,
	// without setting a new skin for it.
	for ( int i = 0; i< KPNUMCACHES; ++i )
	{
		if ( pCache[i]->UsesSkin(nSkinID) )
			return pCache[i]->Add(nVertices, nIndices, pVertices, pIndices);

		// If it does not use the same skin:

		// Check if the cache is empty
		if ( pCache[i]->IsEmpty() )
			pEmptyCache = pCache[i];

		// Find the bigger cache
		if ( pCache[i]->GetNumVertices() > pFullestCache->GetNumVertices() )
			pFullestCache = pCache[i];

	} // ! for i

	// If none of the current caches use the same skin,
	// try to add the data to an empty cache
	if ( pEmptyCache )
	{
		pEmptyCache->SetSkin(nSkinID);
		return pEmptyCache->Add(nVertices, nIndices, pVertices, pIndices);
	}

	// If there is no empty cache we have no other option
	// but flushing the fullest cache and reuse that.
	pFullestCache->Flush();	// We render data here :) Cool!
	pFullestCache->SetSkin(nSkinID);
	return pFullestCache->Add(nVertices, nIndices, pVertices, pIndices);

} // Render vertex & index lists


// Render Static Buffer ////
////////////////////////////
/*
	Sends the content of the cache to the renderer device and
	resets the cache to accept new content.
*/
HRESULT KPD3DVertexCacheManager::Render(UINT nSBufferID)
{
	KPRENDERSTATE rs = m_pKPD3D->GetShadeMode();

	// Is this a valid static buffer id?
	if ( nSBufferID >= m_numSB )
	{
		Log("Render: Invalid static buffer id: %d", nSBufferID);
		return KP_INVALIDPARAM;
	}

	/*
	// Is there any data in the static buffer to be rendered?
	if ( m_pSB[nSBufferID].numVertices <= 0 )
		return KP_OK;
	*/

	// The active cache is invalid from now on
	m_dwActiveCache = KPNOTEXTURE;

	// Is this the active cache?
	if ( m_dwActiveSB != nSBufferID )
	{
		// Is the index buffer used?
		if ( m_pSB[nSBufferID].bIndices )
			m_pDevice->SetIndices(m_pSB[nSBufferID].pIB);	// Set the index data

		m_pDevice->SetStreamSource(0, m_pSB[nSBufferID].pVB, 0, m_pSB[nSBufferID].nStride); // Binds the vertex buffer to the 0th device data stream

		// Make this the active static buffer
		m_dwActiveSB = nSBufferID;

	} // ! if not active cache

	// Check whether the device already uses this skin (material/color)
	// If it does, we do not have to send it over the bus again, saving time with it.
	if ( m_pKPD3D->GetActiveSkinID() != m_pSB[nSBufferID].nSkinID )
	{
		// Get the skin object
		KPSKIN *pSkin = &(m_pSkinManager->m_pSkins[m_pSB[nSBufferID].nSkinID]);

		// Set new material for the device
		KPMATERIAL *pMaterial = &(m_pSkinManager->m_pMaterials[pSkin->nMaterial]);
		
		////
		//	 SOLID SHADING
		////
	
		// If it's set to solid shading (filling with a solid color)
		if ( rs == RS_SHADE_SOLID )
		{
			/*
			// Set up a material for the device
			D3DMATERIAL9 mat = {
			pMaterial->Diffuse.fR,	pMaterial->Diffuse.fG,	pMaterial->Diffuse.fB,	pMaterial->Diffuse.fA,
			pMaterial->Ambient.fR,	pMaterial->Ambient.fG,	pMaterial->Ambient.fB,	pMaterial->Ambient.fA, 
			pMaterial->Specular.fR, pMaterial->Specular.fG, pMaterial->Specular.fB, pMaterial->Specular.fA, 
			pMaterial->Emissive.fR, pMaterial->Emissive.fG, pMaterial->Emissive.fB, pMaterial->Emissive.fA,
			pMaterial->fPower
			};
			*/
			m_pDevice->SetMaterial( (D3DMATERIAL9*)pMaterial );

			// Render with textures?
			if ( m_pKPD3D->UsesTextures() )
			{

				// Set the texture for the device
				for (UINT i = 0; i < 8; ++i)
				{
					// If there is a texture
					if ( pSkin->nTexture[i] != KPNOTEXTURE )
					{
						// Prepare texture
						LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)(m_pSkinManager->m_pTextures[pSkin->nTexture[i]].pData);

						// Set the texture
						if ( FAILED( m_pDevice->SetTexture(i, pTexture) ) )
							Log("Render: IDirect3DDevice9::SetTexture() failed Texture: %s Stage: %d", m_pSkinManager->m_pTextures[pSkin->nTexture[i]].Name, i);

						// Set the texture properties
						m_pDevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, 0);			// Use u,v coordinates to position the texture
						m_pDevice->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);	// Select texture color
						m_pDevice->SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CURRENT);
						m_pDevice->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_MODULATE);	// Texture color blending operation 
					}
					else
						m_pDevice->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);		// disable the unused stages

				} // ! for textures

			}
			// Render Without Textures?
			else
			{
				m_pDevice->SetTexture(0, NULL);
				m_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			}

		} // ! if solid shading
		
		////
		//	 WIREFRAME
		////

		// If it's not solid shading, then it's some variation of wireframe mode
		else
		{
			KPCOLOR wireColor = m_pKPD3D->GetWireColor();

			// Set up a material
			D3DMATERIAL9 wireMaterial = {
				wireColor.fR,	wireColor.fG,	wireColor.fB,	wireColor.fA,	// Diffuse
				wireColor.fR,	wireColor.fG,	wireColor.fB,	wireColor.fA,	// Ambient
				0.0f,			0.0f,			0.0f,			1.0f,
				0.0f,			0.0f,			0.0f,			1.0f,
				1.0f
			};

			m_pDevice->SetMaterial(&wireMaterial);

			// We don't need texture for our wireframes
			m_pDevice->SetTexture(0, NULL);
		}

		////
		//	 ALPHA BLENDING
		////

		// Set alpha blending for the device
		if ( pSkin->bAlpha )
		{
			m_pDevice->SetRenderState(D3DRS_ALPHAREF,	50);					// Reference alpha value pixels are tested against
			m_pDevice->SetRenderState(D3DRS_ALPHAFUNC,	D3DCMP_GREATEREQUAL);	// Accept\Reject a pixel based on its alpha value
			m_pDevice->SetRenderState(D3DRS_SRCBLEND,	D3DBLEND_SRCALPHA);		// Alpha blend transparency values
			m_pDevice->SetRenderState(D3DRS_DESTBLEND,	D3DBLEND_INVSRCALPHA);	// Alpha blend transparency values
			m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,  TRUE);			// Per pixel alpha testing based on ALPHAFUNC and ALPHAREF
			m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);			// Enable alpha blended transparency, based on SRC/DEST BLEND	

		}
		else
		{
			m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE,  FALSE);
			m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}

		// Make this the active skin
		m_pKPD3D->SetActiveSkinID( m_pSB[nSBufferID].nSkinID);
	
	} // ! if not active skin

	// Set Flexible Vertex Format
	m_pDevice->SetFVF(m_pSB[nSBufferID].dwFVF);

	////
	//	 RENDERING
	////

	// Do we have indexed primitives?
	if ( m_pSB[nSBufferID].bIndices )
	{

		switch ( rs )
		{
			// Render POINT
		case RS_SHADE_POINTS:
			if ( FAILED( m_pDevice->DrawPrimitive(D3DPT_POINTLIST, 0, m_pSB[nSBufferID].numVertices) ) )
			{
				Log("Render: Unable to render point list");
				return KP_FAIL;
			}
			break;

			// Render LINE LIST
		case RS_SHADE_LINES:
			if ( FAILED( m_pDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, m_pSB[nSBufferID].numVertices, 0, m_pSB[nSBufferID].numIndices/2) ) )
			{
				Log("Render: Unable to render indexed line list");
				return KP_FAIL;
			}
			break;

			// Render HULL WIREFRAME LINESTRIP
		case RS_SHADE_HULLWIRE:
			if ( FAILED( m_pDevice->DrawIndexedPrimitive(D3DPT_LINESTRIP, 0, 0, m_pSB[nSBufferID].numVertices, 0, m_pSB[nSBufferID].numVertices) ) )
			{
				Log("Render: Unable to render indexed hull wireframe linestip");
				return KP_FAIL;
			}
			break;

			// RENDER SOLID OR WIREFRAME POLYGON
		case RS_SHADE_SOLID:
		case RS_SHADE_TRIWIRE:
		default:
			if ( FAILED( m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_pSB[nSBufferID].numVertices, 0, m_pSB[nSBufferID].numTriangles) ) )
			{
				Log("Render: Unable to render indexed triangle wireframe or solid triangle list");
				return KP_FAIL;
			}
		}

	} // ! if indexed primitives
	// else non indexed primitives
	else
	{
		switch ( rs )
		{
			// Render POINT
		case RS_SHADE_POINTS:
			if ( FAILED( m_pDevice->DrawPrimitive(D3DPT_POINTLIST, 0, m_pSB[nSBufferID].numVertices) ) )
			{
				Log("Render: Unable to render point list");
				return KP_FAIL;
			}
			break;

			// Render HULL WIREFRAME LINESTRIP
		case RS_SHADE_LINES:
			if ( FAILED( m_pDevice->DrawPrimitive(D3DPT_LINELIST, 0, m_pSB[nSBufferID].numVertices/2) ) )
			{
				Log("Render: Unable to render non-indexed line list");
				return KP_FAIL;
			}
			break;

			// Render HULL WIREFRAME LINESTRIP
		case RS_SHADE_HULLWIRE:
			if ( FAILED( m_pDevice->DrawPrimitive(D3DPT_LINESTRIP, 0, m_pSB[nSBufferID].numVertices) ) )
			{
				Log("Render: Unable to render non-indexed hull wireframe linestip");
				return KP_FAIL;
			}
			break;

			// RENDER SOLID OR WIREFRAME POLYGON
		case RS_SHADE_SOLID:
		case RS_SHADE_TRIWIRE:
		default:
			if ( FAILED( m_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_pSB[nSBufferID].numTriangles) ) )
			{
				Log("Render: Unable to render non-indexed triangle wireframe or solid triangle list");
				return KP_FAIL;
			}
		}
	}

	return KP_OK;

} // ! Render static buffer


// ForcedFlush ////
///////////////////
/*
	Forces the rendering of all caches of a given vertex type to the backbuffer.
	This will automatically happen when EndScene() is called for the render device.

	Parameters:
		VertexID	: KPVERTEXID type objet specifying the vertex type

	Returns:
		KP_OK			: upon success

		KP_INVALIDID	: upon invalid vertex id
		KP_FAIL			: upon any other error
*/
HRESULT KPD3DVertexCacheManager::ForcedFlush(KPVERTEXID VertexID)
{
	KPD3DVertexCache	**pCache = NULL;	// Pointer to a vertex cache pointer
	HRESULT				hr = KP_OK;

	// Determine which cache to use
	switch ( VertexID )
	{
	case VID_UU:				// Untransformed, unlit vertex
		pCache = m_CacheUU;
		break;

	case VID_UL:				// Untransformed, lit vertex
		pCache = m_CacheUL;	
		break;

	default:					// Unknown vertex type
		Log("ForcedFlush: Invalid vertex type");
		return KP_INVALIDID;
	}

	// Flush the caches
	for ( int i = 0; i < KPNUMCACHES; ++i )
	{
		if ( FAILED( hr = pCache[i]->Flush() ) )
			Log("ForcedFlush: Unable to flush cache. Type: %d, Num: %d", VertexID, i);

	} // ! for num caches

	return hr;

} // ! ForcedFlush

// ForcedFlushAll ////
//////////////////////
/*
	Forces the rendering of all caches of a every vertex type to the backbuffer.
	This will automatically happen when EndScene() is called for the render device.

	Returns:
		KP_OK			: upon success

		KP_FAIL			: upon any other error
*/
HRESULT KPD3DVertexCacheManager::ForcedFlushAll(void)
{
	HRESULT hr = KP_OK;

	// Flush all the UU caches
	for ( int i = 0; i < KPNUMCACHES; ++i )
	{
		if ( ! m_CacheUU[i]->IsEmpty() )
			if ( FAILED ( hr = m_CacheUU[i]->Flush() ) )
				Log("ForcedFlushAll: Unable to flush UU Cache! Id: %d", i);

	}

	// Flush all the UU caches
	for ( int i = 0; i < KPNUMCACHES; ++i )
	{
		if ( ! m_CacheUL[i]->IsEmpty() )
			if ( FAILED ( hr = m_CacheUL[i]->Flush() ) )
				Log("ForcedFlushAll: Unable to flush UL Cache! Id: %d", i);
	}

	return hr;

} // ! ForcedFlushAll


// Invalidate States ////
/////////////////////////
//
// Invalidates all of the active Skin, Static and Dinamic cache flags
void KPD3DVertexCacheManager::InvalidateStates(void)
{
	m_pKPD3D->SetActiveSkinID(KPNOTEXTURE);
	m_dwActiveSB	= KPNOTEXTURE;
	m_dwActiveCache = KPNOTEXTURE;
}


// Create Static Buffer ////
////////////////////////////
/*
	Creates a static buffer for the supplied data. Once the static buffer is
	created it can not be changed.

	Parameters:
		VertexID	: KPVERTEXID type object specifying the vertex format
		nSkinID		: UINT type falue specifying the Skin the vertices use
		nVertices	: UINT type value specifying the number of vertices
		nIndices	: UINT type value specifying the number of indices
		pVertices	: Pointer to the vertex data
		pIndices	: Pointer to the index data
		pSBufferID	: [OUT] Pointer to an UINT type value the ID of the new static buffer can be returned into

	Returns:
		KP_OK			: upon success

		KP_OUTOFMEMORY	: upon not enough memory
		KP_INVALIDID	: upon invalid vertex format id
		KP_CREATEBUFFER	: upon failure to create vertex or index buffer
		KP_BUFFERLOCK	: upon failure to lock the buffer
*/
HRESULT KPD3DVertexCacheManager::CreateStaticBuffer(KPVERTEXID VertexID, UINT nSkinID, UINT nVertices, UINT nIndices, 
													const void *pVertices, const WORD *pIndices, UINT *pSBufferID)
{

	HRESULT hr;
	DWORD	dwActualFVF;
	void	*pData;

	// Check if we have enough space for another static buffer
	if ( m_numSB >= KPMAX_ID )
	{
		Log("CreateStaticBuffer: Unable to create static buffer: OUT_OF_MEMORY. SB Nr: %d", m_numSB);
		return KP_OUTOFMEMORY;
	}

	// Extend our static cache array if neccessary
	// every time by 50 extra object slots
	if ( (m_numSB % 25 ) == 0 )
	{
		int nSize = (m_numSB+25) * sizeof(KPSTATICBUFFER);
		
		void* tmp = realloc(m_pSB, nSize);
		if ( tmp != NULL )
		{
			m_pSB = (KPSTATICBUFFER*)tmp;
			tmp = NULL;
		}
		else
		{
			free(m_pSB);
			free(tmp);
			Log("CreateStaticBuffer: Unable to extend static buffer: OUT_OF_MEMORY. SB Nr: %d, New size: %d", m_numSB, nSize);
			return KP_OUTOFMEMORY;
		}

		/*
		m_pSB = (KPSTATICBUFFER*)realloc(m_pSB, nSize);

		if ( ! m_pSB )
		{
			Log("CreateStaticBuffer: Unable to extend static buffer: OUT_OF_MEMORY. SB Nr: %d, New size: %d", m_numSB, nSize);
			return KP_OUTOFMEMORY;
		}
		*/
	}

	// Set Static Buffer properties
	m_pSB[m_numSB].numVertices	= nVertices;
	m_pSB[m_numSB].numIndices	= nIndices;
	m_pSB[m_numSB].nSkinID		= nSkinID;
	m_pSB[m_numSB].pVB			= NULL;
	m_pSB[m_numSB].pIB			= NULL;

	// Determine the size and format of vertices
	switch ( VertexID )
	{
	case VID_UU:
		m_pSB[m_numSB].nStride	= sizeof(VERTEX);
		m_pSB[m_numSB].dwFVF	= FVF_VERTEX;
		break;
	case VID_UL:
		m_pSB[m_numSB].nStride	= sizeof(LVERTEX);
		m_pSB[m_numSB].dwFVF	= FVF_LVERTEX;
		break;
	default:
		Log("CreateStaticBuffer: Invalid vertex id: %d", VertexID );
		return KP_INVALIDID;
	}

	////
	//  Create the index buffer if it is required
	////

	if ( nIndices > 0 )
	{
		m_pSB[m_numSB].bIndices	= true;
		m_pSB[m_numSB].numTriangles = nIndices / 3;

		hr = m_pDevice->CreateIndexBuffer(nIndices * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT,
										  &m_pSB[m_numSB].pIB, NULL);
		if ( FAILED(hr) )
		{
			Log("CreateStaticBuffer: Unable to create index buffer");
			return KP_CREATEBUFFER;
		}

		// Lock the index buffer
		if( SUCCEEDED( m_pSB[m_numSB].pIB->Lock(0, 0, (void**)(&pData), 0) ) )
		{
			memcpy(pData, pIndices, nIndices*sizeof(WORD));
			m_pSB[m_numSB].pIB->Unlock();
		}
		else
		{
			Log("CreateStaticBuffer: Unable to lock index buffer. SB id: %d, VID: %d", m_numSB, VertexID);
			return KP_BUFFERLOCK;
		}

	} // ! if indices
	else
	{
		m_pSB[m_numSB].bIndices = false;
		m_pSB[m_numSB].numTriangles = nVertices/3;
		m_pSB[m_numSB].pIB = NULL;
	}

	// Update the actual FVF flag
	dwActualFVF = m_pSB[m_numSB].dwFVF;

	////
	//  Create vertex buffer
	////
	
	hr = m_pDevice->CreateVertexBuffer(nVertices*m_pSB[m_numSB].nStride, D3DUSAGE_WRITEONLY, dwActualFVF, D3DPOOL_DEFAULT,
									   &m_pSB[m_numSB].pVB, NULL);

	if ( FAILED(hr) )
	{
		Log("CreateStaticBuffer: Unable to create vertex buffer");
		return KP_CREATEBUFFER;
	}

	// Lock the index buffer
	if( SUCCEEDED( m_pSB[m_numSB].pVB->Lock(0, 0, (void**)(&pData), 0) ) )
	{
		memcpy(pData, pVertices, nVertices*m_pSB[m_numSB].nStride);
		m_pSB[m_numSB].pVB->Unlock();
	}
	else
	{
		Log("CreateStaticBuffer: Unable to lock vertex buffer. SB id: %d, VID: %d", m_numSB, VertexID);
		return KP_BUFFERLOCK;
	}

	// Return the new Static Buffer ID
	*pSBufferID = m_numSB;

	++m_numSB;	// increment it

	return KP_OK;

} // ! CreateStaticBuffer



// Log Function ////
////////////////////
void KPD3DVertexCacheManager::Log(char *chFormat, ...)
{
	char msg[256];
	char *pArgs;

	// Point at the argument list
	pArgs = (char*) &chFormat + sizeof(chFormat);

	// Convert arguments to message using the format string
	vsprintf_s(msg, sizeof(msg), chFormat, pArgs);

	fprintf(m_pLog, "[ KPD3DVertexCacheManager ]: ");
	fprintf(m_pLog, msg);
	fprintf(m_pLog, "\n");

//#ifdef _DEBUG
	// Instantly write the buffer into the log file
	// This way, even if the program crashes, you get the information
	fflush(m_pLog);
//#endif

} // ! Log

DWORD KPD3DVertexCacheManager::GetActiveCache(void)
{
	return m_dwActiveCache;
}

void KPD3DVertexCacheManager::SetActiveCache(DWORD dwID)
{
	m_dwActiveCache = dwID;
}

KPD3D* KPD3DVertexCacheManager::GetKPD3D(void)
{
	return m_pKPD3D;
}

KPRENDERSTATE KPD3DVertexCacheManager::GetShadeMode(void)
{
	return m_pKPD3D->GetShadeMode();
}


