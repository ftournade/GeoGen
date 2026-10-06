#pragma once


//TODO move to Core lib


template <uint32_t MaxItemCount>
class RandomPick
{
public:
	RandomPick() { Clear(); }

	inline void Clear();
	inline void AddItem( float _probability );
	inline uint32_t PickRandomly() const;

private:
	float m_Probability[ MaxItemCount ];
	float m_Probabilitysum;
	uint32_t m_ItemCount;	
};


template <uint32_t MaxItemCount>
inline void RandomPick<MaxItemCount>::Clear()
{
	m_ItemCount = 0;
	m_Probabilitysum = 0.0f;
}

template <uint32_t MaxItemCount>
inline void RandomPick<MaxItemCount>::AddItem( float _probability )
{
	assert( m_ItemCount < MaxItemCount );

	m_Probability[ m_ItemCount++ ] = _probability;
	m_Probabilitysum += _probability;
}

template <uint32_t MaxItemCount>
inline uint32_t RandomPick<MaxItemCount>::PickRandomly() const
{
	assert( m_ItemCount > 0 );

	float rnd = Random( 0.0f, m_Probabilitysum * 0.999999f );
	
	float sum = 0.0f;

	for( uint32_t i = 0; i < m_ItemCount; ++i )
	{
		sum += m_Probability[ i ];
		
		if( rnd < sum )
			return i;
	}

	assert( false ); //should never get here
	return 0;
}
