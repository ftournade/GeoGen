#pragma once


namespace xtm
{

	class Vec2i
	{
	public:
		s32 x, y;

	public:
		inline Vec2i() {}
		inline Vec2i( int _x, int _y ) : x(_x), y(_y) {}

		inline Vec2i operator+( const Vec2i& _v ) const
		{
			return Vec2i( x + _v.x, y + _v.y );
		}

		inline Vec2i operator-( const Vec2i& _v ) const
		{
			return Vec2i( x - _v.x, y - _v.y );
		}

		inline Vec2i operator-() const
		{
			return Vec2i( -x, -y );
		}

		inline void operator*=( int _f )
		{
			x *= _f;
			y *= _f;
		}

		inline void operator+=( const Vec2i& _v )
		{
			x += _v.x;
			y += _v.y;
		}

		inline void operator-=( const Vec2i& _v )
		{
			x -= _v.x;
			y -= _v.y;
		}

		inline void operator+=( int _f )
		{
			x += _f;
			y += _f;
		}

		inline void operator/=( int _f )
		{
			x /= _f;
			y /= _f;
		}

		inline Vec2i operator+( int _f ) const
		{
			return Vec2i( x + _f, y + _f );
		}

		inline Vec2i operator-( int _f ) const
		{
			return Vec2i( x - _f, y - _f );
		}

		inline Vec2i operator*( int _f ) const
		{
			return Vec2i( x * _f, y * _f );
		}

		inline Vec2i operator/( int _f ) const
		{
			return Vec2i( x / _f, y / _f );
		}

		inline bool operator==( const Vec2i& _v ) const
		{
			return (x == _v.x) && (y == _v.y);
		}

		inline bool operator!=( const Vec2i& _v ) const
		{
			return (x != _v.x) || (y != _v.y);
		}

	};

}
