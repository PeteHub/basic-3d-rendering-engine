/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - August 2009
 *
 *  File: KPD3DSkinManager.cpp
 *  Description: Direct3D Skin Manager definiton
 *
 *****************************************************************
*/

#include <io.h> // For file access checking
#include "KPD3DSkinManager.h"
#include "KP.h"


#define _RGB16BIT(r,g,b) ((b>>3) | ((g>>2) << 6) | ((r>>3) <<11)) 
#define  RGB16BIT(r,g,b) ((b%32) + ((g%64) << 5) + ((r%32) << 11))

#define RGB24TO16(rgb) ( ( ((rgb) & 0xF80000 ) >> 16) | ( ((rgb) & 0x00FC00 ) >> 11) | ( ((rgb) & 0x0000F8 ) >> 5) )

// Constructor/Destructor ////
//////////////////////////////
KPD3DSkinManager::KPD3DSkinManager(LPDIRECT3DDEVICE9 pDevice, FILE *pLog)
{
	m_numMaterials	= 0;
	m_numTextures	= 0;
	m_numSkins		= 0;
	m_pMaterials	= NULL;
	m_pTextures		= NULL;
	m_pSkins		= NULL;

	m_pDevice		= pDevice;
	m_pLog			= pLog;

	Log("successfully initialized.");
}

KPD3DSkinManager::~KPD3DSkinManager(void)
{
	// Free up our list of textures
	if ( m_pTextures )
	{
		for ( UINT i = 0; i < m_numTextures; ++i )
		{
			if ( m_pTextures[i].Name )
			{
				delete [] m_pTextures[i].Name;
				m_pTextures[i].Name = NULL;
			}

			if ( m_pTextures[i].pData )
			{
				// Cast our data into D3D9 Texture format and call it's release method
				((LPDIRECT3DTEXTURE9)(m_pTextures[i].pData))->Release();
				m_pTextures[i].pData = NULL;
			}

			if ( m_pTextures[i].pColorKeys )
			{
				delete [] m_pTextures[i].pColorKeys;
				m_pTextures[i].pColorKeys = NULL;
			}
		} // ! for number of textures

		// Free up the Textures object
		free( m_pTextures );
		m_pTextures = NULL;

	} // ! if textures


	// Free up the materials
	if ( m_pMaterials )
	{
		free( m_pMaterials );
		m_pMaterials = NULL;
	}

	// Free up the skins
	if ( m_pSkins )
	{
		free( m_pSkins );
		m_pSkins = NULL;
	}

	Log("successfully uninitialized");
	
} // ! ~KPD3DSkinManager()


// ColorEqual ////
//////////////////
//
// Determines whether two colors are equal
inline bool KPD3DSkinManager::ColorEqual(const KPCOLOR *pColorA, const KPCOLOR *pColorB)
{
	// If any of the components differ, they are not equal
	if (	( pColorA->fR != pColorB->fR ) ||
			( pColorA->fG != pColorB->fB ) ||
			( pColorA->fB != pColorB->fB ) ||
			( pColorA->fA != pColorB->fA ) )
			return false;
	return true;
} // ! ColorEqual


// MaterialEqual ////
/////////////////////
//
// Dethermines whether two materials are the same
bool KPD3DSkinManager::MaterialEqual(const KPMATERIAL *pMaterialA, const KPMATERIAL *pMaterialB)
{
	if ( ! ColorEqual(&pMaterialA->Diffuse, &pMaterialB->Diffuse) ||
		 ! ColorEqual(&pMaterialA->Ambient, &pMaterialB->Ambient) ||
		 ! ColorEqual(&pMaterialA->Specular, &pMaterialB->Specular) ||
		 ! ColorEqual(&pMaterialA->Emissive, &pMaterialB->Emissive) ||
		 pMaterialA->fPower != pMaterialB->fPower)
		return false;
	return true;

} // ! MaterialEqual


// GetSkin ////
///////////////
//
// Returns the KPSkin object of the given nSkinID
// If there is no such skin, it returns an empty skin object
KPSKIN KPD3DSkinManager::GetSkin(UINT nSkinID)
{
	if ( nSkinID < m_numSkins )
		return m_pSkins[nSkinID];
	else
	{
		// If there is no skin associated with the skin id
		// we return an empty skin object
		KPSKIN After_Forever_Beautiful_Emptiness = { 0 };
		// Rock on! Then return :-)
		return After_Forever_Beautiful_Emptiness;
	}
} // ! GetSkin


