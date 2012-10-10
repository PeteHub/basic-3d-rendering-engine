/*
 *****************************************************************
 *
 *	KPEngine Source code 
 *	Kovacs Peter - August 2009
 *
 *  File: KPD3D_misc.cpp
 *  Description: Direct3D Render Device helper method definitions
 *
 *****************************************************************
*/

#include "KP.h"
#include "KPD3D.h"


// GetFrustum ////
//////////////////
/*
	Extracts the planes of the current view frustum.
	The view frustum changes every time the view or projection matrices change!

	Params:
		pFrustum	- Address of an array of KPPlane type objects that will be filled
					  with the six planes of the view frustum.
*/
HRESULT KPD3D::GetFrustum(KPPlane *pFrustum)
{
	// All we have to do is extract the plane normals and distance from origin
	// for all six sides of the plane and revert the normals (they should point outward)
	// The shape of the frustrum depends on the projection and view matrices
	// so we will use the ViewProj matrix.

	// LEFT
	pFrustum[0].m_vcNormal.x = -( m_mViewProj._14 + m_mViewProj._11);
	pFrustum[0].m_vcNormal.y = -( m_mViewProj._24 + m_mViewProj._21);
	pFrustum[0].m_vcNormal.z = -( m_mViewProj._34 + m_mViewProj._31);
	pFrustum[0].m_fDistance  = -( m_mViewProj._44 + m_mViewProj._41);
	

	// RIGHT
	pFrustum[1].m_vcNormal.x = -( m_mViewProj._14 - m_mViewProj._11);
	pFrustum[1].m_vcNormal.y = -( m_mViewProj._24 - m_mViewProj._21);
	pFrustum[1].m_vcNormal.z = -( m_mViewProj._34 - m_mViewProj._31);
	pFrustum[1].m_fDistance  = -( m_mViewProj._44 - m_mViewProj._41);

	// TOP
	pFrustum[2].m_vcNormal.x = -( m_mViewProj._14 - m_mViewProj._12);
	pFrustum[2].m_vcNormal.y = -( m_mViewProj._24 - m_mViewProj._22);
	pFrustum[2].m_vcNormal.z = -( m_mViewProj._34 - m_mViewProj._32);
	pFrustum[2].m_fDistance  = -( m_mViewProj._44 - m_mViewProj._42);

	// BOTTOM
	pFrustum[3].m_vcNormal.x = -( m_mViewProj._14 + m_mViewProj._12);
	pFrustum[3].m_vcNormal.y = -( m_mViewProj._24 + m_mViewProj._22);
	pFrustum[3].m_vcNormal.z = -( m_mViewProj._34 + m_mViewProj._32);
	pFrustum[3].m_fDistance  = -( m_mViewProj._44 + m_mViewProj._42);

	// NEAR (SCREEN)
	pFrustum[4].m_vcNormal.x = -m_mViewProj._13;
	pFrustum[4].m_vcNormal.y = -m_mViewProj._23;
	pFrustum[4].m_vcNormal.z = -m_mViewProj._33;
	pFrustum[4].m_fDistance  = -m_mViewProj._43;

	// FAR
	pFrustum[5].m_vcNormal.x = -( m_mViewProj._14 - m_mViewProj._13);
	pFrustum[5].m_vcNormal.y = -( m_mViewProj._24 - m_mViewProj._23);
	pFrustum[5].m_vcNormal.z = -( m_mViewProj._34 - m_mViewProj._33);
	pFrustum[5].m_fDistance  = -( m_mViewProj._44 - m_mViewProj._43);

	// Normalize the normal vectors of the planes
	float fLength;

	for ( int i = 0; i < 6; ++i )
	{
		fLength = pFrustum[i].m_vcNormal.GetLength();
		pFrustum[i].m_vcNormal	/= fLength;
		pFrustum[i].m_fDistance /= fLength;
	}

	return KP_OK;

} // ! GetFrustum


// SetView3D ////
/////////////////
/*
 Sets the view matrix based on the position and orientation of the camera.

 Param:
	vcX		- The unit vector of the camera's x axis. ( Pointing right from the camera)
	vcY		- The unit vector of the camera's Y axis. ( Pointing up from the camera)
	vcZ		- The unit vector of the camera's Z axis. ( Pointing in the direction the camera faces )

	vcPos	- Position vector of the camera.
*/
HRESULT KPD3D::SetView3D(const KPVector &vcX, const KPVector &vcY, const KPVector &vcZ, const KPVector &vcPos)
{
	// If the engine is not running there is nothing to calculate
	if ( !m_bRunning )
	{
		Log("SetView3D: The engine is not running!");
		return KP_FAIL;
	}

	m_mView3D._14 = 0.0f;
	m_mView3D._24 = 0.0f;
	m_mView3D._34 = 0.0f;
	m_mView3D._44 = 1.0f;

	// The first three columns of the matrix contains the unit vector coordinates
	// The last row of the first three columns contain the reverse translation(movement)
	// in the unitvectors direction: tr_axis = axis_unit_vector*camera_position;

	// X (Right)
	m_mView3D._11 = vcX.x;
	m_mView3D._21 = vcX.y;
	m_mView3D._31 = vcX.z;
	m_mView3D._41 = -(vcX*vcPos);

	// Y (Up)
	m_mView3D._12 = vcY.x;
	m_mView3D._22 = vcY.y;
	m_mView3D._32 = vcY.z;
	m_mView3D._42 = -(vcY*vcPos);

	// Z (Direction the camera faces)
	m_mView3D._13 = vcZ.x;
	m_mView3D._23 = vcZ.y;
	m_mView3D._33 = vcZ.z;
	m_mView3D._43 = -(vcZ*vcPos);

	// We have set the view transformation matrix for the device
	if ( FAILED( m_pDevice->SetTransform(D3DTS_VIEW, &m_mView3D) ) )
	{
		Log("SetView3D: IDirect3DDevice9::SetTransForm() returned D3DERR_INVALIDCALL");
		return KP_FAIL;
	}


	// We have to recalculate the concatenated matrices
	// because one of them has changed!
	CalcViewProjMatrix();
	CalcWorldViewProjMatrix();


	return KP_OK;

} // ! SetView3D()


