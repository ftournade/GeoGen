//#include "stdafx.h"
#include "Camera.h"

#include <Core/Vec2.h>
//#include <Core/Vec4.h>
#include <Core/OBBox.h>
#include <Core/BSphere.h>


	Camera::Camera() :
		m_bCamPosDirty(true),
		m_bFrustumDirty(true),
		m_bFrustumCornersDirty(true),
		m_bViewMatrixDirty(true),
		m_Aperture(10.0f),
		m_FocalDistance( 10.0f ),
		m_FocalLength( 0.03f )
	{
		m_zBias.ConstantOffset = 0.01f;
		m_zBias.SlopeBased = 0.0f;
		m_zBias.AlongNormal = -0.01f;

		SetProjectionMatrix( 50.0f, 4.0f / 3.0f, 0.2f, 500.0f );
		SetLogarithmicZBufferConstants( 2.0f );
	}

	void Camera::SetLogarithmicZBufferConstants( float _c )
	{
		float offset = 1.0f;

		m_zLogDistribution = _c;
		
		m_zLogConstant = 1.0f / logf( m_zFar * _c + offset );
	}


	void Camera::SetProjectionMatrix( const Mat44d& _projMatrix )
	{
		m_bCamPosDirty = true;
		m_bFrustumDirty = true;
		m_bFrustumCornersDirty = true;

		double verticalFov;
		double aspectRatio;
		double zNear, zFar;

		bool isOrthographic;

		_projMatrix.RetrieveProjectionParameters( isOrthographic, verticalFov, aspectRatio, zNear, zFar );
		
		m_aspectRatio = (float)aspectRatio;
		m_zNear = (float)zNear;
		m_zFar = (float)zFar;

		m_horizontalFov = (float)verticalFov / m_aspectRatio;

		m_matrixProjection = _projMatrix;
		
		m_xScale = (float)m_matrixProjection._11;
		m_yScale = (float)m_matrixProjection._22;

		SetLogarithmicZBufferConstants( m_zLogDistribution );
	}

	void Camera::SetProjectionMatrix(	float verticalFov,
										float aspectRatio,
										float zNear,
										float zFar,
										bool  bLandscapeMode )
	{

		m_bCamPosDirty = true;
		m_bFrustumDirty = true;
		m_bFrustumCornersDirty = true;

		m_horizontalFov = DegToRad( verticalFov / aspectRatio );

		m_aspectRatio = aspectRatio;

		m_zNear = zNear;
		m_zFar = zFar;

		m_matrixProjection.MakePerspectiveProjection( DegToRad( verticalFov ), aspectRatio, zNear, zFar );

		m_xScale = (float)m_matrixProjection._11;
		m_yScale = (float)m_matrixProjection._22;

		if( bLandscapeMode )
		{
			Mat44d rotate;
			rotate.MakeRotationZ( DegToRad( -90.0f ) );
			m_matrixProjection = rotate * m_matrixProjection;
		}

		SetLogarithmicZBufferConstants( m_zLogDistribution );
	}

	const Vec3d& Camera::GetPosition() const
	{
		if( !m_bCamPosDirty )
			return m_camPos;

		Mat44d viewInv( GetViewMatrix() );
		viewInv.Inverse();

		m_camPos.x = viewInv._41;
		m_camPos.y = viewInv._42;
		m_camPos.z = viewInv._43;

		m_bCamPosDirty = false;

		return m_camPos;
	}

	Ray<float> Camera::GetRay( float u, float v ) const
	{
#if 1
		const Vec3d* pCorners = GetFrustumCorners();

		Vec3d screenPixel = Lerp(	Lerp( pCorners[3], pCorners[2], (scalar)u ),
									Lerp( pCorners[0], pCorners[1], (scalar)u ), (scalar)v );

		Ray<float> ray;
		ray.m_origin = ToVec3( GetPosition() );
		ray.m_dir = ToVec3( screenPixel - ToVec3d( ray.m_origin ) );
		ray.m_dir.Normalize();
		
		return ray;

#else
		Vec3d p;
		p.x =   ( scalar(2.0) * scalar(u) - scalar(1.0) ) / scalar(m_xScale);
		p.y = - ( scalar(2.0) * scalar(v) - scalar(1.0) ) / scalar(m_yScale);
		p.z =  scalar(1.0);
		
		Mat44d m( GetViewMatrix() );

		m.Inverse();

		Ray ray;

		ray.m_origin.x = m._41;
		ray.m_origin.y = m._42;
		ray.m_origin.z = m._43;

		ray.m_dir.x = p.x*m._11 + p.y*m._21 + p.z*m._31;
		ray.m_dir.y = p.x*m._12 + p.y*m._22 + p.z*m._32;
		ray.m_dir.z = p.x*m._13 + p.y*m._23 + p.z*m._33;

		DBG_CHECK( ray.m_dir.IsNormalized() );
		ray.m_dir.Normalize();
#endif
		return ray;
	}


	const ConvexHull<float>& Camera::GetFrustum() const
	{
		if( !m_bFrustumDirty )
			return m_frustum;

		m_frustum.m_Planes.resize( 6 );

		Mat44 viewProj( ToMat44( GetViewMatrix() * GetProjectionMatrix() ) );

		// Near clipping plane
		m_frustum.m_Planes[0].m_Normal.x = viewProj._13;
		m_frustum.m_Planes[0].m_Normal.y = viewProj._23;
		m_frustum.m_Planes[0].m_Normal.z = viewProj._33;
		m_frustum.m_Planes[0].m_Dist = viewProj._43;
		m_frustum.m_Planes[0].Normalize();

		// Far clipping plane
		m_frustum.m_Planes[1].m_Normal.x = viewProj._14 - viewProj._13;
		m_frustum.m_Planes[1].m_Normal.y = viewProj._24 - viewProj._23;
		m_frustum.m_Planes[1].m_Normal.z = viewProj._34 - viewProj._33;
		m_frustum.m_Planes[1].m_Dist = viewProj._44 - viewProj._43;
		m_frustum.m_Planes[1].Normalize();

		// Left clipping plane
		m_frustum.m_Planes[2].m_Normal.x = viewProj._14 + viewProj._11;
		m_frustum.m_Planes[2].m_Normal.y = viewProj._24 + viewProj._21;
		m_frustum.m_Planes[2].m_Normal.z = viewProj._34 + viewProj._31;
		m_frustum.m_Planes[2].m_Dist = viewProj._44 + viewProj._41;
		m_frustum.m_Planes[2].Normalize();

		// Right clipping plane
		m_frustum.m_Planes[3].m_Normal.x = viewProj._14 - viewProj._11;
		m_frustum.m_Planes[3].m_Normal.y = viewProj._24 - viewProj._21;
		m_frustum.m_Planes[3].m_Normal.z = viewProj._34 - viewProj._31;
		m_frustum.m_Planes[3].m_Dist = viewProj._44 - viewProj._41;
		m_frustum.m_Planes[3].Normalize();

		// Bottom clipping plane
		m_frustum.m_Planes[4].m_Normal.x = viewProj._14 + viewProj._12;
		m_frustum.m_Planes[4].m_Normal.y = viewProj._24 + viewProj._22;
		m_frustum.m_Planes[4].m_Normal.z = viewProj._34 + viewProj._32;
		m_frustum.m_Planes[4].m_Dist = viewProj._44 + viewProj._42;
		m_frustum.m_Planes[4].Normalize();

		// Top clipping plane
		m_frustum.m_Planes[5].m_Normal.x = viewProj._14 - viewProj._12;
		m_frustum.m_Planes[5].m_Normal.y = viewProj._24 - viewProj._22;
		m_frustum.m_Planes[5].m_Normal.z = viewProj._34 - viewProj._32;
		m_frustum.m_Planes[5].m_Dist = viewProj._44 - viewProj._42;
		m_frustum.m_Planes[5].Normalize();


		m_bFrustumDirty = false;

		return m_frustum;
	}

	const Vec3d* Camera::GetFrustumCorners() const
	{
		if( !m_bFrustumCornersDirty )
			return m_FrustumCorners;

		Mat44d viewProj( GetViewMatrix() * GetProjectionMatrix() );
		viewProj.GetFrustumCorners( m_FrustumCorners );
		
		m_bFrustumCornersDirty = false;

		return m_FrustumCorners;
	}

	bool Camera::ProjectPointInScreenSpace( const Vec3d& _wsPos, Vec3& _ssPos ) const
	{
		Vec4d tmp2( GetViewMatrix().Transform( Vec4d( _wsPos, (scalar)1.0 ) ) );
		Vec4d tmp3( GetProjectionMatrix().Transform( tmp2 ) );

		//TODO check division by zero

		scalar rcpw = (scalar)1.0 / tmp3.w;
		tmp3.x *= rcpw;
		tmp3.y *= rcpw;
		tmp3.z *= rcpw;

		_ssPos.x = (float)tmp3.x * 0.5f + 0.5f;
		_ssPos.y = (float)tmp3.y * 0.5f + 0.5f;
		_ssPos.z = (float)tmp3.z;

		return	(tmp3.x >= -1.0f) && (tmp3.x <= 1.0f) &&
				(tmp3.y >= -1.0f) && (tmp3.y <= 1.0f) &&
				(tmp3.z > 0.0f);// && (tmp3.z <= 1.0f);
	}

	CameraArcBall::CameraArcBall()
	{
		m_u = 0.0f;
		m_v = 0.0f;

		m_targetDist = 1.0f;
		m_targetPos = Vec3d::Origin;

	}


	void CameraArcBall::Set(const Vec3d& center,float distance,float u, float v)
	{
		SetViewMatrixDirty();

		m_targetPos = center;
		m_targetDist = distance;

		m_u=u;
		m_v=v;
	}

	void CameraArcBall::SetCenter(const Vec3d& center)
	{
		SetViewMatrixDirty();

		m_targetPos = center;
	}

	void CameraArcBall::SetDistance(float distance)
	{
		SetViewMatrixDirty();

		m_targetDist=distance;
	}

	void CameraArcBall::Rotate(float dU, float dV)
	{
		SetViewMatrixDirty();

		m_u += dU;
		m_v += dV;
	}

	void CameraArcBall::Pan(float dU, float dV)
	{
		SetViewMatrixDirty();

		m_targetPos += ToVec3d( GetRightDirection() * dU * m_targetDist );
		m_targetPos += ToVec3d( GetUpDirection() * dV * m_targetDist );

	}


	void CameraArcBall::Zoom(float zoomFactor, float minDist)
	{
		SetViewMatrixDirty();

		if( zoomFactor == 0.0f )
			m_targetDist = FLT_MAX;

		m_targetDist /= zoomFactor;

		if(m_targetDist < minDist)
			m_targetDist=minDist;
	}


	void CameraArcBall::UpdateViewMatrix() const
	{
		//TODO (from wikipedia)
		//x = r * sin(theta) * cos(phi)
		//y = r * sin(theta) * sin(phi)
		//z = r * cos(theta)

		Mat44d matRot1,matRot2;

		matRot1.MakeRotationX( m_v );
		matRot2.MakeRotationY( m_u );

		matRot1 = matRot1 * matRot2;

		Vec3d unitPos( matRot1.TransformDirection( Vec3d::ZAxis ) );
		Vec3d up( matRot1.TransformDirection( Vec3d::YAxis ) );

		Vec3d pos( m_targetPos + unitPos * m_targetDist );

		Mat44d viewMatrix;
		viewMatrix.MakeLookAt( pos, m_targetPos, up );

		SetViewMatrix( viewMatrix );
	}



	void CameraArcBall::FocusOnBoundingBox( const OBBox<float>& bbox )
	{
		BSphere<float> bsphere;
		bsphere.Set( bbox );
		FocusOnBoundingSphere( bsphere );
	}

	void CameraArcBall::FocusOnBoundingBox( const AABBox<float>& bbox )
	{
		//#pragma warning ("cul")
       // FocusOnBoundingBox( OBBox( bbox, Mat44::Identity ) );
	}

	void CameraArcBall::FocusOnBoundingSphere( const BSphere<float>& bsphere )
	{

		float aspectRatio = GetAspectRatio();

		float fov = GetHorizontalFov();

		if( aspectRatio > 1.0f )
			fov /= aspectRatio; //Take vertical fov instead


		float distance = bsphere.m_Radius / Sin( fov / 2.0f );

		//distance *= 1.3f;

		SetCenter( ToVec3d( bsphere.m_Center ) );
		SetDistance( distance );
	}


	CameraLookAt::CameraLookAt() :
		m_Pos( Vec3d::XAxis ),
		m_Target( Vec3d::Origin ),
		m_Up( Vec3d::YAxis )
	{
	}

	void CameraLookAt::SetPosition( const Vec3d& _pos )
	{
		SetViewMatrixDirty();

		m_Pos = _pos;
	}

	void CameraLookAt::SetTarget( const Vec3d& _target )
	{
		SetViewMatrixDirty();

		m_Target = _target;
	}

	void CameraLookAt::SetUp( const Vec3d& _up )
	{
		SetViewMatrixDirty();

		m_Up = _up;
	}

	void CameraLookAt::UpdateViewMatrix() const
	{
		Mat44d viewMatrix;
		viewMatrix.MakeLookAt( m_Pos, m_Target, m_Up );

		SetViewMatrix( viewMatrix );
	}
