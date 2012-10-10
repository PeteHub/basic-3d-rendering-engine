/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - July 2009
 *
 *  File: KPRenderDevice.h
 *  Description: Rendering related API independant interface definitions
 *				 - Skin Manager Interface
 *				 - Vertex Cache Manager Interface
 *				 - Render Device Interface
 *
 *****************************************************************
*/

#ifndef KPRENDERDEVICE_H
#define KPRENDERDEVICE_H

#include <windows.h>
#include <stdio.h>

#include "../KP3D/KP3D.h"	//!< Needed for the math library
#include "../KPD3d/KP.h"	//!< Needed for the material structure

/*!
	The number of child windows our engine will be able to render into.
	Very useful for editors with multiple views, like a 3d Model Editor, or World Builder.
*/
#define MAX_3DHWND 4

//! Skin Manager Oszt�ly
/*! Anyagok l�trehoz�s�t �s text�r�k bet�lt�s�t vez�rli �s megakad�lyozza m�r l�tez? adatok �jb�li, sz�ks�gtelen t�rol�s�t. */
class KPSkinManager
{
protected:
	KPMATERIAL	*m_pMaterials;		//!< T�rolt anyagok list�ja
	KPTEXTURE	*m_pTextures;		//!< T�rolt text�r�k list�ja
	KPSKIN		*m_pSkins;			//!< T�rolt skinek list�ja

	UINT		m_numSkins;			//!< T�rolt skinek sz�ma
	UINT		m_numMaterials;		//!< T�rolt anyagok sz�ma
	UINT		m_numTextures;		//!< T�rolt text�r�k sz�ma

public:
	KPSkinManager(void) { };		//!< SkinManager interf�sz konstruktora.
	virtual ~KPSkinManager(void) { }; //!< SkinManager interf�sz destruktora

	
	//! �j skint ad hozz� a SkinManagerhez.
	/*!
		\param [in] pAmbient Mutat� egy KPCOLOR objektumra amely megadja a k�rnyezeti f�ny sz�n�t.
		\param [in] pDiffuse Mutat� egy KPCOLOR objektumra amely megadja a sz�rt f�ny sz�n�t.
		\param [in] pEmissive Mutat� egy KPCOLOR objektumra amely megadja a kibocs�jtott f�ny sz�n�t.
		\param [in] pSpecular Mutat� egy KPCOLOR objektumra amely megadja a visszavert f�ny sz�n�t.
		\param [in] fPower float �rt�k amely megadja a sz�rt f�ny �less�g�t.
		\param [out] nSkinID Mutat� egy UINT t�pus� v�ltoz�ra amely befogadja a l�trehozott skin azonos�t�j�t.
		\return KP_OK sikeres v�grehajt�s eset�n.
		\return KP_OUTOFMEMORY mem�ria t�lcsordul�s eset�n.
	*/
	virtual HRESULT			AddSkin(const KPCOLOR *pAmbient, const KPCOLOR *pDiffuse, const KPCOLOR *pEmissive,
									const KPCOLOR *pSpecular, float fPower, UINT *nSkinID)=0;

	//! �j text�r�t csatol egy a SkinManagerben t�rolt skinhez
	/*!
		\param [in] nSkinID UINT t�pus� �rt�k amely megadja a skin azonos�t�j�t amelyhez a text�r�t k�v�njuk csatolni.
		\param [in] chName Mutat� egy sztring t�pus� v�ltoz�ra amely meghat�rozza a text�ra el�r�si �tvonal�t.
		\param [in] bAlpha Boolean tipus� �rt�k amely meghat�rozza, hogy haszn�lunk e Alpha Channelt.
		\param [in] fAlpha Float t�pus� �rt�k amely megadja az alpha channel �rt�k�t, ha haszn�ljuk.
		\param [in] pColorKeys Mutat� egy KPCOLOR t�pus� objektumra amely tartalmazza azon sz�nek list�j�t melyekre alkalmazni k�v�njuk az alpha channelt.
		\param [in] numColorKeys DWORD t�pus� v�ltoz� amely megadja pColorKeys objektum elemeinek sz�m�t
		\return KP_OK sikeres v�grehajt�s eset�n.
		\return KP_OUTOFMEMORY mem�ria t�lcsordul�s eset�n.
	*/
	virtual HRESULT			AddTexture(UINT nSkinID, const char *chName, bool bAlpha, float fAlpha,
									   KPCOLOR *pColorKeys, DWORD numColorKeys)=0;

