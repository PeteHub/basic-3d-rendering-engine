/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: KPD3D_init.cpp
 *  Description: Direct3D Device initialization. This is the entry
 *				 point for the Direct3D Rendering Device DLL
 *
 *****************************************************************
*/


#include <windows.h>			// Type definitions
#include "resource.h"			// Dialog Control IDs
#include "KP.h"					// Some return value definitons
#include "KPD3D.h"				// Class Definition
#include "KPD3DSkinManager.h"	// Skin Manager
#include "KPD3D_vcache.h"		// Vertex Caching

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

// Global variable for dialog callback 
//
// We can not make the callback function static because it needs to access data from the class
// Unfortunately callback functions can not be accessed from inside the class by default.
// To work around this, we put it into a global variable.
KPD3D *g_KPD3D = NULL;

// Used for storing dialog results
KPDEVICEINFO	g_xDevice;	// Device informations
D3DDISPLAYMODE	g_Dspmd;	// Display mode
D3DFORMAT		g_fmtA;		// Front buffer format
D3DFORMAT		g_fmtB;		// Back buffer format


// DLL ENTRY POINT IMPLEMENTATION
/////////////////////////////////
BOOL WINAPI DllEntryPoint(HINSTANCE hDll, DWORD dwMessage, LPVOID lpvReserved)
{
	switch( dwMessage )
	{
	case DLL_PROCESS_ATTACH:
		// When we attach to the dll
		// dll initialization should come here
		break;
	case DLL_PROCESS_DETACH:
		// When we release the dll
		break;
	default:
		break;
	};
	return true;
}

// FUNCTION DEFINITIONS
//////////////////////////////


// CreateRenderDevice
/////////////////////
//
// This is one of the two external functions available from outside of the dll
//
// This function creates a new KPD3D rendering device object and returns it through the pointer pass to it as second parameter.
extern "C" __declspec(dllexport) HRESULT CreateRenderDevice( HINSTANCE hDll, LPKPRENDERDEVICE *pDevice )
{
	if ( !*pDevice )
	{
		*pDevice = new KPD3D( hDll );
		return KP_OK;
	}
	return KP_FAIL;
}


// ReleaseRenderDevice
//////////////////////
//
// This is one of the two external functions available from outside of the dll
//
// This function releases the rendering device object created by the dll. 
// The rendering device could be released through the pointer itself, but it is only reasonable
// to let the dll itself clean up the objects it created. The delete call will call the KPD3D class' deconstructor.
extern "C" __declspec(dllexport) HRESULT ReleaseRenderDevice( LPKPRENDERDEVICE *pDevice )
{
	if ( !*pDevice )
		return KP_FAIL;

	delete *pDevice;
	*pDevice = NULL;
	return KP_OK;
}


// KPD3D Constructor
////////////////////
//
KPD3D::KPD3D(HINSTANCE hDLL)
{
	m_hDLL				= hDLL;
	m_pEnum				= NULL;
	m_pD3D				= NULL;
	m_pDevice			= NULL;
	m_pLog				= NULL;
	m_ClearColor		= D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f);
	
	m_bRunning			= false;
	m_bIsSceneRunning	= false;

	m_pSkinManager		= NULL;
	m_pVertexMan		= NULL;

	m_ShadeMode			= RS_SHADE_SOLID;
	m_bUseTextures		= true;

	m_pFont				= NULL;
	m_numFonts			= 0;

	m_nActivehWnd		= 0;

	g_KPD3D				= this;		// As long as we only make a single object of this class,
									// this can serve as a global pointer to the object.

	fopen_s(&m_pLog, "Log_KPRenderDevice.txt", "w");
	Log("initializing... ");
}

// KPD3D ~Destructor
////////////////////
//
KPD3D::~KPD3D()
{
	Release();
}

// KPD3D Release
////////////////
//
void KPD3D::Release(void)
{
	if ( m_pEnum )
	{
		delete m_pEnum;
		m_pEnum = NULL;
	}
	
	if ( m_pSkinManager )
	{
		delete m_pSkinManager;
		m_pSkinManager = NULL;
	}

	if ( m_pVertexMan )
	{
		delete m_pVertexMan;
		m_pVertexMan = NULL;
	}

	if ( m_pFont )
	{
		free(m_pFont);
		m_pFont = NULL;
	}

	if ( m_pDevice )
	{
		m_pDevice->Release();
		m_pDevice = NULL;
	}

	if ( m_pD3D )
	{
		m_pD3D->Release();
		m_pD3D = NULL;
	}

	Log("successfully uninitialized.");
	fclose(m_pLog);
}