// GetMaterial ////
///////////////////
//
// Returns the KPMaterial object connected to the given material ID
// or an empty material if it does not exsist
KPMATERIAL KPD3DSkinManager::GetMaterial(UINT nMaterialID)
{
	if ( nMaterialID < m_numMaterials )
		return m_pMaterials[nMaterialID];
	else
	{
		// If no such material exsist, return an empty one
		KPMATERIAL I_AM_EMPTY = { 0 };
		return I_AM_EMPTY;
	}
} // ! GetMaterial


// AddSkin ////
///////////////
//
// Creates a new skin object using a specified material
// The ID of the new skin object will be returned through nSkinID pointer.
HRESULT KPD3DSkinManager::AddSkin(const KPCOLOR *pAmbient, const KPCOLOR *pDiffuse, const KPCOLOR *pEmissive, 
								  const KPCOLOR *pSpecular, float fPower, UINT *nSkinID)
{
	/*
		Every skin must have a material that defines how it behaves when lit, 
		but it does not have to neccessarily have a texture assigned to it.

		The Skin list will dynamically change size during the course of the 
		runtime, we will have to expand it. In order to minimize the amount of 
		times we will have to reinitialize the list, we will allocate memory
		for an extra 50 objects every time. The skin structure is fairly small
		so there wont be much overhead.
	*/

	int size;		// Temp variable used for calculations

	// If there are no skins stored or it's a multiple fo 50
	// We need to allocate more memory for the skin ids.
	if ( ( m_numSkins % 25 ) == 0 )
	{
		// Calculate the new size of the list
		size = ( m_numSkins + 25 ) * sizeof(KPSKIN);

		// Reallocate the list with the increased size.
		void* tmp = realloc(m_pSkins, size);
		if ( tmp != NULL)
		{
			m_pSkins = (KPSKIN*)tmp;
			tmp = NULL;
		}
		else
		{
			free(m_pSkins);
			free(tmp);
			Log("AddSkin: Unable to reallocate SKIN object"); 
			return KP_OUTOFMEMORY;
		}

		/*
		if ( ( m_pSkins = (KPSKIN*)realloc(m_pSkins, size) ) == NULL )
		{
			Log("AddSkin: Unable to reallocate SKIN object"); 
			return KP_OUTOFMEMORY;
		}
		*/
	}

	// Create the material of the Skin
	KPMATERIAL material;
	material.Ambient	= *pAmbient;
	material.Diffuse	= *pDiffuse;
	material.Emissive	= *pEmissive;
	material.Specular	= *pSpecular;
	material.fPower		=  fPower;

	// Check wheter the material alrady exsists in our lists
	// This can get very expensive on calculations, 17 comparisons / material. 
	// Considering that the size of the KPMATERIAL structure is 5*sizeof(float)
	// Which is on nowadays computers 160bit, 20 byte, 65535 materials would 
	// take rougly 1.4mb space. It might worth considering skipping this check.

	bool bMaterial = false;		// Material already exsists?
	UINT nMaterialID;			// Our material's ID

	for ( nMaterialID = 0; nMaterialID < m_numMaterials; ++nMaterialID )
	{
		if ( MaterialEqual(&material, &m_pMaterials[nMaterialID]) )
		{
			bMaterial = true;
			break;
		}
	}

	// if exsists, just store it's ID
	// otherwise we have to create it
	if ( bMaterial )
	{
		m_pSkins[m_numSkins].nMaterial = nMaterialID;
	}
	else
	{
		m_pSkins[m_numSkins].nMaterial = m_numMaterials;

		// Check wheter we need to expans the material list
		if ( ( m_numMaterials % 50 ) == 0 )
		{
			// Calculate the size of the new material list
			size = ( m_numMaterials + 50 ) * sizeof(KPMATERIAL);
			
			// Reallocate the material list with the new size
			void* tmp = realloc(m_pMaterials, size);
			if ( tmp != NULL )
			{
				m_pMaterials = (KPMATERIAL*)tmp;
				tmp = NULL;
			}
			else
			{
				free(m_pMaterials);
				free(tmp);
				Log("AddSkin: Unable to reallocate MATERIAL Object");
				return KP_OUTOFMEMORY;
			}

			/*
			if ( ( m_pMaterials = (KPMATERIAL*)realloc(m_pMaterials, size) ) == NULL )
			{
				Log("AddSkin: Unable to reallocate MATERIAL Object");
				return KP_OUTOFMEMORY;
			}
			*/
		}

		// Copy the material data into the list
		memcpy_s(&m_pMaterials[m_numMaterials], sizeof(KPMATERIAL), &material, sizeof(KPMATERIAL));
		++m_numMaterials;
	}

	// By default we set alpha blending to false.
	m_pSkins[m_numSkins].bAlpha = false;

	// We set the Skin textures KPNOTEXTURE
	for ( int i = 0; i < 8; ++i )
		m_pSkins[m_numSkins].nTexture[i] = KPNOTEXTURE;

	// Save the SkinID and increment the skin counter
	*nSkinID = m_numSkins;
	++m_numSkins;

	return KP_OK;

} // ! AddSkin