	//! Visszaadja a k�v�nt SkinMangagerben t�rolt skin objektumot.
	/*!
		\param [in] nSkinID	UINT t�pus� �rt�k amely megadja a viszzaadni k�v�nt skin azonos�t�j�t.
		\return	Az adott skin azonos�t�hoz tartoz� KPSKIN t�pus� vagy �res skin objektum.
		\return
		\return
	*/
	virtual KPSKIN			GetSkin(UINT nSkinID)=0;

}; // ! KPSkinManager


//! Vertex Cache Manager Oszt�ly
/*!	Ez az oszt�ly felel?s a grafikus ezk�z mem�ri�j�ban tal�lhat� statikus vertex �s index bufferek �s
	a RAM-ban t�rolt dinamikus vertex bufferek fel�gyelet��rt �s ezek optim�lis renderel�s��rt.
*/
class KPVertexCacheManager
{
	public:
		KPVertexCacheManager(void) { };	//!< VertexCacheManager interf�sz konstruktora.
		virtual ~KPVertexCacheManager(void) { };	//!< VertexCacheManager interf�sz konstruktora.

		//! L�trehoz egy statikus buffert a grafikus ezk�z mem�ri�j�ban. Ezt a buffert a k�s?bbiekben nem lehet megv�ltoztatni.
		/*!
			\param [in] VertexID KPVERTEXID t�pus� objektum amely meghat�rozza a vertex t�pus�t.
			\param [in] nSkinID UINT t�pus� �rt�k amely meghat�rozza a vertexek �ltal haszn�lt skint.
			\param [in] nVertices UINT t�pus� �rt�k amely megadja a vertexek sz�m�t.
			\param [in] nIndices UINT t�pus� �rt�k amely megadja a vertex indexek sz�m�t.
			\param [in] pVertices Mutat� egy bufferre amely tartalmazza a vertexeket.
			\param [in] pIndices Mutat� egy WORD t�pus� bufferre amely tartalmazza a vertex indexeket
			\param [out] pSBufferID Mutat� egy UINT t�pus� v�ltoz�ra amely befogadja a l�trehozott statikus buffer azonos�t�j�t.
			\return KP_OK sikeres v�grehajt�s eset�n.
			\return KP_OUTOFMEMORY mem�ria t�lcsordul�s eset�n.
			\return KP_INVALIDID �rv�nytelen vertex t�pus eset�n.
			\return KP_CREATEBUFFER a statikus �s index bufferek l�trehoz�sa k�zben felmer�l? hib�k eset�n.
			\return KP_BUFFERLOCK sikertelen buffer lock eset�n.
		*/
		virtual HRESULT	CreateStaticBuffer(KPVERTEXID VertexID, UINT nSkinID, UINT nVertices, UINT nIndices,
										   const void *pVertices, const WORD *pIndices, UINT *pSBufferID) = 0;

		//! A gyors�t�t�rba helyezi a renderelni k�v�nt vertex �s index list�k azonos�t�it. Elegend? mennyis�g? adat eset�n a k�perny?re renderel.
		/*!
			A gyors�t�t�rba helyezett adatok csak akkor ker�lnek renderel�sre ha elegend? adatmennyis�g gy�lemlett fel.
			Ahhoz hogy azonnali renderel�st k�nyszer�ts�nk ki a VertexCacheManagert?l, h�vjuk meg a ForcedFlush vagy ForceFlushAll met�dusokat.

			Parameters:
				\param [in] VertexID KPVERTEXID t�pus� objektum amely megadja a vertexek t�pus�t
				\param [in] nSkinID	 UINT t�pus� �rt�k amely megadja a vertexek �ltal haszn�lt skin azonos�t�j�t
				\param [in] nVertices UINT t�pus� �rt�k amely megadja a vertexek sz�m�t
				\param [in] nIndices Uint t�pus� �rt�k amely megadja a vertex indexek sz�m�t
				\param [in] pVertices Mutat� egy a vertexeket tartalmaz� t�mbre.
				\param [in] pIndices Mutat� egy WORD t�pus� a vertex indexeket tartalmaz� t�mbre.
				\return KP_OK sikeres v�grehajt�s eset�n.

		*/
		virtual HRESULT	Render(KPVERTEXID VertexID, UINT nSkinID, UINT nVertices, UINT nIndices,
							   const void *pVertices, const WORD *pIndices) = 0;

