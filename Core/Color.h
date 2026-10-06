#ifndef XTM_COLOR_H
#define XTM_COLOR_H


#include <Core/Utility.h>
#include <Core/Vec3.h>

#ifdef XTM_D3D
//	#include <d3dx9math.h>
#endif

#if 1//def XTM_GL
	#define CONVERT_COLOR( c ) (c).ToA8B8G8R8()	
#else
	#define CONVERT_COLOR( c ) (c).ToR8G8B8A8()
#endif


	
	class Color
	{
	public:
		float r, g, b, a;

		static const Color Transparent;

		static const Color Black;
		static const Color White;
		static const Color Grey;

		static const Color Red;
		static const Color Green;
		static const Color Blue;

		static const Color Yellow;
		static const Color Purple;
		static const Color Cyan;
		
		static const Color Orange;
		static const Color Pink;

		inline Color() {}
		inline Color( float _r, float _g, float _b, float _a = 1.0f ) : r(_r),g(_g),b(_b),a(_a) {}
		inline Color( u8 _r, u8 _g, u8 _b, u8 _a = 255 ) : r(((float)_r)/255.0f),g(((float)_g)/255.0f),b(((float)_b)/255.0f),a(((float)_a)/255.0f) {}
		inline Color( const Color& _c ) : r(_c.r),g(_c.g),b(_c.b),a(_c.a) {}
		inline Color( const Vec3& _c, float _a = 1.0f ) : r(_c.x),g(_c.y),b(_c.z),a(_a) {}

		inline void			Clamp();
		inline u32			ToA8R8G8B8() const;
		inline u32			ToR8G8B8A8() const;
		inline u32			ToA8B8G8R8() const;
		inline u32			ToWin32COLORREF() const;
		inline void			FromWin32COLORREF( u32 _c );
		inline Vec3			rgb() const { return Vec3(r,g,b); }

		void				ToHSV( float& _h, float& _s, float& _v ) const;
		void				FromHSV( float _h, float _s, float _v, float _a = 1.0f );

#ifdef XTM_D3D
//		inline D3DXCOLOR	ToD3DXCOLOR() const;
#endif
		inline float&	operator[]( s32 _idx );
		inline float	operator[]( s32 _idx ) const;

		inline Color	operator+( const Color& _c ) const;
		inline Color	operator-( const Color& _c ) const;
		inline Color	operator*( const Color& _c ) const;
		inline Color	operator/( const Color& _c ) const;

		inline Color	operator*( float _f ) const;
		inline Color	operator/( float _f ) const;

		inline bool		operator==( const Color& _c ) const;
		inline void		operator+=( const Color& _c );
		inline void		operator-=( const Color& _c );
		inline void		operator*=( const Color& _c );
		inline void		operator/=( const Color& _c );

		inline void		operator*=( float _f );
		inline void		operator/=( float _f );
      
	};
    
	inline Color Modulate( const Color& _a, const Color& _b );

	#include "Color.inl"

#endif
