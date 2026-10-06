//#include "stdafx.h"

#include "FileStream.h"

#include <Core/FileSystem.h>
#include <Core/Log.h>

#if defined(USE_MACOSX_CORE_FOUNDATION_API)
	#include <CoreFoundation/CoreFoundation.h>
	#include <CoreServices/CoreServices.h>
	//#include <CoreFoundation/CFString.h>
#endif


	FileStream::FileStream() :
		FileHandle(NULL)
	{
	}


	FileStream::~FileStream()
	{
		Close();
	}


	bool FileStream::Open( const char* filename, const char* attrib )
	{
		if( IsOpen() )
			return false;

#ifdef XTM_WIN32
		errno_t err = fopen_s( &FileHandle, filename, attrib );
		if( err != 0 ) 
		{
			LOG( "Warning: Failed to open file %s (%s)", filename, attrib );
			return false;
		}
		
		return true;
#elif defined(XTM_MACOSX) && defined(USE_MACOSX_CORE_FOUNDATION_API)
		CurPos = 0;

		//first try in application bundle
//		SplitPath
		CFStringRef resourceName = CFStringCreateWithCString ( NULL, filename, kCFStringEncodingASCII );
		CFStringRef resourceType = CFStringCreateWithCString ( NULL, "", kCFStringEncodingASCII );
		CFStringRef subDirName = CFStringCreateWithCString ( NULL, "", kCFStringEncodingASCII );

		CFBundleRef mainBundle = CFBundleGetMainBundle(); //TODO cache this
		CFURLRef url = CFBundleCopyResourceURL( mainBundle, resourceName, resourceType, subDirName );

		if( !url )
			return false;

		FSRef fsref;
		CFURLGetFSRef( url, &fsref );

		HFSUniStr255 dataForkName;
		FSGetDataForkName( &dataForkName );

		OSErr err = FSOpenFork( &fsref,
								dataForkName.length,
								dataForkName.unicode,
								fsRdPerm, //TODO ...
								&FileHandle );

		if( err != noErr )
		{
			return false;
		}

		return true;
		// Get the main bundle for the app

		//CFURLCreateFromFileSystemRepresentation

#else
		FileHandle = fopen( filename, attrib );

		if( ( FileHandle == NULL ) && strchr( attrib, 'w' ) )
		{
			//Try again but create the necessary directory arborescence first
			FileSystem::Get().CreateDirectory( filename );
			FileHandle = fopen( filename, attrib );
		}

		if( FileHandle == NULL )
			return false;


		return true;
#endif


	}


	void FileStream::Close()
	{
		if( FileHandle )
		{
			#if defined(USE_MACOSX_CORE_FOUNDATION_API)
				FSCloseFork( FileHandle );
			#else
				fclose( FileHandle );
			#endif
			FileHandle = NULL;
		}
	}



	FileStream::OffsetType FileStream::WriteBytes( const byte* _pData, OffsetType _length )
	{
		#if defined(USE_MACOSX_CORE_FOUNDATION_API)
			//TODO
		#else
			return (OffsetType)fwrite( _pData, 1, (size_t)_length, FileHandle );
		#endif
	}


	FileStream::OffsetType FileStream::ReadBytes( byte* _pData, OffsetType _length )
	{
		#if defined(USE_MACOSX_CORE_FOUNDATION_API)
			ByteCount actualCount;
			FSReadFork( FileHandle, fsFromStart, CurPos, _length, _pData, &actualCount );
			CurPos += actualCount;
			return actualCount;
		#else
			return (OffsetType)fread( _pData, 1, (size_t)_length, FileHandle );
		#endif
	}


	void FileStream::Flush()
	{
		#if defined(USE_MACOSX_CORE_FOUNDATION_API)
			FSFlushFork( FileHandle );
		#else
			fflush( FileHandle );
		#endif
	}


	FileStream::OffsetType FileStream::Tell()
	{
		#if defined(USE_MACOSX_CORE_FOUNDATION_API)
			return CurPos;
		#elif defined(XTM_WIN32)
			return _ftelli64( FileHandle );
		#else
			return (FileStream::OffsetType)ftell( FileHandle );
		#endif
	}


	bool FileStream::Seek( FileStream::OffsetType _offset, SeekType _seek )
	{
		#if defined(USE_MACOSX_CORE_FOUNDATION_API)
			switch( _seek )
			{
				case SeekCur:	CurPos += _offset; return true;
				case SeekStart: CurPos = _offset; return true;
				case SeekEnd:	CurPos = GetSize() + _offset; return true;
			};
		#elif defined(XTM_WIN32)
			switch( _seek )
			{
				case SeekCur:	return _fseeki64( FileHandle, _offset, SEEK_CUR ) == 0;
				case SeekStart: return _fseeki64( FileHandle, _offset, SEEK_SET ) == 0;
				case SeekEnd:	return _fseeki64( FileHandle, _offset, SEEK_END ) == 0;
			};
		#else
			switch( _seek )
			{
				case SeekCur:	return fseek( FileHandle, _offset, SEEK_CUR ) == 0;
				case SeekStart: return fseek( FileHandle, _offset, SEEK_SET ) == 0;
				case SeekEnd:	return fseek( FileHandle, _offset, SEEK_END ) == 0;
			};
		#endif

		return false;
	}


	FileStream::OffsetType FileStream::GetSize()
	{
		OffsetType fileSize;

		#if defined(USE_MACOSX_CORE_FOUNDATION_API)
			SInt64 forkSize;
			FSGetForkSize( FileHandle, &forkSize );
		#else
			OffsetType restorePos = Tell();

			Seek( 0, SeekEnd );

			fileSize = Tell();

			Seek( restorePos, SeekStart );
		#endif
		return fileSize;
	}