		//! A statikus buffer tartalm�t a k�perny?re rendereli.
		/*!
			\param [in] nSBufferID UINT t�pus� v�ltoz� amely megadja a renderelni k�v�nt statikus buffer azonos�t�j�t
			\return KP_OK sikeres v�grehajt�s eset�n.
			\return KP_INVALIDPARAM �rv�nytelen statikus buffer azonos�t� eset�n.
			\return KP_FAIL renderel�s sor�n bek�vetkez? hib�k eset�n.
		*/
		virtual HRESULT Render(UINT nSBufferID) = 0;

		//! A gyors�t�t�rban tal�lhat� �sszes buffer tartalm�t a k�perny?re rendereli.
		/*!
			\return KP_OK sikeres v�grehajt�s eset�n.
			\return KP_FAIL hiba eset�n.
		*/
		virtual HRESULT ForcedFlushAll(void) = 0;

		//! A gyors�t�t�rban tal�lhat� adott t�pus� vertexeket t�rol� �sszes dinamikus buffer tartalm�t a k�perny?re rendereli.
		/*!
			\param [in] VertexID KPVERTEXID t�pus� �rt�k amely meghat�rozza a vertexek t�pus�t.

			\return KP_OK sikeres v�grehajt�s eset�n.
			\return KP_INVALIDID �rv�nytelen vertex t�pus eset�n.
			\return KP_FAIL hiba eset�n.
		*/
		virtual HRESULT ForcedFlush(KPVERTEXID VertexID) = 0;

		// Resets all the active skin, texture and static buffer flags,
		// effectively forcing the cache manager into recalculating everything.
		//! Invalidates all of the active Skin, Static and Dinamic cache flags

		//! Lenull�zza az aktu�lis skin, statikus �s dinamikus bufferek azonos�t�it.
		virtual void    InvalidateStates(void)=0;

}; // ! Vertex Cache Manager


//! Render Device Oszt�ly
/*!
 This is the render device interface of the engine. Makes our engine Graphic API independant.
 The public methods listed here are the only way for the user to 
 interact the underlying implementation of the graphics API itself.
*/
class KPRenderDevice
{
	protected:
		HWND					m_hWndMain;				//!< Az alkalmaz�s f? ablak�nak azonos�t�ja
		HWND					m_hWnd[MAX_3DHWND];		//!< Renderel�sre alkalmas ablakok azonos�t�inak list�ja
		UINT					m_nNumhWnd;				//!< Renderel�sre alkalmas ablakok aktu�lis sz�ma
		UINT					m_nActivehWnd;			//!< A jelenleg akt�v ablak sz�ma
		HINSTANCE				m_hDLL;					//!< A bet�lt�tt DLL azonos�t�ja
		DWORD					m_dwWidth;				//!< K�perny? sz�less�ge
		DWORD					m_dwHeight;				//!< K�perny? magass�ga
		bool					m_bWindowed;			//!< Teljes k�perny? vagy ablak �zemm�d
		char					m_chAdapter[256];		//!< A grafikus adapter neve
		FILE					*m_pLog;				//!< Log f�jl azonos�t�ja
		bool					m_bRunning;				//!< Fut jelenleg a motor?!

		KPSkinManager			*m_pSkinManager;	//!< Mutat� a SkinManagerre
		KPVertexCacheManager	*m_pVertexMan;		//!< Mutat� a VertexCacheManagerre

		// VIEW / PROJECTION ATTRIBUTES
		///////////////////////////////////

		float					m_fNear;				//!< A l�t�t�r k�zeli s�kj�nak t�vols�ga
		float					m_fFar;					//!< A l�t�t�r t�voli s�kj�nak t�vols�ga
		int						m_nStage;				//!< Az aktu�lis render szint sz�ma
		KPENGINEMODE			m_Mode;					//!< Az aktu�lis renderel�si �zemm�d
		KPVIEWPORT				m_ViewPort[4];			//!< L�t�terek list�ja

