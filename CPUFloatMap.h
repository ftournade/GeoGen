#pragma once

class CPUFloatMap
{
public:
	CPUFloatMap();
	CPUFloatMap( const CPUFloatMap& _rhs );
	~CPUFloatMap();

	void Resize( uint32_t _sx, uint32_t _sy );
	void Fill( float _v );

	inline       float* GetData()       { return m_pData; }
	inline const float* GetData() const { return m_pData; }

	inline uint32_t GetWidth() const { return m_SX; }
	inline uint32_t GetHeight() const { return m_SX; }

	void operator=( const CPUFloatMap& _rhs );

	const float& operator()( const Vec2i& p ) const
	{ 
		assert( (p.x >= 0) && (p.x < (int)m_SX) );
		assert( (p.y >= 0) && (p.y < (int)m_SY) );
		return m_pData[ p.y * m_SX + p.x ];
	}

	float& operator()( const Vec2i& p )
	{
		assert( (p.x >= 0) && (p.x < (int)m_SX) );
		assert( (p.y >= 0) && (p.y < (int)m_SY) );
		return m_pData[ p.y * m_SX + p.x ];
	}

	//TODO bilinear, bicubic etc
private:
	uint32_t m_SX, m_SY;

	float* m_pData;
};