// SetViewLookAt ////
/////////////////////
/*
	Sets the view matrix based on the camera's and the point's position the camera is looking at.

	Params:
	vcCamera	- KPVector value specifying the location of the camera
	vcPoint		- KPVector value specifying the location of the point the camera is pointing at.
	vcWorldUp	- KPVector value specifying the up unit vector of the world coordinate sytem.

*/
HRESULT KPD3D::SetViewLookAt(const KPVector &vcCamera, const KPVector &vcPoint, const KPVector &vcWorldUp)
{
	KPVector	vcDirection, vcUp, vcTmp;
	float		fDot, fLength;

	// The direction vector is the difference of the point the
	// camera is looking at and the position of the camera
	// in unit vector form.
	vcDirection = vcPoint - vcCamera;
	vcDirection.Normalize();

	// Up vector
	/*
		To get the up vector, first we need the new normalized direction vector
		then, we calculate the dot product of the world up vector and the new direction vector

		Since both of their length is 1, their dot product will equal cos(alpha), alpha being the angle
		the two vector enclose.
		The up vector is perpendicular to the new direction vector. To calculate this vector
		we must calculate the scalar projection of the world up vector onto the new direction vector:
		|vcWorldUP| * cos(alpha), but because they are both normals, it equals |direction| * cos(alpha),
		which basically means, we scale the direction vector with the two vectors dot product.

		Once we are done with this, the Up vector can be calculated by: WorldUP - ScaledDirection;

		However, if the direction vector and the world up are almost paralelly aligned, the UP vector will be
		too small to handle. In this case we can try the Y and Z axes of the world system.
	*/

	fDot	= vcWorldUp * vcDirection;	// Dot product
	vcTmp	= vcDirection * fDot;		// skalar projection (scaling with cos(a)
	vcUp	= vcWorldUp - vcTmp;		// Calculate the up vector

	fLength = vcUp.GetLength();

	// Now check whether the up vector is too short
	if ( fLength < EPSILON )
	{
		// If it is, lets try the Y world axis
		KPVector vcWorld;
		vcWorld.Set(0.0f, 1.0f, 0.0f);

		vcTmp	= vcDirection * vcDirection.y;
		vcUp	= vcWorld - vcTmp;

		fLength = vcUp.GetLength();

		// if it's still too small, check Z
		if ( fLength < EPSILON )
		{
			vcWorld.Set(0.0f, 0.0f, 1.0f);

			vcTmp = vcDirection * vcDirection.z;
			vcUp = vcWorld - vcTmp;

			fLength = vcUp.GetLength();
			
			// Worst case, we could not find an UP vector and fail it.
			if ( fLength < EPSILON )
			{
				Log("SetViewLookAt: Could not find proper UP vector for camera!");
				return KP_FAIL;
			}

		}

	}

	// Let's normalize our UP vector
	vcUp /= fLength;

	// Get the right vector
	// The right vector is simply the cross product of the two other vectors.
	KPVector vcRight;
	vcRight.Cross(vcUp, vcDirection);

	// Set our new View Matrix
	return SetView3D(vcRight, vcUp, vcDirection, vcCamera);

} // ! SetViewLookAt()


// SetClippingPlanes ////
/////////////////////////
/*
	Specifies the distance of the near and far end of the view frustum in the camera's direction.
	The near end must be in front of the camera and the closer the far end is, the higher the 
	z-buffer accuracy is in its [0.0f - 1.0f] interval.

	Params:
		fNear:	Floating-point value specifying the near end of the frustum in the
				camera's direction from the camera's location.
		fFar:	Floating-point value specifying the far end of the frustum in the
				camera's direction from the camera's location.
*/
void KPD3D::SetClippingPlanes(float fNear, float fFar)
{
	// Make some checks in order to prevent 'user errors' :P
	if ( fNear <= 0.0f ) m_fNear = 0.01f;
	else m_fNear = fNear;

	if ( fFar < 1.0f )	m_fFar = 1.0f;
	else m_fFar	= fFar;

	if ( m_fNear >= m_fFar )
	{
		m_fNear = m_fFar;
		m_fFar	= m_fNear + 1.0f;
	}

	// Update the projection matrices ////

	// 2D projection and view matrices
	Prepare2D();

	// Orthogonal projection
	float Q = 1.0f		/ (m_fFar - m_fNear); // Reciprocal distance of the two planes
	float X = m_fNear	/ (m_fNear - m_fFar);

	m_mProjO[0]._33 = m_mProjO[1]._33 = m_mProjO[2]._33 = m_mProjO[3]._33 = Q;
	m_mProjO[0]._43 = m_mProjO[1]._43 = m_mProjO[2]._43 = m_mProjO[3]._43 = X;

	// Perspective projection
	Q *= m_fFar;
	X  = -Q * m_fNear;

	m_mProjP[0]._33 = m_mProjP[1]._33 = m_mProjP[2]._33 = m_mProjP[3]._33 = Q;
	m_mProjP[0]._43 = m_mProjP[1]._43 = m_mProjP[2]._43 = m_mProjP[3]._43 = X;

} // ! SetClippingPlanes


