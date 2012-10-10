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

//! Skin Manager Osztály
/*! Anyagok létrehozását és textúrák betöltését vezérli és megakadályozza már létez? adatok újbóli, szükségtelen tárolását. */
class KPSkinManager
{
protected:
	KPMATERIAL	*m_pMaterials;		//!< Tárolt anyagok listája
	KPTEXTURE	*m_pTextures;		//!< Tárolt textúrák listája
	KPSKIN		*m_pSkins;			//!< Tárolt skinek listája

	UINT		m_numSkins;			//!< Tárolt skinek száma
	UINT		m_numMaterials;		//!< Tárolt anyagok száma
	UINT		m_numTextures;		//!< Tárolt textúrák száma

public:
	KPSkinManager(void) { };		//!< SkinManager interfész konstruktora.
	virtual ~KPSkinManager(void) { }; //!< SkinManager interfész destruktora

	
	//! Új skint ad hozzá a SkinManagerhez.
	/*!
		\param [in] pAmbient Mutató egy KPCOLOR objektumra amely megadja a környezeti fény színét.
		\param [in] pDiffuse Mutató egy KPCOLOR objektumra amely megadja a szórt fény színét.
		\param [in] pEmissive Mutató egy KPCOLOR objektumra amely megadja a kibocsájtott fény színét.
		\param [in] pSpecular Mutató egy KPCOLOR objektumra amely megadja a visszavert fény színét.
		\param [in] fPower float érték amely megadja a szórt fény élességét.
		\param [out] nSkinID Mutató egy UINT típusú változóra amely befogadja a létrehozott skin azonosítóját.
		\return KP_OK sikeres végrehajtás esetén.
		\return KP_OUTOFMEMORY memória túlcsordulás esetén.
	*/
	virtual HRESULT			AddSkin(const KPCOLOR *pAmbient, const KPCOLOR *pDiffuse, const KPCOLOR *pEmissive,
									const KPCOLOR *pSpecular, float fPower, UINT *nSkinID)=0;

	//! Új textúrát csatol egy a SkinManagerben tárolt skinhez
	/*!
		\param [in] nSkinID UINT típusú érték amely megadja a skin azonosítóját amelyhez a textúrát kívánjuk csatolni.
		\param [in] chName Mutató egy sztring típusú változóra amely meghatározza a textúra elérési útvonalát.
		\param [in] bAlpha Boolean tipusú érték amely meghatározza, hogy használunk e Alpha Channelt.
		\param [in] fAlpha Float típusú érték amely megadja az alpha channel értékét, ha használjuk.
		\param [in] pColorKeys Mutató egy KPCOLOR típusú objektumra amely tartalmazza azon színek listáját melyekre alkalmazni kívánjuk az alpha channelt.
		\param [in] numColorKeys DWORD típusú változó amely megadja pColorKeys objektum elemeinek számát
		\return KP_OK sikeres végrehajtás esetén.
		\return KP_OUTOFMEMORY memória túlcsordulás esetén.
	*/
	virtual HRESULT			AddTexture(UINT nSkinID, const char *chName, bool bAlpha, float fAlpha,
									   KPCOLOR *pColorKeys, DWORD numColorKeys)=0;

	//! Visszaadja a kívánt SkinMangagerben tárolt skin objektumot.
	/*!
		\param [in] nSkinID	UINT típusú érték amely megadja a viszzaadni kívánt skin azonosítóját.
		\return	Az adott skin azonosítóhoz tartozó KPSKIN típusú vagy üres skin objektum.
		\return
		\return
	*/
	virtual KPSKIN			GetSkin(UINT nSkinID)=0;

}; // ! KPSkinManager


//! Vertex Cache Manager Osztály
/*!	Ez az osztály felel?s a grafikus ezköz memóriájában található statikus vertex és index bufferek és
	a RAM-ban tárolt dinamikus vertex bufferek felügyeletéért és ezek optimális rendereléséért.
*/
class KPVertexCacheManager
{
	public:
		KPVertexCacheManager(void) { };	//!< VertexCacheManager interfész konstruktora.
		virtual ~KPVertexCacheManager(void) { };	//!< VertexCacheManager interfész konstruktora.