// Init Method
//////////////
//
// This function initializes the render device
// Copies data from paramters to member variables.
//
// hWnd				- Handle of the application's main window
// *hWnd3D			- Array of render window handles 
// nNumWnd			- Number of render window handles in hWnd3D
// nMinDepth		- Minimum depth for Depth Buffer
// nMinStencil		- Mininum depth for Stencil Buffer
// bSaveLog			- Whether you want a logfile or not
HRESULT KPD3D::Init(HWND hWnd, const HWND *hWnd3D, int nNumhWnd, int nMinDepth, int nMinStencil, bool bSaveLog)
{
	int nResult;

	// Prepare the log file
	if ( !m_pLog )
		return KP_FAIL;

	// How many child windows do we want to use for rendering?
	// The maximum amount of windows are defined in MAX_3DHWND
	// Anything more than that will be reverted to the max.
	if ( nNumhWnd > 0 )
	{
		if ( nNumhWnd > MAX_3DHWND )
			nNumhWnd = MAX_3DHWND;

		memcpy( &m_hWnd[0], hWnd3D, sizeof(HWND)*nNumhWnd);		// Copy the array of child windows handles into our member attribute
		m_nNumhWnd = nNumhWnd;
	}
	// Else we use a single rendering window
	else
	{
		m_hWnd[0]	 = hWnd;
		m_nNumhWnd	 = 0;
	}

	// Set the main rendering window
	m_hWndMain = hWnd;

	// Do we want to use Stencil Buffer?
	// If the Stencil Buffer bits (buffer size) is greater than 0 then we want.
	if ( nMinStencil > 0 )
		m_bStencil = true;

	m_pEnum = new KPD3DEnum(nMinDepth, nMinStencil);						// Generate or Enumeration Object

	// Open up the settings dialog
	// We can not use the DlgProc because it is part of a class
	// so we create a wrapper for it.
	nResult = DialogBox(m_hDLL, "dlgChangeDevice", hWnd, DlgProcWrap);		

	if ( nResult == -1 )		// Error opening the dialog
		return KP_FAIL;
	else if ( nResult == 0 )
		return KP_CANCELLED;	// The user cancelled 		
	else
		return Go();			// User OK'd the dialog, let's start the Direct3D device
} // ! Init

// DlgProcWrap Function
///////////////////////
//
// This is a wrapper for the DlgProc method.
// Makes it possible to use the DlgProc callback function
// from inside the KPD3D class,
BOOL CALLBACK DlgProcWrap( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return g_KPD3D->DlgProc(hDlg, message, wParam, lParam);
}