		// RENDER STATE ATTRIBUTES
		//////////////////////////////

		KPCOLOR					m_clrWireframe;			//!< Dr�th�l� sz�ne
		KPRENDERSTATE			m_ShadeMode;			//!< Vertex shading t�pusa


	public:
		KPRenderDevice(void) {};	//!< KPRenderDevice interf�sz konstruktora
		virtual ~KPRenderDevice(void) {}; //!< KPRenderDevice interf�sz destruktora

		// INITIALIZE / RELEASE
		//////////////////////////

		//! L�trehozza �s alap�llapotba �ll�tja a grafikus ezk�zt.
		/*!
			\param [in] hWnd HWND t�pus� �rt�k amely megadja az alkalmaz�s f?ablak�nak azonos�t�j�t.
			\param [in] hWnd3D Mutat� egy HWND tipus� t�mbre mely megadja a renderel�sre alkalmas ablakok azonos�t�it.
			\param [in] nNumhWnd Int t�pus� �rt�k amely megadja a hWnd3D t�mbben tal�lhat� renderel�sre alkamas ablakok sz�m�t.
			\param [in] nMinDepth Int t�pus� �rt�k amely megadja a Depth Buffer minim�lis m�lys�g�t.
			\param [in] nMinStencil Int t�pus� �rt�k amely megadja a Stencil Buffer minim�lis m�lys�g�t.
			\param [in] bSaveLog Boolean t�pus� �rt�k amely meghat�rozza, hogy haszn�ljunk e Log f�jlt vagy sem.
			\return KP_OK sikeres v�grehajt�s eset�n.
			\return KP_CANCELLED a be�llit�sok ablak megszak�t�sa eset�n.
			\return KP_CREATEAPI a grafikus API inicializ�l�sa sor�n fell�p? hiba eset�n
			\return KP_CREATEDEVICE a grafikus ezk�z inicializ�l�sa sor�n fell�p? hiba eset�n
			\return KP_FAIL b�rmely m�s hiba eset�n.
		*/
		virtual HRESULT			Init(HWND hWnd, const HWND *hWnd3D, int nNumhWnd, int nMinDepth, int nMinStencil, bool bSaveLog) = 0;

		//! Felszabad�tja a grafikus ezk�z �ltal lefoglalt er?forr�sokat
		virtual void			Release(void) = 0;

		//! Ellen?rzi, hogy fut e a KPEngine motor.
		/*!
			\return true ha a KPEngine motor jelenleg fut.
			\return false ha a KPEngine motor nem fut.
		*/
		virtual bool			IsRunning(void) = 0;

		//! Ellen?rzi, hogy ablak m�dban fut e az alkalmaz�s
		/*!
			\return true ha a KPEngine motor jelenleg ablakban fut.
			\return false ha a KPEngine motor teljes k�perny?n fut
		*/
		virtual bool			IsWindowed(void) = 0;

		//! Visszaadja a renderelesre alkalmas ablakok szamat
		/*!
			\return Int tipusu ertek amely megadja a renderelesre alkalmas ablakok szamat
		*/
		virtual int				GetNumRenderWindows(void) = 0;

		//! Visszaadja egy, a renderelesre alkalmas ablak azonositojat
		/*!
			\return HWND tipusu ertek amely megadja a renderelesre alkalmas ablak azonositojat
		*/
		virtual HWND			GetRenderWindowHandle(int) = 0;

		// MANAGERS
		////////////////
		
		//! K�zvetlen hozz�f�r�st tesz lehet?v� a SkinManagerhez
		/*!
			\return Mutat� egy KPSkinManager t�pus� objektumra
		*/
		virtual					KPSkinManager* GetSkinManager(void) = 0;

		//! K�zvetlen hozz�f�r�st tesz lehet?v� a VertexCacheManagerhez
		/*!
			\return Mutat� egy KPVertexCacheManager t�pus� objektumra
		*/
		virtual					KPVertexCacheManager* GetVertexManager(void)=0;

		// VIEW / PROJECTION 
		////////////////////////

		//! Visszaadja a l�t�t�r s�kjait
		/*!
			\param [out] pFrustum Mutat� egy KPPlane t�pus� t�mbre amely befogadja a l�t�t�r hat s�kj�t.
			\return KP_OK sikeres v�grehajt�s eset�n.
		*/
		virtual HRESULT			GetFrustum(KPPlane* pFrustum) = 0;

