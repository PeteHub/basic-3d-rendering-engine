/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: KPRenderer.h
 *  Description: Static library for graphics API loading
 *
 *****************************************************************
*/

#ifndef KPRENDERER_H
#define KPRENDERER_H

#include "KPRenderDevice.h"

class KPRenderer
{
	public:
		KPRenderer(HINSTANCE hInst);					// Takes Windows instance handle of the application that uses the engine
		~KPRenderer(void);

		HRESULT				CreateDevice(char *chAPI);	// Creates the object that implements the RenderDevice
		void				Release(void);				// Releases the object
		LPKPRENDERDEVICE 	GetDevice(void);			// Returns the pointer to the RenderDevice
		HINSTANCE			GetModule(void);			// Returns the handle of the loaded DLL

	private:
		KPRenderDevice		*m_pDevice;					// Pointer to the object that implements the RenderDevice
		HINSTANCE			m_hInst;					// Application's Windows instance handle
		HMODULE				m_hDLL;						// The handle of the loaded DLL

}; // ! KPRenderer class

typedef class KPRenderer *LPKPRENDERER;					// Pointer to a KPRenderer type object

#endif // ! KPRENDERER_H