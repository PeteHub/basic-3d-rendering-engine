/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: KPRenderer.cpp
 *  Description: Render device API loader definiton
 *				
 *				 This static library is the one which decides
 *				 which DLL to load. The loaded DLL will represent
 *				 the render device that can be used to output
 *				 graphics to the screen.
 *
 *****************************************************************
*/

#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "KPRenderer.h"


// Constructor
// Stores the application's Windows Handle and nulls out the other attributes.
KPRenderer::KPRenderer(HINSTANCE hInst)
{
	m_hInst		=	hInst;
	m_hDLL		=	NULL;
	m_pDevice	=	NULL;

}

// Destructor
// Simply Releases the RendererDevice
KPRenderer::~KPRenderer(void)
{
	Release();
}

// CreateDevice
// Creates the object that implements the interface for the RenderDevice
//
// It takes a string as parameter that specifies which graphics API DLL we have to load.
HRESULT KPRenderer::CreateDevice(char *chAPI)
{
	char buffer[300];

	if (strcmp(chAPI, "Direct3D") == 0 )		// We want to load the Direct3D implementation of the interface
	{
		m_hDLL = LoadLibrary("KPD3D.dll");		// Load the Direct3D Interface Implementation
		if (!m_hDLL)
		{
			MessageBox(NULL, "Loading KPD3D.dll failed.", "KPEngine - error", MB_OK | MB_ICONERROR);
			return E_FAIL;
		}
	}
	// TODO: OpenGL Support Can be added here
	else
	{
		_snprintf_s(buffer, sizeof(buffer),300,"API '%s' not supported.", chAPI);
		MessageBox(NULL, buffer, "KPEngine - error", MB_OK | MB_ICONERROR);
		return E_FAIL;
	}


	// If we got this far that means we have managed to load the DLL.
	// Now we want to access objects from the DLL, but by default, we can not see inside the DLL.
	// Because of this, the DLL has an exported function called CreateRenderDevice() which will give us access to the objects.
	// In order to reach this function, we need to get a pointer to it.
	CREATERENDERDEVICE _CreateRenderDevice = NULL;		// We create a temporary function pointer to access our CreateRenderDevice() function
	HRESULT hr;

	// Then we have to search the DLL for the function called 'CreateRenderDevice' and get a pointer to it's address
	// We have to implicitly cast this pointer, otherwise we would not now know what is the actual implementation of the function we got the address for.
	// This way we will know  what parameter list the function needs and what its return value is.
	_CreateRenderDevice = (CREATERENDERDEVICE) GetProcAddress(m_hDLL, "CreateRenderDevice");
	
	if ( ! _CreateRenderDevice )
		return E_FAIL;

	// Now we can call the function from inside the DLL, and receive a valid KPRenderDevice object from the DLL.
	// Now we have access to everything we need from the graphics API, and we don't even have to know what graphics API we use! 
	hr = _CreateRenderDevice(m_hDLL, &m_pDevice);

	if ( FAILED(hr) )
	{
		MessageBox(NULL, "CreateRenderDevice() from lib failed.", "KPEngine - error", MB_OK | MB_ICONERROR);
		m_pDevice = NULL;
		return E_FAIL;
	}

	return S_OK;
} // ! CreateDevice

// Release
// Releases the renderindevice and cleans up the dll objects
//
// Similarily to CreateDevice, we will have to use function pointers to access another function that implements
// ReleaseRenderDevice. The device could be released through m_pDevice too, but it was created by the DLL itself,
// it is only reasonable to let the DLL clean it up too. 
void KPRenderer::Release(void)
{
	// We create a temporary function pointer to access the ReleaseRenderDevice function from the DLL
	RELEASERENDERDEVICE _ReleaseRenderDevice = NULL;
	HRESULT hr;

	if ( m_hDLL )
	{
		// Search the DLL for the pointer to the exported DLL function 'ReleaseRenderDevice'
		_ReleaseRenderDevice = (RELEASERENDERDEVICE) GetProcAddress(m_hDLL, "ReleaseRenderDevice");
	}

	// call the DLL's release function
	if ( m_pDevice )
	{
		hr = _ReleaseRenderDevice(&m_pDevice);
		if ( FAILED(hr) )
			m_pDevice = NULL;
	}

} // ! Release


// Returns the pointer to the RenderDevice
LPKPRENDERDEVICE KPRenderer::GetDevice(void)
{
	return m_pDevice;
}


// Returns the handle of the loaded DLL
HINSTANCE KPRenderer::GetModule(void)
{ 
	return m_hDLL;
}
