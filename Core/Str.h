#ifndef XTM_STRING_H
#define XTM_STRING_H


#include <Core/StandardLib.h>
#include <Core/IByteStream.h>
//
#include <Core/Debug.h>

namespace xtm
{
#if 0
	typedef std::basic_string< char, std::char_traits<char>, std::allocator<char> > Str;
#else

	template< class T >
	class TStr
	{
	public:
		typedef u32 size_type;

		static const size_type npos = -1;

	public:
		TStr();
		/*explicit*/ TStr( const T* _str );
		TStr( const TStr& _str );

		~TStr();

		//Read only methods

		inline size_type length() const;
		inline size_type size() const; //same as length()

		inline bool empty() const;

		inline const T* c_str() const;

		size_type find( const TStr& _str ) const;
		size_type rfind( const TStr& _str ) const;
		size_type find( const T* _str ) const;
		size_type rfind( const T* _str ) const;
		size_type find( const T* _str, u32 _length ) const;
		size_type rfind( const T* _str, u32 _length ) const;
		size_type find( T _c, size_type _offset = 0 ) const;
		size_type rfind( T _c, size_type _offset = 0 ) const;

		TStr substr( size_type _offset, size_type _count = npos ) const;

		void resize( size_type _size, T _c = T() );

		void insert( size_type _pos, size_type _count, T _ch );
		void erase( size_type _pos = 0, size_type _count = npos );

		inline const T& operator[]( const size_type _i ) const;

		TStr operator+( const TStr& _str ) const;
		TStr operator+( T _c ) const;


		//Read/Write methods

		void clear();

		void format( const char* _format, ... );

		TStr& operator=( const TStr& _str );

		void operator+=( const TStr& _str );
		void operator+=( T _char );

		inline T& operator[]( const size_type _i );

		bool operator==( const TStr& _str ) const;
		bool operator==( const T* _str ) const;
		bool operator!=( const TStr& _str ) const;
		bool operator<( const TStr& _str ) const;

	protected:
		void MakeUnique();
		void MakeUniqueAndResize( size_type _offset, size_type _size );

	private:
		struct StringData
		{
			T*		m_pStr;
			u32		m_RefCount;
			u32		m_Size;
			u32		m_Allocated;
		};

		StringData* m_pStringData;
	};


	typedef TStr< char > Str;

#include "Str.inl"

#endif

	Str	Format( const char* _format, ... );

	void Replace( Str& _str, char _from, char _to ); //replaces every occurence of _from to _to

	void AppendSlash( Str& _filename );

	Str AppendPath( const Str& _path1, const Str& _path2 );

	Str GetParentDirectory( const Str& _path );



	//	Str  GetFilePath( const Str& _filename );
	void StripFilePath( Str& _filepath );

	//	Str  GetFilename( const Str& _filename );
	void StripFilename( Str& _filename );

	Str  GetFileExtension( const Str& _filename );
	void StripFileExtension( Str& _filename );

	void SplitFilePath( const Str& _fullPath, Str& _path, Str& _name, Str& _extension );

	void StripLODIndex( Str& _filePath ); //E.g. : Convert "mesh_05" to "mesh"

	bool MakePathRelativeTo( Str& _path, const Str& _basePath );

	void NormalizeFilePath( Str& _path ); //Resolve '..' and '.' (eg "./toto/lulu/../anarchy" becomes "d:/temp/toto/anarchy" )

	Str ToLower( const Str& _str );

	inline IByteStream::OffsetType Write( IByteStream& _stream, const Str& _str );
	inline IByteStream::OffsetType Read( IByteStream& _stream, Str& _str );




	// INLINE IMPLEMENTATION //

	inline IByteStream::OffsetType Write( IByteStream& _stream, const Str& _str )
	{
		u32 strSize = (u32)_str.length();

		IByteStream::OffsetType numWroteBytes = Write( _stream, strSize );

		for( u32 i = 0; i < strSize; ++i )
			numWroteBytes += Write( _stream, _str[ i ] );

		return numWroteBytes;
	}

	inline IByteStream::OffsetType Read( IByteStream& _stream, Str& _str )
	{
		u32 strSize;

		IByteStream::OffsetType numReadBytes = Read( _stream, strSize );

		_str.resize( strSize );

		for( u32 i = 0; i < strSize; ++i )
			numReadBytes += Read( _stream, _str[ i ] );

		return numReadBytes;
	}
}

#endif //XTM_STRING_H