		//! Be�ll�tja a kamera m�trixot a kamera helyzet�t?l �s orient�ci�j�t�l f�gg?en.
		/*!
			\param [in] vcX KPVector objektum amely megadja a kamera x tengely menti egys�gvektor�t.
			\param [in] vcY KPVector objektum amely megadja a kamera y tengely menti egys�gvektor�t.
			\param [in] vcZ KPVector objektum amely megadja a kamera z tengely menti egys�gvektor�t.
			\param [in] vcPos KPVector objektum amely megadja a kamera helyzet�t.
			\return KP_OK sikeres v�grehajt�s eset�n.
			\return KP_FAIL hiba eset�n.

		*/
		virtual HRESULT			SetView3D(const KPVector &vcX, const KPVector &vcY, const KPVector &vcZ, const KPVector &vcPos) = 0;

		//! Be�ll�tja a kamera m�trixot a kamera �s egy pont helyzet�t?l f�gg?en amelyre a kamera n�z.
		/*!
			\param [in] vcFrom KPVector objektum amely megadja a kamera helyzet�t.
			\param [in] vcAt KPVector objektum amely megadja annak a pontnak a helyzet�t amelyre a kamera n�z.
			\param [in] vcWorldUp KPVektor objektum amely megadja a vil�g koordin�ta-rendszer�nek felfel� mutat� egys�gvektor�t.
			\return KP_OK sikeres v�grehajt�s eset�n.
			\return KP_FAIL hiba eset�n.

		*/
		virtual HRESULT			SetViewLookAt(const KPVector &vcFrom, const KPVector &vcAt, const KPVector &vcWorldUp) = 0;

		//! Be�ll�tja a l�t�t�r k�zeli �s t�voli s�kjainak a kamer�t�l val� t�vols�g�t
		/*!
			\param [in] fNear Float t�pus� v�ltoz� amely megadja a l�t�t�r k�zeli s�kj�nak t�vols�g�t.
			\param [in] fFar Float t�pus� v�ltoz� amely megadja a l�t�t�r t�voli s�kj�nak t�vols�g�t.
		*/
		virtual void			SetClippingPlanes(float fNear, float fFar) = 0;

		//! Be�ll�tja a vil�g transzform�ci� m�trix�t
		/*!
			\param [in] mWorld Mutat� egy KPMatrix t�pus� objektumra amely megadja az �j vil�g transzform�ci� m�trixot.
		*/
		virtual void			SetWorldTransform(const KPMatrix *mWorld) = 0;

		// RENDER STATE
		///////////////////

		//! Be�ll�tja a figyelmen k�v�l hagyhat� polygonok orient�ci�j�t.
		/*!
			\param [in] rs KPRENDERSTATE t�pus� v�ltoz� amely megadja a polygonok orient�ci�j�t.
		*/
		virtual void			SetBackfaceCulling(KPRENDERSTATE rs)=0;

		//! Be�ll�tja a depth bufferhez val� hozz�f�r�s�t.
		/*!
			\param [in] rs KPRENDERSTATE t�pus� v�ltoz� amely megadja a depth bufferhez val� hozz�f�r�s t�pus�t.
		*/
		virtual void			SetDepthBufferMode(KPRENDERSTATE rs)=0;

		//! Be�ll�tja a poligonok kit�lt�s�nek m�dj�t.
		/*!
			\param [in] rs KPRENDERSTATE t�pus� v�ltoz� amely megadja a poligon kit�lt�s�nek m�dj�t.
			\param [in] f Float t�pus� v�ltoz� amely megadja a kirajzoland� pontok m�ret�t.
			\param [in] clrWireFrame Mutat� egy KPCOLOR t�pus� objektumra amely meghat�rozza a dr�th�l� sz�n�t.
		*/
		virtual void			SetShadeMode(KPRENDERSTATE rs, float f, const KPCOLOR* clrWireFrame)=0;

		//! Visszaadja a poligonok kit�lt�s�nek m�dj�t.
		/*!
			\return KPRENDERSTATE t�pus� �rt�k amely megadja a poliginok kit�lt�s�nek m�dj�t.
		*/
		virtual KPRENDERSTATE	GetShadeMode(void)=0;

