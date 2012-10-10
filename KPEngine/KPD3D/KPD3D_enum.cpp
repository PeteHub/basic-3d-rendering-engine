/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: KPD3D_enum.cpp
 *  Description: Direct3D Device Capability Enumeration definition
 *
 *****************************************************************
*/

#include "resource.h"      // control id's
#include "KP.h"           // return values and stuff
#include "KPD3D.h"        // class definition


// some common backbuffer formats
const UINT g_nFormats_B=9;
const D3DFORMAT g_fmtBackBuffer[] = {
           D3DFMT_R8G8B8,
		   D3DFMT_A8R8G8B8, 
           D3DFMT_X8R8G8B8,
		   D3DFMT_R5G6B5, 
           D3DFMT_A1R5G5B5,
		   D3DFMT_X1R5G5B5,
           D3DFMT_X4R4G4B4,
		   D3DFMT_A4R4G4B4,
           D3DFMT_A2B10G10R10 
};


// some good multisample modes, list best ones last
const UINT g_nMS = 8;
const D3DMULTISAMPLE_TYPE g_msType[] = { 
           D3DMULTISAMPLE_NONE,
           D3DMULTISAMPLE_2_SAMPLES,
           D3DMULTISAMPLE_3_SAMPLES,
           D3DMULTISAMPLE_4_SAMPLES,
           D3DMULTISAMPLE_5_SAMPLES,
           D3DMULTISAMPLE_6_SAMPLES,
           D3DMULTISAMPLE_7_SAMPLES,
           D3DMULTISAMPLE_8_SAMPLES
};



// Enum Function
////////////////
//
// Enumerates all available display modes and fills the given combo
// boxes with corresponding values.
//
// The input parameters are the handles of the dialog box controls
// and the already opened log file handle
HRESULT KPD3DEnum::Enum(HWND hAdapter,
						HWND hMode,
						HWND hDevice,
						HWND hAdapterFmt,
						HWND hBackFmt,
						HWND hWnd,
						HWND hFull,
						FILE *pLog)
{
   HRESULT hr;

   if (pLog)
	   m_pLog = pLog;

   // Finally, we create the Direct3D object here
   // This is the first D3D object a graphical application creates
   // and the last one it releases. All enumeration and capability
   // checking is accessed through this object.
   // This way we can actually select our devices without/before creating them.
   // If successful, this function returns a pointer to an IDirect3D9 interface.
   if ( NULL == ( m_pD3D = Direct3DCreate9(D3D_SDK_VERSION) ) )
      return KP_CREATEAPI;		// If the call failed return error

   // We assign the control handles to our class attributes
   m_hADAPTER    = hAdapter;
   m_hMODE       = hMode;
   m_hDEVICE     = hDevice;
   m_hADAPTERFMT = hAdapterFmt;
   m_hBACKFMT    = hBackFmt;
   m_hWND        = hWnd;
   m_hFULL       = hFull;

   // We have no adapters yet, so we zero this value
   m_dwNumAdapters = 0;

   // We set some basic display properties
   // We will require at least 640x480 resolution with 16 bit colour depht
   m_nMinWidth  = 640;
   m_nMinHeight = 480;
   m_nMinBits   = 16;
   
   // What surface/front buffer pixel formats will we allow?
   m_fmtAdapter[0] = D3DFMT_X8R8G8B8;  // 32 Bit
   m_fmtAdapter[1] = D3DFMT_X1R5G5B5;  // 15 Bit
   m_fmtAdapter[2] = D3DFMT_R5G6B5;    // 16 Bit
   m_nNumFmt = 3;

   // We retrieve the current display mode of our default (primary, ordinal 0) adapter
   if (FAILED(m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &m_dspmd)))
   {
      if (m_pLog) {
         fprintf(m_pLog, "Enum: GetAdapterDisplayMode() failed \n");	// Log the error!
         fflush(m_pLog);
      }
   }

   hr = EnumAdapters();				// Eenumerate the available adapters we have
   
   // Now that we have gathered all the details about our adapters
   // we do not need the Direct3D object anymore until we actually
   // initialize the renderer.
   m_pD3D->Release();
   m_pD3D = NULL;

   if(FAILED(hr) || !hAdapter)
      return KP_FAIL;

   // List the available graphics adapters in the respective combo box
   SendMessage(m_hADAPTER, CB_RESETCONTENT, 0, 0);	// First we clear the combo box's content

   for (UINT i=0; i < m_dwNumAdapters; ++i)		// After that, we fill it up with the available adapters
	   AddItem(m_hADAPTER, m_xAdapterInfo[i].d3dAdapterIdentifier.Description, &m_xAdapterInfo[i]);

   // We select the first adapter and if neccessary scroll it into view.
   // After that, we update the rest of the dialog based on our selection.
   SendMessage(m_hADAPTER, CB_SETCURSEL, (WPARAM)0, 0);
   ChangedAdapter();

   // After this, we select the first item of each combo box.
   SendMessage(m_hADAPTER, CB_SETCURSEL, (WPARAM)0, 0);
   SendMessage(m_hMODE,    CB_SETCURSEL, (WPARAM)0, 0);
   SendMessage(m_hDEVICE,  CB_SETCURSEL, (WPARAM)0, 0);
   SendMessage(m_hBACKFMT, CB_SETCURSEL, (WPARAM)0, 0);
   SendMessage(m_hADAPTERFMT, CB_SETCURSEL, (WPARAM)0, 0);

   return KP_OK;
   } // !Enum


