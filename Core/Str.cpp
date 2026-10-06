/*
#ifdef _HAS_EXCEPTIONS
#undef _HAS_EXCEPTIONS
#define _HAS_EXCEPTIONS 0
#endif
*/
//#include "stdafx.h"

#include "Str.h"


	Str Format( const char* _format, ... )
	{
		const int maxSize = 2048;
		char buffer[ maxSize ];

		va_list args;

		va_start( args, _format );
		int count = Xvsnprintf( buffer, maxSize, _format, args );
		va_end( args );

		CHECK_MSG( count != -1, "Buffer overflow while formating string (printf)" );

		return Str( buffer );
	}


	void AppendSlash( Str& _filename )
	{
		Str::size_type length = _filename.length();

		if( length > 0 )
		{
			char lastChar = _filename[ length - 1 ];

			if( (lastChar != '/') && (lastChar != '\\') )
				_filename += '/';
		}
		else
			_filename += '/';

	}

	Str AppendPath( const Str& _path1, const Str& _path2 )
	{
		DBG_CHECK( (_path2[0] != '/') && (_path2[0] != '\\' ) );

		Str path( _path1 );
		AppendSlash( path );
		path += _path2;
		return path;
	}


	Str GetParentDirectory( const Str& _path )
	{

		Str path;

		Str::size_type length = _path.length();

		if( length > 0 )
		{
			char lastChar = _path[ length - 1 ];

			if( (lastChar == '/') || (lastChar == '\\') )
				path = _path.substr( 0, length - 1 );
			else
				path = _path;
		}
		else
			path = _path;

		Str::size_type extSlash = path.rfind('/');
		Str::size_type extBackSlash = path.rfind('\\');
		Str::size_type stripPos = 0;

		if( extSlash != Str::npos )
			stripPos = Max( stripPos, extSlash );

		if( extBackSlash != Str::npos )
			stripPos = Max( stripPos, extBackSlash );

		if( stripPos != 0 )
			return path.substr( 0, stripPos );

		return Str();
	}

	void StripLODIndex( Str& _filePath )
	{
		Str::size_type pos = _filePath.rfind( '_' );
		Str::size_type length = _filePath.length();

		if( ( pos == Str::npos ) || ( pos >= length - 1 ) ) //TODO test
			return;

		for( Str::size_type i=pos + 1 ; i < length ; ++i )
		{
			if( !IsDigit( _filePath[i] ) )
				return;
		}

		_filePath = _filePath.substr( 0, pos );

	}

	Str GetFileExtension( const Str& _filename )
	{
		Str::size_type ext = _filename.rfind('.');

		if( ext != Str::npos )			
			return _filename.substr( ext + 1 ); //TODO extension.ToLowerCase()
		else
			return Str();
	}

	void SplitFilePath( const Str& _fullPath, Str& _path, Str& _name, Str& _extension )
	{
		Str::size_type  n = _fullPath.length();

		//Get extension

		Str::size_type extOffset = _fullPath.rfind('.');
		Str::size_type extSize;

		if( extOffset != Str::npos )
		{
			_extension = _fullPath.substr( extOffset + 1 ); //TODO extension.ToLowerCase()
			extSize = _extension.length() + 1;
		}
		else
		{
			_extension.clear();
			extSize = 0;
		}


		//Get filename and path

		Str::size_type	nameOffset = Str::npos,
						nameOffset1 = _fullPath.rfind('/'),
						nameOffset2 = _fullPath.rfind('\\');


		if( nameOffset1 != Str::npos )
		{
			if( nameOffset2 != Str::npos )
				nameOffset = Max( nameOffset1, nameOffset2 );
			else
				nameOffset = nameOffset1;
		}
		else if( nameOffset2 != Str::npos )
		{
			nameOffset = nameOffset2;
		}

		if( nameOffset != Str::npos )
		{
			_name = _fullPath.substr( nameOffset + 1, extOffset - nameOffset - 1 );
			_path = _fullPath.substr( 0, nameOffset  );
		}
		else
		{
			_name = _fullPath.substr( 0, n - extSize );
			_path.clear();
		}
	}

	void StripFileExtension( Str& _filename )
	{
		Str::size_type ext = _filename.rfind('.');

		if( ext != Str::npos )
			_filename = _filename.substr( 0, ext );
	}

	void StripFilename( Str& _filepath )
	{
		Str::size_type extSlash = _filepath.rfind('/');
		Str::size_type extBackSlash = _filepath.rfind('\\');
		Str::size_type stripPos = 0;
		
		if( extSlash != Str::npos )
			stripPos = Max( stripPos, extSlash );
		
		if( extBackSlash != Str::npos )
			stripPos = Max( stripPos, extBackSlash );
		
		if( stripPos != 0 )
			_filepath = _filepath.substr( 0, stripPos + 1 );
	}
	
	void StripFilePath( Str& _filepath )
	{
		Str::size_type extSlash = _filepath.rfind('/');
		Str::size_type extBackSlash = _filepath.rfind('\\');
		Str::size_type stripPos = 0;

		if( extSlash != Str::npos )
			stripPos = Max( stripPos, extSlash );

		if( extBackSlash != Str::npos )
			stripPos = Max( stripPos, extBackSlash );

		if( stripPos != 0 )
			_filepath = _filepath.substr( stripPos + 1 );
	}

	void Replace( Str& _str, char _from, char _to )
	{
		Str::size_type l = _str.length();

		for( Str::size_type i=0 ; i < l ; ++i )
		{
			if( _str[i] == _from )
				_str[i] = _to;
		}
	}

	bool MakePathRelativeTo( Str& _path, const Str& _basePath )
	{
		Str::size_type l = _basePath.length();

		if( l > _path.length() )
			return false;

		for( Str::size_type i=0 ; i < l ; ++i )
		{
			if( Xtolower( _basePath[i] ) != Xtolower( _path[i] ) )
				return false;
		}

		_path = _path.substr( l );

		return true;
	}

	void NormalizeFilePath( Str& _path )
	{
		Str res;
		Str::size_type offset = 0, lastDir = 0;
		
		Replace( _path, '\\', '/' );

		while( true )
		{
			Str::size_type p = _path.find( '/', offset );

			if( p == Str::npos )
			{
				if( !res.empty() )
					res += '/';

				res += _path.substr( offset );
				break;
			}

			Str s = _path.substr( offset, p - offset );

			offset = p + 1;

			if( s == "." )
			{
				if( res.empty() )
				{
					//TODO working directory
				}
			}
			else if( s == ".." )
			{
				//Remove the most recently added directory from 'res'
				res = res.substr( 0, lastDir );
				lastDir = res.rfind( '/' );
			}
			else
			{
				lastDir = res.length();
				
				if( !res.empty() )
					res += '/';

				res += s;
			}
			
		}

		_path = res;
	}


	Str ToLower( const Str& _str )
	{
		Str lower;

		Str::size_type length = _str.length();

		if( length == 0 )
			return _str;

		lower.resize( length );

		for( Str::size_type i=0 ; i < length ; ++i )
		{
			lower[i] = Xtolower( _str[i] );
		}

		return lower;
	}

