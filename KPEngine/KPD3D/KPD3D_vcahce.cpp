/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - August 2009
 *
 *  File: KPD3D_vcache.cpp
 *  Description: Direct3D Vertex Cache definition
 *
 *****************************************************************
*/

#include "KPD3D_vcache.h"

bool KPD3DVertexCache::UsesSkin(UINT nSkinID)
{
	return (m_nSkinID == nSkinID);
}

bool KPD3DVertexCache::IsEmpty(void)
{
	if ( 0 < m_numVertices )
		return false;

	return true;
}

int KPD3DVertexCache::GetNumVertices(void)
{
	return m_numVertices;
}

// Log Function ////
////////////////////
void KPD3DVertexCache::Log(char *chFormat, ...)
{
	char msg[256];
	char *pArgs;

	// Point at the argument list
	pArgs = (char*) &chFormat + sizeof(chFormat);

	// Convert arguments to message using the format string
	vsprintf_s(msg, sizeof(msg), chFormat, pArgs);

	fprintf(m_pLog, "[ KPD3DVertexCache ]: ");
	fprintf(m_pLog, msg);
	fprintf(m_pLog, "\n");

//#ifdef _DEBUG
	// Instantly write the buffer into the log file
	// This way, even if the program crashes, you get the information
	fflush(m_pLog);
//#endif

} // ! ::Log()


// KPD3DVertexCache ////
////////////////////////
/*
	Creates the a dynamic vertex cache object.

	Params:
		nMaxVertices	: UINT type value specifying the maximum amount of vertices to be cached in a batch
		nMaxIndices		: UINT type value specifying the maximum amount of indices to be cached in a batch
		UINT			: UINT type value specifying the stride of a vertex (size of a single vertex element)
		pSkinmanager	: Address of a KPD3DSkinManager object
		pDevice			: Address of a Direct3D9 device
		pVCM			: Address of a KPD3DVertexCacheManager object managing this cache
		dwID			: DWORD type value specifying the ID of this vertex cache
		dwFVF			: DWORD type value specifying a combination of FVF format flags defining the format of the vertices
		pLog			: Address of a FILE object used for logging important messages
*/
KPD3DVertexCache::KPD3DVertexCache(UINT nMaxVertices, UINT nMaxIndices, UINT nStride, KPD3DSkinManager *pSkinManager, 
								   LPDIRECT3DDEVICE9 pDevice, KPD3DVertexCacheManager *pVCM, DWORD dwID, DWORD dwFVF, FILE *pLog)
{
	m_pDevice			= pDevice;
	m_pSkinManager		= pSkinManager;
	m_pVCM				= pVCM;
	m_numMaxVertices	= nMaxVertices;
	m_numMaxIndices		= nMaxIndices;
	m_numVertices		= 0;
	m_numIndices		= 0;
	m_dwID				= dwID;
	m_nStride			= nStride;
	m_pLog				= pLog;
	m_pVB				= NULL;
	m_pIB				= NULL;

	HRESULT	hr;

	// Set the skin to KPNOTEXTURE, being an impossible value to get serving as 'NULL'
	m_Skin.nMaterial = KPNOTEXTURE;
	for ( UINT i = 0; i < 8; ++i )
		m_Skin.nTexture[i] = KPNOTEXTURE;

	m_nSkinID = KPNOTEXTURE;

	// Create a non FVF dynamic & write only vertex buffer
	hr = pDevice->CreateVertexBuffer(nMaxVertices * nStride, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 
									 0, D3DPOOL_DEFAULT, &m_pVB, NULL);

	if ( FAILED(hr) )
	{
		Log("Unable to create vertex buffer");
		m_pVB	= NULL;
	}

	// Create a dynamic & write only index buffer
	hr = pDevice->CreateIndexBuffer(nMaxIndices * sizeof(WORD), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 
									D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, NULL);
	if ( FAILED(hr) )
	{
		Log("Unable to create index buffer");
		m_pIB	= NULL;
	}

} // ! KPD3DVertexCache

KPD3DVertexCache::~KPD3DVertexCache(void)
{
	if ( m_pVB )
	{
		m_pVB->Release();
		m_pVB = NULL;
	}
	if ( m_pIB )
	{
		m_pIB->Release();
		m_pIB = NULL;
	}
}