// Prepare2D ////
/////////////////
//
// Builds an orthogonal projection matrix and a view matrix that treats 
// vertices as if they were given with screen coordinates in their x and y components.
// Good for drawing 2d objects on the screen, HUDs/GUIs for example
void KPD3D::Prepare2D(void)
{
	float tx, ty, tz;	// Translation values for the view matrix

	// Set the projection and view matrices to identity matrices
	memset(&m_mProj2D, 0, sizeof(D3DMATRIX));
	memset(&m_mView2D, 0, sizeof(D3DMATRIX));
	m_mView2D._11 = m_mView2D._33 = m_mView2D._44 = 1.0f;

	// Build the orthogonal projection matrix
	m_mProj2D._11	= 2.0f / (float)m_dwWidth;	//X
	m_mProj2D._22	= 2.0f / (float)m_dwHeight; //Y
	m_mProj2D._33	= 1.0f / (m_fFar-m_fNear);	//Z
	m_mProj2D._43	= -m_fNear * m_mProj2D._33; // -m_fNear*(1.0f/(m_fFar-m_fNear))
	m_mProj2D._44	= 1.0f;

	// Build the 2D view matrix
	tx = -(m_dwWidth*0.5f);
	ty = m_dwHeight*0.5f;
	tz = m_fNear + 0.1f;

	m_mView2D._22 = -1.0f;
	m_mView2D._41 = tx;
	m_mView2D._42 = ty;
	m_mView2D._43 = tz;
	
} // ! Prepare2D()


// CalcPerspProjMatrix ////
///////////////////////////
/*
	Calculates the projection matrix of a perspective projection of a 3D scene onto a 2D projection plane.

	Params:
		fFOV:		Floating-point value specifying the field of horizontal view
		fAspect:	Floating-point value specifying the aspect ratio (viewport height / width)
		m:			Address to a D3DMATRIX type object to store the Perspective Projection Matrix
*/
HRESULT KPD3D::CalcPerspProjMatrix(float fFOV, float fAspect, D3DMATRIX *m)
{
	/*
		Math: http://www.codeguru.com/cpp/misc/misc/math/article.php/c10123__3/
	*/

	if ( fabs(m_fFar - m_fNear) < 0.01f )
	{
		Log("Unable to calculate the perspective projection matrix: Far-Near is too small!");
		return KP_FAIL;
	}

	float fovSIN, fovCOS;

	fovSIN = sinf(fFOV/2);

	if ( fabs(fovSIN) < 0.01f )
	{
		Log("Unable to calculate the perspective projection matrix: sin(FOV/2) is too small!");
		return KP_FAIL;
	}

	fovCOS = cosf(fFOV/2);

	float x = fAspect	* (fovCOS / fovSIN);
	float y = 1.0f		* (fovCOS / fovSIN);
	float z = m_fFar	/ (m_fFar - m_fNear);

	// Zero the matrix
	memset(m, 0, sizeof(D3DMATRIX));

	(*m)._11 = x;
	(*m)._22 = y;
	(*m)._33 = z;
	(*m)._34 = 1.0f;
	(*m)._43 = -z * m_fNear;

	return KP_OK;

} // ! CalcPerspProjectMatrix


// CalcViewProjMatrix ////
//////////////////////////
//
// Concatenates the view and projection matrices relevant to the current engine mode and stage.
void KPD3D::CalcViewProjMatrix(void)
{
	KPMatrix *pView, *pProjection;

	// Set the two matrices based on the current engine mode
	// We cast the D3DMATRIX types to KPMatrix so we can multiply them easier
	
	// Use Perspective mode as default mode
	pView		= (KPMatrix*)&m_mView3D;					// View Matrix
	pProjection = (KPMatrix*)&(m_mProjP[m_nStage]);			// Projection matrix of the current stage

	// Orthogonal mode
	if ( m_Mode == EMD_ORTHOGONAL )
		pProjection = (KPMatrix*)&(m_mProjO[m_nStage]);		// View Matrix is the same

	if ( m_Mode == EMD_TWOD )
	{
		pView		= (KPMatrix*)&m_mView2D;				// 2d View Matrix
		pProjection	= (KPMatrix*)&m_mProj2D;				// 2d Projection Matrix
	}

	// Set a pointer at the concatenated matrix
	KPMatrix *pM = (KPMatrix*)&m_mViewProj;

	// Concatenate the proper matrices. ORDER IS IMPORTANT!
	*pM	= (*pView) * (*pProjection);

} // ! CalcViewPRojMatrix