// AddTexture ////
//////////////////
//
// Adds a texture to a given skin and sets alpha blending
HRESULT KPD3DSkinManager::AddTexture(UINT nSkinID, const char *chName, bool bAlpha, float fAlpha, KPCOLOR *pColorKeys, DWORD numColorKeys)
{
	// Check whether we got a valid skin id
	if ( m_numSkins <= nSkinID )
		return KP_INVALIDID;

	// Check whether we have any free texture slots
	if ( m_pSkins[nSkinID].nTexture[7] != KPNOTEXTURE )
	{
		Log("AddTexture: SkinID: %d has no free texture slots", nSkinID);
		return KP_BUFFERSIZE;
	}

	///////////////////////////////////////////////////
	
	// Let's see whether we have this texture already stored
	bool bTexture	= false;
	UINT nTextureID = 0;

	for ( nTextureID = 0; nTextureID < m_numTextures; ++nTextureID )
	{
		// Check whether the texture already exsists in our list
		if ( strcmp( chName, m_pTextures[nTextureID].Name ) == 0 )
		{
			bTexture = true;
			break;
		}
	}

	// Load a new texture if it does not exsist
	if ( !bTexture )
	{
		// Reallocate our texture index if neccessary
		if ( ( m_numTextures % 25 ) == 0 )
		{
			// Calculate the new size of the index
			int size = ( m_numTextures + 25 ) * sizeof(KPTEXTURE);

			// Reallocate the index with the new size
			void* tmp = realloc(m_pTextures, size);
			if ( tmp != NULL)
			{
				m_pTextures = (KPTEXTURE*)tmp;
				tmp = NULL;
			}
			else
			{
				free(m_pTextures);
				free(tmp);
				Log("AddTexture: unable to reallocate Texture object");
				return KP_OUTOFMEMORY;
			}
			/*
			if ( ( m_pTextures = (KPTEXTURE*)realloc(m_pTextures, size) ) == NULL )
			{
				Log("AddTexture: unable to reallocate Texture object");
				return KP_OUTOFMEMORY;
			}
			*/

		}

		// Check whether we should use alpha blending 
		if ( bAlpha )
			m_pSkins[nSkinID].bAlpha = true;
		else
			m_pTextures[m_numTextures].pColorKeys = NULL;

		// Store the texture name
		m_pTextures[m_numTextures].Name = new char[strlen(chName)+1];
		memcpy_s(m_pTextures[m_numTextures].Name, sizeof(char)*(strlen(chName)+1), chName, strlen(chName)+1);

		// Create our D3D texture
		HRESULT hr = CreateTexture(&m_pTextures[m_numTextures]);
		if ( FAILED(hr) )
		{
			Log("AddTexture: Unable to create texture: \"%s\"", m_pTextures[m_numTextures].Name);
			return hr;
		}

		// Set Alpha Blending if needed
		if ( bAlpha )
		{
			// Create our texture and set it's attributes
			KPTEXTURE *pTMPTexture		= &m_pTextures[m_numTextures];

			pTMPTexture->numColorKeys	= numColorKeys;
			pTMPTexture->pColorKeys		= new KPCOLOR[numColorKeys];
			memcpy_s(pTMPTexture->pColorKeys, sizeof(pTMPTexture->pColorKeys), pColorKeys, sizeof(KPCOLOR)*numColorKeys);

			// Convert it into Direct3D texture format
			LPDIRECT3DTEXTURE9 pTexture = (LPDIRECT3DTEXTURE9)pTMPTexture->pData;


			// Set the alpha colorkeys
			// It is imperative that we set the alpha keys for the texture before
			// we try to apply the general transparency because SetAlphaKey is looking for
			// 1.0f alpha values and SetTransparency modifies these values to fine tune 
			// the general transparency of the texture
			for ( DWORD i = 0; i < numColorKeys; ++numColorKeys )
			{
				// Have to convert the colours from float to DWORD
				// That's how D3D works. float*255
				hr = SetAlphaKey(&pTexture, UCHAR(pColorKeys[i].fR*255), UCHAR(pColorKeys[i].fG*255),
											UCHAR(pColorKeys[i].fB*255), UCHAR(pColorKeys[i].fA*255) );

				if ( FAILED(hr) )
				{
					Log("AddTexture: ERRNO:%f Unable to set alpha color keys for texture: \"%s\"",hr, m_pTextures[m_numTextures].Name);
					return hr;
				}
			}

			// Set general transparency
			hr = SetTransparency(&pTexture, UCHAR(fAlpha*255));
			if ( FAILED(hr) )
			{
				Log("AddTexture: ERRNO:%f Unable to set transparency for texture: \"%s\"",hr, m_pTextures[m_numTextures].Name);
				return hr;
			}

		} // ! bAlpha

		// Get a backup of the current ID and increment it
		nTextureID = m_numTextures;
		++m_numTextures;

	} // ! bTexture

	for ( int i = 0; i < 8; ++i )
	{
		// We put our texture ID into the first free slot
		if ( m_pSkins[nSkinID].nTexture[i] == KPNOTEXTURE )
		{
			m_pSkins[nSkinID].nTexture[i] = nTextureID;
			break;
		}
	}

	return KP_OK;

} // ! AddTexture


