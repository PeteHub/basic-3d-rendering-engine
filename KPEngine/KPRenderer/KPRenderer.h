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

//! Loads the DLL of the graphics API and initializes the render device
class KPRenderer
{
	public:
		//! Constructor
		/*!
			\param [in] hInst Window instance handle of the parent application
		*/
		KPRenderer(HINSTANCE hInst);					// Takes Windows instance handle of the application that uses the engine
		
		//! Destructor
		~KPRenderer(void);

		//! Loads the implementation of a graphics API and creates the rendering device.
		/*!
			\param [in] chAPI Pointer to a null-terminated string specifying the graphics API to use.
			\return S_OK upon success
			\return E_FAIL upon failure
		*/
		HRESULT				CreateDevice(char *chAPI);	// Creates the object that implements the RenderDevice

		//! Releases the rendering device.
		void				Release(void);				// Releases the object

		//! Retrieves the rendering device.
		/*!
			\return Pointer to a LPKPRENDERDEVICE object representing the rendering device interface.
		*/
		LPKPRENDERDEVICE 	GetDevice(void);			// Returns the pointer to the RenderDevice

		//! Retrieves the loaded DLL
		/*!
			\return HINSTANCE type handle of the loaded DLL.
		*/
		HINSTANCE			GetModule(void);			// Returns the handle of the loaded DLL

	private:
		KPRenderDevice		*m_pDevice;					//!< Pointer to the object that implements the RenderDevice
		HINSTANCE			m_hInst;					//!< Application's Windows instance handle
		HMODULE				m_hDLL;						//!< The handle of the loaded DLL

}; // ! KPRenderer class

typedef class KPRenderer *LPKPRENDERER;					//!< Pointer to a KPRenderer type object

#endif // ! KPRENDERER_H