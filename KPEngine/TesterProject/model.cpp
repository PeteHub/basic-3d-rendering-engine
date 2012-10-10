/*
 *****************************************************************
 *
 *	NOTE: This file is not part of the thesis, it's the modified
 *		  source code of the model loader class of
 *		  3D Game Engine Programming - Stefan Zerbs with Oliver
 *		  Duvel.
 *
 *  File: model.cpp
 *  Description: Model Loader Implementation
 *
 *****************************************************************
*/
#include "KPRenderDevice.h"
#include "model.h"


// KPModel Constructor ////
///////////////////////////
KPModel::KPModel(const char *chFile, KPRenderDevice *pDevice, FILE *pLog)
{
	m_pDevice		= pDevice;

	m_bReady		= false;
	m_pLog			= pLog;
	m_pVertices		= NULL;
	m_pIndices		= NULL;
	m_pSkins		= NULL;
	m_pCount		= NULL;
	m_pBufferID		= NULL; 

	m_nNumSkins		= 0;
	m_nNumVertices	= 0;
	m_nNumIndices	= 0;

	// Get a handle to the model file
	fopen_s(&m_pFile, chFile, "r");

	if ( m_pFile )
	{
		ReadFile();			// Read the content of the model file
		fclose(m_pFile);	// Release the file handle

		// Loaded the model, we are ready.
		fprintf(m_pLog, "Modell initialized\n");
		m_bReady = true;
	}
	else
		// Could not open the file
		fprintf(m_pLog, "Unable to open model file\n");	

} // ! KPModel Constructor


// KPModel Destructor ////
//////////////////////////
KPModel::~KPModel()
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

	if (m_pCount)
	{
		delete [] m_pCount;
		m_pCount = NULL;
	}

	if (m_pBufferID)
	{
		delete [] m_pBufferID;
		m_pBufferID = NULL;
	}

	m_bReady = false;

} // ! ~KPModel Destructor
 

// Render Model ////
////////////////////
HRESULT KPModel::Render(void) {
	HRESULT  hr = KP_OK;

	for ( UINT i = 0; i < m_nNumSkins; ++i )
	{
		if ( FAILED( m_pDevice->GetVertexManager()->Render(m_pBufferID[i]) ) )
			hr = KP_FAIL;
		else
			hr = KP_OK;
	}

	return hr;

} // ! Render()

