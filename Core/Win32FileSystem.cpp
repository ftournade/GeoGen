//#include "stdafx.h"
#include "FileSystem.h"

#include <Core/Debug.h>


	FileSystem::FileSystem()
	{
	}

	FileSystem::~FileSystem()
	{
	}

	bool FileSystem::FindFiles( const Str& _searchPath,
							   IFindFile* _pFindFileCB,
							   bool _bRecursive )
	{
		_pFindFileCB->BeginFileSearch();
		
		bool result = _FindFiles( _searchPath, _pFindFileCB, _bRecursive );
		
		_pFindFileCB->EndFileSearch();

		return result;
	}
	
	bool FileSystem::_FindFiles( const Str& _searchPath,
								IFindFile* _pFindFileCB,
								bool _bRecursive )
	{

		CHECK( !_searchPath.empty() );


		WIN32_FIND_DATAA findData;


		//Append a slash to the path if necessary

		Str searchPath( _searchPath );

		AppendSlash( searchPath );


		//Append * wildcard to search string

		Str searchString( searchPath + '*' );

		HANDLE searchHandle = FindFirstFileA(	searchString.c_str(),
												&findData );

		if( searchHandle == INVALID_HANDLE_VALUE )
			return true;

		bool bContinueSearching = true;

		do
		{

			if( findData.cFileName[0] == '.' )
				continue;

			FileInfo fileInfo;

			fileInfo.m_Filename = findData.cFileName;
			fileInfo.m_FullPath = searchPath + fileInfo.m_Filename;

			if( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				fileInfo.m_IsDirectory = true;

				if( !_pFindFileCB->OnFindFile( fileInfo ) )
					bContinueSearching = false;

				if( _bRecursive )
				{
					Str subdir( searchPath + fileInfo.m_Filename );

					if( !_FindFiles( subdir, _pFindFileCB, true ) )
						bContinueSearching = false;
				}
			}
			else
			{
				fileInfo.m_IsDirectory = false;

				if( !_pFindFileCB->OnFindFile( fileInfo ) )
					bContinueSearching = false;
			}

		}
		while( bContinueSearching && FindNextFileA( searchHandle, &findData ) );


		FindClose( searchHandle );

		return bContinueSearching;

	}


	bool FileSystem::FileExist( const char* _file )
	{
		bool bExist;

		WIN32_FIND_DATAA findData;

		HANDLE searchHandle = FindFirstFileA( _file,	&findData );

		if( searchHandle == INVALID_HANDLE_VALUE )
			bExist = false;
		else
			bExist = true;

		FindClose( searchHandle );

		return bExist;
	}

	bool FileSystem::GetFileDate( const char* _file, FileSystem::FileDate& _date )
	{
		WIN32_FILE_ATTRIBUTE_DATA attribs;

		if( !GetFileAttributesExA( _file, GetFileExInfoStandard, (LPVOID)&attribs ) )
			return false;

		_date  = ((u64)attribs.ftLastWriteTime.dwHighDateTime) << 32;
		_date |= ((u64)attribs.ftLastWriteTime.dwLowDateTime);

		return true;
	}

	bool FileSystem::GetFileSize( const char* _file, FileSystem::FileSize& _size )
	{
		WIN32_FILE_ATTRIBUTE_DATA attribs;

		if( !GetFileAttributesExA( _file, GetFileExInfoStandard, (LPVOID)&attribs ) )
			return false;

		_size  = ((u64)attribs.nFileSizeHigh) << 32;
		_size |= ((u64)attribs.nFileSizeLow);

		return true;
	}

	bool FileSystem::GetFileInfos( const char* _file, FileInfos& _infos )
	{
		WIN32_FILE_ATTRIBUTE_DATA attribs;

		if( !GetFileAttributesExA( _file, GetFileExInfoStandard, (LPVOID)&attribs ) )
			return false;

		_infos.Date  = ((u64)attribs.ftLastWriteTime.dwHighDateTime) << 32;
		_infos.Date |= ((u64)attribs.ftLastWriteTime.dwLowDateTime);


		_infos.Size  = ((u64)attribs.nFileSizeHigh) << 32;
		_infos.Size |= ((u64)attribs.nFileSizeLow);

		return true;
	}

	bool FileSystem::CreateDirectory( const char* _path )
	{
		
		TVector< Str >  dirsToCreate;
		
		Str dir( _path );

		NormalizeFilePath( dir );
		
		if( dir.rfind( '.' ) != Str::npos )
		{
			//Remove filename

			Str::size_type s = dir.rfind( '/' );
			Str::size_type s2 = dir.rfind( '\\' );
			
			if( s2 != Str::npos )
			{
				if( s != Str::npos )
				{
					s = Max( s, s2 );
				}
				else
				{
					s = s2;
				}
			}
			
			if( s == Str::npos ) 
				return true;
			
			dir = dir.substr( 0, s );
		}
		   
		
		while( true )
		{
			Str::size_type s = dir.rfind( '/' );
			Str::size_type s2 = dir.rfind( '\\' );
			
			if( s2 != Str::npos )
			{
				if( s != Str::npos )
				{
					s = Max( s, s2 );
				}
				else
				{
					s = s2;
				}
			}
			
			if( s == Str::npos )
				break;

			if( FileExist( dir.c_str() ) )
				break;
			
			dirsToCreate.push_back( dir );

			dir = dir.substr( 0, s );
		}
		
		for( s32 i = dirsToCreate.size() - 1 ; i >= 0  ; --i )
		{
			const Str& curDir = dirsToCreate[i];
						
			if( !::CreateDirectoryA( curDir.c_str(), NULL )  )
				return false;
						
			//chmod( curDir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO );
		}
						
		return true;
	}
	
	bool FileSystem::DeleteFile( const char* _file )
	{
		//return remove( _file ) != -1;
		return ::DeleteFileA( _file ) ? true : false;
	}

	bool FileSystem::CopyFile( const char* _srcFile, const char* _dstFile, bool _bOverwrite )
	{
		return ::CopyFileA( _srcFile, _dstFile, !_bOverwrite ) ? true : false;
	}
