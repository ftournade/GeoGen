#ifndef _FILE_STREAM_H
#define _FILE_STREAM_H

#include "IByteStream.h"

#ifdef _MACOS_X
	//Doesnt compile in "device target" !!!??
	//#define USE_MACOSX_CORE_FOUNDATION_API
#endif

#if defined(USE_MACOSX_CORE_FOUNDATION_API)
	#include <MacTypes.h>
#else
	#include <stdio.h>
#endif

	    
	class FileStream :	public IByteStream
	{
	public:
		FileStream();
		virtual ~FileStream();

		/******************/
		/* Member methods */
		/******************/

				bool	Open( const char* filename, const char* attrib );

				void	Close();

		inline	bool	IsOpen() const										{ return FileHandle != NULL; } 


		/*************************/
		/* IByteStream interface */
		/*************************/

	public:

		virtual OffsetType	WriteBytes( const byte* _pData, OffsetType _length );

		virtual OffsetType	ReadBytes( byte* _pData, OffsetType _length );

		virtual void		Flush();

		virtual OffsetType	Tell();

		virtual bool		Seek( OffsetType _offset, SeekType _seek );

		virtual OffsetType	GetSize();

	private:
		#if defined(USE_MACOSX_CORE_FOUNDATION_API)
			SInt16	FileHandle;
			SInt64	CurPos;
		#else
			FILE*	FileHandle;
		#endif

	};


#endif
