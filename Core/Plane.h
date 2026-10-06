#pragma once

#include <Core/Utility.h>

#include "Vec3.h"
#include "Ray.h"

namespace xtm
{

	enum PlaneClassify
	{
		POSITIVE_PLANE_SIDE = 1,
		NEGATIVE_PLANE_SIDE = 2,
		CROSSING_PLANE = 3
	};
	
	template <class T>
	class Plane  
	{
	public:
		inline Plane() {}

		inline Plane( T A, T B, T C, T D ); //TODO Deprecate ?
		inline Plane( const TVec3<T>& normal, T D );
		inline Plane( const TVec3<T>& normal, const TVec3<T>& point );
		inline Plane( const TVec3<T>& A, const TVec3<T>& B, const TVec3<T>& C );

		inline T SignedDistance( const TVec3<T>& p ) const;
		
		inline void Normalize();
		inline void Flip();
		
		inline bool IntersectRay(	const Ray<T>& ray,
									TVec3<T>& intersection,
									T& dist ) const;

		inline TVec3<T> ProjectPointOnPlane( const TVec3<T>& _point ) const;
		inline TVec3<T> MirrorPoint( const TVec3<T>& _point ) const;

	public:
		TVec3<T>	m_Normal;
		T			m_Dist;

	};


	//*** Implementation ***

	//TODO move in Plane.inl

	template <class T>
	inline Plane<T>::Plane( T A, T B, T C, T D )
		:
		m_Normal( A, B, C ),
		m_Dist(D)
	{
	}

	template <class T>
	inline Plane<T>::Plane( const TVec3<T>& normal, T D )
		:
		m_Normal( normal ),
		m_Dist(D)
	{
		DBG_CHECK( m_Normal.IsNormalized() );
	}

	template <class T>
	inline Plane<T>::Plane( const TVec3<T>& normal, const TVec3<T>& point )
		:
		m_Normal( normal ),
		m_Dist( -Dot( normal, point ) )
	{
		DBG_CHECK( m_Normal.IsNormalized() ); 
	}

	template <class T>
	inline Plane<T>::Plane( const TVec3<T>& A, const TVec3<T>& B, const TVec3<T>& C )
	{
		m_Normal = Cross( B-A, C-A );
		m_Normal.Normalize();
		
		m_Dist = - Dot( A, m_Normal );
	}

	template <class T>
	inline void Plane<T>::Normalize()
	{
		T rcpNorm = (T)1.0 / m_Normal.Length();

		m_Normal *= rcpNorm;

		m_Dist *= (T)rcpNorm; //TODO not sure...
	}

	template <class T>
	inline void Plane<T>::Flip()
	{
		m_Normal *= -1.0f;
		m_Dist = - m_Dist;
	}

	template <class T>
	inline T Plane<T>::SignedDistance( const TVec3<T>& p ) const
	{
		return Dot( m_Normal, p ) + m_Dist;
	}
	
	template <class T>
	inline bool Plane<T>::IntersectRay(	const Ray<T>& ray,
										TVec3<T>& intersection,
										T& dist ) const

	{
		T NdotRayDir = Dot( m_Normal, ray.m_dir );
		
		if( !IsEpsilonNull( NdotRayDir ) )
		{
			dist = - SignedDistance( ray.m_origin ) / NdotRayDir;

			if( dist < (T)0.0 )
				return false;
						       
			intersection = ray.m_origin + ray.m_dir * dist;
			
			//assert( fabs(SignedDistance( intersection )) < 0.00001f );
			
			return true;
		}
		else
		{
			return false;
		}

	}


	template <class T>
	inline TVec3<T> Plane<T>::ProjectPointOnPlane( const TVec3<T>& _point ) const
	{
		return _point - m_Normal * SignedDistance( _point );
	}

	template <class T>
	inline TVec3<T> Plane<T>::MirrorPoint( const TVec3<T>& _point ) const
	{
		return _point - m_Normal * ( SignedDistance( _point ) * (T)2.0 );
	}

	/*
	class CStream;
	CStream& operator>>( CStream& stream, Plane& p );
	CStream& operator<<( CStream& stream, const Plane& p );
	*/
}

