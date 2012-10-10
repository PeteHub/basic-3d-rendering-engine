/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: KPRenderDevice.h
 *  Description: Rendering related API independant interface definitions
 *				 - Skin Manager Interface
 *				 - Vertex Cache Manager Interface
 *				 - Render Device Interface
 *
 *****************************************************************
*/

#ifndef KPRENDERDEVICE_H
#define KPRENDERDEVICE_H

#include <windows.h>
#include <stdio.h>

#include "../KP3D/KP3D.h"	// Needed for the math library
#include "../KPD3d/KP.h"	// Needed for the material structure

// The number of child windows our engine will be able to render into.
// Very useful for editors with multiple views, like a 3d Model Editor, or World Builder.
#define MAX_3DHWND 4


// SkinManager Interface ////
/////////////////////////////
//
// Creates materials and loads textures while making sure there
// is no redundant texture data.
class KPSkinManager
{
protected:
	KPMATERIAL	*m_pMaterials;		// Materials stored here
	KPTEXTURE	*m_pTextures;		// Textures...
	KPSKIN		*m_pSkins;			// Skins...

	UINT		m_numSkins;			// number of Skins, used as ID too
	UINT		m_numMaterials;		// ...Materials
	UINT		m_numTextures;		// ...Textures

public:
	KPSkinManager(void) { };
	virtual ~KPSkinManager(void) { };

	// Creates a new skin object (and a new material for it) and adds it to the manager.
	// *nSkinID will contain the ID of the new skin
	virtual HRESULT			AddSkin(const KPCOLOR *pAmbient, const KPCOLOR *pDiffuse, const KPCOLOR *pEmissive,
									const KPCOLOR *pSpecular, float fPower, UINT *nSkinID)=0;

	// Creates a new texture object and adds it to the manager
	virtual HRESULT			AddTexture(UINT nSkinID, const char *chName, bool bAlpha, float fAlpha,
									   KPCOLOR *pColorKeys, DWORD numColorKeys)=0;

	// Access data members
	virtual KPSKIN			GetSkin(UINT nSkinID)=0;

}; // ! KPSkinManager


// Vertex Cache Manager /////
/////////////////////////////
//
// This class manages the static vertex buffer and index buffer in the VRAM,
// dynamic vertex buffers in RAM, vertex reordering, batching and performance
// optimal rendering only:P
class KPVertexCacheManager
{
	public:
		KPVertexCacheManager(void) { };
		virtual ~KPVertexCacheManager(void) { };

		// Creates a static buffer and fills it with data. The index pointer is optional. Returns an ID to the
		// created static buffer.
		virtual HRESULT	CreateStaticBuffer(KPVERTEXID VertexID, UINT nSkinID, UINT nVertices, UINT nIndices,
										   const void *pVertices, const WORD *pIndices, UINT *pSBufferID) = 0;

		// Renders from user pointer, automatically created dynamic buffer, uses caching for better performance
		virtual HRESULT	Render(KPVERTEXID VertexID, UINT nSkinID, UINT nVertices, UINT nIndices,
							   const void *pVertices, const WORD *pIndices) = 0;

		// Renders the static buffer
		virtual HRESULT Render(UINT nSBufferID) = 0;

		// Force all cached dynamic data to be rendered immediately
		// Usually used to render data before changing major rendering settings (for example render state or projection matrices)
		virtual HRESULT ForcedFlushAll(void) = 0;
		virtual HRESULT ForcedFlush(KPVERTEXID VertedID) = 0;

		// Resets all the active skin, texture and static buffer flags,
		// effectively forcing the cache manager into recalculating everything.
		virtual void    InvalidateStates(void)=0;

}; // ! Vertex Cache Manager


// KPRenderDevice ////
//////////////////////
//
// This is the render device interface of the engine. Makes our engine Graphic API independant.
// The public methods listed here are the only way for the user to 
// interact the underlying implementation of the graphics API itself.
class KPRenderDevice
{
	protected:
		HWND					m_hWndMain;				// main window handle
		HWND					m_hWnd[MAX_3DHWND];		// render window(s)
		UINT					m_nNumhWnd;				// number of render windows
		UINT					m_nActivehWnd;			// active window
		HINSTANCE				m_hDLL;					// DLL handle
		DWORD					m_dwWidth;				// screen width
		DWORD					m_dwHeight;				// screen height
		bool					m_bWindowed;			// full screen or windowed mode
		char					m_chAdapter[256];		// name of the graphics adapter
		FILE					*m_pLog;				// logfile
		bool					m_bRunning;				// are we running?!