		//! Létrehoz egy statikus buffert a grafikus ezköz memóriájában. Ezt a buffert a kés?bbiekben nem lehet megváltoztatni.
		/*!
			\param [in] VertexID KPVERTEXID típusú objektum amely meghatározza a vertex típusát.
			\param [in] nSkinID UINT típusú érték amely meghatározza a vertexek által használt skint.
			\param [in] nVertices UINT típusú érték amely megadja a vertexek számát.
			\param [in] nIndices UINT típusú érték amely megadja a vertex indexek számát.
			\param [in] pVertices Mutató egy bufferre amely tartalmazza a vertexeket.
			\param [in] pIndices Mutató egy WORD típusú bufferre amely tartalmazza a vertex indexeket
			\param [out] pSBufferID Mutató egy UINT típusú változóra amely befogadja a létrehozott statikus buffer azonosítóját.
			\return KP_OK sikeres végrehajtás esetén.
			\return KP_OUTOFMEMORY memória túlcsordulás esetén.
			\return KP_INVALIDID érvénytelen vertex típus esetén.
			\return KP_CREATEBUFFER a statikus és index bufferek létrehozása közben felmerül? hibák esetén.
			\return KP_BUFFERLOCK sikertelen buffer lock esetén.
		*/
		virtual HRESULT	CreateStaticBuffer(KPVERTEXID VertexID, UINT nSkinID, UINT nVertices, UINT nIndices,
										   const void *pVertices, const WORD *pIndices, UINT *pSBufferID) = 0;

		//! A gyorsítótárba helyezi a renderelni kívánt vertex és index listák azonosítóit. Elegend? mennyiség? adat esetén a képerny?re renderel.
		/*!
			A gyorsítótárba helyezett adatok csak akkor kerülnek renderelésre ha elegend? adatmennyiség gyülemlett fel.
			Ahhoz hogy azonnali renderelést kényszerítsünk ki a VertexCacheManagert?l, hívjuk meg a ForcedFlush vagy ForceFlushAll metódusokat.

			Parameters:
				\param [in] VertexID KPVERTEXID típusú objektum amely megadja a vertexek típusát
				\param [in] nSkinID	 UINT típusú érték amely megadja a vertexek által használt skin azonosítóját
				\param [in] nVertices UINT típusú érték amely megadja a vertexek számát
				\param [in] nIndices Uint típusú érték amely megadja a vertex indexek számát
				\param [in] pVertices Mutató egy a vertexeket tartalmazó tömbre.
				\param [in] pIndices Mutató egy WORD típusú a vertex indexeket tartalmazó tömbre.
				\return KP_OK sikeres végrehajtás esetén.

		*/
		virtual HRESULT	Render(KPVERTEXID VertexID, UINT nSkinID, UINT nVertices, UINT nIndices,
							   const void *pVertices, const WORD *pIndices) = 0;

		//! A statikus buffer tartalmát a képerny?re rendereli.
		/*!
			\param [in] nSBufferID UINT típusú változó amely megadja a renderelni kívánt statikus buffer azonosítóját
			\return KP_OK sikeres végrehajtás esetén.
			\return KP_INVALIDPARAM érvénytelen statikus buffer azonosító esetén.
			\return KP_FAIL renderelés során bekövetkez? hibák esetén.
		*/
		virtual HRESULT Render(UINT nSBufferID) = 0;

		//! A gyorsítótárban található összes buffer tartalmát a képerny?re rendereli.
		/*!
			\return KP_OK sikeres végrehajtás esetén.
			\return KP_FAIL hiba esetén.
		*/
		virtual HRESULT ForcedFlushAll(void) = 0;

		//! A gyorsítótárban található adott típusú vertexeket tároló összes dinamikus buffer tartalmát a képerny?re rendereli.
		/*!
			\param [in] VertexID KPVERTEXID típusú érték amely meghatározza a vertexek típusát.

			\return KP_OK sikeres végrehajtás esetén.
			\return KP_INVALIDID érvénytelen vertex típus esetén.
			\return KP_FAIL hiba esetén.
		*/
		virtual HRESULT ForcedFlush(KPVERTEXID VertexID) = 0;

