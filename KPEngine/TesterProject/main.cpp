/*
 *****************************************************************
 *
 *	KPEngine Demo Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: main.cpp
 *  Description: KPEngine Demo implementation. This code is not
 *				 part of the thesis, only serves as a preview
 *				 of the engine's capabilities
 *
 *****************************************************************
*/

#define WIN32_MEAN_AND_LEAN

#include "resource.h"
#include "KPRenderer.h"
#include "KP.h"
#include "main.h"
#include "kpmodel.h"

// Link our static library awesomeness :)
#pragma comment(lib, "KPRenderer.lib")
#pragma comment(lib, "KP3D.lib")

// Windows variables
HWND		g_hWnd			= NULL;						// Handle to our window
HINSTANCE	g_hInst			= NULL;						// Handle to differenciate classes of different DLLs
TCHAR		g_szAppClass[]	= TEXT("KPEngine Ablak");	// Name of the window

// Application variables
BOOL g_bIsActive	= FALSE;	// Is our window active?
bool g_bDone		= false;	// Stay inside the main loop?
UINT g_setShade		= 3;		// Switch shading modes in 3d view
UINT g_nFontID		= 0;		// Id of our font type
UINT g_nRotate		= 0;		// Amount of rotation
FILE *pLog			= NULL;		// Application log file
KPCOLOR g_clrWire;

// Some variables to make the wireframe colors cycle
float g_signR = 1.0f;
float g_signG = 1.0f;
float g_signB = 1.0f;

// KPEngine Objects
LPKPRENDERER		g_pRenderer	= NULL;
LPKPRENDERDEVICE	g_pDevice	= NULL;
KPModel				*g_pModel	= NULL;

// Path name for file
char fileName[MAX_PATH] = "";



// WinMain Entry Function
/////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEX	wndclass;
	HMENU		hMenu;
	HWND		hWnd;
	MSG			msg;

	fopen_s(&pLog, "log_demo.txt", "w");
	if ( !pLog )
		return 0;

	// Initialize our window class
	wndclass.hIconSm			= LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hIcon				= LoadIcon(NULL,IDI_APPLICATION);
	wndclass.cbSize				= sizeof(wndclass);
	wndclass.cbClsExtra			= 0;
	wndclass.cbWndExtra			= 0;
	wndclass.hInstance			= hInstance;
	wndclass.hCursor			= LoadCursor(NULL,IDC_ARROW);
	wndclass.hbrBackground		= (HBRUSH)(COLOR_WINDOW);
	wndclass.lpszMenuName		= NULL;
	wndclass.lpszClassName		= g_szAppClass;
	wndclass.style				= CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
	wndclass.lpfnWndProc		= MsgProc;

	if ( RegisterClassEx( &wndclass ) == 0 )
		return 0;

	// Load the menubar from the resource file
	if ( ( hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1)) ) == NULL )
	{
		fprintf_s(pLog,"error: LoadMenu() failed: %i \n", GetLastError() );
		return 0;
	}



	if ( !( hWnd = CreateWindowEx(NULL, g_szAppClass, "KPEngine Szakdolgozat Bemutato - Kovacs Peter - 2009", WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX | WS_VISIBLE,
								  GetSystemMetrics(SM_CXSCREEN)/2 - 400,  GetSystemMetrics(SM_CYSCREEN)/2 - 300,
								  800, 600, NULL, hMenu, hInstance, NULL) ) )
	{
		fprintf_s(pLog,"error: CreateWindowEx() failed: %i \n", GetLastError() );
		return 0;
	}

	g_hWnd = hWnd;
	g_hInst = hInstance;

	// Initialize SIMD if possible
	IsSSESupported();

	// Open file dialog for the first time
	OpenFileDialog(fileName, g_hWnd, "Wavefront OBJ (*.obj)\0*.obj\0Minden Fájl (*.*)\0*.*\0");

	// Start the Engine !!!
	///////////////////////

	StartRenderingEngine();

	if ( fileName[0] != '\0' )
		g_pModel = new KPModel(fileName, g_pDevice, pLog);

	// Main Loop of Doom! :)
	while ( ! g_bDone )
	{
		while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		////
		//	Render a Frame
		////

		if ( g_bIsActive )
		{
			// Limit fps ~ 60, PC dependant
			Sleep(60);

			if ( g_pDevice->IsRunning() && g_pModel )
			{

				Tick(0);

				// If we are in windowed mode, fire up another 3 child windows
				if ( g_pDevice->IsWindowed() )
				{
					Tick(1);

					Tick(2);

					Tick(3);

				} // ! if windowed mode

			} // ! if we are running

		} // ! if active

	} // ! main loop


	// Clean up the stuff
	if ( pLog )
	{
		fclose(pLog);
		pLog = NULL;
	}

	ProgramCleanup();
	UnregisterClass(g_szAppClass, hInstance);


		//return (int)msg.wParam;
		return 0;
} // ! WinMain

