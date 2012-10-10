/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - August 2009
 *
 *  File: KPD3D_vcache.h
 *  Description: Direct3D Cache Management
 *				 - Vertex Cache 
 *				 - Vertex Cache Manager
 *
 *****************************************************************
*/

#ifndef KPD3DVCACHE_H
#define KPD3DVCACHE_H

#include "KPD3D.h"
#include "KPD3DSkinManager.h"

#define KPNUMCACHES 10			// Number of caches used by the Vertex Cache Manager

class KPD3DVertexCache;
class KPD3DVertexCacheManager;
struct KPSTATICBUFFER;


// Vertex Cache ////
////////////////////
//
//	Stores triangles having the same material and texture (skin) that can be rendered together in one call.
//  
class KPD3DVertexCache
{
	// http://www.mvps.org/directx/articles/vcache.htm

	public:
		KPD3DVertexCache(UINT nMaxVertices, UINT nMaxIndices, UINT nStride, KPD3DSkinManager *pSkinManager,
						 LPDIRECT3DDEVICE9 pDevice, KPD3DVertexCacheManager *pVCM, DWORD dwID, DWORD dwFVF, FILE * pLog);
		~KPD3DVertexCache(void);

		// This is the actual rendering call. Renders the contents of a vertex cache object
		HRESULT Flush();
		
		// Appends data to the buffer, vetrices or vertices and indices.
		HRESULT	Add(UINT nVertices, UINT nIndices, const void *pVertices, const WORD *pIndices);

		// Changes the skin Id of the vertex cache object
		void	SetSkin(UINT nSkinID);

		// Determines whether the vertex cache uses the specified texture
		bool	UsesSkin(UINT nSkinID);

		// Determines whether the vertex cache is empty
		bool	IsEmpty(void);

		// Retrieves the number of vertices in the vertex cache
		int		GetNumVertices(void);

	private:
		LPDIRECT3DVERTEXBUFFER9	m_pVB;				// Vertex buffer
		LPDIRECT3DINDEXBUFFER9	m_pIB;				// Index buffer
		LPDIRECT3DDEVICE9		m_pDevice;			// Rendering Device
		KPSKIN					m_Skin;				// Skin the vertices use
		DWORD					m_dwID;				// Id of the active vertex cache
		DWORD					m_dwFVF;			// Combination of FVF format flags defining the format of the vertices
		UINT					m_nSkinID;			// Skin ID

		KPD3DSkinManager		*m_pSkinManager;	// Skin Manager
		KPD3DVertexCacheManager *m_pVCM;			// Vertex Cache Manager
		FILE					*m_pLog;			// Log file

		UINT					m_numMaxVertices;	// Number of maximum vertices to cache into a single batch
		UINT					m_numMaxIndices;	// Number of maximum indices to cache into a single batch
		UINT					m_numVertices;		// Actual number of vertices in the vertex buffer
		UINT					m_numIndices;		// Actual number of indices int he index buffer
		UINT					m_nStride;			// Size of one vertex

		void	Log(char *chFormat, ... );

}; // ! KPD3D Vertex Cache


// Vertex Cache Manager ////
////////////////////////////
//
// Fills the vertex cache objects by batching similar triangles together.
// Also responsible for managing VRAM static buffers.

class KPD3DVertexCacheManager : public KPVertexCacheManager
{
	public:
		KPD3DVertexCacheManager(KPD3DSkinManager *pSkinManager, LPDIRECT3DDEVICE9 pDevice, KPD3D *pKPD3D,
								UINT numMaxVertices, UINT numMaxIndices, FILE *pLog);
		~KPD3DVertexCacheManager(void);

		// Creates a static buffer and fills it with data. The index pointer is optional. Returns an ID to the
		// created static buffer.
		HRESULT	CreateStaticBuffer(KPVERTEXID VertexID, UINT nSkinID, UINT nVertices, UINT nIndices,
								   const void *pVertices, const WORD *pIndices, UINT *pSBufferID);

		// Renders from user pointer, automatically creates dynamic buffer, uses caching for better performance
		HRESULT	Render(KPVERTEXID VertexID, UINT nSkinID, UINT nVertices, UINT nIndices,
					   const void *pVertices, const WORD *pIndices);
		
		// Renders the static buffer
		HRESULT Render(UINT nSBufferID);

		// Forces all cached dynamic data of a vertex type to be rendered immediately
		// Usually used to render data before changing major rendering settings (for example render state or projection matrices)
		HRESULT ForcedFlush(KPVERTEXID VertexID);

		// Forces all cached dynamic data of all vertex types to be rendered immediately
		// Usually used to render data before changing major rendering settings (for example render state or projection matrices)
		HRESULT	ForcedFlushAll(void);

		// Resets all the active skin, static- and dynamic buffer flags
		void    InvalidateStates(void);

		// Retrives the active vertex cache id
		DWORD			GetActiveCache(void);

		// Defines a vertex cache as active
		void			SetActiveCache(DWORD dwID);

		// Retrieves the handle for the Direct3D device that operates the Vertex Cache Manager
		KPD3D*			GetKPD3D(void);

		// Retrieves the Shading/Filling mode the Direct3D device uses
		KPRENDERSTATE	GetShadeMode(void);

	private:
		// Interfaces
		KPD3DSkinManager	*m_pSkinManager;			// Pointer to the Skin Manager
		LPDIRECT3DDEVICE9	m_pDevice;					// Pointer to the Direct3D device
		KPD3D				*m_pKPD3D;					// Pointer to the parent, that manages the VCM
		
		// Cache objects
		KPSTATICBUFFER		*m_pSB;						// Static Buffer
		KPD3DVertexCache	*m_CacheUU[KPNUMCACHES];	// Untransformed, Unlit dynamic vertex cache
		KPD3DVertexCache	*m_CacheUL[KPNUMCACHES];	// Untransformed, lit dynamic vertex cache

		UINT				m_numSB;					// Number of static buffers
		DWORD				m_dwActiveCache;			// Active dynamic vertex cache ID
		DWORD				m_dwActiveSB;				// Active static buffer ID
		FILE				*m_pLog;					// Log file

		void			Log(char* chFormat, ...);

}; // ! Vertex Cache Manager

// Static Buffer Structure ////
///////////////////////////////
typedef struct KPSTATICBUFFER
{
	int		nStride;				// Size of one vertex
	UINT	nSkinID;				// ID of the skin used by these vertices
	bool	bIndices;				// Are they using index list?
	int		numVertices;			// Number of vertices
	int		numIndices;				// Number of indices
	int		numTriangles;			// Number of triangles
	DWORD	dwFVF;					// FVF flags
	LPDIRECT3DVERTEXBUFFER9 pVB;	// D3D vertex buffer
	LPDIRECT3DINDEXBUFFER9	pIB;	// D3D index buffer

} KPSTATICBUFFER;

#endif // ! KPD3DVCACHE_H