		// Resets all the active skin, texture and static buffer flags,
		// effectively forcing the cache manager into recalculating everything.
		//! Invalidates all of the active Skin, Static and Dinamic cache flags

		//! Lenullázza az aktuális skin, statikus és dinamikus bufferek azonosítóit.
		virtual void    InvalidateStates(void)=0;

}; // ! Vertex Cache Manager


//! Render Device Osztály
/*!
 This is the render device interface of the engine. Makes our engine Graphic API independant.
 The public methods listed here are the only way for the user to 
 interact the underlying implementation of the graphics API itself.
*/
class KPRenderDevice
{
	protected:
		HWND					m_hWndMain;				//!< Az alkalmazás f? ablakának azonosítója
		HWND					m_hWnd[MAX_3DHWND];		//!< Renderelésre alkalmas ablakok azonosítóinak listája
		UINT					m_nNumhWnd;				//!< Renderelésre alkalmas ablakok aktuális száma
		UINT					m_nActivehWnd;			//!< A jelenleg aktív ablak száma
		HINSTANCE				m_hDLL;					//!< A betöltött DLL azonosítója
		DWORD					m_dwWidth;				//!< Képerny? szélessége
		DWORD					m_dwHeight;				//!< Képerny? magassága
		bool					m_bWindowed;			//!< Teljes képerny? vagy ablak üzemmód
		char					m_chAdapter[256];		//!< A grafikus adapter neve
		FILE					*m_pLog;				//!< Log fájl azonosítója
		bool					m_bRunning;				//!< Fut jelenleg a motor?!

		KPSkinManager			*m_pSkinManager;	//!< Mutató a SkinManagerre
		KPVertexCacheManager	*m_pVertexMan;		//!< Mutató a VertexCacheManagerre

		// VIEW / PROJECTION ATTRIBUTES
		///////////////////////////////////

		float					m_fNear;				//!< A látótér közeli síkjának távolsága
		float					m_fFar;					//!< A látótér távoli síkjának távolsága
		int						m_nStage;				//!< Az aktuális render szint száma
		KPENGINEMODE			m_Mode;					//!< Az aktuális renderelési üzemmód
		KPVIEWPORT				m_ViewPort[4];			//!< Látóterek listája

		// RENDER STATE ATTRIBUTES
		//////////////////////////////

		KPCOLOR					m_clrWireframe;			//!< Drótháló színe
		KPRENDERSTATE			m_ShadeMode;			//!< Vertex shading típusa


	public:
		KPRenderDevice(void) {};	//!< KPRenderDevice interfész konstruktora
		virtual ~KPRenderDevice(void) {}; //!< KPRenderDevice interfész destruktora

		// INITIALIZE / RELEASE
		//////////////////////////

		//! Létrehozza és alapállapotba állítja a grafikus ezközt.
		/*!
			\param [in] hWnd HWND típusú érték amely megadja az alkalmazás f?ablakának azonosítóját.
			\param [in] hWnd3D Mutató egy HWND tipusú tömbre mely megadja a renderelésre alkalmas ablakok azonosítóit.
			\param [in] nNumhWnd Int típusú érték amely megadja a hWnd3D tömbben található renderelésre alkamas ablakok számát.
			\param [in] nMinDepth Int típusú érték amely megadja a Depth Buffer minimális mélységét.
			\param [in] nMinStencil Int típusú érték amely megadja a Stencil Buffer minimális mélységét.
			\param [in] bSaveLog Boolean típusú érték amely meghatározza, hogy használjunk e Log fájlt vagy sem.
			\return KP_OK sikeres végrehajtás esetén.
			\return KP_CANCELLED a beállitások ablak megszakítása esetén.
			\return KP_CREATEAPI a grafikus API inicializálása során fellép? hiba esetén
			\return KP_CREATEDEVICE a grafikus ezköz inicializálása során fellép? hiba esetén
			\return KP_FAIL bármely más hiba esetén.
		*/
		virtual HRESULT			Init(HWND hWnd, const HWND *hWnd3D, int nNumhWnd, int nMinDepth, int nMinStencil, bool bSaveLog) = 0;