// CalcWorldViewProjMatrix ////
///////////////////////////////
//
// Concatenates the World, View and Projection matrices based on the current engine mode and stage.
void KPD3D::CalcWorldViewProjMatrix(void)
{
	KPMatrix *pWorld, *pView, *pProjection;

	// Set the three  matrices based on the current engine mode
	// We cast the D3DMATRIX types to KPMatrix so we can multiply them easier
	
	// Use Perspective mode as default mode
	pWorld		= (KPMatrix*)&m_mWorld;						// World Matrix
	pView		= (KPMatrix*)&m_mView3D;					// View Matrix
	pProjection = (KPMatrix*)&(m_mProjP[m_nStage]);			// Projection matrix of the current stage

	// Orthogonal mode
	if ( m_Mode == EMD_ORTHOGONAL )
		pProjection = (KPMatrix*)&(m_mProjO[m_nStage]);		// View Matrix is the same

	if ( m_Mode == EMD_TWOD )
	{
		pView		= (KPMatrix*)&m_mView2D;				// 2d View Matrix
		pProjection	= (KPMatrix*)&m_mProj2D;				// 2d Projection Matrix
	}

	// Set a pointer at the concatenated matrix
	KPMatrix *pM = (KPMatrix*)&m_mWorldViewProj;

	// Concatenate the proper matrices. ORDER IS IMPORTANT!
	*pM	= (*pWorld) * (*pView) * (*pProjection);

} // ! CalcWorldViewProjMatrix


// SetMode ////
///////////////
/*
	Sets the engine rendering mode and the appropriate matrices.

	Params:
		mode:	KPENGINEMODE enum value specifying the rendering mode.
		nStage:	Integer value specifying the rendering stage.


*/
HRESULT KPD3D::SetMode(KPENGINEMODE Mode, int nStage)
{
	// Is the engine running at all?
	if ( !m_bRunning )
	{
		Log("SetMode: The engine is not running!");
		return E_FAIL;
	}

	D3DVIEWPORT9 d3dVP;

	// Make sure the stage is valid
	if ( (nStage > 3) || (nStage < 0) )
		nStage = 0;

	m_Mode = Mode;

	// Flush all caches prior to changing the mode because of
	// major changes in the rendering way
	m_pVertexMan->ForcedFlushAll();

	m_nStage = nStage;

	// Set the viewport
	d3dVP.X			= m_ViewPort[nStage].x;
	d3dVP.Y			= m_ViewPort[nStage].y;
	d3dVP.Width		= m_ViewPort[nStage].width;
	d3dVP.Height	= m_ViewPort[nStage].height;
	d3dVP.MinZ		= 0.0f;
	d3dVP.MaxZ		= 1.0f;

	if ( FAILED( m_pDevice->SetViewport(&d3dVP) ) )
	{
		Log("SetMode: IDirect3DDevice9::SetViewport failed.");
		return KP_FAIL;
	}

	// If it's perspective or orthogonal projection
	if ( Mode != EMD_TWOD )
	{
		// View Matrix
		if ( FAILED( m_pDevice->SetTransform(D3DTS_VIEW, &m_mView3D) ) )
		{
			Log("SetMode: IDirect3DDevice9::SetTransform failed to set view matrix for perspective or orthogonal projection.");
			return KP_FAIL;
		}

		// Perspective Mode
		if ( m_Mode == EMD_PERSPECTIVE )
		{
			if ( FAILED( m_pDevice->SetTransform(D3DTS_PROJECTION, &m_mProjP[nStage]) ) )
			{
				Log("SetMode: IDirect3DDevice9::SetTransform failed to set perspective projection matrix for stage: %d.", nStage);
				return KP_FAIL;
			}
		}
		// Orthogonal Mode
		else
		{
			if ( FAILED( m_pDevice->SetTransform(D3DTS_PROJECTION, &m_mProjO[nStage]) ) )
			{
				Log("SetMode: IDirect3DDevice9::SetTransform failed to set orthogonal projection matrix for stage: %d.", nStage);
				return KP_FAIL;
			}
		}

		// Matrices have changed, need to recalculate the concatenates!
		CalcViewProjMatrix();
		CalcWorldViewProjMatrix();

	} // ! if not TWOD
	// if EMD_TWOD
	else
	{
		// View Matrix
		if ( FAILED( m_pDevice->SetTransform(D3DTS_VIEW, &m_mView2D) ) )
		{
			Log("SetMode: IDirect3DDevice9::SetTransform failed to set TWOD view matrix.");
			return KP_FAIL;
		}

		// Projection Matrix
		if ( FAILED( m_pDevice->SetTransform(D3DTS_PROJECTION, &m_mProj2D) ) )
		{
			Log("SetMode: IDirect3DDevice9::SetTransform failed to set TWOD projection matrix.");
			return KP_FAIL;
		}
	}

	return KP_OK;

} // ! SetMode


