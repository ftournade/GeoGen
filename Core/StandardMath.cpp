//#include "stdafx.h"
#include "StandardMath.h"

	u32 Sqrti( u32 _i )
	{
		//TODO it may not be the fastest ...
		
		s32 nv, v = _i >> 1, c = 0;
		if( !v )
			return _i;
		
		do
		{
			nv = (v + _i / v) >> 1;
			if( Abs( v - nv ) <= 1 )
				return nv;
			v = nv;
		}
		while (c++ < 25);
		
		return nv;
	}
	
	
	u32	Log2( float _f )
	{
		#if defined(XTM_MACOSX) || defined(XTM_IPHONE)
			return ilogbf( _f );
		#else
			const float b = 1.4426950f; // this is 1.0f / log( 2.0f );
			float r = log( _f ) * b;
			return ((u32)(r + 0.5f));
		#endif
	}


	bool IsPow2( u16 _v )
	{
		u16 p = 1;
		
		while( p < _v ) 
			p *= 2;
		
		return ( p == _v );
	}
	
	bool IsPow2( u32 _v )
	{
		u32 p = 1;
		
		while( p < _v ) 
			p *= 2;
		
		return ( p == _v );
	}
	

