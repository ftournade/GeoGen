inline Color Modulate( const Color& _a, const Color& _b )
{
	return Color( _a.r * _b.r, _a.g * _b.g, _a.b * _b.b, _a.a * _b.a );
}

inline u32 Color::ToWin32COLORREF() const
{
	//TODO Clamp(); ?
	return ( (((u32)(b*255.0f)) << 16) | (((u32)(g*255.0f)) << 8) | ((u32)(r*255.0f)) );
}

inline void	Color::FromWin32COLORREF( u32 _c )
{
	r = (_c & 0xFF) / 255.0f;
	g = ((_c >> 8)  & 0xFF) / 255.0f;
	b = ((_c >> 16) & 0xFF) / 255.0f;
	a = 1.0f;
}

inline u32 Color::ToA8R8G8B8() const
{
	//TODO Clamp(); ?
	return ( (((u32)(a*255.0f)) << 24) | (((u32)(r*255.0f)) << 16) | (((u32)(g*255.0f)) << 8) | ((u32)(b*255.0f)) );
}

inline u32 Color::ToR8G8B8A8() const
{
	//TODO Clamp(); ?
	return ( (((u32)(r*255.0f)) << 24) | (((u32)(g*255.0f)) << 16) | (((u32)(b*255.0f)) << 8) | ((u32)(a*255.0f)) );
}

inline u32 Color::ToA8B8G8R8() const
{
	//TODO Clamp(); ?
	return ( (((u32)(a*255.0f)) << 24) | (((u32)(b*255.0f)) << 16) | (((u32)(g*255.0f)) << 8) | ((u32)(r*255.0f)) );
}

#ifdef XTM_D3D
/*
inline D3DXCOLOR Color::ToD3DXCOLOR() const
{
	return D3DXCOLOR( r, g, b, a );
}
*/
#endif


inline void	Color::Clamp()
{
	r = ::Clamp( r, 0.0f, 1.0f );
	g = ::Clamp( g, 0.0f, 1.0f );
	b = ::Clamp( b, 0.0f, 1.0f );
	a = ::Clamp( a, 0.0f, 1.0f );
}

inline float& Color::operator[]( s32 _idx )
{
	DBG_CHECK( (_idx >= 0) && (_idx < 4) );

	return *(&r + _idx);
}

inline float Color::operator[]( s32 _idx ) const
{
	DBG_CHECK( (_idx >= 0) && (_idx < 4) );

	return *(&r + _idx);
}

inline bool Color::operator==( const Color& _c ) const
{
	return (r == _c.r) && (g == _c.g) && (b == _c.b) && (a == _c.a);
}


//inline Color Color::operator+( float _f ) const;
//inline Color Color::operator-( float _f ) const;
inline Color Color::operator*( float _f ) const
{
	return Color(	r * _f,
					g * _f,
					b * _f,
					a * _f );
}

inline Color Color::operator/( float _f ) const
{
	float oof = 1.0f / _f;

	return Color(	r * oof,
					g * oof,
					b * oof,
					a * oof );
}

inline Color Color::operator/( const Color& _c ) const
{
	return Color(	r / _c.r,
					g / _c.g,
					b / _c.b,
					a / _c.a );
}

//inline void	Color::operator+=( float _f );
//inline void	Color::operator-=( float _f );

inline void	Color::operator*=( float _f )
{
	r *= _f;
	g *= _f;
	b *= _f;
}

inline void	Color::operator*=( const Color& _c )
{
	r *= _c.r;
	g *= _c.g;
	b *= _c.b;
	a *= _c.a;
}

inline void	Color::operator/=( float _f )
{
	float rcp = 1.0f / _f;
	r *= rcp;
	g *= rcp;
	b *= rcp;
}

//inline Color Color::operator*( const Color& _c ) const;


inline Color Color::operator+( const Color& _c ) const
{
	return Color(	r + _c.r,
					g + _c.g,
					b + _c.b,
					a + _c.a );
}

inline Color Color::operator-( const Color& _c ) const
{
	return Color(	r - _c.r,
					g - _c.g,
					b - _c.b,
					a - _c.a );
}

inline void Color::operator+=( const Color& _c )
{
	r += _c.r;
	g += _c.g;
	b += _c.b;
}