// DlgProc Callback
///////////////////
//
// Callback function that processes the settings dialog messages
BOOL CALLBACK KPD3D::DlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL bWnd = false;

	// Get the handles of the controls on the dialog
	HWND hFULL			= GetDlgItem(hDlg, IDC_FULL);			// Fullscreen mode radio button
	HWND hWND			= GetDlgItem(hDlg, IDC_WND); 			// Windowed mode radio button
	HWND hADAPTER		= GetDlgItem(hDlg, IDC_ADAPTER);		// Graphics Adapter combo-box
	HWND hMODE			= GetDlgItem(hDlg, IDC_MODE);			// Video mode combo-box
	HWND hADAPTERFMT	= GetDlgItem(hDlg, IDC_ADAPTERFMT);		// Front Buffer / Adapter Format combo-box
	HWND hBACKFMT		= GetDlgItem(hDlg, IDC_BACKFMT);		// Back Buffer Format combo-box
	HWND hDEVICE		= GetDlgItem(hDlg, IDC_DEVICE);			// Rendering Device Type combo-box (HAL,RAF etc)

	// Dialog Control Event Handling
	switch (message)
	{

	// Set Window Mode radio button checked by default
	case WM_INITDIALOG:
		{
			SendMessage(hWND, BM_SETCHECK, BST_CHECKED, 0);	
			this->m_pEnum->Enum(hADAPTER, hMODE, hDEVICE, hADAPTERFMT, hBACKFMT, hWND, hFULL, m_pLog);
			return true;
		}

	// Some dialog controls have changed
	case WM_COMMAND:
		{
			switch( LOWORD(wParam) )	// LOWORD() macro returns the low-order word of a DWORD, which is in this case the control's ID
			{
			// OK Button pressed, we process the selected values of the controls
			case IDOK:
				{
					m_bWindowed = SendMessage(hFULL, BM_GETCHECK, 0, 0) != BST_CHECKED;		// If fullscreen is not checked -> windowed mode
					m_pEnum->GetSelections(&g_xDevice, &g_Dspmd, &g_fmtA, &g_fmtB);
					GetWindowText(hADAPTER, m_chAdapter, 256);
					EndDialog(hDlg, 1);	// Close the dialog, return with OK signal, 1
					return true;
				} break;

			// Cancel Button pressed
			case IDCANCEL:
				{
					EndDialog(hDlg, 0); // Close the dialog, return with CANCEL signal, 0
					return true;
				} break;
			
			// Graohics adapter combo box value changed
			case IDC_ADAPTER:
				{
					if( HIWORD(wParam) == CBN_SELCHANGE )	// HIWORD() macro returns the high-order word, which is the notification message
						m_pEnum->ChangedAdapter();			// In this case, CBN_SELCHANGE means that selection change happened in the combo box.
				} break;

			// Device type combo box value changed
			case IDC_DEVICE:
				{
					if( HIWORD(wParam) == CBN_SELCHANGE )
						m_pEnum->ChangedDevice();
				} break;

			// Adapter format / Front Buffer combo box value changed
			case IDC_ADAPTERFMT:
				{
					if( HIWORD(wParam) == CBN_SELCHANGE )
						m_pEnum->ChangedAdapterFmt();
				} break;

			// Backbuffer combo box is indeed missing :) The change of this value does not have any influence on the other control's possible options
			// unlike for example Device Type, where HAL might not have as many formats as the REFerence renderer.

			// Fullscreen radiobox selection changed
			case IDC_FULL:
				// Same as Windowed Mode change
			// Windowed Mode radiobox selection changed
			case IDC_WND:
				{
						m_pEnum->ChangedWindowMode();
				} break;
			} // !switch WM_COMMAND
		} break; // !case WM_COMMAND
	} // !switch message

	return false;
} //! DlgProc Callback


