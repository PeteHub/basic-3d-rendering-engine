/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: main.h
 *  Description: KPEngine Demo declarations
 *
 *****************************************************************
*/

#ifndef MAIN_H
#define MAIN_H

#include <assert.h>

LRESULT WINAPI MsgProc(HWND, UINT, WPARAM, LPARAM);
void	StartRenderingEngine(void);
void	RenderModel(void);
HRESULT ProgramStartup(char *chAPI);
HRESULT ProgramCleanup(void);
HRESULT Tick(UINT nWID);
bool	OpenFileDialog(char strfileName[], HWND hOwner, char* filter);
float	DToRad(int degree);

#endif