// ReadFile ////
////////////////
void KPModel::ReadFile(void)
{
	KPCOLOR	cDiffuse,
			cAmbient,
			cSpecular,
			cEmissive;

	UINT     nNumFaces=0, nNumTris=0;
	float    fPower=0;
	char     Line[80];
	char     Texture[80];
	TRI     *pTris=NULL;


	////
	// READ SKINS
	////

	SEEK(Line, "BEGIN_SKINS");				// Find this line
	NEXT(Line);								// Skip opening bracket

	sscanf_s(Line, "%d;", &m_nNumSkins);	// Number of skins

	// Read each skin from the model file
	m_pSkins = new UINT[m_nNumSkins];
	for ( UINT i = 0; i < m_nNumSkins; ++i )
	{
		NEXT(Line);
		NEXT(Line);

		sscanf_s(Line, "%f, %f, %f, %f;",	&cAmbient.fR, &cAmbient.fG, &cAmbient.fB, &cAmbient.fA);
		NEXT(Line);

		sscanf_s(Line, "%f,%f,%f,%f;",		&cDiffuse.fR, &cDiffuse.fG, &cDiffuse.fB, &cDiffuse.fA);
		NEXT(Line);

		sscanf_s(Line, "%f,%f,%f,%f;",		&cEmissive.fR, &cEmissive.fG, &cEmissive.fB, &cEmissive.fA);
		NEXT(Line);

		sscanf_s(Line, "%f,%f,%f,%f,%f;",	&cSpecular.fR, &cSpecular.fG, &cSpecular.fB, &cSpecular.fA, &fPower);
		NEXT(Line);

		sscanf_s(Line, "%s", &Texture, sizeof(Texture));

		// add the skin to the skin manager
		m_pDevice->GetSkinManager()->AddSkin(&cAmbient, &cDiffuse, &cEmissive, &cSpecular, fPower, &m_pSkins[i]);
		m_pDevice->GetSkinManager()->AddTexture(m_pSkins[i], Texture, false, 0, NULL, 0);

		NEXT(Line); // skip closing bracket
	}


	////
	// READ VERTICES
	////

	rewind(m_pFile);							// Start from the beginning of the file

	SEEK(Line, "BEGIN_VERTICES");				// Find this line
	NEXT(Line);									// Skip the opening bracket

	sscanf_s(Line, "%d;", &m_nNumVertices);		// Read the number of vertices
	m_pVertices   = new VERTEX[m_nNumVertices];	// Create an array to store the vertices

	// Read the vertices
	for ( UINT i = 0; i < m_nNumVertices; ++i )
	{
		NEXT(Line);
		sscanf_s(Line, "%f,%f,%f,%f,%f;", &m_pVertices[i].x, 
			&m_pVertices[i].y, &m_pVertices[i].z,
			&m_pVertices[i].tu, &m_pVertices[i].tv);
	}


	////
	// Read the number of polygons(faces)
	// This way we can cound the triangles we will need
	////

	rewind(m_pFile);							// Start from the beginning of the file

	SEEK(Line, "BEGIN_FACES");					// Find this line
	NEXT(Line);									// Skip opening bracket

	sscanf_s(Line, "%d;", &nNumFaces);			// Read the number of faces
	pTris = new TRI[nNumFaces];					// Create an array to store the triangles

	// Read the triangles
	for ( UINT i = 0; i < nNumFaces; ++i )
	{
		NEXT(Line);
		sscanf_s(Line, "%d,%d,%d;%d", &pTris[nNumTris].i0,
			&pTris[nNumTris].i1, &pTris[nNumTris].i2, 
			&pTris[nNumTris].nMat);
		nNumTris++;
	}

	// Sort our triangles based on material id
	qsort((void*)pTris, nNumTris, sizeof(TRI), (CMPFUNC)SortTriangles);


	////
	// Count how many indices use the same material
	////

	UINT nOldMat	= pTris[0].nMat;

	// Create two UINT arrays and zero them
	m_pCount		= new UINT[m_nNumSkins];
	m_pBufferID		= new UINT[m_nNumSkins];
	memset(m_pCount, 0, sizeof(UINT)*m_nNumSkins);
	memset(m_pBufferID,   0, sizeof(UINT)*m_nNumSkins);

	// Create a new array for storing the indices
	m_pIndices		= new WORD[nNumTris*3];
	m_nNumIndices	= nNumFaces*3;

	for ( UINT i = 0; i < nNumTris; ++i )
	{
		// copy indices in indexlist
		m_pIndices[i*3]     = pTris[i].i0;
		m_pIndices[i*3 + 1] = pTris[i].i1;
		m_pIndices[i*3 + 2] = pTris[i].i2;

		// count indices per material
		if (pTris[i].nMat != nOldMat)
			nOldMat = pTris[i].nMat;
		else
			m_pCount[pTris[i].nMat] += 3;
	}


	// Add our model data to the vertex manager, into the static buffer of the
	// graphics device, for fast rendering
	WORD *pIndices=m_pIndices;
	for ( UINT i = 0; i < m_nNumSkins; pIndices += m_pCount[i], ++i )
	{

		if ( FAILED( m_pDevice->GetVertexManager()->CreateStaticBuffer(VID_UU, m_pSkins[i], m_nNumVertices,	m_pCount[i],
																	   m_pVertices, pIndices, &m_pBufferID[i]) ) )
			fprintf(m_pLog, "Failed to create static buffer!\n");
	}

	fprintf(m_pLog, "Model loaded\n");

	delete [] pTris;

} // ReadFile



// Sorting algorithm for sorting our triangles by material
/*
	Returns:
		1 if arg1 > arg2
	   -1 if arg 1 < arg2
	    0 if they are equal
*/
int SortTriangles(const TRI *arg1, const TRI *arg2)
{

	if (arg1->nMat > arg2->nMat)
		return 1;
	else if (arg1->nMat < arg2->nMat)
		return -1;
	else
		return 0;

} // ! SortTriangles



// Checks whether a substring is part of a string
// returns the position of the first character of the substring if true, -1 if false
int IsInStr(const char *string, const char *substring)
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

	// Suche die folgenden Buchstaben des Substrings
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

} // ! IsInStr