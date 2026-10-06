#ifndef _IBYTE_STREAM_H
#define _IBYTE_STREAM_H

#include "StandardLib.h"

namespace xtm
{

	class IByteStream
	{
	public:
		typedef s64	OffsetType;
		
		enum SeekType
		{
			SeekCur,
			SeekStart,
			SeekEnd
		};

	protected:
		IByteStream() {}

	public:

		virtual ~IByteStream() {}		

		/*************/
		/* Interface */
		/*************/
			
		virtual OffsetType	WriteBytes( const byte* _pData, OffsetType _length )=0;

		virtual OffsetType	ReadBytes( byte* _pData, OffsetType _length )=0;
	
		virtual void		Flush()=0;

		virtual OffsetType	Tell()=0;

		virtual bool		Seek( OffsetType _offset, SeekType _seek )=0;

		virtual OffsetType	GetSize()=0;
	
	};


	template < class T >
	inline IByteStream::OffsetType Write( IByteStream& _stream, const T& _data )
	{
		return _stream.WriteBytes( (const byte*)&_data, sizeof(T) );
	}

	template < class T >
	inline IByteStream::OffsetType Read( IByteStream& _stream, T& _data )
	{
		return _stream.ReadBytes( (byte*)&_data, sizeof(T) );
	}



}

#endif