		//! Enged�lyezi a text�r�k haszn�lat�s
		/*!
			\param [in] bUse Boolean t�pus� �rt�k amely meghat�rozza, hogy enged�lyzettek e a textur�k.
		*/
		virtual void			UseTextures(bool bUse)=0;

		// LIGHTNING
		////////////////
		
		//! Be�ll�tja a k�rnyezeti f�ny sz�n�t
		/*!
			\param [in] fR Float t�pus� v�ltoz� amely megadja a piros sz�nkomponens intenzit�s�t.
			\param [in] fG Float t�pus� v�ltoz� amely megadja a z�ld sz�nkomponens intenzit�s�t.
			\param [in] fB Float t�pus� v�ltoz� amely megadja a k�k sz�nkomponens intenzit�s�t.
		*/
		virtual void			SetAmbientLight(float fR, float fG, float fB) =0;


		// FONTS
		////////////

		//! L�trehoz �s elt�rol egy font objektumot.
		/*!
			\param [in] chType Mutat� egy sztring t�pus� v�ltoz�ra amely megadja a font bet?t�pus�t
			\param [in] nWeight Int t�pus� �rt�k amely megadja a font f�lk�v�rs�g�t.
			\param [in] bItalic Boolean t�pus� �rt�k amely meghat�rozza, hogy d?lt legyen e a font.
			\param [in] bUnderlined Boolean t�pus� �rt�k amely meghat�rozza, hogy al�h�zott legyen e a font.
			\param [in] bStrikeOut Boolean t�pus� �rt�k amely meghat�rozza, hogy �th�zott legyen e a font.
			\param [in] dwSize DWORD t�pus� �rt�k amely megadja a font m�ret�t.
			\param [out] pFontID Mutat� egy UINT t�pus� v�ltoz�ra amely befogadja az �jonnan l�trehozott font azonos�t�j�t.
			\return KP_OK sikeres v�grehajt�s eset�n.
			\return KP_INVALIDPARAM nem l�tez? pFontID eset�n.
			\return KP_FAIL minden m�s hiba eset�n.
		*/
		virtual HRESULT			CreateMyFont(const char *chType, int nWeight, bool bItalic, bool bUnderlined, bool bStrikeOut, DWORD dwSize, UINT *pFontID)=0;

		//! Sz�veget rajzol a k�perny?re.
		/*!
			\param [in] nFontID
			\param [in] x Int t�pus� �rt�k amely megadja a font v�zszintes helyzet�t a k�perny?n.
			\param [in] y Int t�pus� �rt�k amely megadja a font f�gg?leges helyzet�t a k�perny?n.
			\param [in] a UCHAR t�pus� �rt�k mely megadja a font �ttetsz?s�g�t.
			\param [in] r UCHAR t�pus� �rt�k mely megadja a font piros sz�nkomponens�nek intenzit�s�t.
			\param [in] g UCHAR t�pus� �rt�k mely megadja a font z�ld sz�nkomponens�nek intenzit�s�t.
			\param [in] b UCHAR t�pus� �rt�k mely megadja a font k�k sz�nkomponens�nek intenzit�s�t.
			\param [in] chFormat Mutat� egy sztring t�pus� v�ltoz�ra amely meghat�rozza a sz�veg form�z�s�t.
			\param [in] ... A kiirand� param�terek.
			\return KP_OK sikeres v�grehajt�s eset�n.
			\return KP_INVALIDPARAM nem l�tez? nFontID eset�n.
			\return KP_FAIL minden m�s hiba eset�n.
		*/
		virtual HRESULT			DrawTxt(UINT nFontID, int x, int y, UCHAR a, UCHAR r, UCHAR g, UCHAR b, char *chFormat, ...)=0;

		// RENDERING
		////////////////

		//! Be�ll�tja az aktu�lis ablakot amelybe renderel�s t�rt�nik.
		/*!
			\param [in] nHwnd UINT t�pus� �rt�k amely megadja az �j render ablak sz�m�t.
			\return KP_OK sikeres v�grehajt�s eset�n.
			\return KP_FAIL hiba eset�n.
		*/
		virtual HRESULT			UseWindow(UINT nHwnd) = 0;