// MsgProc function
///////////////////
//
// Processes windows messages
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_ACTIVATE:
		g_bIsActive = (BOOL)wParam;
		break;

		// Keypress events
	case WM_KEYDOWN:
		if ( wParam == VK_ESCAPE )
		{
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
		else if ( wParam == VK_SPACE )
		{
			g_setShade++;
			g_setShade%=4;
		}
		break;

		// Events from the main window, for example from the menubar
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{

		// Open file dialog
		case IDM_OPEN:
			if ( OpenFileDialog(fileName, g_hWnd, "Wavefront OBJ (*.obj)\0*.obj\0All Files (*.*)\0*.*\0") )
			{
				delete g_pModel;
				g_pModel = new KPModel(fileName, g_pDevice, pLog);
			}
			break;

		// Exit the application
		case IDM_EXIT:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		break;

		// Destroy window
	case WM_DESTROY:
		{
			g_bDone = true;
			PostQuitMessage(0);
			return 1;
		}
		break;
	default:
		break;
	} // ! msg switch

	return DefWindowProc(hWnd, msg, wParam, lParam);

} // ! MsgProc

void StartRenderingEngine()
{
	HRESULT		hr;

	if ( FAILED( hr = ProgramStartup("Direct3D") ) )
	{
		fprintf_s(pLog,"Error starting the application. DirectX is possibly not up-to date! Check http://www.microsoft.com/downloads/details.aspx?FamilyID=2DA43D38-DB71-4C1B-BC6A-9B6652CD92A3 \n");
		g_bDone = true;
	}
	else if ( (int)hr == KP_CANCELLED )
	{
		fprintf_s(pLog,"error: ProgramStartup() cancelled \n");
		g_bDone = true;
	}

	// Otherwise, all is good, we can proceed with initializing
	else
	{
		// Set Global Device Settings
		g_pDevice->SetMode(EMD_PERSPECTIVE, 0);
		g_pDevice->SetClearColor(0.1f, 0.1f, 0.1f);
		g_pDevice->SetAmbientLight(0.8f,0.8f,0.8f);

		// Create the camera vectors (Right, Up, Direction, Position)
		KPVector	vCamRight(1,0,0),
					vCamUp(0,1,0),
					vCamDirection(0,0,1),
					vCamPosition(0,0,0);

		KPVector	vText(5, 8, 32);

		g_clrWire.fR = 0.4f;
		g_clrWire.fG = 0.4f;
		g_clrWire.fB = 0.4f;

		// Set up the camera
		g_pDevice->SetView3D(vCamRight, vCamUp, vCamDirection, vCamPosition);

		// Enable model textures
		g_pDevice->UseTextures(true);

	}
}

// ProgramStartup Function
//////////////////////////
//
// Creates the render device object
// chAPI is a string representing the rendering API, "Direct3D" in our case
HRESULT ProgramStartup(char * chAPI)
{
	HWND hWnd3D[4];
	RECT rcWnd;
	int x	= 0;
	int y	= 0;
	int x2	= 0;
	int y2	= 0;

	// We don't have OpenGL implemented
	if ( strcmp(chAPI, "OpenGL") == 0 )
		return S_OK;

	// Create the renderer object
	g_pRenderer = new KPRenderer(g_hInst);

	// Create the renderer device
	if ( FAILED( g_pRenderer->CreateDevice(chAPI) ) )
		return E_FAIL;

	// Save pointer to the render device
	g_pDevice = g_pRenderer->GetDevice();

	if ( g_pDevice == NULL )
		return E_FAIL;

	// Query client area size
	GetClientRect(g_hWnd, &rcWnd);

	// Loop through our child render windows (4 in our case)
	// set their positions and create them
	for ( int i=0; i < 4; ++i )
	{
		if ( ( i == 0 ) || ( i == 2 ) )
			x = 4;
		else
			x = rcWnd.right /2 + 1;

		if ( ( i == 0 ) || ( i == 1 ) )
			y = 4;
		else
			y = rcWnd.bottom/2 + 1;

		hWnd3D[i] = CreateWindowEx(WS_EX_CLIENTEDGE,
									TEXT("static"), NULL,
									WS_CHILD | SS_BLACKRECT | WS_VISIBLE,
									x, y,
									rcWnd.right/2-5, rcWnd.bottom/2-5,
									g_hWnd, NULL, g_hInst, NULL);
	} // ! child window for

	// Initialize the render device
	// Pop up the settings dialog box

	HRESULT hr = g_pDevice->Init(g_hWnd,	// Main window handle
							hWnd3D, // child window handle array
							4,		// number of children windows
							16,		// 16bit zbuffer
							8,		// 0 bit stencil buffer
							false); // logfile
	if ( FAILED(hr) )
	{
      fprintf(pLog, "Failed to initialize render device.\n");
      return E_FAIL;
	}

	// Create Font
	if ( FAILED( g_pDevice->CreateMyFont("Arial", FW_NORMAL, false, false, false, 48, &g_nFontID ) ) )
	{
		fprintf(pLog, "Error creating default font!\n");
		return KP_FAIL;
	}

	g_pDevice->UseWindow(0);
	g_pDevice->InitStage(0.8f, NULL, 0);

	return KP_OK;

} // ! ProgramStartup


