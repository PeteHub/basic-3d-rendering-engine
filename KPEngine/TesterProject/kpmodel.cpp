#include <assert.h>

#include "kpmodel.h"

KPModel::KPModel(const char *filePath, KPRenderDevice *pDevice, FILE *pLog)
{
	assert(filePath);
	assert(pDevice);
	assert(pLog);

	if( !pDevice || !pLog || !m_pFile || !filePath || filePath == "" )
		return;

	m_pDevice		= pDevice;
	m_pLog			= pLog;

	m_bReady		= false;
	m_numMaterials	= 0;

	m_pSkins		= NULL;

	m_numVertices	= 0;
	m_pVertices		= NULL;

	m_numIndices	= 0;
	m_pIndices		= NULL;

	m_pBufferID		= NULL;

	fopen_s(&m_pFile, filePath, "r");
	if ( m_pFile && LoadFile() )
	{
		m_bReady = true;
		fclose(m_pFile);

		fprintf(m_pLog, "Modell sikeresen betöltve\n");
	}
	else
		fprintf(m_pLog, "Hiba a modell betöltése során. Rossz fájl formátum, vagy a fájl nem létezik!\n");	
}

KPModel::~KPModel(void)
{
	if (m_pVertices)
	{
		delete [] m_pVertices;
		m_pVertices = NULL;
	}

	if (m_pIndices)
	{
		delete [] m_pIndices;
		m_pIndices = NULL;
	}

	if (m_pSkins)
	{
		delete [] m_pSkins;
		m_pSkins = NULL;
	}

	if (m_pBufferID)
	{
		delete [] m_pBufferID;
		m_pBufferID = NULL;
	}

	m_bReady = false;
}

