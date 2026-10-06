#pragma once

#include <Core/IFindFile.h>

namespace xtm
{

	class FileSystem
	{
		DECLARE_SINGLETON( FileSystem )
		
		inline static FileSystem& Get();

	public:
	
	#ifdef XTM_WIN32
		typedef u64 FileDate;
		typedef u64 FileSize;
	#else
		typedef u32 FileDate;
		typedef u32 FileSize;
	#endif
		struct FileInfos
		{
			FileDate Date;
			FileSize Size;
			//TODO more infos....
		};

	public:		
		~FileSystem();

		//DEPRECATED
		inline const Str& GetDataPath()
		{
			static Str str;
			return str;
		}

		bool FindFiles( const Str& _searchPath,
						IFindFile* _pFindFileCB,
						bool _bRecursive );

		bool FileExist( const char* _file );

		bool GetFileDate( const char* _file, FileDate& _date );

		bool GetFileSize( const char* _file, FileSize& _size );

		bool GetFileInfos( const char* _file, FileInfos& _infos );
		
		bool CreateDirectory( const char* _path );

		bool CopyFile( const char* _srcFile, const char* _dstFile, bool _bOverwrite = false );

		bool DeleteFile( const char* _file );
		
	private:
		bool _FindFiles( const Str& _searchPath,
						IFindFile* _pFindFileCB,
						bool _bRecursive );
		
	};

	IMPLEMENT_SINGLETON( FileSystem )

}