// GetSelectedItem function
///////////////////////////
//
// Retrieve the current value of a combo box.
void* GetSelectedItem(HWND hWnd) {
   WPARAM index = (WPARAM)( (int)(DWORD)SendMessage(hWnd, CB_GETCURSEL, 0, 0) );	// The index of the currently selected item, if any.
   return ( (void*)SendMessage(hWnd, CB_GETITEMDATA, index, 0) );					// Retrieve and return the value associated of the item of index
   }


// AddItem function
///////////////////////////
//
// Adds an item to a combo box and sets its value
void AddItem(HWND hWnd, char *ch, void *pData) {
   WPARAM index = (WPARAM)( (int)(DWORD)SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)ch) );	// Adds the ch string to the combo box.
   SendMessage(hWnd, CB_SETITEMDATA, index, (LPARAM)pData);									// Specifies the value of the newly added item.
   }

// ContainsString function
///////////////////////////
//
// Looks for a matching string among the items of a combo box
bool ContainsString(HWND hWnd, char *ch)
{
   int n = (int)SendMessage(hWnd, CB_GETCOUNT, 0, 0);								// Gets the number of items in the combo box.
   char buffer[200];

   for(int i=0; i< n; ++i)
   {
      SendMessage(hWnd, CB_GETLBTEXT, (WPARAM)i, (LPARAM)buffer);					// Retrieves a string at the index in the combo box
      if (lstrcmp(buffer, ch) == 0)													// Compare 
         return true;
   } // ! for
   return false;
}


// GetSelections Method
///////////////////////
//
// Retrieve the selected settings from the comboboxes
void KPD3DEnum::GetSelections(KPDEVICEINFO *pD, D3DDISPLAYMODE *dspmd, D3DFORMAT *fmtA, D3DFORMAT *fmtB)
{

   if (pD)
      memcpy(pD, GetSelectedItem(m_hDEVICE), sizeof(KPDEVICEINFO) );		// Retrieve the Device Type
   if (fmtA)
      memcpy(fmtA, GetSelectedItem(m_hADAPTERFMT), sizeof(D3DFORMAT) );		// Retrieve the Adapter Format / Front Buffer format
   if (fmtB)
      memcpy(fmtB, GetSelectedItem(m_hBACKFMT), sizeof(D3DFORMAT) );		// Retrieve the Backbuffer Format

   if (dspmd) {
      D3DDISPLAYMODE *pDspmd = (D3DDISPLAYMODE*)GetSelectedItem(m_hMODE);	// Retrieve the selected Display Mode
	  // Display Mode combobox is disabled if we choose windowed mode so pDspmd will be NULL in this case.
	  // TODO: allow changing display mode in windowed mode too.
      if (pDspmd)	// fullscreen
		  memcpy(dspmd, pDspmd, sizeof(D3DDISPLAYMODE) );
      else			// windowed mode
         memcpy(dspmd, &m_dspmd, sizeof(D3DDISPLAYMODE) );
   }
}



// ChangedAdapter Method
////////////////////////
//
// We call this method after the adapter selection combobox changed.
// It updates the list of devices and calls ChangedDevice()
void KPD3DEnum::ChangedAdapter(void)
{
   KPADAPTERINFO *pA = NULL;

   SendMessage(m_hDEVICE, CB_RESETCONTENT, 0, 0);				// Reset the Device Type combobox

   pA = (KPADAPTERINFO *)GetSelectedItem(m_hADAPTER);			// Retrieve the Selected Graphics Adapter

   for (UINT i=0; i < pA->nNumDevs; ++i)						// Add the supported Device Types to the combobox
	   AddItem(m_hDEVICE, D3DDevTypeToString(pA->d3dDevs[i].d3dDevType), &pA->d3dDevs[i]);

   SendMessage(m_hDEVICE, CB_SETCURSEL, (WPARAM)0, 0);			// Select the first item of the combobox
   
   // At this point we call ChangedDevice() method to update the settings
   // that depend on the device type.
   ChangedDevice();
} 