bool KPModel::LoadFile(void)
{
	char	buffer[512];						// buffer for the obj file
	char	mtlfileName[MAX_PATH] = "";			// path to the material library file
	FILE	*mtlfile;							// handle of the material library file
	char	materialName[100];					// name of the currently active material

	UINT	v0=0,v1=0,v2=0,vt0=0,vt1=0,vt2=0;	// temporary vetrex and vertex texture index ids
	UINT	numVertices = 0, numTextCoords = 0; // temporary data counters
	UINT	numGroups = 0;
	UINT	*numFaces;							// tmp counter for faces of each group
	VERTEX  *v  = NULL;							// tmp vertex and vertex texture coordinate buffers
	VERTEX  *vt = NULL;
	UINT	vi =0, vti = 0,	gi = 0;				// tmp indexes
	KPVector vMin, vMax;						// Max and min coordinates of the object
	UINT	vCount=0, iCount=0;



	if (!m_pDevice)
		return false;

	numFaces = new UINT[65534];
	ZeroMemory(numFaces, sizeof(UINT)*65534);

	////
	//	First we have to load the materials and textures
	////

	while ( fgets(buffer, sizeof(buffer), m_pFile) != NULL )
	{
		// Load the Material Libraries
		//////////////////////////////

		// If the line starts with a Material Library definition
		if ( IsInString(buffer, "mtllib ") == 0 )
		{
			// Get the file path from buffer and open the file
			sscanf_s(buffer, "mtllib %s", mtlfileName, sizeof(mtlfileName));

			// Open the Material Library
			fopen_s(&mtlfile, mtlfileName, "r");
			assert(mtlfile);

			if ( mtlfile )
			{
				LoadMaterials(mtlfile);
				fclose(mtlfile);
			}
			else
				return false;
		}

	} // ! while loading materials

	rewind(m_pFile);

	////
	//	Count the number of vertex, texture, group and face entries
	////

	while ( fgets(buffer, sizeof(buffer), m_pFile) != NULL )
	{
		if ( buffer[0] == 'v' && buffer[1] == ' ' )
			numVertices++;
		else if ( buffer[0] == 'v' && buffer[1] == 't' )
			numTextCoords++;
		else if	( buffer[0] == 'f' && buffer[1] == ' ' )
		{
			iCount+=3;
			numFaces[numGroups-1]++;
		}
		else if ( IsInString(buffer, "usemtl ") != -1 )
			numGroups++;
	} // ! while

	vCount = numVertices;

	rewind(m_pFile);
	
	////
	//	Allocate memory for the arrays
	////

	try
	{
		v			= new VERTEX[numVertices];
		vt			= new VERTEX[numTextCoords];
		m_pBufferID = new UINT[m_numMaterials];
	}
	catch (std::bad_alloc)
	{
		delete[] v;
		delete[] vt;
		v		= NULL;
		vt		= NULL;

		return false;
	}

	////
	//	Start reading the data
	////

	while ( fgets(buffer, sizeof(buffer), m_pFile) != NULL )
	{
		if ( buffer[0] == 'v' && buffer[1] == ' ' )
		{
			sscanf_s(buffer, "v  %f %f %f", &v[vi].x, &v[vi].y, &v[vi].z);
			vi++;
		}
		else if ( buffer[0] == 'v' && buffer[1] == 't' )
		{
			sscanf_s(buffer, "vt %f %f", &vt[vti].tu, &vt[vti].tv);
			vti++;
		}
		else if ( IsInString(buffer, "usemtl ") != -1 )
		{
			// Now we can allocate vertex and index buffers
			try
			{
				m_numVertices = numFaces[gi]*3;
				m_pVertices = new VERTEX[m_numVertices];
				m_numIndices= m_numVertices;
				m_pIndices	= new WORD[m_numIndices];
			}
			catch (std::bad_alloc)
			{
				delete m_pVertices;
				delete m_pIndices;
			}

			// Change the active material to the specified one
			sscanf_s(buffer, "usemtl %s", materialName, sizeof(materialName));

			// Loop through the face entries belonging to this group
			for ( UINT i = 0; i< numFaces[gi]; ++i )
			{
				// Read new line from file
				if ( fgets(buffer, sizeof(buffer), m_pFile) != NULL )
				{
					// Fill temprorary variables with face data
					if ( strstr(buffer, "/") )
						sscanf_s(buffer, "f %d/%d %d/%d %d/%d", &v0, &vt0, &v1, &vt1, &v2, &vt2);
					else
						sscanf_s(buffer, "f %d %d %d", &v0, &v1, &v2);

					// Construct the vertices and indices
					//
					int idx = i*3;

					memcpy(&m_pVertices[idx], &v[v0-1], sizeof(VERTEX));
					memcpy(&m_pVertices[idx+1], &v[v1-1], sizeof(VERTEX));
					memcpy(&m_pVertices[idx+2], &v[v2-1], sizeof(VERTEX));

					if ( strstr(buffer, "/") )
					{
						m_pVertices[idx].tu = vt[vt0-1].tu;
						m_pVertices[idx].tv = vt[vt0-1].tv;
						m_pVertices[idx+1].tu = vt[vt1-1].tu;
						m_pVertices[idx+1].tv = vt[vt1-1].tv;
						m_pVertices[idx+2].tu = vt[vt2-1].tu;
						m_pVertices[idx+2].tv = vt[vt2-1].tv;
					}

					// TODO: Vector normals for better lightning.


					// Build the vertex index list
					//
					m_pIndices[idx]	  = idx;
					m_pIndices[idx+1] = idx+1;
					m_pIndices[idx+2] = idx+2;

				} // ! if read line

			} // ! for faces
			
			// Add data to the vertex cache manager
			//
			if ( ! m_pDevice->GetVertexManager() )
				return false;

			if ( FAILED( m_pDevice->GetVertexManager()->CreateStaticBuffer(VID_UU, m_pSkins[MapMaterial(materialName)], m_numVertices,	m_numIndices, m_pVertices, m_pIndices, &m_pBufferID[MapMaterial(materialName)]) ) )
				return false;
			// we are done with this material group, set index to next
			gi++;

			delete m_pVertices; m_pVertices = NULL;
			delete m_pIndices; m_pIndices = NULL;
			m_numVertices = m_numIndices = 0;
		} // ! else usemtl
	} // ! while

	// Find Min and Max point of the object
	// Find half length diameter of the bounding box
	memcpy(&vMin, &v[0], sizeof(float)*3);
	memcpy(&vMax, &v[0], sizeof(float)*3);

	for (UINT i=1; i< numVertices; ++i)
	{
		if ( vMin.x > v[i].x )
			 vMin.x = v[i].x;
		if ( vMin.y > v[i].y )
			 vMin.y = v[i].y;
		if ( vMin.z > v[i].z )
			 vMin.z = v[i].z;

		if ( vMax.x < v[i].x )
			 vMax.x = v[i].x;
		if ( vMax.y < v[i].y )
			 vMax.y = v[i].y;
		if ( vMax.z < v[i].z )
			 vMax.z = v[i].z;
	}

	m_vCenter = (vMax + vMin) * 0.5f;
	m_fHalfLength = ((vMax - vMin) * 0.5f).GetLength();

	delete[] numFaces;
	delete[] v;
	delete[] vt;
	v  = NULL;
	vt = NULL;
	numFaces = NULL;

	m_numVertices = vCount;
	m_numIndices = iCount;
	return true;

} // ! LoadFile



