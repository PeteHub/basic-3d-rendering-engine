/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: KPD3D.h
 *  Description: Direct3D implementation of the Renderer interface
 *				 - Device enumeration class and structures
 *				 - Direct3D render device interface
 *
 *****************************************************************
*/

#ifndef KPD3D_H
#define KPD3D_H

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "KP.h"
#include "../KP3D/KP3D.h"
#include "../KPRenderer/KPRenderDevice.h"


#pragma comment(lib, "KP3D.lib")

// Flexible Vertex Format Macros ////
/////////////////////////////////////
// They specify the contents of the Vertex structure
#define FVF_VERTEX	( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 )		// Untransformed, normal vector, 1 texture coordinate set
#define FVF_LVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE| D3DFVF_TEX1 )		// Untransformed, diffuse color, 1 texture coordinate set

#define LPD3DVDECLARATION9	LPDIRECT3DVERTEXDECLARATION9				// Shorten this beast a bit :)

// ENUMERATION STRUCTURES
/////////////////////////
//
/*

Enumeration
|
+-- AdapterInfos[0]
| |
| +-- DisplayModes[0]
| +-- DisplayModes[1]
| ...
| |
| +-- DeviceInfos[0]
| | |
| | +-- DeviceCombos[0]
| | | |
| | | +-- VPTypes
| | | +-- DSFormats
| | | +-- MSTypes
| | | +-- MSQualityLevels
| | | +-- DSMSConflicts
| | | +-- PresentIntervals
| | +-- DeviceCombos[1]
| | ...
| +-- DeviceInfos[1]
| ...
+-- AdapterInfos[1]
...

*/

// KPCOMBOINFO STRUCTURE
/////////////////////////
//
// Stores a combination of adapter formats and back buffer formats that is
// combatible with a particular D3D device
struct KPCOMBOINFO
{
	UINT				nAdapter;			// The adapter id it belongs to
	D3DDEVTYPE			d3dDevType;			// HAL, SW, REF?
	bool				bWindowed;			// Windowed mode or fullscreen?

	D3DFORMAT			fmtAdapter;			// Front buffer format
	D3DFORMAT			fmtBackBuffer;		// Back buffer format
	D3DFORMAT			fmtDepthStencil;	// Z/Stencil buffer format

	DWORD				dwBehavior;			// Vertex processing
	D3DMULTISAMPLE_TYPE msType;				// Multisample type
};


// KPDEVICEINFO STRUCTURE
/////////////////////////
//
// Stores information about the different devices supported by the adapters
// There are usually two, maximum three for each device, HAL, REF and SW
struct KPDEVICEINFO
{
	UINT				nAdapter;			// The adapter it belongs to
	D3DDEVTYPE			d3dDevType;			// HAL, SW, REF?
	D3DCAPS9			d3dCaps;			// Device capabilities
	KPCOMBOINFO			d3dCombo[80];		// Combinations
	UINT				nNumCombo;			// Number of combinations
};

// KPADAPTERINFO STRUCTURE
//////////////////////////
//
// Stores information about the display adapters
struct KPADAPTERINFO
{
	D3DADAPTER_IDENTIFIER9	d3dAdapterIdentifier;
	UINT					nAdapter;		// The adapter it belongs to, this is the adapter ordinal (0 for primary)
	D3DDISPLAYMODE			d3dDspmd[150];	// Display Modes
	UINT					nNumModes;		// Number of display modes
	KPDEVICEINFO			d3dDevs[3];		// Devices
	UINT					nNumDevs;		// Number of devices
};


// Helper functions for Enumerations
////////////////////////////////////
//
TCHAR*    D3DDevTypeToString(D3DDEVTYPE devType);
TCHAR*    D3DFormatToString(D3DFORMAT format);
TCHAR*    BehaviorTypeToString(DWORD vpt);
void*     GetSelectedItem(HWND hWnd);
void      AddItem(HWND hWnd, char *ch, void *pData);
bool      ContainsString(HWND hWnd, char *ch);

// KPD3DEnum Class
//////////////////
//
// The enumeration class is used to fill up the enumeration structures, check device capabilities
// and query the D3D devices for their supports. Also handles the settings dialog window
class KPD3DEnum
{
	public:
		KPD3DEnum(int nMinDepth, int nMinStencil) { m_nMinDepth = nMinDepth; m_nMinStencil = nMinStencil; }
		~KPD3DEnum(void) {}

