#pragma once

//Axis-aligned rectangle used by the node editor UI (position + size, in pixels)
struct UIRect
{
	Vec2 Pos, Size;

	bool PointInRect( const Vec2& p ) const
	{
		return (p.x >= Pos.x) && (p.y >= Pos.y) && (p.x <= Pos.x + Size.x) && (p.y <= Pos.y + Size.y);
	}

	Vec2 Center() const
	{
		return Pos + Size * 0.5f;
	}

	UIRect operator*( float f ) const
	{
		UIRect r;
		r.Pos = Pos * f;
		r.Size = Size * f;

		return r;
	}

	UIRect operator/( float f ) const
	{
		UIRect r;
		r.Pos = Pos / f;
		r.Size = Size / f;

		return r;
	}

	UIRect operator+( const Vec2& v ) const
	{
		UIRect r;
		r.Pos = Pos + v;
		r.Size = Size;

		return r;
	}

	UIRect operator-( const Vec2& v ) const
	{
		UIRect r;
		r.Pos = Pos - v;
		r.Size = Size;

		return r;
	}
};