// InitStage ////
/////////////////
/*
	Initializes a rendering stage.

	Params:
		fFov:	Floating-point value specifying the field of view
		VP:		KPVIEWPORT value specifying the viewport dimensions or NULL for full screen.
		nStage:	Integer value specifying the rendering stage.
*/
HRESULT KPD3D::InitStage(float fFOV, KPVIEWPORT *pVP, int nStage)
{
	float fAspectRatio;

	if ( !pVP )
	{
		// Set the viewport of the given stage to full screen dimensions
		m_ViewPort[nStage].x		= 0;
		m_ViewPort[nStage].y		= 0;
		m_ViewPort[nStage].width	= m_dwWidth;
		m_ViewPort[nStage].height	= m_dwHeight;
	}
	else
		// Set the viewport of the given stage to the given values
		memcpy(&m_ViewPort[nStage], pVP, sizeof(KPVIEWPORT));

	if ( (nStage > 3) || (nStage < 0 ) )
		nStage = 0;

	fAspectRatio = (float)(m_ViewPort[nStage].height) / m_ViewPort[nStage].width;

	// Build the projection matrices
	// Perspective
	if ( FAILED( CalcPerspProjMatrix(fFOV, fAspectRatio, &m_mProjP[nStage]) ) )
	{
		Log("InitStage: Unable to calculate perpective projection matrix!");
		return KP_FAIL;
	}

	// Orthogonal
	memset(&m_mProjO[nStage], 0, sizeof(D3DMATRIX));	// Fill the matrix with 0s
	m_mProjO[nStage]._11 = 2.0f / m_ViewPort[nStage].width;
	m_mProjO[nStage]._22 = 2.0f / m_ViewPort[nStage].height;
	m_mProjO[nStage]._33 = 1.0f / (m_fFar - m_fNear);
	m_mProjO[nStage]._43 = m_fNear / (m_fNear - m_fFar);
	m_mProjO[nStage]._44 = 1.0f;

	return KP_OK;

} // ! InitStage()


// Transform3Dto2D ////
///////////////////////
/*
	Transforms a point from the 3D world space to the 2D screen space.

	Params:
		vcPoint:	Pointer to KPVector object specifying the position in 3D world space
*/
POINT KPD3D::Transform3Dto2D(const KPVector &vcPoint)
{
	/*
		This function is very useful in case we want to use the object's screen space location 
		for any reason. For example HUD related information (targeting recangle etc)

		Since the GPU does not return the transformed values to us, we have to transform 
		manually.
	*/

	POINT point;									// The transformed point
	float fVPX, fVPY, fX, fY, fW, fWInv;			// Various values for computation
	DWORD dwWidth, dwHeight;						// Active viewport dimensions or full screen dimensions in case of TWOD

	// If the engine is in TWOD 2d mode, we use the whole surface of the screen
	if ( m_Mode == EMD_TWOD )
	{
		dwWidth		= m_dwWidth;
		dwHeight	= m_dwHeight;
	}
	// Otherwise we use the actual viewport's dimensions
	else
	{
		dwWidth		= m_ViewPort[m_nStage].width;
		dwHeight	= m_ViewPort[m_nStage].height;
	}


	// Math: http://www.codeguru.com/cpp/misc/misc/math/article.php/c10123__3/

	// Transform the coordinates using the ViewProj matrices to [-1.0f, 1.0f] dimensions, Z is not needed, the screen is 2d!
	fX = m_mViewProj._11*vcPoint.x	+  m_mViewProj._21*vcPoint.y  +  m_mViewProj._31*vcPoint.z + m_mViewProj._41;
	fY = m_mViewProj._12*vcPoint.x	+  m_mViewProj._22*vcPoint.y  +  m_mViewProj._32*vcPoint.z + m_mViewProj._42;
	fW = m_mViewProj._14*vcPoint.x	+  m_mViewProj._24*vcPoint.y  +  m_mViewProj._34*vcPoint.z + m_mViewProj._44;

	// At this point the transformed values are in [-1.0f, 1.0f] range to be independant
	// from screen or viewport resolutions.
	// We have to bring them back into the viewports coordinate system [0.0f, 1.0f]. 
	// To do this, we have to move the point:
	//
	// -1.0f <= POINT <= 1.0f 
	// 0.0f <= POINT +1.0f <= 2.0f
	// 0.0f * VPDIM <= (POINT + 1.0f)*VPDIM <= 2.0f * VPDIM
	// 0.0f <= (POINT + 1.0f)*VPDIM/2 <= 1.0f
	// ^ That's what we need!

	// Set the clipping values, width/2, height/2
	fVPX = float(dwWidth >> 1);
	fVPY = float(dwHeight >> 2);

	// Save a bit with precalculating the inverse of fW
	fWInv = 1.0f / fW;

	point.x = (long)( (1.0f + (fX * fWInv)) * fVPX );
	point.y = (long)( (1.0f + (fY * fWInv)) * fVPY );

	return point;

} // Transform3Dto2D Point

