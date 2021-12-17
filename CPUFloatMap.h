#pragma once

#include <Core/Debug.h>
#include <Core/Vec2i.h>

class CPUFloatMap
{
public:
	CPUFloatMap();
	CPUFloatMap( const CPUFloatMap& _rhs );
	~CPUFloatMap();

	void Resize( u32 _sx, u32 _sy );
	void Fill( float _v );

	inline       float* GetData()       { return m_pData; }
	inline const float* GetData() const { return m_pData; }

	inline u32 GetWidth() const { return m_SX; }
	inline u32 GetHeight() const { return m_SX; }

	void operator=( const CPUFloatMap& _rhs );

	const float& operator()( const xtm::Vec2i& p ) const
	{ 
		DBG_CHECK( (p.x >= 0) && (p.x < (int)m_SX) );
		DBG_CHECK( (p.y >= 0) && (p.y < (int)m_SY) );
		return m_pData[ p.y * m_SX + p.x ];
	}

	float& operator()( const xtm::Vec2i& p )
	{
		DBG_CHECK( (p.x >= 0) && (p.x < (int)m_SX) );
		DBG_CHECK( (p.y >= 0) && (p.y < (int)m_SY) );
		return m_pData[ p.y * m_SX + p.x ];
	}

	//TODO bilinear, bicubic etc
private:
	u32 m_SX, m_SY;

	float* m_pData;
};

