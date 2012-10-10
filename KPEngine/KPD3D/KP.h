/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: KP.h
 *  Description: Collection of common data structures, enumerations and engine definitions
 *				 - Error Codes
 *				 - Color, Material, Texture, Skin structures
 *				 - Vertex and Vertex type structures
 *				 - Rendering State, Viewport and Projection structures
 *
 *****************************************************************
*/

#ifndef KP_H
#define KP_H

#include <windows.h>


// Specific/Custom Error Messages ////
//////////////////////////////////////
#define KP_OK                   S_OK
#define KP_CANCELLED            0x82000000		// Report No Error
#define KP_FAIL                 0x82000001		// General Error 

#define KP_CREATEAPI			0x82000002
#define KP_CREATEDEVICE			0x82000003
#define KP_CREATEBUFFER			0x82000004
#define KP_INVALIDPARAM			0x82000005
#define KP_INVALIDID			0x82000006
#define KP_BUFFERSIZE			0x82000007
#define KP_BUFFERLOCK			0x82000008
#define KP_NOTCOMPATIBLE		0x82000009
#define KP_OUTOFMEMORY			0x8200000a
#define KP_FILENOTFOUND			0x8200000b
#define KP_INVALIDFILE			0x8200000c
#define KP_NOSHADERSUPPORT		0x8200000d


// Most of these structures are based on the D3D9 SDK
// but provided here independently to prevent reliance on any specific API.

// Color Type Structures ////
/////////////////////////////

/*
	Describes the RGBA color values

	fR,fG,fB are floating-point values specifying the red, green and blue
	components of a color respectively.

	fA is a floating-point value specifying the alpha component of the color.

	The values generally are in range from 0.0f through 1.0f, with 0.0f being black
	or in case of the alpha component, completely transparent.

	NOTE:	You can set themembers of this structure to values outside the range of 0 through 1.
			Values greater than 1 will produce strong lights that usually wash out the scene, 
			while negative values produce dark lights that actually remove light from a scene.
*/
typedef struct KPCOLOR_RGBA
{
	union
	{
		struct
		{
			float fR;	
			float fG;
			float fB;
			float fA;	
		};
		float SetAll[4];
	};
} KPCOLOR_RGBA, KPCOLOR;

// Material Type Structures ////
////////////////////////////////

/*
	Specifies material properties.

	Diffuse, Ambient, Specular and Emissive values specify the different color values
	of a material, respectively. See KPCOLOR for more.

	fPower is a floating-point value specifying the sharpness of specular highlights.
	Higher value means sharper highlight.
*/
typedef struct KPMATERIAL
{
	KPCOLOR Diffuse;
	KPCOLOR Ambient;
	KPCOLOR Specular;
	KPCOLOR Emissive;
	float	fPower;

} KPMATERIAL;


// Texture Type Structures ////
///////////////////////////////

/*
	fAlpha
		Floating-point value specifying the overall transparency of the texture.
		It is in the range 0.0f through 1.0f, 0.0f being completely transparent.

	*Name
		Pointer to a string holding the name of the graphics file used as texture.
		Also used as an ID for the texture, which means it is unique.

	*pData
		Pointer to the texture data.

	*pColorKeys
		An array storing the properties of different colors of the texture;

	numColorKeys
		The number of Color Keys in the array.	
*/

typedef struct KPTEXTURE
{
	float	fAlpha;
	char	*Name;
	void	*pData;
	KPCOLOR	*pColorKeys;
	DWORD	numColorKeys;
} KPTEXTURE;


// Skin Structure Types ////
////////////////////////////

/*
	Describes the Skins.

	bAlpha
		Specifies whether the skin is transparent.

	nMaterial
		Unsigned int value specifying the ID of the material assigned to the skin.

	nTexture[]
		Array of unsigned int values specifying the IDs of the textures assigned to the skin.
*/
typedef struct KPSKIN
{
	bool	bAlpha;
	UINT	nMaterial;
	UINT	nTexture[8];	// Could check device caps for single-pass multitexturing limit

} KPSKIN;


// Viewport Structures ////
///////////////////////////
/*
	Defines an area on the back buffer that receives the rendered image.
	A viewport can be lesser or equal to the client area of the window.
	There can be multple viewports in a single client area.
	
	(for example, 3d editors have usually 4 default viewports, for top, left, right
	and perspective view of the same scene at the same time)
*/
typedef struct KPVIEWPORT
{
	DWORD	x;	// Upper left corner
	DWORD	y;	
	DWORD	width;
	DWORD	height;
} KPVIEWPORT;


// Game Engine Modes ////
/////////////////////////
typedef enum KPENGINEMODE_ENUM
{
	EMD_PERSPECTIVE,		// Perspective projection mode (distance of the viewer applied to the projection)
	EMD_TWOD,				// In this mode, the world coordinates equal the screen coordinates (Good for drawing 2d on screen)
	EMD_ORTHOGONAL			// Distance of the viewer is not applied (generally used by 3d editors)
} KPENGINEMODE;

// Vertex Format Structures ////
////////////////////////////////

typedef enum KPVERTEXID
{
	VID_UU,					// Untransformed and unlit vertex
	VID_UL					// Untransformed and lit vertex

} KPVERTEXID;

// VID_UU Structure
typedef struct VERTEX
{
	float x, y, z;			// Untransformed position coordinates of the vertex
	float vcNormal[3];		// Vertex Normal
	float tu, tv;			// Texture Coordinates

} VERTEX;

// VID_UL Structure
typedef struct LVERTEX
{
	float x, y, z;			// Untransformed position coordinates of the vertex
	DWORD Color;			// Diffuse Color
	float tu, tv;			// Texture Coordinates

} LVERTEX;


// Engine Render States ////
////////////////////////////
typedef enum KPRENDERSTATE
{
	RS_CULL_CW,			// Culling back faces clockwise
	RS_CULL_CCW,		// Culling back faces counter-clockwise
	RS_CULL_NONE,		// Don't cull backfaces
	RS_DEPTH_READWRITE, // Read and Write depth buffer
	RS_DEPTH_READONLY,  // No writes to depth buffer
	RS_DEPTH_NONE,		// Neither reads nor writes from/to depth buffer
	RS_SHADE_POINTS,	// Render vertices as points / Fill Mode Point
	RS_SHADE_TRIWIRE,	// Render wireframe triangles
	RS_SHADE_LINES,		// Render lines
	RS_SHADE_HULLWIRE,	// Render wireframe polygons / Fill Mode Wireframe
	RS_SHADE_SOLID		// Render solid triangles / Fill Mode Solid

} KPRENDERSTATE;


#endif // !KP_H