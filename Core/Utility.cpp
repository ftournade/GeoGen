//#include "stdafx.h"

#include "Utility.h"
#include "StandardLib.h"

namespace xtm
{

	//"Jenkins One-at-a-time hash" from Bob Jenkins

	u32	CRC32( const byte* _pData, u32 _Size )
	{
		u32 hash=0;
		u32 i;

		for(i = 0 ; i < _Size ; ++i )
		{
			hash += _pData[i];
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}
		hash += (hash << 3);
		hash ^= (hash >> 11);
		hash += (hash << 15);
		return hash;
	}


	FORCE_INLINE void __Radix( u32 _byte,
								u16 _count,
								u16* _source,
								u16* _dest,
								const byte* _sortKey,
								u32			_sortKeyStride,
								u32			_sortKeyOffset )
	{
		u16 count[256];
		u16 index[256];
		ZeroMemory( count, sizeof(count) );

		//char buffer[512];
		//sprintf( buffer, "///////// %d /////////\r\n", _byte );
		//OutputDebugString( buffer );

		for( u32 i=0; i < _count; ++i )
		{
			const byte* sortKey = _sortKey + _source[i] * _sortKeyStride + _sortKeyOffset;
			//sprintf( buffer, "%f\r\n", *((const float*)sortKey) );
			//OutputDebugString( buffer );

			u8 sortKeyByte = sortKey[ _byte ];
			count[ sortKeyByte ]++;
		}

		index[0]=0;

		for( u32 i=1 ; i < 256 ; ++i )
			index[i] = index[i-1] + count[i-1];

		for( u32 i=0; i < _count ; ++i )
		{
			const byte* sortKey = _sortKey + _source[i] * _sortKeyStride + _sortKeyOffset;
			u8 sortKeyByte = sortKey[ _byte ];
			_dest[ index[sortKeyByte]++ ] = _source[i];
		}

	}

	void RadixSort16(	u16			_count,
						u16*		_temp1,		//size: _count
						u16*		_temp2,		//size: _count
						u16**		_out,		//size: _count
						const byte* _sortKey,	//size: _count
						u32			_sortKeyStride,
						u32			_sortKeyOffset,
						u32			_sortKeySize )

	{
		for( u16 i=0 ; i < _count ; ++i )
			_temp1[i] = i;

		for( u32 i=0 ; i < _sortKeySize ; ++i )
		{
			//u32 byteIndex = _sortKeySize - i - 1;
			u32 byteIndex = i;
			__Radix( byteIndex, _count, _temp1, _temp2,
					_sortKey, _sortKeyStride, _sortKeyOffset );
			TSwap( _temp1, _temp2 );
		}

		*_out = _temp1;
	}

}