// SetSkin ////
///////////////
/*
	Sets the texture and material combination used by vertices of this cache object

	Params:
		nSkinID		: UINT type value specifying the ID of the Skin to be set
*/
void KPD3DVertexCache::SetSkin(UINT nSkinID)
{
	// Prior to changing the skin of the cache, we
	// must flush the current content, because it was set up
	// using a different skin
	if ( ! UsesSkin(nSkinID) )
	{
		KPSKIN *pNewSkin = &(m_pSkinManager->GetSkin(nSkinID));

		if ( ! IsEmpty() )
			Flush();	// Render the currently cached stuff before changing the cache properties

		// Set the new skin
		memcpy(&m_Skin, pNewSkin, sizeof(KPSKIN));
		m_nSkinID = nSkinID;

		// Let the vertex cache manager know that things have changed
		// so the active cache is not valid/active anymore
		m_pVCM->InvalidateStates();
	}

} // ! SetSkin


// Add ////
///////////
/*
	Appends vertex and index data to the cache. If the cache is full,
	it flushes it before. 

	Params:
		nVertices		: UINT type value specifying the number of vertices to add to the cache
		nIndices		: UINT type value specifying the number of indices to add to the cache.
		pVertices		: Pointer to a vertex list
		pIndices		: Pointer to an index list. This value is optional. If no index list is
						  is available, use NULL.
*/
HRESULT KPD3DVertexCache::Add(UINT nVertices, UINT nIndices, const void *pVertices, const WORD *pIndices)
{
	DWORD	dwFlags;							// Flags for D3D
	WORD	*tmpI	= NULL;						// Pointer to index buffer
	BYTE	*tmpV	= NULL;						// Pointer to vertex buffer
	int		nSizeV	= m_nStride * nVertices;	// Size of the vertex data
	int		nSizeI	= sizeof(WORD) * nIndices;  // Size of the index data
	int		nPosV;								// Vertex index 
	int		nPosI;								// Index index :)

	// First check if the data fits into our bffer
	if ( nVertices > m_numMaxVertices || nIndices > m_numMaxIndices )
	{
		Log("Add: Data can't fit into cace! nV:%d // %d, nI:%d // %d",nVertices, m_numMaxVertices, nIndices, m_numMaxIndices);
		return KP_BUFFERSIZE;
	}

	// Now check whether the data fits into our current cache
	// or we will have to flush it out first
	if ( (nVertices+m_numVertices > m_numMaxVertices) || (nIndices+m_numIndices > m_numMaxIndices ) )
	{
		if ( FAILED( Flush() ) )
		{
			Log("Add: Unable to flush vertex cache");
			return KP_FAIL;
		}
	}

	// If the cache is totally empty
	// we ask Direct3D to discard the current cache memory upon lock
	if ( m_numVertices == 0 )
	{
		// reset the indices
		nPosV = nPosI = 0;

		// Set the discard flag
		dwFlags = D3DLOCK_DISCARD;
	}
	// Otherwise prepare for appending data
	else
	{
		nPosV = m_nStride	 * m_numVertices;
		nPosI = sizeof(WORD) * m_numIndices;

		// Make sure we are not overwriting anything. This speeds up appending a bit on D3D's part.
		dwFlags = D3DLOCK_NOOVERWRITE;
	}

	// Lock the vertex cache and obtain pointers to the buffer memories
	if ( FAILED( m_pVB->Lock(nPosV, nSizeV, (void**)&tmpV, dwFlags) ) )
	{
		Log("Add: Unable to lock the vertex buffer");
		return KP_BUFFERLOCK;
	}

	if ( FAILED( m_pIB->Lock(nPosI, nSizeI, (void**)&tmpI, dwFlags) ) )
	{
		Log("Add: Unable to lock the index buffer");
		return KP_BUFFERLOCK;
	}


	// Now we can append our vertex data to the vertex buffer
	memcpy(tmpV, pVertices, nSizeV);
	m_numVertices += nVertices;					// update the number of verticles

	// And the index data to the index buffer
	int nLastIndex = m_numIndices;				// The latest index of the cache

	if ( ! pIndices )
		nIndices = nVertices;					// if no index list is supplied, the number of indices = number of verticles

	for (UINT i = 0; i < nIndices; ++i)
	{
		if ( pIndices != NULL )
			tmpI[i] = pIndices[i] + nLastIndex; // Last index of old cache + this index
		else
			tmpI[i] = i + nLastIndex;			// Else just increment the last index of the old cache by i

		++m_numIndices;							// update the number of indexes
	}


	// Finally, unlock the buffers
	m_pVB->Unlock();
	m_pIB->Unlock();

	return KP_OK;

} // ! Add