void KPModel::LoadMaterials(FILE *file)
{

	assert(file);

	if (!file || !m_pDevice)
		return;

	char	buffer[128];						// buffer for the material library file
	KPCOLOR	cA, cD, cS, cE;						// Ambient, Diffuse, Specular and Emissing color values
	char	matName[100];						// Material Name
	char	textureFilePath[MAX_PATH];			// Path of the texture file, if there is any
	long int offset = 0;						// Offset of the previously read line

	ZeroMemory(matName, sizeof(matName));

	// Count the number of materials in the file
	//////////////////////////////////////////////////////
	while ( fgets( buffer, sizeof(buffer), file) != NULL )
	{
		if ( IsInString(buffer, "newmtl ") == 0 )
		{
			// Store the material's name
			sscanf_s(buffer, "newmtl %s", matName, sizeof(matName));
			//g_matMap[matName] = m_numMaterials++;
			m_MaterialMap[m_numMaterials++] = matName;
		}
	}
	// Back to the beginning of the file
	//fseek(file, 0, SEEK_SET);
	rewind(file);
	
	// Prepate the array of skin IDs
	m_pSkins = new UINT[m_numMaterials];

	// Load the material data
	/////////////////////////
	while ( fgets( buffer, sizeof(buffer), file) != NULL )
	{
		// If it is a beginning of the first material description
		// Process the properties
		if ( IsInString(buffer, "newmtl ") == 0 )
		{
			ZeroMemory(&cA, sizeof(KPCOLOR));
			ZeroMemory(&cD, sizeof(KPCOLOR));
			ZeroMemory(&cS, sizeof(KPCOLOR));
			ZeroMemory(&cE, sizeof(KPCOLOR));
			ZeroMemory(matName, sizeof(matName));
			ZeroMemory(textureFilePath, sizeof(textureFilePath));

			// Get the material's name
			sscanf_s(buffer, "newmtl %s", matName, sizeof(matName));

			// Start reading the material properties
			while ( fgets( buffer, sizeof(buffer), file) != NULL )
			{

				// if this line is the beginning of a new material description
				if ( IsInString(buffer, "newmtl ") == 0 )
				{
					// Just one line back and break the loop
					fseek(file, offset, SEEK_SET);
					break;
				}
				else
				{
					// Store the last line that was not a new entry
					offset = ftell(file);

					// Look for diffuse texture name
					if ( IsInString(buffer, "map_Kd") != -1 )
						sscanf_s(buffer, "\tmap_Kd %s", textureFilePath, sizeof(textureFilePath));

					// Look for color values
					else if ( IsInString(buffer, "Ka ") != -1 )
						sscanf_s(buffer, "\tKa %f %f %f", &cA.fR, &cA.fG, &cA.fB);
					else if ( IsInString(buffer, "Kd ") != -1 )
						sscanf_s(buffer, "\tKd %f %f %f", &cD.fR, &cD.fG, &cD.fB);
					else if ( IsInString(buffer, "Ks ") != -1 )
						sscanf_s(buffer, "\tKs %f %f %f", &cS.fR, &cS.fG, &cS.fB);
					else if ( IsInString(buffer, "Ke ") != -1 )
						sscanf_s(buffer, "\tKe %f %f %f", &cE.fR, &cE.fG, &cE.fB);

				} // ! if
			} // ! while properties
			
			// Save the material data
			if ( strlen(matName) > 0 && matName != '\0' )
			{

				if( !m_pDevice->GetSkinManager())
					return;

				m_pDevice->GetSkinManager()->AddSkin(&cA, &cD, &cE, &cS, 1.0, &m_pSkins[MapMaterial(matName)]);
				m_pDevice->GetSkinManager()->AddTexture(m_pSkins[MapMaterial(matName)],textureFilePath, false, 0, NULL, 0);

			}
		} // ! if entry

	} // ! while
		
} // ! LoadMaterials

UINT KPModel::MapMaterial(const char *mName)
{
	for (int i=0; i<128; ++i)
	{
		if ( m_MaterialMap[i] == mName )
			return i;
	}

	return 65535;
} // ! MapMaterial

HRESULT KPModel::Render(void)
{
	HRESULT  hr = KP_OK;

	for ( UINT i = 0; i < m_numMaterials; ++i )
	{
		if ( FAILED( m_pDevice->GetVertexManager()->Render(m_pBufferID[i]) ) )
			hr = KP_FAIL;
		else
			hr = KP_OK;
	}

	return hr;

} // ! Render

KPVector KPModel::GetCenter()
{
	return m_vCenter;
}

float KPModel::GetHalfLength()
{
	return m_fHalfLength;
}

UINT KPModel::GetNumVertices(void)
{
	return m_numVertices;
}

UINT KPModel::GetNumIndices(void)
{
	return m_numIndices;
}

UINT KPModel::GetNumMaterials(void)
{
	return m_numMaterials;
}

int IsInString(const char *string, const char *substring)
{
	char a,c;
	int  nStart,j;
	int  nLng_SubStr = strlen(substring),	// Length of substring
		 nLng_Str    = strlen(string);		// Length of string

	// If the substring is too short or longer than the string itself
	// return -1
	if ((nLng_SubStr <= 1) || (nLng_SubStr > nLng_Str))
		return -1;

	// Get the first letter of the substring
	memcpy(&a, &substring[0], sizeof(char));
	nStart = strcspn(string, &a);

	// Start looking for a match beginning with the first character of the substring
	while (nStart < nLng_Str)
	{
		// If the first characters are not equal, continue to the next cycle
		if (string[nStart] != a)
		{
			nStart++;
			continue;
		}

		// If we found a matching character
		// Compare the following characters
		for (j = 1; j < nLng_SubStr; ++j)
		{
			memcpy(&c, &substring[j], sizeof(char));
			if (string[nStart+j] != c)
				break;
		} // for [nLng_SubStr]

		// If we found the whole substring
		// return the position of the first character of the substring
		// in the string
		if (j == nLng_SubStr)
			return nStart;

		// else, check the next character
		else
			nStart ++;
	} // while

	// if we run through the whole string, and could not find a match,
	// return false
	return -1;

} // ! IsInString