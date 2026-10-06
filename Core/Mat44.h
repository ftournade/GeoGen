#pragma once
#ifndef XTM_MAT44_H
#define XTM_MAT44_H

#include "Vec3.h"
#include "Vec4.h"
#include "Mat33.h"


#include <Core/Utility.h> //For TSwap

namespace xtm
{
	template <class T> class Plane;
	//template <typename T> class TMat44;
}

namespace xtm
{

	template <typename T>
	class TMat44
	{
	public:
		inline			TMat44();
		inline			TMat44(	T f11,T f12,T f13,T f14,
								T f21,T f22,T f23,T f24,
								T f31,T f32,T f33,T f34,
								T f41,T f42,T f43,T f44 );

		inline			TMat44( const TMat33<T>& _rot, const TVec3<T>& _trans );
		
						TMat44( const T* ptr );

				void	MakeIdentity();

				void	MakeTranslation( const TVec3<T>& trans );

				void	MakeScaling( const TVec3<T>& scale );
				void	RemoveScaling();

				void	MakeRotationX( T angle );
				void	MakeRotationY( T angle );
				void	MakeRotationZ( T angle );

				void	MakeRotation( const TVec3<T>& axis, T angle );
				void	MakeRotationYawPitchRoll( T _yaw, T _pitch, T _roll );

				void	MakePerspectiveProjection( T _verticalFov, T _aspectRatio, T _zNear, T _zFar );
				bool	RetrieveProjectionParameters( bool& isOrthographic, T& _verticalFov, T& _aspectRatio, T& _zNear, T& _zFar ) const;

				void	MakeOrthoProjection( T _width, T _height, T _zNear, T _zFar );
				void	MakeOrthoProjection( T _left, T _right, T _top, T _bottom, T _zNear, T _zFar );

				void	MakePlanarProjectionOmni( const Plane<T>& _plane, const TVec3<T>& _projectorPos );
				void	MakePlanarProjectionDir( const Plane<T>& _plane, const TVec3<T>& _projectorDir );

				void	MakeLookAt( const TVec3<T>& position,
									const TVec3<T>& target,
									const TVec3<T>& up );

				void	Transpose();

				bool	Inverse();

				void	Mirror( const Plane<T>& _plane );

		inline	TVec4<T>	Transform( const TVec4<T>& v ) const;
		inline	TVec3<T>	TransformDirection( const TVec3<T>& v ) const;
		inline	TVec3<T>	TransformPosition( const TVec3<T>& v ) const;

		inline	T			GetDeterminant() const;

		inline	TVec3<T>	GetTranslation() const;
		inline	void		SetTranslation( const TVec3<T>& t );

				void		GetYawPitchRoll( T& _yaw, T& _pitch, T& _roll ) const;

		inline	TVec3<T>	GetXAxis() const;
		inline	TVec3<T>	GetYAxis() const;
		inline	TVec3<T>	GetZAxis() const;

		inline	void		SetXAxis( const TVec3<T>& axis );
		inline	void		SetYAxis( const TVec3<T>& axis );
		inline	void		SetZAxis( const TVec3<T>& axis );

				//Given a view/proj (camera or light) it returns the 8 frustum corner points
				bool		GetFrustumCorners( TVec3<T>* _pCorners ) const;

		inline	TMat44&	operator=( const T* ptr );
				TMat44	operator*( const TMat44& ) const;
				void	operator*=( const TMat44& );
				bool	operator==( const TMat44& ) const;

		//Debug
		
		static	bool	UnitTest();
		static	T		UnitTestDiff( const TMat44& a, const TMat44& b );
		
				bool	IsValid( bool _bStrict=true ) const;
				bool	IsOrthonormal() const;
				bool	Orthonormalize();
		
				void	Dump() const;
	public:
		union
		{
			struct 
			{
				T	_11,_12,_13,_14,
					_21,_22,_23,_24,
					_31,_32,_33,_34,
					_41,_42,_43,_44;
			};


			T m[4][4];
		};

					
		static const TMat44 Identity;

	};

	
	template <typename T>
	inline TMat44<T> Inverse( const TMat44<T>& _m );

	template <typename T>
	inline TMat44<T> Transpose( const TMat44<T>& _m );

	//Transform 'position' vector by matrix
	