// ChangedDevice Method
///////////////////////
//
// Call when device type selection changed. This will reset radio button
// states for windowed/fullscreen mode and call ChangedWindowMode().
void KPD3DEnum::ChangedDevice(void)
{
   KPDEVICEINFO *pD=NULL;
   bool			bWindowed	= false;
   bool			bFullscreen	= false;

   pD = (KPDEVICEINFO *)GetSelectedItem(m_hDEVICE);

   // Check if windowed mode is supported by the given combination
   for (UINT i=0; i < pD->nNumCombo; ++i)
   {
	   if (pD->d3dCombo[i].bWindowed)
		   bWindowed = true;
	   else
		   bFullscreen = true;
   }

   // Enable/Disable the radio buttons based on what's supported
   EnableWindow(m_hWND,  bWindowed);
   EnableWindow(m_hFULL, bFullscreen);

   // If windowed mode is not supported, but the windowed mode radio button was/is checked
   // then we check the Fullscreen mode radiobutton.
   if (!bWindowed && ( SendMessage(m_hWND, BM_GETCHECK, 0, 0) == BST_CHECKED ) )
      SendMessage(m_hFULL, BM_SETCHECK, BST_CHECKED, 0);

   // At this point we call ChangedWindowMode() method
   // and update the settings that depend on fullscreen/windowed mode
   ChangedWindowMode();
} // ! ChangedDevice 

// ChangedWindowMode Method
///////////////////////////
//
// Call when window/fullscreen radiobutton selection changed.
// It updated the available display modes and buffer formats.
// After that it calls ChangedAdapterFmt()
void KPD3DEnum::ChangedWindowMode(void)
{
   KPADAPTERINFO	*pA=NULL;
   KPDEVICEINFO		*pD=NULL;
   KPCOMBOINFO		*pC=NULL;
   char				buffer[100];

   pA = (KPADAPTERINFO *)GetSelectedItem(m_hADAPTER);
   pD = (KPDEVICEINFO  *)GetSelectedItem(m_hDEVICE);

   // Resets the display mode and adapter format comboboxes
   SendMessage(m_hADAPTERFMT, CB_RESETCONTENT, 0, 0);
   SendMessage(m_hMODE,       CB_RESETCONTENT, 0, 0);

   // If Windowed Mode is checked
   if (SendMessage(m_hWND,BM_GETCHECK,0,0)==BST_CHECKED)
   {
	   AddItem(m_hADAPTERFMT, D3DFormatToString(m_dspmd.Format), &m_dspmd.Format);		// Use desktop format in windowed mode

	   sprintf_s(buffer, sizeof(buffer),"%d x %d", m_dspmd.Width, m_dspmd.Height);		// Convert desktop resolution to "W x H" format
	   SendMessage(m_hMODE, CB_ADDSTRING, 0, (LPARAM)buffer);							// Add desktop resolution to display mode combobox

	   // At this point we want to disable the Adapter Format and Display Mode comboboxes
	   // because in windowed mode we use the desktop settings.
	   // TODO: allow display mode change in windowed mode.
	   EnableWindow(m_hADAPTERFMT, false);
	   EnableWindow(m_hMODE, false);

   }
   // If Fullscreen Mode is the checked 
   else
   {
	   // We clear the items of the Adapter Format and Display Mode comboboxes
	   // and if they were disabled, we reenable them
	   SendMessage(m_hADAPTERFMT, CB_RESETCONTENT, 0, 0);
	   SendMessage(m_hMODE, CB_RESETCONTENT, 0, 0);
	   EnableWindow(m_hADAPTERFMT, true);
	   EnableWindow(m_hMODE, true);

      // Update the supported Adapter/Frontbuffer formats based on the Device combos.
	  for (UINT i=0; i < pD->nNumCombo; ++i)
	  {
		  pC = &pD->d3dCombo[i];	// We step through each combo that is supported by the device

		  if ( !ContainsString(m_hADAPTERFMT, D3DFormatToString(pC->fmtAdapter) ) )			// if the format is not on the list yet
			  AddItem(m_hADAPTERFMT, D3DFormatToString(pC->fmtAdapter), &pC->fmtAdapter);	// we add it.
      }
   }

   // We make the first item on their list active
   SendMessage(m_hADAPTERFMT, CB_SETCURSEL, (WPARAM)0, 0);
   SendMessage(m_hMODE,       CB_SETCURSEL, (WPARAM)0, 0);

   // At this point we call ChangedAdapterFmt() to update the rest of the settings
   ChangedAdapterFmt();
} // ! ChangedWindowMode


