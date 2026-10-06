#pragma once

#include <Core/TVector.h>
#include <algorithm> //for std:sort

namespace xtm
{
	enum Interpolation
	{
		Corner, //LeftTangent == RightTangent == 0
		BezierCorner, 
		BezierSmooth //LeftTangent == - RightTangent
	};

	template <class T>
	T CubicBezier( const T& A, const T& B, const T& C, const T& D, float t )
	{
		float t2 = t * t;
		float tr = 1.0f - t;
		float tr2 = tr * tr;

		float a = tr2 * tr;
		float b = 3.0f * tr2 * t;
		float c = 3.0f * tr * t2;
		float d = t2 * t;

		return A * a + B * b + C * c + D * d;
	}

	template <class T>
	struct Keyframe
	{
		float			Time;
		T				Key;
		T				LeftTangent, RightTangent;
		Interpolation	KeyType;
	};

	template <class T>
	class Keyframer
	{
	public:
		//Keyframer();
		//~Keyframer();

		float GetStartTime() const;
		float GetEndTime() const;
		float GetDuration() const;

		//GetLoopingMode() const;

		T GetValue( float _time ) const;

		u32 AddKey(	float _time,
					const T& _val,
					Interpolation _keyType = Corner );

		u32 AddKey( float _time,
					const T& _val, const T& _leftTgt, const T& _rightTgt,
					Interpolation _keyType = Corner );

		void DelKey( u32 _keyId );
		u32 SetKeyTime( u32 _keyId, float _time );

		inline const	TVector< Keyframe<T> >& GetKeys() const	{ return m_Keys; }
		inline			TVector< Keyframe<T> >& GetKeys()		{ return m_Keys; }

		void SortKeys();

	private:
		TVector< Keyframe<T> > m_Keys;
		//TODO m_LoopingMode
	};

	template <class T>
	float Keyframer<T>::GetStartTime() const
	{
		if( !m_Keys.empty() )
			return m_Keys[0].Time;
		else
			return 0.0f;
	}

	template <class T>
	float Keyframer<T>::GetEndTime() const
	{
		if( !m_Keys.empty() )
			return m_Keys[ m_Keys.size() - 1 ].Time;
		else
			return 0.0f;
	}

	template <class T>
	float Keyframer<T>::GetDuration() const
	{
		return GetEndTime() - GetStartTime();
	}

	template <class T>
	T Keyframer<T>::GetValue( float _time ) const
	{
		//DBG_CHECK( !isnan( _time ) );
		//DBG_CHECK( _time >= 0.0f );//TEST

		if( m_Keys.empty() )
			return T();

		//TODO loop mode and interp modes
		u32 i = 0;
		while( ( i < m_Keys.size() ) && ( _time > m_Keys[i].Time ) )
		{
			++i;
		}

		if( i == 0 )
		{
			return m_Keys[0].Key;
		}
		else if( i < m_Keys.size() )
		{
			float t =	(_time - m_Keys[i-1].Time) /
						(m_Keys[i].Time - m_Keys[i-1].Time);
			
			
			//return Lerp( m_Keys[i-1].Key, m_Keys[i].Key, t );
			return CubicBezier( m_Keys[ i - 1 ].Key, m_Keys[ i - 1 ].RightTangent, m_Keys[ i ].LeftTangent, m_Keys[ i ].Key, t );
		}
		else
		{
			//beyond last key (TODO use loop mode)
			return m_Keys[ m_Keys.size() - 1 ].Key;
		}

	}

	template <class T>
	u32 Keyframer<T>::AddKey(	float _time,
								const T& _val,
								Interpolation _type )
	{

		//WARNING not fully tested ! probable bugs

		//TODO search for duplicates ?

		m_Keys.add( 1 );

		for( s32 i=m_Keys.size()-1 ; i >= 1 ; --i )
		{
			if( m_Keys[ i - 1 ].Time < _time )
			{
				m_Keys[ i ].Key = _val;
				m_Keys[ i ].Time = _time;
				m_Keys[ i ].KeyType = _type;
				m_Keys[ i ].LeftTangent = _val;
				m_Keys[ i ].RightTangent = _val;
				return i;
			}
			else
			{
				m_Keys[ i ] = m_Keys[ i - 1 ];
			}
		}

		//The key lies in first position
		if( m_Keys.size() > 1 )
			m_Keys[ 1 ] = m_Keys[ 0 ];

		m_Keys[ 0 ].Key = _val;
		m_Keys[ 0 ].Time = _time;
		m_Keys[ 0 ].KeyType = _type;
		m_Keys[ 0 ].LeftTangent = _val;
		m_Keys[ 0 ].RightTangent = _val;

		return 0;
	}


	template <class T>
	u32 Keyframer<T>::AddKey(	float _time,
								const T& _val, const T& _leftTgt, const T& _rightTgt,
								Interpolation _type )
	{

		//WARNING not fully tested ! probable bugs

		//TODO search for duplicates ?

		m_Keys.add( 1 );

		for( s32 i = m_Keys.size() - 1 ; i >= 1 ; --i )
		{
			if( m_Keys[ i - 1 ].Time < _time )
			{
				m_Keys[ i ].Key = _val;
				m_Keys[ i ].Time = _time;
				m_Keys[ i ].KeyType = _type;
				m_Keys[ i ].LeftTangent = _leftTgt;
				m_Keys[ i ].RightTangent = _rightTgt;
				return i;
			}
			else
			{
				m_Keys[ i ] = m_Keys[ i - 1 ];
			}
		}

		//The key lies in first position
		if( m_Keys.size() > 1 )
			m_Keys[ 1 ] = m_Keys[ 0 ];

		m_Keys[ 0 ].Key = _val;
		m_Keys[ 0 ].Time = _time;
		m_Keys[ 0 ].KeyType = _type;
		m_Keys[ 0 ].LeftTangent = _leftTgt;
		m_Keys[ 0 ].RightTangent = _rightTgt;

		return 0;
	}


	template <class T>
	void Keyframer<T>::DelKey( u32 _pointId )
	{
		if( m_Keys.size() < 2 )
		{
			m_Keys.clear();
			return;
		}

		for( u32 i = _pointId ; i < m_Keys.size() - 1 ; ++i )
		{
			m_Keys[i] = m_Keys[i+1];
		}

		m_Keys.pop_back();
	}

	template <class T>
	u32 Keyframer<T>::SetKeyTime( u32 _keyId, float _time )
	{
		T val( m_Keys[ _keyId ].Key );

		DelKey( _keyId );
		return AddKey( _time, val );

		//m_Keys[ _keyId ].Time = _time;
	}

	template <class T>
	void Keyframer<T>::SortKeys()
	{
		auto sortByTime = []( const Keyframe<T>& a, const Keyframe<T>& b )
		{
			return a.Time < b.Time;
		};

		std::sort( m_Keys.begin(), m_Keys.end(), sortByTime );
	}

}