// Go method
////////////
//
// This method is repsonsible for starting the Direct3D device
// We loop through the gathered valid device combos
// (a combination of front and back buffer formats, an adapter, device type, VP type and depthstencil format.)
// and choose the one that matches the user settings. From this object we retrieve the values
// we need to set the presentation parameters and initialize our Direct3D device.
HRESULT KPD3D::Go(void)
{
	KPCOMBOINFO	xCombo;
	HRESULT		hr;
	HWND		hwnd;

	// Create the main Direct3D object here,
	// This is the first D3D object a graphical application creates
	// and the last one it releases. All enumeration and capability
	// checking is accessed through this object.
	// This way we can actually select our devices without/before creating them.
	// If successful, this function returns a pointer to an IDirect3D9 interface.

	// If it already exsists, release it. Basically we recreate/restart it.
	if ( m_pD3D )
	{
		m_pD3D->Release();
		m_pD3D = NULL;
	}

	m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );

	if ( !m_pD3D )
	{
		Log("error: Direct3DCreate8()");
		return KP_CREATEAPI;
	}

	// Loop through all our valid combos and find the first one that matches our criterias.
	for (UINT i=0; i < g_xDevice.nNumCombo; ++i)
	{
		if (	( g_xDevice.d3dCombo[i].bWindowed		== m_bWindowed )				// Windowed mode?
			&&	( g_xDevice.d3dCombo[i].d3dDevType		== g_xDevice.d3dDevType )		// Device Type? SW,HW,HAL?
			&&	( g_xDevice.d3dCombo[i].fmtAdapter		== g_fmtA )						// Front Buffer Format
			&&	( g_xDevice.d3dCombo[i].fmtBackBuffer	== g_fmtB )	)					// Back Buffer Format
		{
			xCombo = g_xDevice.d3dCombo[i];
			break;
		}
	}

	// Fill in the presentation parameters structure
	ZeroMemory( &m_d3dpp, sizeof(m_d3dpp));

	m_d3dpp.Windowed					= m_bWindowed;
	m_d3dpp.BackBufferCount				= 1;
	m_d3dpp.BackBufferFormat			= g_Dspmd.Format;
	m_d3dpp.EnableAutoDepthStencil		= TRUE;
	m_d3dpp.MultiSampleType				= xCombo.msType;
	m_d3dpp.AutoDepthStencilFormat		= xCombo.fmtDepthStencil;
	m_d3dpp.SwapEffect					= D3DSWAPEFFECT_DISCARD;

	// Do we have a valid Stencil Buffer format?
	if (	( xCombo.fmtDepthStencil	== D3DFMT_D24S8 )	// 32bit buffer, 24 depth, 8 stencil
		||	( xCombo.fmtDepthStencil	== D3DFMT_D24X4S4 ) // 32bit buffer, 24 depth, 4 stencil
		||	( xCombo.fmtDepthStencil	== D3DFMT_D15S1 ) )	// 16bit buffer, 15 depth, 1 stencil
		m_bStencil = true;
	else
		m_bStencil = false;

	// Are we running in fullscreen mode?
	if ( !m_bWindowed )
	{
		m_d3dpp.hDeviceWindow		= hwnd = m_hWndMain;	// Set the main render window as the presentation surface
		m_d3dpp.BackBufferWidth		= g_Dspmd.Width;		// Set Backbuffer Width to match screen resolution
		m_d3dpp.BackBufferHeight	= g_Dspmd.Height;		// Set Backbuffer Height to match screen resolution
		ShowCursor(FALSE);									// Hide the cursor in fullscreen mode
	}
	// Are we running in windowed mode?
	else
	{
		m_d3dpp.hDeviceWindow		= hwnd = m_hWnd[0];
		m_d3dpp.BackBufferWidth		= GetSystemMetrics(SM_CXSCREEN); // The width of the screen of the primary display monitor, in pixels.
		m_d3dpp.BackBufferHeight	= GetSystemMetrics(SM_CYSCREEN); // The height of the screen of the primary display monitor, in pixels.
	}

	// Create the Direct3D device now, that we set up everything for it.
	// store it in m_pDevice
	hr = m_pD3D->CreateDevice( g_xDevice.nAdapter, g_xDevice.d3dDevType, hwnd, xCombo.dwBehavior, &m_d3dpp, &m_pDevice);
	
	// Create the swap chains if they are needed ( more than one rendering window ) to present multiple views from the same device
	// There can be only one swap chain at full screen at a time on each adapter.
	if ( ( m_nNumhWnd > 0 ) && m_bWindowed )
	{
		for (UINT i=0; i < m_nNumhWnd; ++i)
		{
			m_d3dpp.hDeviceWindow = m_hWnd[i];		// Define which surface should this swapchain render onto
			m_pDevice->CreateAdditionalSwapChain(&m_d3dpp, &m_pChain[i]);	// Create and store the swapchain
		}
	}

	// Free the enumeration object, it is not needed anymore.
	delete m_pEnum;
	m_pEnum = NULL;

	// Check for errors during device creation AFTER we freed up the enumeration object.
	if ( FAILED(hr) )
	{
		Log("error: IDirect3D::CreateDevice()");
		return KP_CREATEDEVICE;
	}
	Log("Direct3D Device initialization successful! ");
	LogDeviceCaps(&g_xDevice.d3dCaps);

	m_bRunning			= true;		// The Direct3D render device is up and running
	m_bIsSceneRunning	= false;	// But we are not rendering any scenes yet.
	m_dwWidth			= m_d3dpp.BackBufferWidth;
	m_dwHeight			= m_d3dpp.BackBufferHeight;

	FirstTimeInitialization();

	return KP_OK;

} // ! Go