// ChangedAdapterFmt Method
///////////////////////////
//
// Call when adapter format selection changed.
// It will update the display mode and backbuffer format comboboxes.
void KPD3DEnum::ChangedAdapterFmt(void)
{
	KPADAPTERINFO	*pA=NULL;
	KPDEVICEINFO	*pD=NULL;
	KPCOMBOINFO		*pC=NULL;
	D3DDISPLAYMODE	*dspmd		= NULL;
	D3DFORMAT		*pFmt		= NULL;
	char			buffer[200];
	bool			bWindowed;

	pA   = (KPADAPTERINFO *)GetSelectedItem(m_hADAPTER);						// Retrieve the info of the selected graphic adapter
	pD   = (KPDEVICEINFO *)GetSelectedItem(m_hDEVICE);							// Retrive the details of the selected device
	pFmt = (D3DFORMAT *)GetSelectedItem(m_hADAPTERFMT);							// Retrieve the selected Adapter/Front buffer format

	bWindowed = ( SendMessage(m_hWND, BM_GETCHECK, 0, 0) == BST_CHECKED );		// Is windowed mode checked?!
	SendMessage(m_hBACKFMT, CB_RESETCONTENT, 0, 0);								// Clear the Backbuffer combobox

   // If Fullscreen Mode is checked
   if (!bWindowed)
   {
	   SendMessage(m_hMODE, CB_RESETCONTENT, 0, 0);								// Clear the Display Mode combobox

	   // Loop through all available display modes
	   for (UINT i=0; i < pA->nNumModes; ++i)
	   {
		   dspmd = &pA->d3dDspmd[i];
		   sprintf_s(buffer,sizeof(buffer),"%d x %d", dspmd->Width, dspmd->Height); // Convert display mode to "W x H" string

		   if ( !ContainsString(m_hMODE, buffer) )								// If the display mode is missing from the combo box
			   AddItem(m_hMODE, buffer, &pA->d3dDspmd[i]);						// we add it.
	   }

      AddItem(m_hBACKFMT, D3DFormatToString(*pFmt), &(*pFmt) );					// Match the Backbuffer format with the Frontbuffer format
																				// Fullscreen applications can not do color conversation
																				// between the buffers, so the formats have to match
	  
	  SendMessage(m_hMODE, CB_SETCURSEL, (WPARAM)0, 0);							// Select the first display-mode as default
   }
   // If Windowed Mode is checked
   else
   {
	   for (UINT i=0; i < pD->nNumCombo; ++i)
	   {
		   pC = &pD->d3dCombo[i];

		   // We add all those backbuffer formats to the combo box, that are valid device combinations
		   // in windowed mode for the current adapter/frontbuffer format.
		   if ( (bWindowed == pC->bWindowed) && (*pFmt == pC->fmtAdapter) )
		   {
			   if ( !ContainsString(m_hBACKFMT, D3DFormatToString(pC->fmtAdapter) ) )				// If this backbuffer format is missing from the combobox
                AddItem(m_hBACKFMT, D3DFormatToString(pC->fmtBackBuffer), &pC->fmtBackBuffer);		// we add it.
           }
       }
   }

   SendMessage(m_hBACKFMT, CB_SETCURSEL, (WPARAM)0, 0);							// Select the first item from the list as default
} // ! ChangedAdapterFmt

// Quicksort Comparator Function
////////////////////////////////
//
// Compares pairs of values (pointers to elements) type-casted to void*
// The parameters are casted back to D3DDISPLAYMODE* and compared.
// The return value represents whether arg1 is less than, equal to or greater than
// arg2 by returning, respectively, a negative value, zero or a positive value.
//
// We order by Width, Height, Display Format, RefreshRate

// Sidenote: Functions that are used as parameters of another funtions must be declared as __cdecl
//			 Fortunately __cdecl is the default calling convention for c/c++ non winapi functions
//			 so there is no need to specifically declare it
int SortModesComparator(const void* arg1, const void* arg2)
{
	D3DDISPLAYMODE* pdm1 = (D3DDISPLAYMODE*)arg1;
	D3DDISPLAYMODE* pdm2 = (D3DDISPLAYMODE*)arg2;

	if (pdm1->Width > pdm2->Width)
		return 1;
	if (pdm1->Width < pdm2->Width)
		return -1;
	if (pdm1->Height > pdm2->Height)
		return 1;
	if (pdm1->Height < pdm2->Height)
		return -1;
	if (pdm1->Format > pdm2->Format)
		return 1;
	if (pdm1->Format < pdm2->Format)
		return -1;
	if (pdm1->RefreshRate > pdm2->RefreshRate)
		return 1;
	if (pdm1->RefreshRate < pdm2->RefreshRate)
		return -1;
   return 0;
} // ! SortModesComparator


