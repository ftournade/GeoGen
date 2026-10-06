#pragma once
#ifndef XTM_VEC3_H
#define XTM_VEC3_H


#include <Core/StandardMath.h>
#include <Core/StandardLib.h>

#include <Core/Vec2.h>
//#include <Core/Vec4.h>

#include <Core/Utility.h>
#include <Core/Debug.h>

#define DBG_MAX_FLOAT 100000.0f
#define DBG_CHECK_VEC3( v ) \
	DBG_CHECK( v.IsValid() && (Abs(v.x)<DBG_MAX_FLOAT) && (Abs(v.y)<DBG_MAX_FLOAT) && (Abs(v.z)<DBG_MAX_FLOAT) )

namespace xtm
{
	template <class T>	
	class TVec3;

	template <class T>	inline T			AngleBetweenNormalizedVectors( const TVec3<T>& vec1, const TVec3<T>& vec2 );

	template <class T>	inline T			Dot(			const TVec3<T>& _a, const TVec3<T>& _b);
	template <class T>	inline TVec3<T>		Cross(			const TVec3<T>& _a, const TVec3<T>& _b);
	template <class T>	inline T			DistanceSquared(const TVec3<T>& _a, const TVec3<T>& _b);
	template <class T>	inline T			Distance(		const TVec3<T>& _a, const TVec3<T>& _b);
	template <class T>	inline TVec3<T>		Modulate(		const TVec3<T>& _a, const TVec3<T>& _b);
	template <class T>	inline TVec3<T>		Normalized(		const TVec3<T>& _v );
	template <class T>	inline TVec3<T>		Reflect(		const TVec3<T>& _v, const TVec3<T>& _normal );
	template <class T>	inline bool			Refract(		const TVec3<T>& _lightDir, const TVec3<T>& _normal, T _ior, TVec3<T>& _refractedDir ); //return false in case of total internal reflection ( see http://steve.hollasch.net/cgindex/render/refraction.txt or http://en.wikipedia.org/wiki/Snell's_law )
	template <class T>	inline TVec3<T>		Min(			const TVec3<T>& _a, const TVec3<T>& _b);
	template <class T>	inline TVec3<T>		Max(			const TVec3<T>& _a, const TVec3<T>& _b);
	template <class T>	inline TVec3<T>		Floor(			const TVec3<T>& _v );
	template <class T>	inline TVec3<T>		Ceil(			const TVec3<T>& _v );
	template <class T>	inline TVec3<T>		Exp(			const TVec3<T>& _v );
	template <class T>	inline TVec3<T>		Pow(			const TVec3<T>& _v, T _f );
	template <class T>	inline TVec3<T>		Abs(			const TVec3<T>& _v );
	template <class T>	inline TVec3<T>		Sign( const TVec3<T>& _v );


	template <class T>	TVec3<T> SphericalToCartesian( T _longitude, T _latitude, T _radius );

	template <class T> T SignedTriangleArea( const TVec3<T>& A, const TVec3<T>& B, const TVec3<T>& C );


	template <class T>
	class TVec3
	{
	public:
		T x, y, z;

		static const TVec3 XAxis;
		static const TVec3 YAxis;
		static const TVec3 ZAxis;
		static const TVec3 Origin;
		static const TVec3 Zero;
		static const TVec3 One;
		static const TVec3 Min;
		static const TVec3 Max;

		inline			TVec3();
		inline			TVec3(const T* pVec);
		inline			TVec3(T _x,T _y,T _z);
		inline			TVec3(const TVec2<T>& vec, T _z);
//		inline			TVec3(const TVec3<T>& vec);
		inline			TVec3(const TVec3<float>& vec);
		inline			TVec3(const TVec3<double>& vec);
	//	inline			TVec3(const Vec4& vec);

		inline	void	Set(T _x,T _y,T _z);
		inline	void	Set(const T* pVec);
				void	SetRandomDir();

		inline	T		GetDimension( u32 _dim ) const { DBG_CHECK( _dim < 3); return ((T*)this)[_dim]; }

		inline	void	Normalize();
		inline	void	CheckForNullAndNormalize();

		inline	T		Dot(const TVec3& vec) const;

		inline	TVec3	Cross(const TVec3& vec) const;

		inline	void	Saturate();

		//inline bool	IsNull() {return ((x);}

		inline	T		LengthSquared() const;

		inline	T		Length() const;

		inline	bool	IsNormalized() const;

				bool	IsValid() const;

		/*
		inline void Transform(const CMatrix & mat);

		inline TVec3 Transform(const CMatrix & mat) const;
		*/

		inline TVec2<T> xy() const { return TVec2<T>( x, y ); }
		inline TVec2<T> xz() const { return TVec2<T>( x, z ); }
		inline TVec2<T> yz() const { return TVec2<T>( y, z ); }
		
		inline	T&	operator[]( u32 _index );
		inline	T	operator[]( u32 _index ) const;

		inline	TVec3&	operator=(const TVec3& vec);

		inline	TVec3&	operator=(const T* pVec);

		inline	void	operator*=(T scalar);

		inline	void	operator*=(const TVec3& vec);

		inline	void	operator/=(T scalar);

		inline	TVec3	operator*(T scalar) const;

		inline	TVec3	operator*(const TVec3& vec) const;

		inline	TVec3	operator/(T scalar) const;

		inline	TVec3	operator/(const TVec3& vec) const;

		inline	void	operator+=(const TVec3& vec);
		inline	void	operator+=(T scalar);

		inline	void	operator-=(const TVec3& vec);

		inline	TVec3	operator+(const TVec3& vec) const;
		inline	TVec3	operator+(T scalar) const;

		inline	TVec3	operator-(const TVec3& vec) const;
		inline	TVec3	operator-(T scalar) const;

		inline	TVec3	operator-() const;

		inline	bool	operator==(const TVec3& vec) const;

		inline	bool	operator<( const TVec3& rhs ) const;

	};


	#include "Vec3.inl"

	template <class T> const TVec3<T> TVec3<T>::XAxis( 1, 0, 0 );
	template <class T> const TVec3<T> TVec3<T>::YAxis( 0, 1, 0 );
	template <class T> const TVec3<T> TVec3<T>::ZAxis( 0, 0, 1 );
	template <class T> const TVec3<T> TVec3<T>::Origin( 0, 0, 0 );
	template <class T> const TVec3<T> TVec3<T>::Zero( 0, 0, 0 );
	template <class T> const TVec3<T> TVec3<T>::One( 1, 1, 1 );
	template <class T> const TVec3<T> TVec3<T>::Min( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	template <class T> const TVec3<T> TVec3<T>::Max(  FLT_MAX,  FLT_MAX,  FLT_MAX );

	//#pragma warning (disable : 4231)

	template class TVec3<float>;
	template class TVec3<double>;

	typedef TVec3<float>	Vec3;
	typedef TVec3<double>	Vec3d;

	inline Vec3 ToVec3( const Vec3d& _v )
	{
		return Vec3( (float)_v.x, (float)_v.y, (float)_v.z );
	}

	inline Vec3 ToVec3( const Vec3& _v )
	{
		return _v;
	}

	inline Vec3d ToVec3d( const Vec3& _v )
	{
		return Vec3d( (double)_v.x, (double)_v.y, (double)_v.z );
	}

	inline Vec3d ToVec3d( const Vec3d& _v )
	{
		return _v;
	}

}


#endif
