#pragma once
#ifndef XTM_STANDARD_MATH_H
#define XTM_STANDARD_MATH_H


#include <float.h>
//#include <stdlib.h>


#define XTM_PI			3.141592654f
#define XTM_2PI			(2.0f * 3.141592654f)
#define XTM_PI_DOUBLE	3.1415926535897932384626433832795

namespace xtm
{
	#ifdef XTM_WIN32
		#define IsNan		_isnan
		#define IsFinite	_finite
	#elif defined(XTM_IPHONE)
		#define IsNan		isnan
		
		#ifdef TARGET_IPHONE_SIMULATOR
			#define IsFinite(v)	true //fixing gdb error: Detected an attempt to call a symbol in system libraries that is not present on the iPhone: __isfinitef called from ...
		#else
			#define IsFinite	isfinite
		#endif
	#else
		#define IsNan		isnan
		#define IsFinite	finite
	#endif

	#if defined(XTM_WIN32) || defined(XTM_IPHONE) || defined(XTM_MACOSX) || defined(XTM_LINUX)

		template <class T>
		inline T Square( T x ) { return x * x; }

		inline float	Sqrt( float _f )									{ return sqrtf( _f ); }
		inline double	Sqrt( double _f )									{ return sqrt( _f ); }
		u32	Sqrti( u32 _i );
	
		inline float	Cos( float _a )										{ return cosf( _a ); }
		inline double	Cos( double _a )									{ return cos( _a ); }

		inline float	Sin( float _a )										{ return sinf( _a ); }
		inline double	Sin( double _a )									{ return sin( _a ); }

		inline float	Tan( float _a )										{ return tanf( _a ); }
		inline double	Tan( double _a )									{ return tan( _a ); }

		inline float	ACos( float _a )									{ return acosf( _a ); }
		inline double	ACos( double _a )									{ return acos( _a ); }

		inline float	ASin( float _a )									{ return asinf( _a ); }
		inline double	ASin( double _a )									{ return asin( _a ); }

		inline float	ATan( float _a )									{ return atanf( _a ); }
		inline double	ATan( double _a )									{ return atan( _a ); }


		inline void		SinCos( float _a, float& _oSin , float& _oCos )		{ _oCos = cosf( _a ); _oSin = sinf( _a ); }
		inline void		SinCos( double _a, double& _oSin , double& _oCos )	{ _oCos = cos( _a ); _oSin = sin( _a ); }

		inline float	Abs( float _f )										{ return fabsf( _f ); }
		inline double	Abs( double _f )									{ return fabs( _f ); }
		inline s32		Abs( s32 _i )										{ return abs( _i ); }

		inline float	Floor( float _f )									{ return floorf( _f ); }
		inline float	Ceil( float _f )									{ return ceilf( _f ); }

		u32	Log2( float _f );
		inline float	Log10( float _f )									{ return log10f( _f ); }
	
		inline float	Pow( float _f, float _e )							{ return powf( _f, _e ); }
		inline double	Pow( double _f, double _e )							{ return pow( _f, _e ); }

		inline float	Exp( float _f )										{ return expf( _f ); }
		inline double	Exp( double _f )									{ return exp( _f ); }
		
		bool	IsPow2( u16 _v );
		bool	IsPow2( u32 _v );

	#else

		#error "Must define platform specific math functions"

	#endif
	
	
	inline float SphereVolume( float _radius )							{ return (XTM_PI * 4.0f / 3.0f) * _radius * _radius * _radius; }
	inline float CylinderVolume( float _height, float _radius )			{ return XTM_PI * _radius * _radius * _height; }
	inline float CapsuleVolume( float _height, float _radius )			{ return CylinderVolume( _height, _radius ) + SphereVolume( _radius ); }


	template <class T>
	inline T DegToRad( T _angle )	{ return (_angle * (T)XTM_PI) / (T)180.0; }

	template <class T>
	inline T RadToDeg( T _angle )	{ return (_angle * (T)180.0) / (T)XTM_PI; }

	inline float FeetToMeter( float _dist )	{ return _dist * 0.3048f; }
	inline float MeterToFeet( float _dist )	{ return _dist / 0.3048f; }

	inline float MsToKph( float _speed )	{ return _speed * 3.6f; }
	inline float KphToMs( float _speed )	{ return _speed / 3.6f; }

	inline float KphToMph( float _speed )	{ return _speed * 0.6214f; }
	inline float MphToKph( float _speed )	{ return _speed / 0.6214f; }

	inline float MsToMph( float _speed )	{ return KphToMph( MsToKph( _speed ) ); }
	inline float MphToMs( float _speed )	{ return KphToMs( MphToKph( _speed ) ); }

	inline float MachToKph( float _speed )	{ return _speed * 1200.0f; }
	inline float KphToMach( float _speed )	{ return _speed / 1200.0f; }

	inline float MachToMs( float _speed )	{ return KphToMs( MachToKph( _speed ) ); }
	inline float MsToMach( float _speed )	{ return KphToMach( MsToKph( _speed ) ); }

	template <class T>
	inline T Step( T val, T threshold )
	{
		return (val <= threshold) ? (T)0 : (T)1;
	}

	template <class T>
	inline T CubicSCurve( T a )
	{
		return (a * a * ((T)3.0 - (T)2.0 * a));
	}

	template <class T>
	inline T SmoothStep( T val, T threshold, T smoothness )
	{
		return  ( val <= threshold - smoothness ) ? (T)0 :
		( val >= threshold + smoothness ) ? (T)1 :
		CubicSCurve( ( val - ( threshold - smoothness ) ) / ( smoothness * (T)2 ) );
	}

	
}

#endif