// First Time Initialization ///
////////////////////////////////
/*
	Configures the Render device after initialization.
	Sets up the managers, default renderint states.
	Creates the standard materials, textures, skins, matrices,
	clipping planes, texture filtering, viewport, lightning :)
*/
HRESULT KPD3D::FirstTimeInitialization(void)
{
	// Initialize the CPU

	CPUINFO info;
	GetCPUInfo(&info);

	LogCpuCaps(&info);

	if ( IsSSESupported() )
		Log("Using SIMD.");
	else
		Log("Not using SIMD.");

	// Initialize the Managers
	m_pSkinManager	= new KPD3DSkinManager(m_pDevice, m_pLog);

	m_pVertexMan	= new KPD3DVertexCacheManager( (KPD3DSkinManager*)m_pSkinManager, m_pDevice, this, 3000, 4500, m_pLog);

	// Set the default render states
	m_pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);			// Enable lightning
	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);		// Cull counter clockwise
	m_pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);		// Enable Z/Depth Buffering

	// Set-up the default material
	memset(&m_DefMaterial, 0, sizeof(D3DMATERIAL9));
	m_DefMaterial.Ambient.r = m_DefMaterial.Ambient.g = m_DefMaterial.Ambient.b = m_DefMaterial.Ambient.a = 1.0f;

	if ( FAILED( m_pDevice->SetMaterial(&m_DefMaterial) ) )
	{
		Log("FirstTimeInitialization: unable to set default material!");
		return KP_FAIL;
	}


	// Set up texture fultering
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);	// Magnification Filter
	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);	// Minification Filter
	m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);	// Mipmap filter during minification

	// Set Active Skin to NONE
	SetActiveSkinID(KPNOTEXTURE); // No active skin

	// Create default viewport object
	KPVIEWPORT vpView = { 0, 0, m_dwWidth, m_dwHeight };

	// Default Engine mode is set for Perspective Projection
	m_Mode		= EMD_PERSPECTIVE;
	m_nStage	= -1;

	// Set view matrix to identity matrix
	memset(&m_mView3D, 0, sizeof(D3DMATRIX));
	m_mView3D._11 = m_mView3D._22 = m_mView3D._33 = m_mView3D._44 = 1.0f;

	// Set default clipping plane
	SetClippingPlanes(0.1f, 1000.0f);

	// Set default ambient light level
	SetAmbientLight( 1.0f, 1.0f, 1.0f);

	// Set Perspective Projection to stage 0 & Activate it
	if ( FAILED( InitStage(0.8f, &vpView, 0) ) )
	{
		Log("FirstTimeInitialization: Unable to initialize stage 0 rendering mode.");
		return KP_FAIL;
	}
	if ( FAILED( SetMode(EMD_PERSPECTIVE, 0) ) )
	{
		Log("FirstTimeInitialization: Unable to set rendering mode to Perspective Projection.");
		return KP_FAIL;
	}

	Log("Final Initialization is successfully completed. The engine is ready to rock ! :)");

	return KP_OK;

} // ! FirstTimeInitialization


// UseWindow Method
///////////////////
//
// This method changes the active swap chain of the device to another one.
// Until the next switch, Direct3D will render into the window connected 
// to the active swap chain.
HRESULT KPD3D::UseWindow(UINT nHwnd)
{
	LPDIRECT3DSURFACE9 pBack = NULL;	// The Direct3D surface to be filled with the back buffer

	// In full screen we can't really switch between windows :)
	if ( !m_d3dpp.Windowed )
		return KP_OK;
	// If the we try to switch to a window that does not exsist
	else if ( nHwnd >= m_nNumhWnd )
		return KP_FAIL;

	// Retrieve the back buffer of the swap chain connected to the window
	// We only use 1 backbuffer / swapchain, so the ID is 0
	// The BB type is MONO, LEFT or RIGHT (for stereo)
	// Direct3D 9 does not support stereo view though, so stick with MONO
	if ( FAILED( m_pChain[nHwnd]->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pBack) ) )
	{
		Log("error: Failed to change active swap chain \n");
		return KP_FAIL;
	}

	// Activate the new render target for the device
	m_pDevice->SetRenderTarget(0, pBack);

	pBack->Release();
	m_nActivehWnd = nHwnd;

	return KP_OK;
}


// Is Windowed ////
///////////////////
bool KPD3D::IsWindowed(void)
{
	return m_bWindowed;
}

// Render Functions
///////////////////

