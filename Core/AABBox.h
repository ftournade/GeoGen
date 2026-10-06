#pragma once
#ifndef XTM_AABBOX_H
#define XTM_AABBOX_H


#include <Core/Vec3.h>
#include <Core/Plane.h>
#include <Core/OBBox.h>

#include <Core/Utility.h>

namespace xtm
{
	template <class T>
	class Ray;

	//class OBBox;
	template <class T>
	class AABBox
	{
	 public:
		
		TVec3<T> m_min, m_max;

		inline AABBox() {}
		inline AABBox( const TVec3<T>& _min, const TVec3<T>& _max ) : m_min(_min), m_max(_max) {}

		void Set( const Vec3* _points, u32 _numPoints );
		void Set( const Vec3d* _points, u32 _numPoints );

		//void Set( const OBBox& obbox );

		inline void Set( const TVec3<T>& _min, const TVec3<T>& _max )		{ m_min = _min; m_max = _max; }

		TVec3<T> GetCorner( u32 i ) const;
		void GetCorners( TVec3<T>* _pCorners ) const; //Vec3d[8]
		
		void Clear();
		inline bool IsNull() const;

		inline TVec3<T> Size() const;

		inline TVec3<T> Center() const;

		inline T Volume() const;
		inline T SurfaceArea() const;
		inline u32 MaxDimension() const;

		T ShortestDistanceToPoint( const TVec3<T>& _p ) const;
		
		PlaneClassify Classify( const Plane<T>& _plane ) const;
		
		//CAABBox operator+( const CAABBox& bbox ) const;
		AABBox operator*( T _scale ) const;
		AABBox operator*( const TVec3<T>& _scale ) const;

		void operator+=( const AABBox& _bbox );

		inline void operator=( const TVec3<T>& _point );
		inline void operator+=( const TVec3<T>& _point );

		//bool IntersectRay( const Ray<T>& _ray, T& _minDist, T& _maxDist ) const;
		bool IntersectRay(	const Ray<T>& _ray, const TVec3<T>& _invDir, u32 _dirIsNeg[3], 
							T _minRayLength = 0.0, T _maxRayLength = 99999999999999999.0 ) const;

		bool IntersectTriangle( const TVec3<T>& _a, const TVec3<T>& _b, const TVec3<T>& _c ) const;

		bool IntersectAABBox( const AABBox<T>& _box ) const;
	};

	#include "AABBox.inl"

	/*
	class CStream;
	CStream& operator>>( CStream& stream, CAABBox& m );
	CStream& operator<<( CStream& stream, const CAABBox& m );
*/
}

#endif