		//! Felszabadítja a grafikus ezköz által lefoglalt er?forrásokat
		virtual void			Release(void) = 0;

		//! Ellen?rzi, hogy fut e a KPEngine motor.
		/*!
			\return true ha a KPEngine motor jelenleg fut.
			\return false ha a KPEngine motor nem fut.
		*/
		virtual bool			IsRunning(void) = 0;

		//! Ellen?rzi, hogy ablak módban fut e az alkalmazás
		/*!
			\return true ha a KPEngine motor jelenleg ablakban fut.
			\return false ha a KPEngine motor teljes képerny?n fut
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
		
		//! Közvetlen hozzáférést tesz lehet?vé a SkinManagerhez
		/*!
			\return Mutató egy KPSkinManager típusú objektumra
		*/
		virtual					KPSkinManager* GetSkinManager(void) = 0;

		//! Közvetlen hozzáférést tesz lehet?vé a VertexCacheManagerhez
		/*!
			\return Mutató egy KPVertexCacheManager típusú objektumra
		*/
		virtual					KPVertexCacheManager* GetVertexManager(void)=0;

		// VIEW / PROJECTION 
		////////////////////////

		//! Visszaadja a látótér síkjait
		/*!
			\param [out] pFrustum Mutató egy KPPlane típusú tömbre amely befogadja a látótér hat síkját.
			\return KP_OK sikeres végrehajtás esetén.
		*/
		virtual HRESULT			GetFrustum(KPPlane* pFrustum) = 0;

		//! Beállítja a kamera mátrixot a kamera helyzetét?l és orientációjától függ?en.
		/*!
			\param [in] vcX KPVector objektum amely megadja a kamera x tengely menti egységvektorát.
			\param [in] vcY KPVector objektum amely megadja a kamera y tengely menti egységvektorát.
			\param [in] vcZ KPVector objektum amely megadja a kamera z tengely menti egységvektorát.
			\param [in] vcPos KPVector objektum amely megadja a kamera helyzetét.
			\return KP_OK sikeres végrehajtás esetén.
			\return KP_FAIL hiba esetén.

		*/
		virtual HRESULT			SetView3D(const KPVector &vcX, const KPVector &vcY, const KPVector &vcZ, const KPVector &vcPos) = 0;

		//! Beállítja a kamera mátrixot a kamera és egy pont helyzetét?l függ?en amelyre a kamera néz.
		/*!
			\param [in] vcFrom KPVector objektum amely megadja a kamera helyzetét.
			\param [in] vcAt KPVector objektum amely megadja annak a pontnak a helyzetét amelyre a kamera néz.
			\param [in] vcWorldUp KPVektor objektum amely megadja a világ koordináta-rendszerének felfelé mutató egységvektorát.
			\return KP_OK sikeres végrehajtás esetén.
			\return KP_FAIL hiba esetén.

		*/
		virtual HRESULT			SetViewLookAt(const KPVector &vcFrom, const KPVector &vcAt, const KPVector &vcWorldUp) = 0;

		//! Beállítja a látótér közeli és távoli síkjainak a kamerától való távolságát
		/*!
			\param [in] fNear Float típusú változó amely megadja a látótér közeli síkjának távolságát.
			\param [in] fFar Float típusú változó amely megadja a látótér távoli síkjának távolságát.
		*/
		virtual void			SetClippingPlanes(float fNear, float fFar) = 0;

		//! Beállítja a világ transzformáció mátrixát
		/*!
			\param [in] mWorld Mutató egy KPMatrix típusú objektumra amely megadja az új világ transzformáció mátrixot.
		*/
		virtual void			SetWorldTransform(const KPMatrix *mWorld) = 0;

		// RENDER STATE
		///////////////////

		//! Beállítja a figyelmen kívül hagyható polygonok orientációját.
		/*!
			\param [in] rs KPRENDERSTATE típusú változó amely megadja a polygonok orientációját.
		*/
		virtual void			SetBackfaceCulling(KPRENDERSTATE rs)=0;