// Transform2Dto3d ////
///////////////////////
/*
	Transforms a 2D point in screen space into a ray in 3d world space.

	Params:
		point:		Pointer to a POINT type object specifying the 2D point in screen space.
		vcOrigin:	Address of a KPVECTOR object that will be used to return the ray's origin.
		vcDirection:Address of a KPVECTOR object that will be used to return the ray's direction.
*/
void KPD3D::Transform2Dto3D(const POINT &point, KPVector *vcOrigin, KPVector *vcDirection)
{
	// This function is especially useful for hit scan calculations.

	D3DMATRIX	*pView = NULL, *pProj = NULL;
	KPMatrix	mInvView;
	KPVector	vcScreen;
	DWORD		dwWidth, dwHeight;

	// If the engine uses TWOD 2d mode, use the full screen dimension and the 2d view matrix
	if ( m_Mode = EMD_TWOD )
	{
		dwWidth		= m_dwWidth;
		dwHeight	= m_dwHeight;
		pView		= &m_mView2D;
	}
	// Otherwise its Orthogonal or Perspective mode
	else
	{
		dwWidth		= m_ViewPort[m_nStage].width;
		dwHeight	= m_ViewPort[m_nStage].height;
		pView		= &m_mView3D;

		if ( m_Mode == EMD_PERSPECTIVE )
			pProj	= &m_mProjP[m_nStage];
		else
			pProj	= &m_mProjO[m_nStage];
	}

	// Now we have to bring the coordinates from the viewport/screen [0.0f, 1.0f] dimensions
	// back to [-1.0f, 1.0f] dimensions, practically have to do the inverse of Transform3Dto2D.

	vcScreen.x	= ( (point.x * 2.0f/dwWidth)  - 1.0f ) / pProj->_11;
	vcScreen.y	= ( (point.y * 2.0f/dwHeight) - 1.0f ) / pProj->_22;

	// Now we have to invert the view matrix too
	mInvView.InverseOf( *((KPMatrix*)&m_mView3D) );

	// Cast a ray from the screen into world space
	vcDirection->x = vcScreen.x*mInvView._11  +  vcScreen.y*mInvView._21  +  vcScreen.z*mInvView._31;
	vcDirection->y = vcScreen.x*mInvView._12  +  vcScreen.y*mInvView._22  +  vcScreen.z*mInvView._32;
	vcDirection->z = vcScreen.x*mInvView._13  +  vcScreen.y*mInvView._23  +  vcScreen.z*mInvView._33;

	// The origin is the translated position
	vcOrigin->x	= mInvView._41;
	vcOrigin->y	= mInvView._42;
	vcOrigin->z	= mInvView._43;

	// Ensure that it is a unit vector
	vcOrigin->Normalize();

} // ! Transform2Dto3D

// Set Ambient Light ////
/////////////////////////
//
// Set ambient light level to given values
void KPD3D::SetAmbientLight(float fR, float fG, float fB)
{
	// Render everything from the cache before changing lightning properties
	m_pVertexMan->ForcedFlushAll();

	m_pDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_COLORVALUE(fR, fG, fB, 1.0f));

} // ! SetAmbientLight

// CreateMyFont ////
//////////////////
/*
	Creates a D3DXFont object based on the specified parameters.

	Parameters:
		chType			: String value specifying the font type. Eg. "Arial" or "Times New Roman".
		nWeight			: Integer value specifying the weight of the font. 0 is the default ~700 is bold
		bItalic			: Boolean value specifying Italic style
		bUnderlined		: Boolean value specifying Underlines style
		bStrikeOut		: Boolean value specifying StrikeOut style
		dwSize			: DWORD value specifying the font size
		pFontID			: [OUT] Pointer to an integer type value receiving the newly created font's ID

	Returns:
		KP_OK			: upon success
		KP_FAIL			: upon failure
*/
HRESULT KPD3D::CreateMyFont(const char *chType, int nWeight, bool bItalic, bool bUnderlined, bool bStrikeOut, DWORD dwSize, UINT *pFontID)
{
	HRESULT hr;
	HDC		hDC;		// Handle to the device contect of an object
	int		nHeight;	// Font height

	if ( ! pFontID )
	{
		Log("CreateMyFont: Invalid pointer parameter to font id");
		return KP_INVALIDPARAM;
	}

	hDC		= GetDC( NULL );	// With NULL, retrieves the device contect of the whole screen

	// Scale our font based on screen size and DPI
	// LOGPIXELSY : Number of pixels per logical inch along the screen height. 
	//nHeight = (dwSize * GetDeviceCaps(hDC, LOGPIXELSY)) / 72; 
	nHeight = dwSize;

	// Don't need the device context anymore
	ReleaseDC(NULL, hDC);


	// Expand the font list size
	m_pFont = (LPD3DXFONT*)realloc(m_pFont, sizeof(LPD3DXFONT)*(m_numFonts+1));

	// Convert our GDI font into D3DX font
	hr = D3DXCreateFont(m_pDevice, nHeight, 0, nWeight, 0, bItalic, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
						DEFAULT_PITCH | FF_DONTCARE, chType, &m_pFont[m_numFonts]);


	if ( SUCCEEDED(hr) )
	{
		*pFontID = m_numFonts;
		++m_numFonts;

		return KP_OK;
	}
	else
	{
		Log("CreateMyFont: Unable to create font:");
		Log("CreateMyFont: Type:\t%s",		chType);
		Log("CreateMyFont: Size:\t%d",		(int)dwSize);
		Log("CreateMyFont: Weight:\t%s:",	nWeight);
		Log("CreateMyFont: Italic:\t%s",	(bItalic)?"true":"false");
		Log("CreateMyFont: Underline: %s",	(bUnderlined)?"true":"false");
		Log("CreateMyFont: StrikeOut: %s",	(bStrikeOut)?"true":"false");
		return KP_FAIL;
	}

} // ! CreateMyFont