// CreateTexture ////
/////////////////////
//
// Loads a 24-bit BMP texture file and sets alpha blending
HRESULT KPD3DSkinManager::CreateTexture(KPTEXTURE *pTexture)
{

	HRESULT hr;

	if ( _access(pTexture->Name, 0) == -1 )
	{
		Log("CreateTexture: File not found: \"%s\"", pTexture->Name);
		return KP_FILENOTFOUND;
	}

	// Create a Direct3D texture object
	LPDIRECT3DTEXTURE9 pTex = NULL;

	// Create a texture from the bmp file and store it in the d3d object
	if ( FAILED( hr = D3DXCreateTextureFromFile(m_pDevice, pTexture->Name, &pTex ) ) )
	{
		switch(hr)
		{
		case D3DERR_INVALIDCALL:
			Log("CreateTexture: IDirect3DDevice::CreateTexture error D3DERR_INVALIDCALL");
			break;
		case D3DERR_OUTOFVIDEOMEMORY:
			Log("CreateTexture: IDirect3DDevice::CreateTexture error D3DERR_OUTOFVIDEOMEMORY");
			break;
		case E_OUTOFMEMORY:
			Log("CreateTexture: IDirect3DDevice::CreateTexture error E_OUTOFMEMORY");
			break;
		case D3DOK_NOAUTOGEN:
			Log("CreateTexture: IDirect3DDevice::CreateTexture error D3DOK_NOAUTOGEN");
			break;
		default:
			Log("CreateTexture: IDirect3DDevice::CreateTexture error UNKNOWN");
		}

		return KP_FAIL;
	}

	// Copy the Direct3D texture data into our texture structure
	pTexture->pData = (KPTEXTURE*)pTex;

	return KP_OK;

} // ! CreateTexture


// MakeD3DColor ////
////////////////////
//
// Converts the separated color channels into a single DWORD structure.
DWORD KPD3DSkinManager::MakeD3DColor(UCHAR R, UCHAR G, UCHAR B, UCHAR A)
{
	/*
		This is exactly the same thing as D3DCOLOR_RGBA() macro does.

		From the D3D9 SDK:

		#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)
		#define D3DCOLOR_ARGB(a,r,g,b) ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

		Basically, we are shifting the color values in reverse order by 8 bits to the left

		& 0xff ensures that each channel is 8 bits long, while does not change the values
		D3DCOLOR is just a typedef for DWORD, so we are good to go!
	*/

	return D3DCOLOR_RGBA(R, G, B, A);

} // ! MakeD3DColor

