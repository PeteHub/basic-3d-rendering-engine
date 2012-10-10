/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - August 2009
 *
 *  File: KPD3DSkinManager.h
 *  Description: Direct3D Skin Management
 *				 - Skin Manager
 *
 *****************************************************************
*/

#ifndef KPSKINMANAGER_H
#define KPSKINMANAGER_H

#include <d3d9.h>
#include "KPD3D.h"

// Maximum 2^16-1 materiels can be stored
// 2^16 is indicating that there is no texture
#define KPMAX_ID 65534
#define KPNOTEXTURE 65535


// KPD3DSkinManager Class ////
//////////////////////////////

class KPD3DSkinManager : public KPSkinManager
{
	// Needs access to the class fields
	friend class KPD3DVertexCache;
	friend class KPD3DVertexCacheManager;

protected:
	LPDIRECT3DDEVICE9	m_pDevice;	// Pointer to Direct3D device
	FILE*				m_pLog;		// Pointer to Log file

	// Compares two colors or materials
	bool		ColorEqual(const KPCOLOR *pColorA, const KPCOLOR *pColorB);
	bool		MaterialEqual(const KPMATERIAL *pMaterialA, const KPMATERIAL *pMaterialB);
	
	// Creates a texture file and sets it's transparency
	HRESULT		CreateTexture(KPTEXTURE *pTexture);

	// Specifies a texture color that will be affected by transpaency
	// Mostly used for removing the unneccessary detail from the texture file
	// (1.0,0.0,1.0) is a common RGB value for Alpha Keys. (Pink)
	HRESULT		SetAlphaKey(LPDIRECT3DTEXTURE9 *ppTexture, UCHAR R, UCHAR G, UCHAR B, UCHAR A);

	// Specifies the amount of transparency
	HRESULT		SetTransparency(LPDIRECT3DTEXTURE9 *ppTexture, UCHAR Alpha);

	// Converts the RGBA color values into a Direct3D9 DWORD color
	// Todo: D3DCOLOR_RGBA(r,g,b,a) is equal and part of D3D9 SDK by default
	DWORD		MakeD3DColor(UCHAR R, UCHAR G, UCHAR B, UCHAR A);

	// Logging errors mostly
	void		Log(char * chFormat, ...);

public:
	KPD3DSkinManager(LPDIRECT3DDEVICE9 pDevice, FILE *pLog);
	~KPD3DSkinManager(void);

	// Adds a new skin to the storage
	HRESULT	AddSkin(const KPCOLOR *pAmbient, const KPCOLOR *pDiffuse, const KPCOLOR *pEmissive,
					const KPCOLOR *pSpecular, float fPower, UINT *nSkinID);

	// Adds a new texture to an already exsisting skin in the storage
	HRESULT AddTexture(UINT nSkinID, const char *chName, bool bAlpha, float fAlpha,
					   KPCOLOR *pColorKeys, DWORD numColorKeys);

	// Returns a stroed skin
	KPSKIN		GetSkin(UINT nSkinID);

	// Returns a stored meterial
	KPMATERIAL	GetMaterial(UINT nMaterialID);
	
}; // ! KPD3DSkinManager

#endif KPSKINMANAGER_H // ! KPSKINMANAGER_H