		//! Beállítja a depth bufferhez való hozzáférését.
		/*!
			\param [in] rs KPRENDERSTATE típusú változó amely megadja a depth bufferhez való hozzáférés típusát.
		*/
		virtual void			SetDepthBufferMode(KPRENDERSTATE rs)=0;

		//! Beállítja a poligonok kitöltésének módját.
		/*!
			\param [in] rs KPRENDERSTATE típusú változó amely megadja a poligon kitöltésének módját.
			\param [in] f Float típusú változó amely megadja a kirajzolandó pontok méretét.
			\param [in] clrWireFrame Mutató egy KPCOLOR típusú objektumra amely meghatározza a drótháló színét.
		*/
		virtual void			SetShadeMode(KPRENDERSTATE rs, float f, const KPCOLOR* clrWireFrame)=0;

		//! Visszaadja a poligonok kitöltésének módját.
		/*!
			\return KPRENDERSTATE típusú érték amely megadja a poliginok kitöltésének módját.
		*/
		virtual KPRENDERSTATE	GetShadeMode(void)=0;

		//! Engedélyezi a textúrák használatás
		/*!
			\param [in] bUse Boolean típusú érték amely meghatározza, hogy engedélyzettek e a texturák.
		*/
		virtual void			UseTextures(bool bUse)=0;

		// LIGHTNING
		////////////////
		
		//! Beállítja a környezeti fény színét
		/*!
			\param [in] fR Float típusú változó amely megadja a piros színkomponens intenzitását.
			\param [in] fG Float típusú változó amely megadja a zöld színkomponens intenzitását.
			\param [in] fB Float típusú változó amely megadja a kék színkomponens intenzitását.
		*/
		virtual void			SetAmbientLight(float fR, float fG, float fB) =0;


		// FONTS
		////////////

		//! Létrehoz és eltárol egy font objektumot.
		/*!
			\param [in] chType Mutató egy sztring típusú változóra amely megadja a font bet?típusát
			\param [in] nWeight Int típusú érték amely megadja a font félkövérségét.
			\param [in] bItalic Boolean típusú érték amely meghatározza, hogy d?lt legyen e a font.
			\param [in] bUnderlined Boolean típusú érték amely meghatározza, hogy aláhúzott legyen e a font.
			\param [in] bStrikeOut Boolean típusú érték amely meghatározza, hogy áthúzott legyen e a font.
			\param [in] dwSize DWORD típusú érték amely megadja a font méretét.
			\param [out] pFontID Mutató egy UINT típusú változóra amely befogadja az újonnan létrehozott font azonosítóját.
			\return KP_OK sikeres végrehajtás esetén.
			\return KP_INVALIDPARAM nem létez? pFontID esetén.
			\return KP_FAIL minden más hiba esetén.
		*/
		virtual HRESULT			CreateMyFont(const char *chType, int nWeight, bool bItalic, bool bUnderlined, bool bStrikeOut, DWORD dwSize, UINT *pFontID)=0;

		//! Szöveget rajzol a képerny?re.
		/*!
			\param [in] nFontID
			\param [in] x Int típusú érték amely megadja a font vízszintes helyzetét a képerny?n.
			\param [in] y Int típusú érték amely megadja a font függ?leges helyzetét a képerny?n.
			\param [in] a UCHAR típusú érték mely megadja a font áttetsz?ségét.
			\param [in] r UCHAR típusú érték mely megadja a font piros színkomponensének intenzitását.
			\param [in] g UCHAR típusú érték mely megadja a font zöld színkomponensének intenzitását.
			\param [in] b UCHAR típusú érték mely megadja a font kék színkomponensének intenzitását.
			\param [in] chFormat Mutató egy sztring típusú változóra amely meghatározza a szöveg formázását.
			\param [in] ... A kiirandó paraméterek.
			\return KP_OK sikeres végrehajtás esetén.
			\return KP_INVALIDPARAM nem létez? nFontID esetén.
			\return KP_FAIL minden más hiba esetén.
		*/
		virtual HRESULT			DrawTxt(UINT nFontID, int x, int y, UCHAR a, UCHAR r, UCHAR g, UCHAR b, char *chFormat, ...)=0;