// ProgramCleanup function
//////////////////////////
//
// Free all allocated resources
HRESULT ProgramCleanup(void)
{

	if ( g_pModel )
	{
		delete g_pModel;
		g_pModel = NULL;
	}

	if ( g_pDevice->GetNumRenderWindows() > 0 )
	{
		for(int i=0;i<g_pDevice->GetNumRenderWindows(); ++i)
		{
			DestroyWindow(g_pDevice->GetRenderWindowHandle(i));
		}
	}

	if ( g_pRenderer )
	{
		delete g_pRenderer;
		g_pRenderer = NULL;
	}

	return S_OK;
} // ! ProgramCleanup


// Program Tick ////
////////////////////
//
// Renders one frame.
HRESULT Tick(UINT nWID)
{
	KPMatrix mWorld;
	KPMatrix mWorld2;
	KPVector vLightPosition;
	mWorld.Identity();
	mWorld2.Identity();
	char strShadeMode[32] = "";


	g_pDevice->SetMode(EMD_PERSPECTIVE, 0);

	// Set up the wireframe color
		if ( g_clrWire.fR > 1.0f || g_clrWire.fR < 0.3f )
			g_signR *= -1.0f;
		if ( g_clrWire.fG > 1.0f || g_clrWire.fG < 0.3f )
			g_signG *= -1.0f;
		if ( g_clrWire.fB > 1.0f || g_clrWire.fB < 0.3f )
			g_signB *= -1.0f;

		
		g_clrWire.fR += g_signR*0.002f;
		g_clrWire.fG += g_signG*0.004f;
		g_clrWire.fB += g_signB*0.008f;

	// Default solid shading mode
	g_pDevice->SetShadeMode(RS_SHADE_SOLID, 1.0f, &g_clrWire);

	// Activate the needed child window
	switch ( nWID )
	{
	case 1:
		g_pDevice->UseWindow(1);
		break;
	case 2:
		g_pDevice->UseWindow(2);
		break;
	case 3:
		g_pDevice->UseWindow(3);
		break;
	case 0:
	default:
		
		switch ( g_setShade )
		{
		case 0:
			g_pDevice->SetShadeMode(RS_SHADE_POINTS, 0.05f, &g_clrWire);
			strcpy_s(strShadeMode,sizeof(char)*32,"Pontháló mód (Vertexek)");
			break;
		case 1:
			g_pDevice->SetShadeMode(RS_SHADE_LINES, 0, &g_clrWire);
			strcpy_s(strShadeMode,sizeof(char)*32,"Vonalháló mód (Élek)");
			break;
		case 2:
			g_pDevice->SetShadeMode(RS_SHADE_TRIWIRE, 0, &g_clrWire);
			g_pDevice->SetBackfaceCulling(RS_CULL_NONE);
			strcpy_s(strShadeMode,sizeof(char)*32,"Háromszögháló mód (Elemek)");
			break;
		case 3:
		default:
			strcpy_s(strShadeMode,sizeof(char)*32,"Kitöltött mód");
			g_pDevice->SetShadeMode(RS_SHADE_SOLID, 0, &g_clrWire);
		}

		g_pDevice->UseWindow(0);

	}

	// Start the rendering sequence, clear all three buffers
	g_pDevice->BeginRendering(true, true, true);

	////
	//	RENDERING CALLS
	////

	switch ( nWID )
	{
	case 1:
		g_pDevice->SetShadeMode(RS_SHADE_POINTS, 0.05f, &g_clrWire);
		g_pDevice->DrawTxt(g_nFontID, 4, 4, 255, 150, 150, 150, "Vertexek");

		//mWorld.RotateX( DToRad(-90) );
		mWorld.RotateY( DToRad(-25) );
		mWorld2.RotateX( DToRad(-15) );
		mWorld = mWorld* mWorld2;

		mWorld.Translate(0.0f, 0.0f, 8.0f);
		g_pDevice->SetWorldTransform(&mWorld);
		RenderModel();
		break;
	case 2:
		g_pDevice->SetShadeMode(RS_SHADE_LINES, 0.05f, &g_clrWire);
		g_pDevice->DrawTxt(g_nFontID, 4, 4, 255, 150, 150, 150, "Élek");

		mWorld.RotateY( DToRad(-25) );
		mWorld2.RotateX( DToRad(-15) );
		mWorld = mWorld* mWorld2;

		mWorld.Translate(0.0f, 0.0f, 8.0f);
		g_pDevice->SetWorldTransform(&mWorld);
		RenderModel();

		break;
	case 3:
		g_pDevice->SetShadeMode(RS_SHADE_TRIWIRE, 0.05f, &g_clrWire);
		g_pDevice->DrawTxt(g_nFontID, 4, 4, 255, 150, 150, 150, "Elemek");

		//mWorld.RotateY( DToRad(-90) );
		mWorld.RotateY( DToRad(-25) );
		mWorld2.RotateX( DToRad(-15) );
		mWorld = mWorld* mWorld2;

		mWorld.Translate(0.0f, 0.0f, 8.0f);
		g_pDevice->SetWorldTransform(&mWorld);
		RenderModel();
		break;
	case 0:
	default:
		g_pDevice->DrawTxt(g_nFontID, 4, 4, 255, 150, 150, 150, "3D Nézet - %s\nSPACE: kitöltési mód váltása\nESC: Kilépés\n\nVertexek: %d\nIndexek: %d\nHáromszögek: %d\nAnyagok: %d",
						strShadeMode, g_pModel->GetNumVertices(), g_pModel->GetNumIndices(), g_pModel->GetNumIndices()/3, g_pModel->GetNumMaterials());

		if ( (g_nRotate%360) == 0 )
			g_nRotate = 1;
		else
			++g_nRotate;

		mWorld.RotateY( DToRad(g_nRotate) );
		mWorld2.RotateX( DToRad(-15) );
		mWorld = mWorld* mWorld2;

		mWorld.Translate(0.0f, 0.0f, 8.0f);
		g_pDevice->SetWorldTransform(&mWorld);
		RenderModel();

	}

	// End the rendering sequence, flip the backbuffer into the frontbuffer
	// and present the scene on the screen
	g_pDevice->EndRendering();

	return KP_OK;

} // ! Tick

