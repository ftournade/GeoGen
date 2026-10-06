#pragma once

#include <Core/TVector.h>
#include <Core/Str.h>


	struct FileInfo
	{
		Str			m_Filename, m_FullPath;
		bool		m_IsDirectory;
		//TODO date ...
	};



	class IFindFile
	{
	public:
		virtual ~IFindFile() {}
		
		virtual			void					BeginFileSearch() {}

		//Must return true if the search must be continued
		virtual			bool					OnFindFile( const FileInfo& _info )=0;
		
		virtual			void					EndFileSearch() {}
	};



	class FindFiles : public IFindFile
	{
	public:
		virtual			bool					OnFindFile( const FileInfo& _info );

		inline const	TVector< FileInfo >&	GetFoundFiles() const { return m_FoundFiles; }

						void					ClearFoundFiles();

	protected:
		TVector< FileInfo > m_FoundFiles;
	};



	class FindFilesByExtension : public FindFiles
	{
	public:

						void					AddExtension( const Str& _ext );

		virtual			bool					OnFindFile( const FileInfo& _info );

	private:
		TVector< Str > m_Extensions;
	};

	class FindFilesByRegex : public FindFiles
	{
	public:
		
		virtual			bool					OnFindFile( const FileInfo& _info );
		
	public:
		Str m_Regex; //TODO real regex
	};