// BeginRendering Method
////////////////////////
//
// This method prepares the video adapter for rendering
// and begins a scene.
// The three parameters control which buffers will be cleared before the rendering begins.
// 
// If you use Depth of Stencil buffers, you almost always want to clear them out. But not otherwise!
// The pixel buffer does not have to be cleared at each frame if you are certain you will rerender each pixel.
// This can save quiet some rendering time.
HRESULT KPD3D::BeginRendering(bool bClearPixel, bool bClearDepth, bool bClearStencil)
{
	DWORD dw	= 0;

	// What surfaces do we want to clear?
	if ( bClearPixel || bClearDepth || bClearStencil )
	{
		if ( bClearPixel )					dw |= D3DCLEAR_TARGET;
		if ( bClearDepth )					dw |= D3DCLEAR_ZBUFFER;
		if ( bClearStencil && m_bStencil )	dw |= D3DCLEAR_STENCIL;

		if ( FAILED( m_pDevice->Clear(0, NULL, dw, m_ClearColor, 1.0f, 0) ) )
		{
			Log("error: KPD3D::BeginScene() Could not clear the surface");
			return KP_FAIL;
		}
	}

	if ( FAILED( m_pDevice->BeginScene() ) )
	{
		Log("error: KPD3D::BeginScene() Could not begin rendering scene");
		return KP_FAIL;
	}

	m_bIsSceneRunning	= true;
	
	return KP_OK;
} // ! BeginRendering


// Clear Method
///////////////
//
// Clears the surface whether it is a render target, stencil buffer or depth buffer
// without presenting it.
HRESULT KPD3D::Clear(bool bClearPixel, bool bClearDepth, bool bClearStencil)
{
	DWORD dw	= 0;

	// Set Clear flags
	if ( bClearPixel )					dw |= D3DCLEAR_TARGET;
	if ( bClearDepth )					dw |= D3DCLEAR_ZBUFFER;
	if ( bClearStencil && m_bStencil )	dw |= D3DCLEAR_STENCIL;

	// If the scene is running, end it first.
	if ( m_bIsSceneRunning )
		m_pDevice->EndScene();

	if ( FAILED( m_pDevice->Clear(0, NULL, dw, m_ClearColor, 1.0f, 0) ) )
	{
		Log("error: KPD3D::Clear() Could not clear the surface");
		return KP_FAIL;
	}

	// Restart the Scene if it was running previously
	if ( m_bIsSceneRunning )
		m_pDevice->BeginScene();

	return KP_OK;
} // ! Clear


// EndRendering Method
//////////////////////
//
// Ends rendering the current scene and presents the results
void KPD3D::EndRendering(void)
{
	// Flush all the caches
	if ( FAILED( m_pVertexMan->ForcedFlushAll() ) )
		Log("EndRendering: Failed to flush all the caches!");

	// Present must be called after the scene is ended or it fails.
	// Only call it ONCE for a swap chain per frame.
	m_pDevice->EndScene();

	if ( m_d3dpp.Windowed && ( m_nNumhWnd > 0 ) )
	{
		if ( FAILED( m_pChain[m_nActivehWnd]->Present(NULL, NULL, NULL, NULL, 0) ) )
			Log("EndRendering: Failed to present active swap chain!");
	}
	else
	{

		m_pDevice->Present( NULL,	// The entire surface is presented
							NULL,	// The entire client area is filled
							NULL,	// The destination window is the one specified in the present parameters
							NULL);	// We don't want to express the contained region in back buffer coordinates
	}

	m_bIsSceneRunning = false;
}

// SetClearColor Method
///////////////////////
//
// Sets the color we want to use to clear our surfaces
void KPD3D::SetClearColor( float fRed, float fGreen, float fBlue )
{
	m_ClearColor = D3DCOLOR_COLORVALUE(fRed, fGreen, fBlue, 1.0f);	// RGB Opaque color
}

// Log ////
///////////
void KPD3D::Log(char *chFormat, ...)
{
	char msg[256];
	char *pArgs;

	// Point at the argument list
	pArgs = (char*) &chFormat + sizeof(chFormat);

	// Convert arguments to message using the format string
	vsprintf_s(msg, sizeof(msg), chFormat, pArgs);

	fprintf(m_pLog, "[ KPD3D ]: ");
	fprintf(m_pLog, msg);
	fprintf(m_pLog, "\n");

//#ifdef _DEBUG
	// Instantly write the buffer into the log file
	// This way, even if the program crashes, you get the information
	fflush(m_pLog);
//#endif

} // ! ::Log()