// EnumAdapters Method
//////////////////////
//
// This method enumerates through all the available graphics adapters in the system
// and gathers all relevant information.
//
HRESULT KPD3DEnum::EnumAdapters(void) {
   D3DDISPLAYMODE d3dDspmd;
   HRESULT        hr;
   UINT           nNumAdapters	= 0;
   UINT           nNumModes		= 0;

   // Check if we have an exsisting IDirect3D9 Interface object
   if (!m_pD3D)
	   return KP_INVALIDPARAM;

   nNumAdapters = m_pD3D->GetAdapterCount();												// Returns the number of adapters on the system.

   // At this point we enumerate through all the adapters on the system and
   // query their properties.
   for (UINT nAdapter=0; nAdapter < nNumAdapters; ++nAdapter)
   {
	   m_xAdapterInfo[m_dwNumAdapters].nAdapter  = nAdapter;								// Ordinal number of the adapter
	   m_xAdapterInfo[m_dwNumAdapters].nNumModes = 0;										// Zero the number of modes

	   // Let's fill our KPADAPTERINFO structure with the information describing this adapter
	   m_pD3D->GetAdapterIdentifier(nAdapter, 0, &m_xAdapterInfo[m_dwNumAdapters].d3dAdapterIdentifier);

	   // Loop to enumerate all the available adapter formats
	   for (UINT nFmt=0; nFmt < m_nNumFmt; nFmt++)
	   {
		   // Get the number of display modes available on this adapter with this format.
		   nNumModes = m_pD3D->GetAdapterModeCount(nAdapter, m_fmtAdapter[nFmt]);

		   for (UINT nMode=0; nMode < nNumModes; nMode++)
		   {
			   // Determine whether the adapter supports the requested format and display mode
			   m_pD3D->EnumAdapterModes(nAdapter, m_fmtAdapter[nFmt], nMode, &d3dDspmd);

			   // If this display mode & format combination is less than our minimum requirements
			   // then skip this one and test the next, else store the information.
			   if (d3dDspmd.Width < m_nMinWidth  || d3dDspmd.Height < m_nMinHeight || GetBits(d3dDspmd.Format) < m_nMinBits)
				   continue;
			   else
			   {
				   m_xAdapterInfo[m_dwNumAdapters].d3dDspmd[m_xAdapterInfo[m_dwNumAdapters].nNumModes] = d3dDspmd;
				   m_xAdapterInfo[m_dwNumAdapters].nNumModes++;
               }
           } // ! for display modes
       } // ! for adapter formats

	   // Once we gathered all the supported combinations, we want to sort them
	   // We will use quicksort algorithm
	   // The comparator function SortModesComparator sorts by:
	   // Width, Height, Format, RefreshRate
	   // http://en.wikipedia.org/wiki/Quicksort
       qsort(m_xAdapterInfo[m_dwNumAdapters].d3dDspmd, m_xAdapterInfo[m_dwNumAdapters].nNumModes, sizeof(D3DDISPLAYMODE), SortModesComparator);

	   // Enumerate the available devices for this adapter
	   if ( FAILED( hr = EnumDevices(m_xAdapterInfo[m_dwNumAdapters]) ) ) 
		   return hr;

	   // If there is at least one available device for a given adapter
	   // then store the information and continue enumerating the adapters
	   if (m_xAdapterInfo[m_dwNumAdapters].nNumDevs > 0)
		   m_dwNumAdapters++;
   } // ! for adapters

   return KP_OK;
} // ! EnumAdapters

// EnumDevices Method
/////////////////////
//
// This method enumerates through all of the available device types
// for a given adapter
HRESULT KPD3DEnum::EnumDevices(KPADAPTERINFO &xAdapter)
{
	KPDEVICEINFO		*pDev;
	HRESULT				hr;

	const D3DDEVTYPE d3dDevTypes[] = { 
		D3DDEVTYPE_HAL,					// Hardware rasterization. Shading is done by sw,hw or mixed t&l
		D3DDEVTYPE_SW,					// A pluggable software device that has been registered with IDirect3D9::RegisterSoftwareDevice
		D3DDEVTYPE_REF					// Software implementation of the D3D features. Very slow, only for debugging.
	};

	xAdapter.nNumDevs = 0;				// Zero out the device type counter

	for (UINT nDev=0; nDev < 3; ++nDev)
	{
		pDev = &xAdapter.d3dDevs[xAdapter.nNumDevs];

		pDev->nNumCombo   = 0;
		pDev->nAdapter    = xAdapter.nAdapter;
		pDev->d3dDevType  = d3dDevTypes[nDev];

		// Retrieve the capabilities of the device, fails if the device type is unsupported
		if ( FAILED( m_pD3D->GetDeviceCaps(pDev->nAdapter, pDev->d3dDevType, &pDev->d3dCaps ) ) )
			continue;

		// Enumerates through the devices and verifies whether a device type can be used on the adapter
		if ( FAILED( hr=EnumCombos(*pDev) ) )
			return hr;

		// If there is no supported combination, check the next device type
		if (pDev->nNumCombo <= 0)
			continue;

		// If we have found a device that has valid combination, then store it.
		xAdapter.nNumDevs++;
	} // ! for

	return KP_OK;
} // ! EnumDevices


