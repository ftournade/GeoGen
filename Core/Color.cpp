//#include "stdafx.h"
#include "Color.h"

namespace xtm
{
	const Color Color::Transparent( 0.0f, 0.0f, 0.0f, 0.0f );

	const Color Color::Black	( 0.0f, 0.0f, 0.0f, 1.0f );
	const Color Color::White	( 1.0f, 1.0f, 1.0f, 1.0f );
	const Color Color::Grey		( 0.5f, 0.5f, 0.5f, 1.0f );

	const Color Color::Red		( 1.0f, 0.0f, 0.0f, 1.0f );
	const Color Color::Green	( 0.0f, 1.0f, 0.0f, 1.0f );
	const Color Color::Blue		( 0.0f, 0.0f, 1.0f, 1.0f );

	const Color Color::Yellow	( 1.0f, 1.0f, 0.0f, 1.0f );
	const Color Color::Purple	( 1.0f, 0.0f, 1.0f, 1.0f );
	const Color Color::Cyan		( 0.5f, 0.5f, 1.0f, 1.0f );

	const Color Color::Orange	( 1.0f, 0.5f, 0.0f, 1.0f );
	const Color Color::Pink		( 1.0f, 0.0f, 0.5f, 1.0f );
	
	void Color::ToHSV( float& _h, float& _s, float& _v ) const
	{
		int i;
		
		float x = Min( Min( r, g ), b );
		_v = Max( Max( r, g ), b );
		
		if( _v == x )
		{
			_h = 0.0f;
			_s = 0.0f;
			
			return; //TODO ...
		}

		float f = (r == x) ? g - b : ((g == x) ? b - r : r - g);
		i = (r == x) ? 3 : ((g == x) ? 5 : 1);
		
		_h = (i - f /(_v - x)) / 6.0f;
		_s = (_v - x) / _v;
		 
	}
	
	void Color::FromHSV( float _h, float _s, float _v, float _a )
	{
		a = _a;
		
	 	int i;
		_h *= 6.0f;
		//if (h == UNDEFINED) RETURN_RGB(v, v, v);
		i = (int)_h;
		
		float f = _h - i;
		
		if ( !(i&1) )
			f = 1.0f - f; // if i is even
		
		float m = _v * (1.0f - _s);
		float n = _v * (1.0f - _s * f);
		
		switch(i)
		{
			case 6:
			case 0: r=_v; g=n;  b=m; break;
			case 1: r=n;  g=_v; b=m; break;
			case 2: r=m;  g=_v; b=n; break;
			case 3: r=m;  g=n;  b=_v; break;
			case 4: r=n;  g=m;  b=_v; break;
			case 5: r=_v; g=m;  b=n; break;
		}

	}


}
