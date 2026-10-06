#pragma once


#include <Core/ConvexHull.h>
//#include <Core/Mat44.h>
#include <Core/Vec2.h>
#include <Core/Vec3.h>

namespace xtm
{
//	template <typename T> class Vec2;
	template <class T> class OBBox;
	template <class T> class AABBox;
	template <class T> class BSphere;

	struct ZBias
	{
		float	ConstantOffset, //Move vertex in Z by a fixed amount
				SlopeBased, //Move vertex in Z depending on the screenspace slope
				AlongNormal; //Move vertex a certain amount along its normal
	};

	class Camera
	{
	 public:
		Camera();
		virtual ~Camera() {}
		
		inline const	Mat44d&			GetViewMatrix() const;

		inline			void			SetViewMatrix( const Mat44d& _viewMatrix ) const;


		inline const	Mat44d&			GetProjectionMatrix() const { return m_matrixProjection; }

						void			SetProjectionMatrix( const Mat44d& _projMatrix );

						void			SetProjectionMatrix(float verticalFov, //Vertical fov in degree
															float aspectRatio,
															float zNear,
															float zFar,
															bool  bLandscapeMode = false );

		inline			float			GetAspectRatio() const			{ return m_aspectRatio; }
		inline			float			GetHorizontalFov() const		{ return m_horizontalFov; }

		inline			float			GetAperture() const				{ return m_Aperture; } //(caution: radius, not diameter !!!)
		inline			void			SetAperture( float _radius )	{ m_Aperture = _radius; } 

		inline			float			GetFocalDistance() const		{ return m_FocalDistance; }
		inline			void			SetFocalDistance( float _dist )	{ m_FocalDistance = _dist; }


		inline			float			GetZNear() const			{ return m_zNear; }
		inline			float			GetZFar() const				{ return m_zFar; }

						void			SetLogarithmicZBufferConstants( float _c ); //Used for large view distances

		//For shadow mapping
		inline			ZBias&			GetZBias()					{ return m_zBias; }
		inline const	ZBias&			GetZBias() const			{ return m_zBias; }

		const			Vec3d&			GetPosition() const;

		inline			Vec3			GetRightDirection() const;
		inline			Vec3			GetUpDirection() const;
		inline			Vec3			GetForwardDirection() const;

						Ray<float>		GetRay( float u, float v ) const;
		
		const			ConvexHull<float>&		GetFrustum() const;

						//return an array to 8 corners
		const			Vec3d*			GetFrustumCorners() const;
				//TODO	bool			GetFrustumCorners( Vec3d* _pCorners ) const;

						//return ZNear, ZFar and 2 two constants used for logarithmic z buffer		
		inline			Vec4			GetZBufferDistribution() const { return Vec4( m_zNear, m_zFar, m_zLogDistribution, m_zLogConstant ); } 

						//Return true if the point is in the view fustrum
						bool			ProjectPointInScreenSpace( const Vec3d& _wsPos, Vec3& _ssPos ) const;
		
		virtual			void			Update( float timeStep ) {}

	 protected:
		virtual			void			UpdateViewMatrix() const {}
		inline			void			SetViewMatrixDirty() const;
	
	private:
		Mat44d				m_matrixProjection;
		float				m_aspectRatio,
							m_horizontalFov,
							m_zNear,
							m_zFar,
							m_xScale,
							m_yScale;

		ZBias				m_zBias;

		float 				m_zLogDistribution, //Logarithmic ZBuffer
							m_zLogConstant;

	
		mutable Mat44d		m_viewMatrix;
		mutable Vec3d		m_camPos;
		mutable ConvexHull<float>	m_frustum;
		mutable Vec3d		m_FrustumCorners[8];
		mutable bool		m_bViewMatrixDirty,
							m_bCamPosDirty, 
							m_bFrustumDirty,
							m_bFrustumCornersDirty;

	public:
	
		//Depth of field
	
		float				m_Aperture;
		float				m_FocalDistance;
		float				m_FocalLength;

	};



	class CameraArcBall : public Camera
	{		
	public:
		CameraArcBall();
		virtual ~CameraArcBall() {}

				void	Set( const Vec3d& center, float distance, float u, float v );
				void	SetCenter( const Vec3d& center );
				void	SetDistance( float distance );
				void	Rotate( float dU, float dV );
				void	Pan( float dU, float dV );
				void	Zoom( float zoomFactor, float minDist=0.0001f );
				
				void	FocusOnBoundingSphere( const BSphere<float>& bsphere );
				void	FocusOnBoundingBox( const AABBox<float>& bbox );
				void	FocusOnBoundingBox( const OBBox<float>& bbox );

	protected:
		virtual void	UpdateViewMatrix() const;
	
	private:
		float		m_u, m_v;
		float		m_targetDist;
		Vec3d		m_targetPos;		

	};


	class CameraLookAt : public Camera
	{
	public:
		CameraLookAt();
		virtual ~CameraLookAt() {}

		void	SetPosition( const Vec3d& _pos );
		void	SetTarget( const Vec3d& _target );
		void	SetUp( const Vec3d& _up );

	//	void	FocusOnBoundingSphere( const BSphere& bsphere );
	//	void	FocusOnBoundingBox( const AABBox& bbox );
	//	void	FocusOnBoundingBox( const OBBox& bbox );

	protected:
		virtual void	UpdateViewMatrix() const;
	private:
		Vec3d		m_Pos, m_Target, m_Up;		

	};

	//-------

	inline void Camera::SetViewMatrixDirty() const
	{
		m_bViewMatrixDirty = true;
		m_bCamPosDirty = true;
		m_bFrustumDirty = true;
	}

	inline void Camera::SetViewMatrix( const Mat44d& viewMatrix ) const
	{
		m_bViewMatrixDirty = false;
		m_bCamPosDirty = true;
		m_bFrustumDirty = true;
		m_bFrustumCornersDirty = true;
		m_viewMatrix = viewMatrix;
	}

	inline const Mat44d& Camera::GetViewMatrix() const
	{
		if( m_bViewMatrixDirty )
		{
			UpdateViewMatrix();
			m_bViewMatrixDirty = false;
		}

		return m_viewMatrix;
	}

	inline Vec3 Camera::GetRightDirection() const
	{
		const Mat44d& v = GetViewMatrix();
		return Vec3( (float)v._11, (float)v._21, (float)v._31 );
	}

	inline Vec3 Camera::GetUpDirection() const
	{
		const Mat44d& v = GetViewMatrix();
		return Vec3( (float)v._12, (float)v._22, (float)v._32 );
	}

	inline Vec3 Camera::GetForwardDirection() const
	{
		const Mat44d& v = GetViewMatrix();
		return Vec3( (float)v._13, (float)v._23, (float)v._33 );
	}

}