		KPSkinManager			*m_pSkinManager;	// pointer to the Skin Manager
		KPVertexCacheManager	*m_pVertexMan;		// Pointer to Vertex Cache Manager

		// VIEW / PROJECTION ATTRIBUTES
		///////////////////////////////////

		float					m_fNear, m_fFar;		// near and far plane
		int						m_nStage;				// rendering stages 0-3
		KPENGINEMODE			m_Mode;					// engine modes: 2d, 3d etc
		KPVIEWPORT				m_ViewPort[4];			// four predefines viewports

		// RENDER STATE ATTRIBUTES
		//////////////////////////////

		KPCOLOR					m_clrWireframe;			// Color of the wireframe
		KPRENDERSTATE			m_ShadeMode;			// Vertex shading mode


	public:
		KPRenderDevice(void) {};
		virtual ~KPRenderDevice(void) {};

		// INITIALIZE / RELEASE
		//////////////////////////

		virtual HRESULT			Init(HWND, const HWND*, int, int, int, bool) = 0;
		virtual void			Release(void) = 0;
		virtual bool			IsRunning(void) = 0;
		virtual bool			IsWindowed(void) = 0;

		// MANAGERS
		////////////////
		
		virtual					KPSkinManager* GetSkinManager(void) = 0;
		virtual					KPVertexCacheManager* GetVertexManager(void)=0;

		// VIEW / PROJECTION 
		////////////////////////

		virtual HRESULT			GetFrustum(KPPlane* pFrustum) = 0;
		virtual HRESULT			SetView3D(const KPVector &vcX, const KPVector &vcY, const KPVector &vcZ, const KPVector &vcPos) = 0;
		virtual HRESULT			SetViewLookAt(const KPVector &vcFrom, const KPVector &vcAt, const KPVector &vcWorldUp) = 0;
		virtual void			SetClippingPlanes(float fNear, float fFar) = 0;
		virtual void			SetWorldTransform(const KPMatrix *mWorld) = 0;

		// RENDER STATE
		///////////////////

		virtual void			SetBackfaceCulling(KPRENDERSTATE rs)=0;
		virtual void			SetDepthBufferMode(KPRENDERSTATE rs)=0;
		virtual void			SetShadeMode(KPRENDERSTATE rs, float f, const KPCOLOR* clrWireFrame)=0;
		virtual KPRENDERSTATE	GetShadeMode(void)=0;
		virtual void			UseTextures(bool bUse)=0;

		// LIGHTNING
		////////////////
		virtual void			SetAmbientLight(float fR, float fG, float fB) =0;

		// FONTS
		////////////
		virtual HRESULT			CreateMyFont(const char *chType, int nWeight, bool bItalic, bool bUnderlined, bool bStrikeOut, DWORD dwSize, UINT *pFontID)=0;
		virtual HRESULT			DrawTxt(UINT nFontID, int x, int y, UCHAR a, UCHAR r, UCHAR g, UCHAR b, char *chFormat, ...)=0;

		// RENDERING
		////////////////

		virtual HRESULT			UseWindow(UINT nHwnd) = 0;
		virtual HRESULT			BeginRendering(bool bClearPixel, bool bClearDepth, bool bClearStencil) = 0;
		virtual void 			EndRendering(void) = 0;
		virtual void			SetClearColor(float fRed, float fGreen, float fBlue) = 0;
		virtual HRESULT			Clear(bool bClearPixel, bool bClearDepth, bool bClearStencil) = 0;

		virtual HRESULT			SetMode(KPENGINEMODE mode, int nStage) = 0;
		virtual HRESULT			InitStage(float fFOV, KPVIEWPORT *pVP, int nStage) = 0;

}; // ! KPRenderDevice class

typedef class KPRenderDevice *LPKPRENDERDEVICE;		// Pointer to this class

// At build time, we do not know which Graphic API will have to be loaded, it only clears up during runtime.
// Because of this, we need to use function pointers to access them.
typedef HRESULT (*CREATERENDERDEVICE)(HINSTANCE hDLL, KPRenderDevice **pInterface);
typedef HRESULT (*RELEASERENDERDEVICE)(KPRenderDevice **pInterface);


#endif // ! KPRENDERDEVICE_H