// SetAlphaKey ////
///////////////////
//
// Sets the alpha channel of all pixels of the given RGB value
// to the amount of transparency specificed.
// Make sure this function is always called before SetTransparency().
HRESULT	KPD3DSkinManager::SetAlphaKey(LPDIRECT3DTEXTURE9 *ppTexture, UCHAR R, UCHAR G, UCHAR B, UCHAR A)
{
	D3DSURFACE_DESC		desc;		// Surface description, used to retrieve some texture details
	D3DLOCKED_RECT		rect;		// Describes a locked rectangular area of a surface.
	DWORD				ColorKey, Color;

	// Make sure our texture is in 32-bit ARGB format
	(*ppTexture)->GetLevelDesc(0, &desc);

	if ( desc.Format != D3DFMT_A8R8G8B8 )
	{
		Log("SetAlphaKey: Invalid texture format");
		return KP_INVALIDPARAM;
	}

	// Build the D3D compatible DWORD ColorKey value
	// Alpha Color Keys have 1.0/255/0xff value in alpha channel
	ColorKey = MakeD3DColor(R, G, B, 255);

	// Build the new Color values for the colors affected by the color key
	if ( A > 0 )
		Color = MakeD3DColor(R, G, B, A);	// If alpha is not 0, set the colors and the alpha value 
	else
		Color = MakeD3DColor(0, 0, 0, A);	// if alpha is 0, it is totally transparent, colors won't matter.

	// Lock the surface to prevent any changes during our operation
	if ( FAILED( (*ppTexture)->LockRect(0, &rect, NULL, 0) ) )
	{
		Log("SetAlphaKey: Unable to lock texture");
		return KP_BUFFERLOCK;
	}

	// Replace the Color Key pixels with their new values
	for ( DWORD y = 0; y < desc.Height; ++y )
	{
		for ( DWORD x = 0; x < desc.Width; ++x )
		{
			// If the pixel is affected by the ColorKey, replace it with new Color
			if ( ((DWORD*)rect.pBits)[y*desc.Width + x] == ColorKey )
				((DWORD*)rect.pBits)[y*desc.Width + x] = Color;

		} // ! for width

	} // ! for height

	// We can unlock the surface now
	(*ppTexture)->UnlockRect(0);

	return KP_OK;

} // ! SetAlphaKey


// SetTransparency ////
///////////////////////
//
// Applies transparency to the whole texture.
// This function must be called AFTER the Alpha Keys were applied.
HRESULT KPD3DSkinManager::SetTransparency(LPDIRECT3DTEXTURE9 *ppTexture, UCHAR Alpha)
{
	D3DSURFACE_DESC	desc;
	D3DLOCKED_RECT	rect;
	DWORD			Color;
	UCHAR			R, G, B, A;

	// Make sure our texture is in 32-bit ARGB format
	(*ppTexture)->GetLevelDesc(0, &desc);

	if ( desc.Format != D3DFMT_A8R8G8B8 )
	{
		Log("SetTransparency: Invalid texture format");
		return KP_INVALIDPARAM;
	}

	// Lock the surface to prevent any changes during our operation
	if ( FAILED( (*ppTexture)->LockRect(0, &rect, NULL, 0) ) )
	{
		Log("SetTransparency: Unable to lock texture");
		return KP_BUFFERLOCK;
	}

	// Apply transparency to every pixel of the surface
	for ( DWORD y = 0; y < desc.Height; ++y )
	{
		for ( DWORD x = 0; x < desc.Width; ++x )
		{
			// Retrieve the color value of the pixel [ rownum + rowwidth + columnnum ]
			// (where columnwidth is sizeof(x) = dword = 4byte
			Color = ((DWORD*)rect.pBits)[y*desc.Width + x];

			// Calculate the color values from pixel color
			A = (UCHAR)( (Color & 0xFF000000) >> 24);
			R = (UCHAR)( (Color & 0x00FF0000) >> 16);
			G = (UCHAR)( (Color & 0x0000FF00) >> 8);
			B = (UCHAR)(  Color & 0x000000FF);

			// Only change alpha if it's smaller than the one provided by the AlphaKey
			// This way we can have 100% transparent key colors while the rest is less
			if ( Alpha < A )
				A = Alpha;

			// Set the new color value with the updated alpha channel
			((DWORD*)rect.pBits)[y*desc.Width + x] = MakeD3DColor(R, G, B, A);

		} // ! for width

	} // ! for height

	// We can unlock the surface now.
	(*ppTexture)->UnlockRect(0);
	
	return KP_OK;

} // ! SetTransparency


// Log Function ////
////////////////////
void KPD3DSkinManager::Log(char *chFormat, ...)
{
	char msg[256];
	char *pArgs;

	// Point at the argument list
	pArgs = (char*) &chFormat + sizeof(chFormat);

	// Convert arguments to message using the format string
	vsprintf_s(msg, sizeof(msg), chFormat, pArgs);

	fprintf(m_pLog, "[ KPD3DSkinManager ]: ");
	fprintf(m_pLog, msg);
	fprintf(m_pLog, "\n");

//#ifdef _DEBUG
	// Instantly write the buffer into the log file
	// This way, even if the program crashes, you get the information
	fflush(m_pLog);
//#endif

} // ! ::Log()