		// This is the main function of the class
		// Enum will start t

		HRESULT Enum(HWND, HWND, HWND, HWND, HWND, HWND, HWND, FILE*);

		// Functions for the settings dialog combobox changes
		void ChangedAdapter(void);
		void ChangedDevice(void);
		void ChangedAdapterFmt(void);
		void ChangedWindowMode(void);

		// This function retrieves the final settings from the dialog
		void GetSelections( KPDEVICEINFO *pD, D3DDISPLAYMODE *dspmd, D3DFORMAT *fmtA, D3DFORMAT *fmtB);

		LPDIRECT3D9		m_pD3D;
		KPADAPTERINFO	m_xAdapterInfo[10];
		DWORD			m_dwNumAdapters;

	private:
		D3DDISPLAYMODE	m_dspmd;			// Current desktop display mode
		D3DFORMAT		m_fmtAdapter[5];	// List of all possible adapter formats
		UINT			m_nNumFmt;			// Number of possible adapter formats
		UINT			m_nMinWidth;		// Minimum screen width
		UINT			m_nMinHeight;		// Minimum screen height
		UINT			m_nMinBits;			// Least amont of back buffer bits
		UINT			m_nMinDepth;		// Least amount of depth buffer bits (z)
		UINT			m_nMinStencil;		// Least amount of stencil buffer bits
		FILE			*m_pLog;			// Log file opened by KPD3D class

		// KPD3D settings dialog GUI handles
		HWND m_hADAPTER;					// Graphics Adapter Combobox
		HWND m_hMODE;						// Display Mode Combobox
		HWND m_hDEVICE;						// Device Type Combobox
		HWND m_hADAPTERFMT;					// Adapter Format Combobox
		HWND m_hBACKFMT;					// Backbuffer Format Combobox
		HWND m_hWND;						// Radio button for Windowed Mode
		HWND m_hFULL;						// Radio Button for Fullscreen Mode

		HRESULT EnumAdapters(void);				// This method will identify the available adapters
		HRESULT EnumDevices(KPADAPTERINFO &);	// This method will enumerate throuth the available devices for the given adapters
		HRESULT EnumCombos(KPDEVICEINFO &);		// This method builds the list of adapter and back buffer format combinations that
												// are compatible with the devices

		UINT	GetBits(D3DFORMAT);
		HRESULT ConfirmDevice(D3DCAPS9*, DWORD, D3DFORMAT);
		bool	ConfirmDepthFmt(KPCOMBOINFO*);

}; // ! KPD3DEnum class


// KPD3D Class
//////////////
//
class KPD3D : public KPRenderDevice
{
	private:
		KPD3DEnum				*m_pEnum;			// Enumeration of the Direct3D interface
		LPDIRECT3D9				m_pD3D;				// Direct3D interface
		LPDIRECT3DDEVICE9		m_pDevice;			// Direct3D Device
		LPDIRECT3DSWAPCHAIN9	m_pChain[MAX_3DHWND]; // Swap chains
		D3DPRESENT_PARAMETERS	m_d3dpp;			// Present parameters
		D3DCOLOR				m_ClearColor;		// Default color to clear the background with
		bool					m_bIsSceneRunning;	// Is the scene running right now?
		bool					m_bStencil;			// Stencil bits
		bool					m_bUseTextures;		// Render with textures?
		D3DMATERIAL9			m_DefMaterial;		// Default material
		UINT					m_nActiveSkin;		// Currently active skin

		// Projection / View 
		////////////////////////

		D3DMATRIX				m_mView2D,			// View matrix for orthogonal projection
								m_mView3D,			// View matrix for perspective projection
								m_mProj2D,			// Projection matrix for 2d orthogonal projection
								m_mProjP[4],		// Projection matrices for perspective projection stages
								m_mProjO[4],		// Projection matrcies for orthogonal projection stages
								m_mWorld,			// The current World matrix
								m_mViewProj,		// Multiplication of current View and Projection matrices
								m_mWorldViewProj;	// Multiplication of current World, View and Projection matrices

		// Fonts
		////////////
		LPD3DXFONT				*m_pFont;			// Font object
		UINT					m_numFonts;			// Number of fonts

		// Lights
		////////////
		D3DLIGHT9				m_pointLight;		// Point Light
		D3DLIGHT9				m_directLight;		// Directional light