	template <typename T>
	inline TVec3<T> operator*( const TVec3<T>& v, const TMat44<T>& m );
	
	template <typename T>
	inline TVec4<T> operator*( const TVec4<T>& v, const TMat44<T>& m );

}

#include "Plane.h"

namespace xtm
{
	#include "Mat44.inl"

	//#pragma warning (disable : 4231)
	/*
	template <typename T> const TMat44<T> TMat44<T>::Identity(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	*/
	template <> const TMat44<float> TMat44<float>::Identity(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	template <> const TMat44<double> TMat44<double>::Identity(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

//	template class TMat44<float>;
//	template class TMat44<double>;

	class Mat44 : public TMat44<float>
	{
	public:
		inline			Mat44() {}
		inline			Mat44(	float f11, float f12, float f13, float f14,
								float f21, float f22, float f23, float f24,
								float f31, float f32, float f33, float f34,
								float f41, float f42, float f43, float f44) :
			TMat44<float>(f11, f12, f13, f14, f21, f22, f23, f24, f31, f32, f33, f34, f41, f42, f43, f44) {}

		inline			Mat44( const TMat44<float>& mtx ) :	TMat44<float>( &mtx.m[0][0] ) {}

		inline			Mat44(const TMat33<float>& _rot, const TVec3<float>& _trans) : TMat44<float>(_rot, _trans) {}

		Mat44(const float* ptr);

		Mat44(const char* _text); //Used to init matrix from XML or other ASCII source

		inline	Mat44&	operator=(const float* ptr);

		inline	Mat44	operator*(const Mat44& mtx ) const { return TMat44<float>::operator*(mtx); }
		void	operator*=(const Mat44&);
		bool	operator==(const Mat44&) const;
	};

	class Mat44d : public TMat44<double>
	{
	public:
		inline			Mat44d() {}
		inline			Mat44d(	double f11, double f12, double f13, double f14,
								double f21, double f22, double f23, double f24,
								double f31, double f32, double f33, double f34,
								double f41, double f42, double f43, double f44) :
			TMat44<double>(f11, f12, f13, f14, f21, f22, f23, f24, f31, f32, f33, f34, f41, f42, f43, f44) {}

		inline			Mat44d(const TMat44<double>& mtx) : TMat44<double>( &mtx.m[0][0] ) {}

		inline			Mat44d(const TMat33<double>& _rot, const TVec3<double>& _trans) : TMat44<double>( _rot, _trans ) {}

		Mat44d(const double* ptr);

		Mat44d(const char* _text); //Used to init matrix from XML or other ASCII source

		inline	Mat44d&	operator=(const double* ptr);

		inline	Mat44d	operator*( const Mat44d& mtx ) const { return TMat44<double>::operator*(mtx); }
		void	operator*=(const Mat44d&);
		bool	operator==(const Mat44d&) const;
	};


	inline Mat44 ToMat44( const Mat44d& _m )
	{
		return Mat44( 	(float)_m.m[0][0], (float)_m.m[0][1], (float)_m.m[0][2], (float)_m.m[0][3],
						(float)_m.m[1][0], (float)_m.m[1][1], (float)_m.m[1][2], (float)_m.m[1][3],
						(float)_m.m[2][0], (float)_m.m[2][1], (float)_m.m[2][2], (float)_m.m[2][3],
						(float)_m.m[3][0], (float)_m.m[3][1], (float)_m.m[3][2], (float)_m.m[3][3] );
	}

	inline Mat44 ToMat44( const Mat44& _m )
	{
		return _m;
	}

	
	inline Mat44d ToMat44d( const Mat44& _m )
	{
		return Mat44d(	(scalar)_m.m[0][0], (scalar)_m.m[0][1], (scalar)_m.m[0][2], (scalar)_m.m[0][3],
						(scalar)_m.m[1][0], (scalar)_m.m[1][1], (scalar)_m.m[1][2], (scalar)_m.m[1][3],
						(scalar)_m.m[2][0], (scalar)_m.m[2][1], (scalar)_m.m[2][2], (scalar)_m.m[2][3],
						(scalar)_m.m[3][0], (scalar)_m.m[3][1], (scalar)_m.m[3][2], (scalar)_m.m[3][3] );
	}

	inline Mat44d ToMat44d( const Mat44d& _m )
	{
		return _m;
	}

}

#endif
