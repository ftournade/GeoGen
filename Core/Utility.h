#pragma once
#ifndef XTM_UTILITY_H
#define XTM_UTILITY_H




#define XTM_EPSILON		1e-6f

#define VERSION_NUMBER( version, subVersion, buildNumber ) (((u8)(version)<<24)|((u8)(subVersion)<<16)|((u16)(buildNumber)))

#ifdef XTM_WIN32
	#pragma warning(disable:4311)
#endif

	template < class T >
	inline bool			IsEpsilonNull( T a )									{ return ( ( a > - (T)XTM_EPSILON ) && ( a < (T)XTM_EPSILON ) ); }

	template < class T >
	inline bool			IsEpsilonEqual( T a, T b )								{ return IsEpsilonNull( a - b ); }

	template < class T >
	inline T			Sign( T val )											{ return ( val >= (T)0 ) ? (T)1 : (T)-1; }

	template < class T >
	inline const T&		Min( const T& a, const T& b )							{ return ( a < b ) ? a : b;	}

	template < class T >
	inline const T&		Max( const T& a, const T& b )							{ return ( a > b ) ? a : b;	}

	template < class T >
	inline	T			Clamp( const T& val, const T& min, const T& max )		{ return Max( Min( val, max ), min ); }

	template < class T >
	inline	T			Saturate( const T& val )								{ return Max( Min( val, (T)1.0 ), (T)0.0 ); }
	
	template <class T, class S>
	inline	T			Lerp( T min, T max, S factor )							{ return min + ( max - min ) * factor; }

	template < class T >
	inline	void		TSwap( T& a, T& b )										{ T tmp = a; a = b; b = tmp; }

	template < class T >
	inline	bool		TIsBetween( const T& val, const T& min, const T& max )	{ return (val >= min) && (val <= max); }

	//Return padding
	inline	u32			NeededPaddingToAlign( u32 _address, u32 _boundary )		{ return ( _boundary - ( _address % _boundary ) ) % _boundary; }

	inline	u32			Align( u32 _address, u32 _boundary )					{ return _address + NeededPaddingToAlign( _address, _boundary ); }

	u32	CRC32( const byte* _pData, u32 _Size );

	template <class T>
	void BubbleSort( T* _pData, u32 _count );
	
	void RadixSort16(	u16			_count,
										u16*		_temp1,		//size: _count
										u16*		_temp2,		//size: _count
										u16**		_out,		//size: _count
										const byte* _sortKey,	//size: _count
										u32			_sortKeyStride,
										u32			_sortKeyOffset,
										u32			_sortKeySize );
	
	inline	float		BytesToMegaBytes( u32 _bytes )							{ return ((float)_bytes) / (1024.0f * 1024.0f); }
	inline	float		BytesToMegaBytes( u64 _bytes )							{ return ((float)_bytes) / (1024.0f * 1024.0f); }

	inline	bool		IsEndOfLine( char c )									{ return (c == '\r') || (c == '\n') || (c == '\0'); }
	
	
	
	
	//  IMPLEMENTATION  //
	
	template <class T>
	void BubbleSort( T* _pData, u32 _count )
	{
		bool bSorted;
		
		do
		{
			bSorted = true;
			
			for( u32 i=1 ; i < _count ; ++i )
			{		
				if( _pData[ i - 1 ] > _pData[ i ] )
				{
					TSwap( _pData[ i - 1 ], _pData[ i ] );
					bSorted = false;
				}
			}
			
		}
		while( !bSorted );
		
	}

#endif
