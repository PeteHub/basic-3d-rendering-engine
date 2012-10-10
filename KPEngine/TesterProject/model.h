/*
 *****************************************************************
 *
 *	NOTE: This file is not part of the thesis, it's the modified
 *		  source code of the model loader class of
 *		  3D Game Engine Programming - Stefan Zerbs with Oliver
 *		  Duvel.
 *
 *  File: model.h
 *  Description: Model Loader class
 *
 *****************************************************************
*/

#ifndef KPMODEL_H
#define KPMODEL_H

#include <stdio.h>
#include "KP.h"
#include "KPRenderDevice.h"
#include "main.h"


typedef struct TRI_TYPE {
   WORD i0, i1, i2;  // original indices
   WORD n0, n1, n2;  // new indices
   UINT nMat;
} TRI;

typedef int (*CMPFUNC) (const void *arg1, const void *arg2);
int SortTriangles(const TRI *arg1, const TRI *arg2);

class KPModel
{
protected:
	KPRenderDevice *m_pDevice;

	UINT    m_nNumSkins;
	UINT   *m_pSkins;

	UINT    m_nNumVertices;
	VERTEX *m_pVertices;


	UINT    m_nNumIndices;
	WORD   *m_pIndices;

	UINT   *m_pCount;    // indices per material
	UINT   *m_pBufferID; // static buffers

	FILE   *m_pFile;
	FILE   *m_pLog;
	bool    m_bReady;

	void ReadFile(void);

public:
	KPModel(const char *chFile, KPRenderDevice *pDevice, FILE *pLog);
	~KPModel(void);

	HRESULT Render(void);
};


#define SEEK(str, key) while (IsInStr(str,key)==-1) fgets(str,80,m_pFile);
#define NEXT(str) fgets(str, 80, m_pFile);
int IsInStr(const char *string, const char *substring);

#endif