		//! Felk�sz�ti a grafikus adapters egy �j sz�nt�r renderel�s�re
		/*!
			\param [in] bClearPixel Boolean t�pus� �rt�k amely meghat�rozza, hogy t�rl?dj�n e a k�perny?.
			\param [in] bClearDepth Boolean t�pus� �rt�k amely meghat�rozza, hogy t�rl?dj�n e a depth buffer.
			\param [in] bClearStencil Boolean t�pus� �rt�k amely meghat�rozza, hogy t�rl?dj�n e a stencil buffer.
			\return KP_OK sikeres v�grehajt�s eset�n.
			\return KP_FAIL hiba eset�n.
		*/
		virtual HRESULT			BeginRendering(bool bClearPixel, bool bClearDepth, bool bClearStencil) = 0;

		//! Lez�rja az aktu�lis sz�nteret �s a gyors�t�t�r tartalm�t a k�perny?re rendereli.
		virtual void 			EndRendering(void) = 0;

		//! Be�ll�tja a k�perny? t�rl�s�hez haszn�lt sz�nt.
		/*!
			\param [in] fRed Float t�pus� v�ltoz� amely meghat�rozza a piros sz�inkomponens intenzit�s�t.
			\param [in] fGreen Float t�pus� v�ltoz� amely meghat�rozza a z�ld sz�inkomponens intenzit�s�t.
			\param [in] fBlue Float t�pus� v�ltoz� amely meghat�rozza a k�k sz�inkomponens intenzit�s�t.
		*/
		virtual void			SetClearColor(float fRed, float fGreen, float fBlue) = 0;

		//! T�rli a k�perny?t, a depth buffert �s a stencil buffert
		/*!
			\param [in] bClearPixel Boolean t�pus� �rt�k amely meghat�rozza, hogy t�rl?dj�n e a k�perny?.
			\param [in] bClearDepth Boolean t�pus� �rt�k amely meghat�rozza, hogy t�rl?dj�n e a depth buffer.
			\param [in] bClearStencil Boolean t�pus� �rt�k amely meghat�rozza, hogy t�rl?dj�n e a stencil buffer.
			\return KP_OK sikeres v�grehajt�s eset�n.
			\return KP_FAIL hiba eset�n.
		*/
		virtual HRESULT			Clear(bool bClearPixel, bool bClearDepth, bool bClearStencil) = 0;

		//! Be�ll�tja a renderer �ltal haszn�lt projekci� t�pus�t.
		/*!
			\param [in] mode KPENGINEMODE t�pus� �rt�k amely megadja a projekci� t�pus�t.
			\param [in] nStage INT t�pus� �rt�k amely megadja mely szinten k�v�njuk megv�ltoztatni a projekci� t�pus�t.
			\return KP_OK sikeres v�grehajt�s eset�n.
			\return KP_FAIL hiba eset�n.
		*/
		virtual HRESULT			SetMode(KPENGINEMODE mode, int nStage) = 0;
		
		//! Be�ll�tja a az adott render szint tulajdons�gait
		/*!
			\param [in] fFOV Float t�pus� �rt�k amely megadja a l�t�sz�get.
			\param [in] pVP Mutat� egy KPVIEWPORT t�pus� objektumra amely meghat�rozza a renderel�s ter�let�t.
			\param [in] nStage Int t�pus� �rt�k amely meghat�rozza a szint azonos�t�j�t melyet be k�v�nunk �ll�tani.
			\return KP_OK sikeres v�grehajt�s eset�n.
			\return KP_FAIL hiba eset�n.
		*/
		virtual HRESULT			InitStage(float fFOV, KPVIEWPORT *pVP, int nStage) = 0;


}; // ! KPRenderDevice class

typedef class KPRenderDevice *LPKPRENDERDEVICE;		//!< Mutat� a KPRenderDevice interf�szre

// At build time, we do not know which Graphic API will have to be loaded, it only clears up during runtime.
// Because of this, we need to use function pointers to access them.
typedef HRESULT (*CREATERENDERDEVICE)(HINSTANCE hDLL, KPRenderDevice **pInterface); //!< Met�dus mutat� az interf�sz el�r�s�re.
typedef HRESULT (*RELEASERENDERDEVICE)(KPRenderDevice **pInterface); //!< Met�dus mutat� az interf�sz el�r�s�re.


#endif // ! KPRENDERDEVICE_H