		////
		//  ----------------------- END OF ATTRIBUTE LIST ----------------------
		////

		// Start the API
		////////////////////

		HRESULT Go(void);
		HRESULT FirstTimeInitialization(void);
		void	Log(char *chFormat, ...);
		void	LogDeviceCaps(D3DCAPS9 *pCaps);
		void	LogCpuCaps(CPUINFO *pInfo);

		// VIEW / PROJECTION
		////////////////////////
		void	CalcViewProjMatrix(void);
		void	CalcWorldViewProjMatrix(void);
		void	Prepare2D(void);
		HRESULT CalcPerspProjMatrix(float fFOV, float fAspect, D3DMATRIX *m);
		void	SetWorldTransform(const KPMatrix *mWorld);

	public:
		KPD3D(HINSTANCE hDLL);
		~KPD3D(void);

		// INITIALIZE / RELEASE
		//////////////////////////

		HRESULT			Init(HWND, const HWND*, int, int, int, bool);
		void			Release(void);
		bool			IsWindowed(void);
		int				GetNumRenderWindows(void);
		HWND			GetRenderWindowHandle(int handle);

		BOOL CALLBACK	DlgProc(HWND, UINT, WPARAM, LPARAM);			// For the user settings dialog box

		// MANAGERS
		////////////////
		
		KPSkinManager*			GetSkinManager(void);
		KPVertexCacheManager*	GetVertexManager(void);

		// VIEW / PROJECTION 
		////////////////////////

		HRESULT			GetFrustum(KPPlane* pFrustum);
		HRESULT			SetView3D(const KPVector &vcX, const KPVector &vcY, const KPVector &vcZ, const KPVector &vcPos);
		HRESULT			SetViewLookAt(const KPVector &vcCamera, const KPVector &vcPoint, const KPVector &vcWorldUp);
		void			SetClippingPlanes(float fNear, float fFar);
		void			Transform2Dto3D(const POINT &point, KPVector *vcOrigin, KPVector *vcDirection) ;
		POINT			Transform3Dto2D(const KPVector &vcPoint);

		// RENDER STATE
		///////////////////

		void			SetBackfaceCulling(KPRENDERSTATE rs);
		void			SetDepthBufferMode(KPRENDERSTATE rs);
		void			SetShadeMode(KPRENDERSTATE rs, float f, const KPCOLOR* clrWireFrame);
		KPRENDERSTATE	GetShadeMode(void);
		KPCOLOR			GetWireColor(void);
		void			UseTextures(bool bUse);
		bool			UsesTextures();

		// SKINS
		///////////
		UINT			GetActiveSkinID(void);
		void			SetActiveSkinID(UINT nSkinID);

		// LIGHTNING
		////////////////
		void			SetAmbientLight(float fR, float fG, float fB);

		// FONTS
		////////////
		HRESULT			CreateMyFont(const char *chType, int nWeight, bool bItalic, bool bUnderlined, bool bStrikeOut, DWORD dwSize, UINT *pFontID);
		HRESULT			DrawTxt(UINT nFontID, int x, int y, UCHAR a, UCHAR r, UCHAR g, UCHAR b, char *chFormat, ...);

		// RENDERING
		////////////////

		bool			IsRunning(void) { return m_bRunning; }
		HRESULT			UseWindow(UINT nHwnd);
		HRESULT			BeginRendering(bool, bool, bool);
		void			EndRendering(void);
		void			SetClearColor(float fRed, float fGreen, float fBlue);
		HRESULT			Clear(bool, bool, bool);

		HRESULT			SetMode(KPENGINEMODE mode, int nStage);
		HRESULT			InitStage(float fFOV, KPVIEWPORT *pVP, int nStage);

}; // ! KPD3D class

// We want to export these funcions in plain C style, this way we don't have to deal with C++
// changing their names and parameter lists due to the OOP overhead.
extern "C" __declspec(dllexport) HRESULT CreateRenderDevice(HINSTANCE hDLL, KPRenderDevice **pInterface);
extern "C" __declspec(dllexport) HRESULT ReleaseRenderDevice(KPRenderDevice **pInterface);

// A wrapper function around DlgProc
// This will help us access the DlgProc callback method from inside the KPD3D class
BOOL CALLBACK DlgProcWrap( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);




#endif KPD3D_H // ! KPD3D_H