// DrawTxt ////
//////////////////
HRESULT KPD3D::DrawTxt(UINT nFontID, int x, int y, UCHAR a, UCHAR r, UCHAR g, UCHAR b, char *chFormat, ...)
{
	RECT rect = { x, y, 100, 50 }; // Specifies the upper left corner of the text

	char chText[1024];			// Text
	char *pArgs;				// Pointer to the arguments of the function

	// Retrieve the optional parameters
	pArgs = (char*)&chFormat + sizeof(chFormat);

	// Combine the formatting text and the arguments
	vsprintf_s(chText, sizeof(chText), chFormat, pArgs);

	if ( nFontID >= m_numFonts )
	{
		Log("DrawTxt: Invalid font id:%d", nFontID);
		return KP_INVALIDPARAM;
	}

	// Calculate the bounding rectangle for the text 
	m_pFont[nFontID]->DrawText(NULL, chText, -1, &rect, DT_CALCRECT, 0);

	// Draw the text
	if ( ( m_pFont[nFontID]->DrawText(NULL, chText, -1, &rect, NULL, D3DCOLOR_ARGB(a,r,g,b)) ) == 0 )
	{
		Log("DrawTxt: Error drawing font! Id:%d, Text: \"%s\"", nFontID, chText);
		return KP_FAIL;
	}

	return KP_OK;

} // ! DrawMyText


void KPD3D::LogDeviceCaps(D3DCAPS9 *pCaps) {

	Log("");
	Log("ADAPTERINFO:");

	if (pCaps->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		Log("   adapter features hardware TnL");

		if (pCaps->DevCaps & D3DDEVCAPS_PUREDEVICE)
			Log("   (pure device possible)");
		else
			Log("   (no pure device possible)");
	}
	else
		Log("   adapter only features software TnL");

	// TEXTURE STUFF
	Log("   max. texture stages: %d" , pCaps->MaxTextureBlendStages);
	Log("   max. textures for single pass: %d" , pCaps->MaxSimultaneousTextures);
	Log("   max. texture width: %d" , pCaps->MaxTextureWidth);
	Log("   max. texture height: %d" , pCaps->MaxTextureHeight);

	// VERTEX SHADER VERSION
	if (pCaps->VertexShaderVersion < D3DVS_VERSION(1,1) )
		Log("   Vertex Shader Version 1.0");
	else if (pCaps->VertexShaderVersion < D3DVS_VERSION(2,0) )
		Log("   Vertex Shader Version 1.1");
	else  
		Log("   Vertex Shader Version 2.0 or better");

	// PIXEL SHADER VERSION
	if (pCaps->PixelShaderVersion < D3DPS_VERSION(1,1) )
		Log("   Pixel Shader Version 1.0");
	else if (pCaps->PixelShaderVersion < D3DPS_VERSION(1,2) )
		Log("   Pixel Shader Version 1.1");
	else if (pCaps->PixelShaderVersion < D3DPS_VERSION(1,3) )
		Log("   Pixel Shader Version 1.2");
	else if (pCaps->PixelShaderVersion < D3DPS_VERSION(1,4) )
		Log("   Pixel Shader Version 1.3");
	else if (pCaps->PixelShaderVersion < D3DPS_VERSION(2,0) )
		Log("   Pixel Shader Version 1.4");
	else
		Log("   Pixel Shader Version 2.0 or better");
	Log("");


	// SCREEN RESOLUTION, FORMAT AND STUFF
	Log("DISPLAYINFO:");

	LPDIRECT3DSURFACE9 pDepthStencil=NULL;
	D3DSURFACE_DESC desc;
	D3DFORMAT Format = D3DFMT_UNKNOWN;
	D3DDISPLAYMODE mode = {0,0,0,D3DFMT_UNKNOWN};

	if (FAILED(m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode)))
		Log("Error: IDirect3D::GetAdapterDisplayMode failed");

	if (FAILED(m_pDevice->GetDepthStencilSurface(&pDepthStencil)))
		Log("Error: IDirect3DDevice::GetDepthStencilSurface failed");
	else {
		if (FAILED(pDepthStencil->GetDesc(&desc)))
		{
			Log("Error: IDirect3DSurface::GetDesc failed");
			Format = D3DFMT_UNKNOWN;
		}
	}

	Log("   Resolution: %dx%d", mode.Width, mode.Height);
	Log("   Refreshrate: %d", mode.RefreshRate);

	// backbuffer format
	switch(mode.Format)
	{
	  case D3DFMT_A2R10G10B10: Log("   Pixelformat: D3DFMT_A2R10G10B10"); break;
	  case D3DFMT_A8R8G8B8:    Log("   Pixelformat: D3DFMT_A8R8G8B8"); break;
	  case D3DFMT_X8R8G8B8:    Log("   Pixelformat: D3DFMT_X8R8G8B8"); break;
	  case D3DFMT_A1R5G5B5 :   Log("   Pixelformat: D3DFMT_A1R5G5B5"); break;
	  case D3DFMT_X1R5G5B5:    Log("   Pixelformat: D3DFMT_X1R5G5B5"); break;
	  case D3DFMT_R5G6B5:      Log("   Pixelformat: D3DFMT_R5G6B5");   break;
	  default: break;
	} // switch

	// depth buffer format
	switch(desc.Format)
	{
	  case D3DFMT_D16_LOCKABLE:  Log("   Depth/Stencil: D3DFMT_D16_LOCKABLE");  break;
	  case D3DFMT_D32F_LOCKABLE: Log("   Depth/Stencil: D3DFMT_D32F_LOCKABLE"); break;
	  case D3DFMT_D32:     Log("   Depth/Stencil: D3DFMT_D32");     break;
	  case D3DFMT_D15S1:   Log("   Depth/Stencil: D3DFMT_D15S1");   break;
	  case D3DFMT_D24S8:   Log("   Depth/Stencil: D3DFMT_D24S8");   break;
	  case D3DFMT_D24X8:   Log("   Depth/Stencil: D3DFMT_D24X8");   break;
	  case D3DFMT_D24X4S4: Log("   Depth/Stencil: D3DFMT_D24X4S4"); break;
	  case D3DFMT_D24FS8:  Log("   Depth/Stencil: D3DFMT_D24FS8");  break;
	  case D3DFMT_D16:     Log("   Depth/Stencil: D3DFMT_D16");     break;
	  default: break;
	} // switch

	pDepthStencil->Release();
	Log("");

} // ! LogDeviceCaps

