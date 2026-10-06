#pragma once

#include "Vec3.h"
#include <Core/Debug.h>


namespace xtm
{
	//template <class T> class TVec3;
	template <class T> class TMat44;

	//typedef TVec3<float> Vec3;
}

namespace xtm
{

	template <class T>
	class Ray
	{
	 public:		
		inline Ray() {}
		
		inline Ray(	const TVec3<T>& _origin, const TVec3<T>& _dir ) : m_origin(_origin), m_dir(_dir) {}

		inline Ray( const Ray<float>& _ray ) :  m_origin(_ray.m_origin), m_dir(_ray.m_dir) {}
		inline Ray( const Ray<double>& _ray ) :  m_origin(_ray.m_origin), m_dir(_ray.m_dir) {}

		// u && v in [0..1]
		inline void SetRayOnSphere( float u, float v );
		
		// u && v in [0..1]
		inline void SetRayOnHemisphere( float u, float v,
										const Vec3& hemisphereDir );
		
		//Assume m_dir is normalized
		inline T SquaredMinimumDistanceFromPoint( const TVec3<T>& point ) const;

		//Assume m_dir is normalized
		inline T MinimumDistanceFromPoint( const TVec3<T>& point ) const;

		inline void Transform( const TMat44<T>& mat );
	public:
		TVec3<T> m_origin, m_dir;
	};
}

#include "Mat44.h"

namespace xtm
{
	#include "Ray.inl"
}
