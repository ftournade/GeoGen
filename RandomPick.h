#pragma once


//TODO move to Core lib

#include <Core/Debug.h>

template <u32 MaxItemCount>
class RandomPick
{
public:
	RandomPick() { Clear(); }

	inline void Clear();
	inline void AddItem( float _probability );
	inline u32 PickRandomly() const;

private:
	float m_Probability[ MaxItemCount ];
	float m_Probabilitysum;
	u32 m_ItemCount;	
};


template <u32 MaxItemCount>
inline void RandomPick<MaxItemCount>::Clear()
{
	m_ItemCount = 0;
	m_Probabilitysum = 0.0f;
}

template <u32 MaxItemCount>
inline void RandomPick<MaxItemCount>::AddItem( float _probability )
{
	DBG_CHECK( m_ItemCount < MaxItemCount );

	m_Probability[ m_ItemCount++ ] = _probability;
	m_Probabilitysum += _probability;
}

template <u32 MaxItemCount>
inline u32 RandomPick<MaxItemCount>::PickRandomly() const
{
	DBG_CHECK( m_ItemCount > 0 );

	float rnd = xtm::Random( 0.0f, m_Probabilitysum * 0.999999f );
	
	float sum = 0.0f;

	for( u32 i = 0; i < m_ItemCount; ++i )
	{
		sum += m_Probability[ i ];
		
		if( rnd < sum )
			return i;
	}

	DBG_CHECK( false ); //should never get here
	return 0;
}