void KPD3D::LogCpuCaps(CPUINFO *pInfo)
{
	Log("");
	Log("CPUINFO:");
	Log("\tVendor Name:\t%s", pInfo->vendorName);
	Log("\tModel Name:\t%s", pInfo->modelName);
	Log("\tFamily:\t\t%d", pInfo->Family);
	Log("\tModel:\t\t%d", pInfo->Model);
	Log("\tStepping:\t%d", pInfo->Stepping);

	Log("");

	Log("\tCPU SIMD SUPPORT:");
	Log("\t\tCPU_FEATURE_MMX:\t%s",		(pInfo->Feature & CPU_FEATURE_MMX)?"Yes":"No");
	Log("\t\tCPU_FEATURE_SSE:\t%s",		(pInfo->Feature & CPU_FEATURE_SSE)?"Yes":"No");
	Log("\t\tCPU_FEATURE_SSE2:\t%s",	(pInfo->Feature & CPU_FEATURE_SSE2)?"Yes":"No");
	Log("\t\tCPU_FEATURE_SSE3:\t%s",	(pInfo->Feature & CPU_FEATURE_SSE3)?"Yes":"No");
	Log("\t\tCPU_FEATURE_SSSE3:\t%s",	(pInfo->Feature & CPU_FEATURE_SSSE3)?"Yes":"No");
	Log("\t\tCPU_FEATURE_SSE41:\t%s",	(pInfo->Feature & CPU_FEATURE_SSE41)?"Yes":"No");
	Log("\t\tCPU_FEATURE_SSE42:\t%s",	(pInfo->Feature & CPU_FEATURE_SSE42)?"Yes":"No");
	Log("\t\tCPU_FEATURE_3DNOW:\t%s",	(pInfo->Feature & CPU_FEATURE_3DNOW)?"Yes":"No");
	Log("\t\tCPU_FEATURE_3DNOWEX:\t%s",	(pInfo->Feature & CPU_FEATURE_3DNOWEX)?"Yes":"No");
	Log("\t\tCPU_FEATURE_MMXEX:\t%s",	(pInfo->Feature & CPU_FEATURE_MMXEX)?"Yes":"No");

	Log("");

	/*
	Log("\tOS SIMD SUPPORT:");
	Log("\t\tCPU_FEATURE_MMX: %s",		(pInfo->OS_Support & CPU_FEATURE_MMX)?"Yes":"No");
	Log("\t\tCPU_FEATURE_SSE: %s",		(pInfo->OS_Support & CPU_FEATURE_SSE)?"Yes":"No");
	Log("\t\tCPU_FEATURE_SSE2: %s",		(pInfo->OS_Support & CPU_FEATURE_SSE2)?"Yes":"No");
	Log("\t\tCPU_FEATURE_SSE3: %s",		(pInfo->OS_Support & CPU_FEATURE_SSE3)?"Yes":"No");
	Log("\t\tCPU_FEATURE_SSSE3: %s",	(pInfo->OS_Support & CPU_FEATURE_SSSE3)?"Yes":"No");
	Log("\t\tCPU_FEATURE_SSE41: %s",	(pInfo->OS_Support & CPU_FEATURE_SSE41)?"Yes":"No");
	Log("\t\tCPU_FEATURE_SSE42: %s",	(pInfo->OS_Support & CPU_FEATURE_SSE42)?"Yes":"No");
	Log("\t\tCPU_FEATURE_3DNOW: %s",	(pInfo->OS_Support & CPU_FEATURE_3DNOW)?"Yes":"No");
	Log("\t\tCPU_FEATURE_3DNOWEX: %s",	(pInfo->OS_Support & CPU_FEATURE_3DNOWEX)?"Yes":"No");
	Log("\t\tCPU_FEATURE_MMXEX: %s",	(pInfo->OS_Support & CPU_FEATURE_MMXEX)?"Yes":"No");
	*/

} // ! LogCpuCaps