		// RENDERING
		////////////////

		//! Beállítja az aktuális ablakot amelybe renderelés történik.
		/*!
			\param [in] nHwnd UINT típusú érték amely megadja az új render ablak számát.
			\return KP_OK sikeres végrehajtás esetén.
			\return KP_FAIL hiba esetén.
		*/
		virtual HRESULT			UseWindow(UINT nHwnd) = 0;

		//! Felkészíti a grafikus adapters egy új színtér renderelésére
		/*!
			\param [in] bClearPixel Boolean típusú érték amely meghatározza, hogy törl?djön e a képerny?.
			\param [in] bClearDepth Boolean típusú érték amely meghatározza, hogy törl?djön e a depth buffer.
			\param [in] bClearStencil Boolean típusú érték amely meghatározza, hogy törl?djön e a stencil buffer.
			\return KP_OK sikeres végrehajtás esetén.
			\return KP_FAIL hiba esetén.
		*/
		virtual HRESULT			BeginRendering(bool bClearPixel, bool bClearDepth, bool bClearStencil) = 0;

		//! Lezárja az aktuális színteret és a gyorsítótár tartalmát a képerny?re rendereli.
		virtual void 			EndRendering(void) = 0;

		//! Beállítja a képerny? törléséhez használt színt.
		/*!
			\param [in] fRed Float típusú változó amely meghatározza a piros szíinkomponens intenzitását.
			\param [in] fGreen Float típusú változó amely meghatározza a zöld szíinkomponens intenzitását.
			\param [in] fBlue Float típusú változó amely meghatározza a kék szíinkomponens intenzitását.
		*/
		virtual void			SetClearColor(float fRed, float fGreen, float fBlue) = 0;

		//! Törli a képerny?t, a depth buffert és a stencil buffert
		/*!
			\param [in] bClearPixel Boolean típusú érték amely meghatározza, hogy törl?djön e a képerny?.
			\param [in] bClearDepth Boolean típusú érték amely meghatározza, hogy törl?djön e a depth buffer.
			\param [in] bClearStencil Boolean típusú érték amely meghatározza, hogy törl?djön e a stencil buffer.
			\return KP_OK sikeres végrehajtás esetén.
			\return KP_FAIL hiba esetén.
		*/
		virtual HRESULT			Clear(bool bClearPixel, bool bClearDepth, bool bClearStencil) = 0;

		//! Beállítja a renderer által használt projekció típusát.
		/*!
			\param [in] mode KPENGINEMODE típusú érték amely megadja a projekció típusát.
			\param [in] nStage INT típusó érték amely megadja mely szinten kívánjuk megváltoztatni a projekció típusát.
			\return KP_OK sikeres végrehajtás esetén.
			\return KP_FAIL hiba esetén.
		*/
		virtual HRESULT			SetMode(KPENGINEMODE mode, int nStage) = 0;
		
		//! Beállítja a az adott render szint tulajdonságait
		/*!
			\param [in] fFOV Float típusú érték amely megadja a látószöget.
			\param [in] pVP Mutató egy KPVIEWPORT típusú objektumra amely meghatározza a renderelés területét.
			\param [in] nStage Int típusú érték amely meghatározza a szint azonosítóját melyet be kívánunk állítani.
			\return KP_OK sikeres végrehajtás esetén.
			\return KP_FAIL hiba esetén.
		*/
		virtual HRESULT			InitStage(float fFOV, KPVIEWPORT *pVP, int nStage) = 0;


}; // ! KPRenderDevice class

typedef class KPRenderDevice *LPKPRENDERDEVICE;		//!< Mutató a KPRenderDevice interfészre

// At build time, we do not know which Graphic API will have to be loaded, it only clears up during runtime.
// Because of this, we need to use function pointers to access them.
typedef HRESULT (*CREATERENDERDEVICE)(HINSTANCE hDLL, KPRenderDevice **pInterface); //!< Metódus mutató az interfész elérésére.
typedef HRESULT (*RELEASERENDERDEVICE)(KPRenderDevice **pInterface); //!< Metódus mutató az interfész elérésére.


#endif // ! KPRENDERDEVICE_H