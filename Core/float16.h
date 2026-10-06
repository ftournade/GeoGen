#pragma once

namespace xtm
{
	class float16
	{
	public:
		inline float16() {}
		inline float16( const float16& _f ) { m_Value = _f.m_Value; }
		float16( float _f );

		operator float() const;

	private:
		u16 m_Value;
	};

}