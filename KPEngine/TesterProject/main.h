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

LRESULT WINAPI MsgProc(HWND, UINT, WPARAM, LPARAM);
HRESULT ProgramStartup(char *chAPI);
HRESULT ProgramCleanup(void);
HRESULT Tick(UINT nWID);
float	DToRad(int degree);

#endif