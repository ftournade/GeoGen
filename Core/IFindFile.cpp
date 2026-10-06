//#include "stdafx.h"
#include "IFindFile.h"


	bool FindFiles::OnFindFile( const FileInfo& _info )
	{
		m_FoundFiles.push_back( _info );
		return true;
	}

	void FindFiles::ClearFoundFiles()
	{
		m_FoundFiles.clear();
	}

	void FindFilesByExtension::AddExtension( const Str& _ext )
	{
		m_Extensions.push_back( ToLower( _ext ) );
	}

	bool FindFilesByExtension::OnFindFile( const FileInfo& _info )
	{
		if( _info.m_IsDirectory )
			return FindFiles::OnFindFile( _info );

		Str ext( ToLower( GetFileExtension( _info.m_Filename ) ) );

		u32 numExtensions = (u32)m_Extensions.size();

		for( u32 iExt=0 ; iExt < numExtensions ; ++iExt )
		{
			if( ext == m_Extensions[ iExt ] )
			{
				return FindFiles::OnFindFile( _info );
			}
		}

		return true;
	}

	
	///////////////
	
	
	
	bool FindFilesByRegex::OnFindFile( const FileInfo& _info )
	{
		//if( _info.m_IsDirectory )
		//	return FindFiles::OnFindFile( _info );
	
		if( _info.m_Filename == m_Regex )
			return FindFiles::OnFindFile( _info );
		
		return true;
	}
		