// Flush ////
/////////////
/*
	Sends the content of the cache to the renderer device and
	resets the cache to accept new content.
*/
HRESULT KPD3DVertexCache::Flush()
{
	KPRENDERSTATE rs = m_pVCM->GetKPD3D()->GetShadeMode();

	// Is there any data in the cache to render?
	if ( m_numVertices <= 0 )
		return KP_OK;

	// Is this the active cache?
	if ( m_pVCM->GetActiveCache() != m_dwID )
	{
		// Set Flexible Vertex Format flags
		m_pDevice->SetFVF(m_dwFVF);

		m_pDevice->SetIndices(m_pIB);						// Set the index data
		m_pDevice->SetStreamSource(0, m_pVB, 0, m_nStride); // Binds the vertex buffer to the 0th device data stream

		// Make this the active cache
		m_pVCM->SetActiveCache(m_dwID);

	} // ! if not active cache

	// Check whether the device already uses this skin (material/color)
	// If it does, we do nothave to send it over the bus again, saving time with it.
	if ( m_pVCM->GetKPD3D()->GetActiveSkinID() != m_nSkinID )
	{
		// Set new material for the device
		LPDIRECT3DTEXTURE9	pTexture	= NULL;
		KPMATERIAL		    *pMaterial	= &(m_pSkinManager->m_pMaterials[m_Skin.nMaterial]);
		
		////
		//	 SOLID SHADING
		////
	
		// If it's set to solid shading (filling with a solid color)
		if ( rs == RS_SHADE_SOLID )
		{
			
			// Set up a material for the device
			m_pDevice->SetMaterial( (D3DMATERIAL9*)pMaterial );


			// Set the texture for the device
			for (UINT i = 0; i < 8; ++i)
			{
				// If there is a texture
				if ( m_Skin.nTexture[i] != KPNOTEXTURE )
				{
					pTexture = (LPDIRECT3DTEXTURE9)(m_pSkinManager->m_pTextures[m_Skin.nTexture[i]].pData);

					// Set the texture
					if ( FAILED( m_pDevice->SetTexture(i, pTexture) ) )
						Log("Flush: IDirect3DDevice9::SetTexture() failed Texture: %s Stage: %d", m_pSkinManager->m_pTextures[m_Skin.nTexture[i]].Name, i);

				}
				else
					// No more textures, we can break from the loop
					break;

			} // ! for



		} // ! if solid shading
		
		////
		//	 WIREFRAME
		////

		// If it's not solid shading, then it's some variation of wireframe mode
		else
		{
			KPCOLOR wireColor = m_pVCM->GetKPD3D()->GetWireColor();

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
		if ( m_Skin.bAlpha )
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
		m_pVCM->GetKPD3D()->SetActiveSkinID(m_nSkinID);
	
	} // ! if not active skin


	////
	//	 RENDERING
	////

	// Choose which primite type to render

	switch ( rs )
	{
	// Render POINT
	case RS_SHADE_POINTS:
		if ( FAILED( m_pDevice->DrawPrimitive(D3DPT_POINTLIST, 0, m_numVertices) ) )
		{
			Log("Flush: Unable to render point list");
			return KP_FAIL;
		}
		break;

	// Render LINE LIST
	case RS_SHADE_LINES:
		if ( FAILED( m_pDevice->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, m_numVertices, 0, m_numIndices/2) ) )
		{
			Log("Flush: Unable to render hull wireframe linestip");
			return KP_FAIL;
		}
		break;

	// Render HULL WIREFRAME LINESTRIP
	case RS_SHADE_HULLWIRE:
		if ( FAILED( m_pDevice->DrawIndexedPrimitive(D3DPT_LINESTRIP, 0, 0, m_numVertices, 0, m_numVertices) ) )
		{
			Log("Flush: Unable to render hull wireframe linestip");
			return KP_FAIL;
		}
		break;

	// RENDER SOLID OR WIREFRAME POLYGON
	case RS_SHADE_SOLID:
	case RS_SHADE_TRIWIRE:
	default:
		if ( FAILED( m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_numVertices, 0, m_numIndices/3) ) )
		{
			Log("Flush: Unable to render triangle wireframe or solid triangle list");
			return KP_FAIL;
		}
	}


	// Reset the cache counters
	m_numVertices	= 0;
	m_numIndices	= 0;

	return KP_OK;

} // ! Flush