// EnumCombos Method
////////////////////
//
// Enumerates through the devices and verifies whether
// a device type can be used on the adapter.
HRESULT KPD3DEnum::EnumCombos(KPDEVICEINFO &xDev)
{
	KPCOMBOINFO		*pCombo;
	bool			bWindowed;
	bool			bFmtCheck	= false;

	xDev.nNumCombo = 0;

	// Loop through all of the adapter formats
	for (UINT nFmt_A=0; nFmt_A < m_nNumFmt; ++nFmt_A) {

		// for all allowed backbuffer formats
		for (UINT nFmt_B=0; nFmt_B < g_nFormats_B; nFmt_B++) {

			// in combination with windowed and fullscreen modes
			for (UINT n=0; n < 2; ++n)
			{
				bWindowed = n==0 ? true : false;
				
				//if (n==0)
				//	bWindowed=true;
				//else
				//	bWindowed=false;

				// Verify if the device can be used on the adapter with the given parameters
				// if it fails, we skip the rest of the cycle and test the next 
				if ( FAILED( m_pD3D->CheckDeviceType(xDev.nAdapter, xDev.d3dDevType, m_fmtAdapter[nFmt_A], g_fmtBackBuffer[nFmt_B], bWindowed) ) )
					continue;

				// We have found a combo that is supported by the system. Let's store it.
				pCombo = &xDev.d3dCombo[xDev.nNumCombo];
				pCombo->nAdapter      = xDev.nAdapter;
				pCombo->d3dDevType    = xDev.d3dDevType;
				pCombo->bWindowed     = bWindowed;
				pCombo->fmtBackBuffer = g_fmtBackBuffer[nFmt_B];
				pCombo->fmtAdapter    = m_fmtAdapter[nFmt_A];


				// Z  /  D E P T H  -  S T E N C I L  B U F F E R
				/////////////////////////////////////////////////

				// We sill have to find the z/depth and stencil buffer formats
				// and the best supported vertex processing type. 

				// Selecting Depth and Stencil buffer formats

				// If we want to use stencil buffer
				if (m_nMinStencil)
				{
					pCombo->fmtDepthStencil = D3DFMT_D24S8;			// Try 32-bit Z-buffer with 24 bits for depth and 8 bits for stencil channel
					bFmtCheck = ConfirmDepthFmt(pCombo);			// Check if this format is compatible with the adapter and backbuffer formats

					if (!bFmtCheck) {
						pCombo->fmtDepthStencil = D3DFMT_D24X4S4;	// fall back and try 32-bit Z-buffer with 24bits for depth and 4 bits for stencil channel
						bFmtCheck = ConfirmDepthFmt(pCombo);		// Check, check!
					}
					if (!bFmtCheck) {
						pCombo->fmtDepthStencil = D3DFMT_D15S1;		// fall back to 16-bit zbuffer with 15 bits for depth and 1 bit for stencil channel
						bFmtCheck = ConfirmDepthFmt(pCombo);		// Checker!
					}
				}
				// else stick to depth buffer only
				else
				{
					if (m_nMinDepth > 24) {
						pCombo->fmtDepthStencil = D3DFMT_D32;		// Try 32-bit Z-buffer bit depth
						bFmtCheck = ConfirmDepthFmt(pCombo);		// Check!
					}
					if ( !bFmtCheck && (m_nMinDepth > 16) )
					{
						pCombo->fmtDepthStencil = D3DFMT_D24X8;     // fall back and try 32-bit z-buffer bit depth using 24 bits for the depth channel.
						bFmtCheck = ConfirmDepthFmt(pCombo);		
					}
					else {
						pCombo->fmtDepthStencil = D3DFMT_D16;		// fall back to 16-bit z-buffer bit depth.
						bFmtCheck = ConfirmDepthFmt(pCombo);
					}
				}

				// If we could not find any valid combinations then skip this combo
				if (!bFmtCheck)
					continue;


				// V E R T E X  P R O C E S S I N G
				///////////////////////////////////

				// Select the vertex processing method
				// DirectX supports both HW and SW VP, but the device capabilities are not identical in both cases.
				// HW caps are variable, depending on the display adapter and driver
				// SW caps are fixed.
				// There is also an option for mixed VP 

				// Can the device support transformation and lighting in hardware?
				if ( (xDev.d3dCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0 )
				{
					// Pure Device? Can the device support rasterization, transform, lighting, and shading in hardware?
					// Tells Direct3D not to provide any emulation services for VP.
					// If the device does not support HW VP, the application can only use post-transformed vertices
					if ( ( (xDev.d3dCaps.DevCaps & D3DDEVCAPS_PUREDEVICE) != 0 ) &&
						   ( ConfirmDevice(&xDev.d3dCaps, D3DCREATE_PUREDEVICE, pCombo->fmtBackBuffer) == KP_OK ) )
					{
							pCombo->dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
					}

					// Hardware VP: Specifies Hardware Vertex Processing
					else if ( ConfirmDevice(&xDev.d3dCaps, D3DCREATE_HARDWARE_VERTEXPROCESSING, pCombo->fmtBackBuffer ) == KP_OK )
							pCombo->dwBehavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;

					// Mixed SW/HW: Specifies mixed (both software and hardware) vertex processing
					else if ( ConfirmDevice(&xDev.d3dCaps, D3DCREATE_MIXED_VERTEXPROCESSING, pCombo->fmtBackBuffer)==KP_OK)
							pCombo->dwBehavior = D3DCREATE_MIXED_VERTEXPROCESSING;
				} // ! if HW T&L

				// If the HW does not support T&L in any shape of form, we have to 
				// fall back to software vertex processing.
				else {
					if ( ConfirmDevice(&xDev.d3dCaps, D3DCREATE_SOFTWARE_VERTEXPROCESSING, pCombo->fmtBackBuffer ) == KP_OK )
							pCombo->dwBehavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
				}


				// M U L T I S A M P L I N G
				////////////////////////////

				// Loop through our list of supported multisampling formats and 
				// determine the available multisampling techniques on the device

				DWORD msQuality;

				for(UINT nMS = g_nMS-1 ; nMS >= 0; --nMS) {
					if ( SUCCEEDED( m_pD3D->CheckDeviceMultiSampleType(pCombo->nAdapter,
																	   pCombo->d3dDevType,
																	   pCombo->fmtBackBuffer,
																	   pCombo->bWindowed,
																	   g_msType[nMS],
																	   &msQuality) ) )
					{
							pCombo->msType = g_msType[nMS];
							break;
					}
				}

				// We have gathered everything for this combo
				// Jump to the next one
				xDev.nNumCombo++;

			} // ! for windowed mode
		} // ! for backbuffer format
	} // ! for adapter format/frontbuffer format

	return KP_OK;
}


// ConfirmDepthFmt Method
/////////////////////////
//
// Determines whether the Depth Format is compatible
// with the adapter/frontbuffer and backbuffer formats
bool KPD3DEnum::ConfirmDepthFmt(KPCOMBOINFO *pCombo)
{
	// Check if the DepthStencil surface format is available
	// and can be used as depth-stencil buffer on the device representing
	// this the adapter.
	if ( FAILED( m_pD3D->CheckDeviceFormat(pCombo->nAdapter,
										   pCombo->d3dDevType,
										   pCombo->fmtAdapter,
										   D3DUSAGE_DEPTHSTENCIL,
										   D3DRTYPE_SURFACE,
										   pCombo->fmtDepthStencil) ) )
		return false;

	// Verifies wether the depth-stencil format is compatible
	// with the backbuffer format in a particular display mode
	if ( FAILED( m_pD3D->CheckDepthStencilMatch(pCombo->nAdapter, 
												pCombo->d3dDevType, 
												pCombo->fmtAdapter, 
												pCombo->fmtBackBuffer,
												pCombo->fmtDepthStencil) ) )
		return false;

	return true;
}


// ConfirmDevice Method
///////////////////////
//
// Checked whether HW or Mixed VP is valid for the device. In case of SW VP it always returns true.
// In case of HW/Mixed VP it verifies whether Alpha Blending is available
HRESULT KPD3DEnum::ConfirmDevice(D3DCAPS9* pCaps, DWORD dwBehavior, D3DFORMAT fmtBackbuffer)
{
	if ( (dwBehavior & D3DCREATE_HARDWARE_VERTEXPROCESSING) || (dwBehavior & D3DCREATE_MIXED_VERTEXPROCESSING) )
	{

		// Alpha Blending: Check whether alpha in texture pixels is supported. 
		if ( !(pCaps->TextureCaps & D3DPTEXTURECAPS_ALPHA) )
		{
			fprintf(m_pLog, "[KPD3D_ENUM] error: no alphablending from texture \n");
			return KP_FAIL;
		}
	}

	return KP_OK;
}


// GetBits Method
/////////////////
//
// Returns the number of bits of the D3DFORMAT given as parameter.
UINT KPD3DEnum::GetBits(D3DFORMAT fmt)
{
	switch(fmt) {
	  case D3DFMT_A2B10G10R10: return 30;
	  case D3DFMT_R8G8B8:      return 24;
	  case D3DFMT_A8R8G8B8:    return 24;
	  case D3DFMT_X8R8G8B8:    return 24;
	  case D3DFMT_R5G6B5:      return 16;
	  case D3DFMT_X1R5G5B5:    return 15;
	  case D3DFMT_A1R5G5B5:    return 15;
	  case D3DFMT_X4R4G4B4:    return 12;
	  case D3DFMT_A4R4G4B4:    return 12;
	  case D3DFMT_R3G3B2:      return 8;
	  case D3DFMT_A8R3G3B2:    return 8;
	  default:                 return 0;
	}
}


// D3DDevTypeToString Function
//////////////////////////////
//
// Converts a D3DDEVTYPE into unicode compatible string.
TCHAR* D3DDevTypeToString(D3DDEVTYPE devType)
{
	switch (devType) {
	  case D3DDEVTYPE_HAL:    return TEXT("D3DDEVTYPE_HAL");
	  case D3DDEVTYPE_SW:     return TEXT("D3DDEVTYPE_SW");
	  case D3DDEVTYPE_REF:    return TEXT("D3DDEVTYPE_REF");
	  default:                return TEXT("Unknown devType");
	}
}


// D3DFormatToString Function
/////////////////////////////
//
// Converts a D3DFORMAT to unicode compatible string.
TCHAR* D3DFormatToString(D3DFORMAT format) {
	switch (format) {
	  case D3DFMT_UNKNOWN:         return TEXT("D3DFMT_UNKNOWN");
	  case D3DFMT_R8G8B8:          return TEXT("D3DFMT_R8G8B8");
	  case D3DFMT_A8R8G8B8:        return TEXT("D3DFMT_A8R8G8B8");
	  case D3DFMT_X8R8G8B8:        return TEXT("D3DFMT_X8R8G8B8");
	  case D3DFMT_R5G6B5:          return TEXT("D3DFMT_R5G6B5");
	  case D3DFMT_X1R5G5B5:        return TEXT("D3DFMT_X1R5G5B5");
	  case D3DFMT_A1R5G5B5:        return TEXT("D3DFMT_A1R5G5B5");
	  case D3DFMT_A4R4G4B4:        return TEXT("D3DFMT_A4R4G4B4");
	  case D3DFMT_R3G3B2:          return TEXT("D3DFMT_R3G3B2");
	  case D3DFMT_A8R3G3B2:        return TEXT("D3DFMT_A8R3G3B2");
	  case D3DFMT_X4R4G4B4:        return TEXT("D3DFMT_X4R4G4B4");
	  case D3DFMT_A2B10G10R10:     return TEXT("D3DFMT_A2B10G10R10");
	  case D3DFMT_D16_LOCKABLE:    return TEXT("D3DFMT_D16_LOCKABLE");
	  case D3DFMT_D32:             return TEXT("D3DFMT_D32");
	  case D3DFMT_D15S1:           return TEXT("D3DFMT_D15S1");
	  case D3DFMT_D24S8:           return TEXT("D3DFMT_D24S8");
	  case D3DFMT_D16:             return TEXT("D3DFMT_D16");
	  case D3DFMT_D24X8:           return TEXT("D3DFMT_D24X8");
	  case D3DFMT_D24X4S4:         return TEXT("D3DFMT_D24X4S4");
	  case D3DFMT_VERTEXDATA:      return TEXT("D3DFMT_VERTEXDATA");
	  case D3DFMT_INDEX16:         return TEXT("D3DFMT_INDEX16");
	  case D3DFMT_INDEX32:         return TEXT("D3DFMT_INDEX32");
	  default:                     return TEXT("Unknown format");
	}
}


// BehaviorTypeToString Function
////////////////////////////////
//
// Converts device create behavior flags to unicode compatible strings.
TCHAR* BehaviorTypeToString(DWORD vpt)
{
	switch (vpt) {
	  case D3DCREATE_SOFTWARE_VERTEXPROCESSING: return TEXT("SOFTWARE_VP");
	  case D3DCREATE_MIXED_VERTEXPROCESSING:    return TEXT("MIXED_VP");
	  case D3DCREATE_HARDWARE_VERTEXPROCESSING: return TEXT("HARDWARE_VP");
	  case D3DCREATE_PUREDEVICE:                return TEXT("PURE_HARDWARE_VP");
	  default:                                  return TEXT("Unknown VP");
	}
}