bool OpenFileDialog(char strfileName[], HWND hOwner, char* filter)
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn,sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner	= hOwner;
	ofn.lpstrFilter	= filter;
	ofn.lpstrFile	= strfileName;
	ofn.nMaxFile	= MAX_PATH;
	ofn.Flags		= OFN_EXPLORER | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt	= "obj";
	ofn.lpstrTitle = "OBJ Modell fájl megnyitása...";

	if ( GetOpenFileName(&ofn) )
		return true;
	else
	{
		switch( CommDlgExtendedError() )
		{
		case CDERR_FINDRESFAILURE:
		case CDERR_LOADRESFAILURE:
		case CDERR_LOADSTRFAILURE:
		case CDERR_LOCKRESFAILURE:
		case FNERR_INVALIDFILENAME:
			MessageBox(NULL, "A kiválasztott fájlt az alkalmazás nem tudja megnyitni!", "KPEngine - hiba", MB_OK | MB_ICONERROR);
			break;
		case CDERR_INITIALIZATION:
		case CDERR_MEMALLOCFAILURE:
		case CDERR_MEMLOCKFAILURE:
		case FNERR_BUFFERTOOSMALL:
			MessageBox(NULL, "Nincs elég memória a muvelet végrehajtásához!", "KPEngine - hiba", MB_OK | MB_ICONERROR);
			break;
		default:
			break;
		}
		return false;
	}
}

float DToRad(int degree)
{
	// 1 degree = PI/180 radian
	return (float)( 0.01745329251994329576923690768489 * degree );
}

void RenderModel(void)
{
	if ( g_pModel )
		g_pModel->Render();
}


