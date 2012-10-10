#include <stdio.h>
#include <windows.h>
#include <string>
#include <map>

#include "KP.h"
#include "KPRenderDevice.h"


#ifndef KPMODEL_H
#define KPMODEL_H

typedef struct STRUCT_FACE
{
   WORD i0, i1, i2;	// Index of the vertices that build this face
   UINT nMat;		// ID of the material applied to this face
} TRIANGLE;

class KPModel
{
protected:
	bool			m_bReady;					// Is the model completely loaded and ready for rendering?!
	FILE			*m_pFile;					// File handle

	std::string		m_MaterialMap[128];			// Array of OBJ material names for mapping to id
	UINT			m_numMaterials;				// Number of materials stored for this model
	UINT			*m_pSkins;					// Array of Skin IDs used by the model

	UINT			m_numVertices;				// Number of vertices building up the model
	VERTEX			*m_pVertices;				// Array of vertices

	UINT			m_numIndices;				// Number of triangles in the object
	WORD			*m_pIndices;				// List of trianle IDs

	KPRenderDevice	*m_pDevice;					// Pointer to the rendering device 

	UINT			*m_pBufferID;				// Static Vertex Cache Buffer id where the model data is stored

	bool	LoadFile(void);						// Reads the OBJ file and loads all the data
	void	LoadMaterials(FILE* file);			// Loads the Material Library of an OBJ file
	UINT	MapMaterial(const char* mName);		// Maps the material string to the ID

public:
	KPModel(const char* filePath, KPRenderDevice *pDevice);
	~KPModel(void);

	HRESULT Render(void);
};

// Checks whether a substring is part of a string
// returns the position of the first character of the substring if true, -1 if false
int IsInString(const char* string, const char* substring);

#endif