#pragma once

#include <Core/Vec3.h>
#include "Plane.h"

	template <class T> class OBBox;
	template <class T> class AABBox;

	template <class T> class TMat44;
	template <class T> class Ray;
	
	template <class T>
	class BSphere
	{
	public:
		inline BSphere() {}
		inline BSphere( const TVec3<T>& _center, T _radius ) : m_Center(_center), m_Radius(_radius), m_SquaredRadius(_radius*_radius) {}

		inline	void			Set( const TVec3<T>& _center, T _radius );
		inline	void			Set( const BSphere<T>& sphere, const TMat44<T>& transform );
	
		inline	void			Set( const OBBox<T>& obbox );
		inline	void			Set( const AABBox<T>& aabbox );
		

		inline	float			ComputeVolume() const;
		inline	float			ComputeSurfaceArea() const;

		inline	PlaneClassify	Classify( const Plane<T>& plane ) const;
		
		inline	bool			RayHitTest( const Ray<T>& ray ) const;
		inline	bool			RayHitTest( const Ray<T>& ray, float _maxRayLength ) const;
		inline	bool			RayHitTest( const Ray<T>& ray, float _maxRayLength, TVec3<T>& _hitPos, TVec3<float>& _hitNormal, T& _dist ) const;
		inline	bool			RayHitTest( const Ray<T>& ray, float _maxRayLength, TVec3<T>& _hitPos, TVec3<double>& _hitNormal, T& _dist ) const;
		inline	T				RayHitDist( const Ray<T>& ray ) const; //Return the distance to the hit ( < 0 if no hit )

	public:
		TVec3<T>		m_Center;

		T				m_Radius,
						m_SquaredRadius;

	};

	template class BSphere<float>;
	template class BSphere<double>;

#include <Core/Mat44.h>
#include <Core/Ray.h>
#include <Core/AABBox.h>

#include "BSphere.inl"
	/*
	class CStream;
	CStream& operator>>( CStream& stream, CBSphere& s );
	CStream& operator<<( CStream& stream, const CBSphere& s );
*/



