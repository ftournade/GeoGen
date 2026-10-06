
template< class T >
TStr<T>::TStr() :
	m_pStringData( nullptr )
{

}

template< class T >
TStr<T>::TStr( const T* _str )
{
	size_type length = _str ? (size_type)Xstrlen( _str ) : 0;

	//TODO use a foolproof strlen method which handles WideChar

	if( length == 0 )
	{
		m_pStringData = nullptr;
		return;
	}

	m_pStringData = xtmNew StringData;

	m_pStringData->m_Allocated = length + 1;
	m_pStringData->m_Size = length;
	m_pStringData->m_pStr = (T*)malloc( m_pStringData->m_Allocated * sizeof(T) );
	m_pStringData->m_RefCount = 1;

	Xmemcpy( m_pStringData->m_pStr, _str, length + 1 );
}


template< class T >
TStr<T>::TStr( const TStr& _str )
{
	m_pStringData = _str.m_pStringData;

	if( m_pStringData )
		m_pStringData->m_RefCount++;
}

template< class T >
TStr<T>::~TStr()
{
	clear();
}

template< class T >
inline typename TStr<T>::size_type TStr<T>::length() const
{
	return m_pStringData ? m_pStringData->m_Size : 0;
}

template< class T >
inline typename TStr<T>::size_type TStr<T>::size() const
{
	return length();
}

template< class T >
inline bool TStr<T>::empty() const
{
	return (!m_pStringData || (m_pStringData->m_Size == 0));
}


template< class T >
inline const T* TStr<T>::c_str() const
{
	return m_pStringData ?
				m_pStringData->m_pStr :
				reinterpret_cast< const T * >( &m_pStringData ); // Trick to return a pointer to an "empty string" (null-terminated)
}

template< class T >
void TStr<T>::clear()
{
	if( m_pStringData )
	{
		--m_pStringData->m_RefCount;

		if( m_pStringData->m_RefCount == 0 )
		{
			DBG_CHECK( m_pStringData->m_pStr );

			free( m_pStringData->m_pStr );
			delete m_pStringData;
		}

		m_pStringData = nullptr;
	}
}


template< class T >
typename TStr<T>::size_type TStr<T>::find( const TStr<T>& _str ) const
{
	return _str.m_pStringData ? find( _str.m_pStringData->m_pStr, _str.m_pStringData->m_Size ) : npos;
}

template< class T >
typename TStr<T>::size_type TStr<T>::rfind( const TStr<T>& _str ) const
{
	return _str.m_pStringData ? rfind( _str.m_pStringData->m_pStr, _str.m_pStringData->m_Size ) : npos;
}

template< class T >
typename TStr<T>::size_type TStr<T>::find( const T* _str ) const
{
	return find( _str, (u32)Xstrlen( _str ) );
}

template< class T >
typename TStr<T>::size_type TStr<T>::rfind( const T* _str ) const
{
	return rfind( _str, (u32)Xstrlen( _str ) );
}

template< class T >
typename TStr<T>::size_type TStr<T>::find( const T* _str, u32 _length ) const
{
	if( !m_pStringData || ( m_pStringData->m_Size == 0 ) )
		return npos;

	const T* cur = m_pStringData->m_pStr;

	const T* a = cur;
	const T* b = _str;

	while( true )
	{

		if( *b == (T)'\0' )
		{
			return (size_type)(cur - m_pStringData->m_pStr); //found it !
		}

		if( *a == (T)'\0' )
		{
			return npos;
		}

		if( *a == *b )
		{
			++a;
			++b;
		}
		else
		{
			//no match, continue on next character
			++cur;
			a = cur;
			b = _str;
		}
	}
}

template< class T >
typename TStr<T>::size_type TStr<T>::rfind( const T* _str, u32 _length ) const
{
	if( !m_pStringData || !_str )
		return npos;

	size_type offset = m_pStringData->m_Size - _length;

	while( offset > 0 )
	{
		if( Xstrncmp( m_pStringData->m_pStr + offset, _str, _length ) == 0 )
		{
			return offset;
		}

		--offset;
	}

	return npos;
}

template< class T >
typename TStr<T>::size_type TStr<T>::find( T _c, size_type _offset ) const
{
	if( !m_pStringData )
		return npos;

	for( u32 i = _offset ; i < m_pStringData->m_Size ; ++i )
	{
		if( m_pStringData->m_pStr[ i ] == _c )
		{
			return i;
		}
	}

	return npos;
}

template< class T >
typename TStr<T>::size_type TStr<T>::rfind( T _c, size_type _offset ) const
{
	if( !m_pStringData )
		return npos;

	for( s32 i = (s32)m_pStringData->m_Size - 1; i >= 0; --i )
	{
		if( m_pStringData->m_pStr[ i ] == _c )
		{
			return i;
		}
	}

	return npos;
}

template< class T >
TStr<T> TStr<T>::substr( size_type _offset, size_type _count ) const
{
	TStr<T> res( *this );

	if( _count == npos )
	{
		_count = size() - _offset;
	}

	res.MakeUniqueAndResize( _offset, _count );

	return res;
}

template< class T >
void TStr<T>::resize( size_type _size, T _c )
{
	if( _size == 0 )
	{
		if( m_pStringData )
		{
			--m_pStringData->m_RefCount;

			if( m_pStringData->m_RefCount == 0 )
			{
				free( m_pStringData->m_pStr );
				delete m_pStringData;
			}

			m_pStringData = nullptr;
		}

		return;
	}

	if( !m_pStringData )
	{
		MakeUniqueAndResize( 0, _size ); //TODO pass _c
	}
	else if( _size > m_pStringData->m_Size )
	{
		//need to grow
		MakeUniqueAndResize( 0, _size ); //TODO pass _c
	}
	else if( _size <= m_pStringData->m_Size )
	{
		//Nothing to do (idea: we could realloc)

	}

	m_pStringData->m_Size = _size;
	m_pStringData->m_pStr[ _size ] = (T)'\0';
}

template< class T >
void TStr<T>::insert( size_type _pos, size_type _count, T _ch )
{
	DBG_CHECK( _count > 0 );

	u32 sz = size();

	MakeUniqueAndResize( 0, sz + _count );

	const T* src = m_pStringData->m_pStr + sz - 1;
	T* dst = m_pStringData->m_pStr + sz - 1 + _count;

	for( u32 i = 0; i < _count; ++i )
	{
		*dst-- = *src--;
	}

	for( u32 i = 0; i < _count; ++i )
	{
		m_pStringData->m_pStr[ _pos + i ] = _ch;
	}

	m_pStringData->m_Size += _count;
	m_pStringData->m_pStr[ m_pStringData->m_Size - 1 ] = (T)'\0';
}

template< class T >
void TStr<T>::erase( size_type _pos, size_type _count )
{
	DBG_CHECK( _count > 0 );
	DBG_CHECK( _pos + _count <= m_pStringData->m_Size );

	MakeUnique();

	const T* src = m_pStringData->m_pStr + _pos + _count;
	T* dst = m_pStringData->m_pStr + _pos;

	u32 n = m_pStringData->m_Size - _pos - _count;

	for( u32 i = 0; i < n ; ++i )
	{
		*dst++ = *src++;
	}

	m_pStringData->m_Size -= _count;
	m_pStringData->m_pStr[ m_pStringData->m_Size - 1 ] = (T)'\0';
}

template< class T >
void TStr<T>::format( const char* _format, ... )
{
	const u32 bufferSize = 2048;
	T buffer[ bufferSize ];

	va_list args;
	va_start( args, _format );

	int sz = vsnprintf_s( buffer, bufferSize - 1, _format, args );
	DBG_CHECK( sz >= 0 );

	MakeUniqueAndResize( 0, sz );
	Xmemcpy( m_pStringData->m_pStr, buffer, sz + 1 );
}

template< class T >
TStr<T>& TStr<T>::operator=( const TStr& _str )
{
	if( &_str == this )
		return *this;

	clear();

	if( _str.m_pStringData )
	{
		m_pStringData = _str.m_pStringData;
		++m_pStringData->m_RefCount;
	}

	return *this;
}

template< class T >
TStr<T> TStr<T>::operator+( const TStr& _str ) const
{
	size_type sz = size() + _str.size();

	TStr<T> res( *this );
	res.MakeUniqueAndResize( 0, sz );

	if( _str.m_pStringData )
		Xmemcpy( res.m_pStringData->m_pStr + (m_pStringData ? m_pStringData->m_Size : 0), _str.m_pStringData->m_pStr, _str.m_pStringData->m_Size );

	res.m_pStringData->m_Size = sz;
	res.m_pStringData->m_pStr[ sz ] = (T)'\0';

	return res;
}

template< class T >
TStr<T> TStr<T>::operator+( T _c ) const
{
	u32 sz = size();

	TStr<T> res( *this );
	res.MakeUniqueAndResize( 0, sz + 1 );

	res.m_pStringData->m_Size = sz + 1;
	res.m_pStringData->m_pStr[ sz ] = _c;
	res.m_pStringData->m_pStr[ sz + 1 ] = (T)'\0';

	return res;
}


template< class T >
void TStr<T>::operator+=( const TStr& _str )
{
	if( _str.size() == 0 )
		return;

	size_type sz = size() + _str.size();

	MakeUniqueAndResize( 0, sz );

	if( _str.m_pStringData )
		Xmemcpy( m_pStringData->m_pStr + m_pStringData->m_Size, _str.m_pStringData->m_pStr, _str.m_pStringData->m_Size );

	m_pStringData->m_pStr[ sz ] = (T)'\0';
	m_pStringData->m_Size = sz;
}

template< class T >
void TStr<T>::operator+=( T _char )
{
	size_type sz = size();

	MakeUniqueAndResize( 0, sz + 1 );

	m_pStringData->m_pStr[ sz ] = _char;
	m_pStringData->m_pStr[ sz + 1 ] = (T)'\0';
	m_pStringData->m_Size = sz + 1;
}


template< class T >
inline T& TStr<T>::operator[]( const size_type _i )
{
	//Note: can't update m_Size here :( must fix this !

	MakeUnique();
	DBG_CHECK( m_pStringData && m_pStringData->m_pStr );
	DBG_CHECK( _i < m_pStringData->m_Size );

	return m_pStringData->m_pStr[ _i ];
}

template< class T >
inline const T& TStr<T>::operator[]( const size_type _i ) const
{
	DBG_CHECK( m_pStringData && m_pStringData->m_pStr );
	DBG_CHECK( _i < m_pStringData->m_Size );

	return m_pStringData->m_pStr[ _i ];
}

template< class T >
bool TStr<T>::operator==( const TStr& _str ) const
{
	DBG_CHECK( m_pStringData && m_pStringData->m_pStr );
	DBG_CHECK( _str.m_pStringData && _str.m_pStringData->m_pStr );

	return	( m_pStringData == _str.m_pStringData ) ||
			( strcmp( m_pStringData->m_pStr, _str.m_pStringData->m_pStr ) == 0 );
}

template< class T >
bool TStr<T>::operator==( const T* _str ) const
{
	DBG_CHECK( m_pStringData && m_pStringData->m_pStr );
	DBG_CHECK( _str );

	return (strcmp( m_pStringData->m_pStr, _str ) == 0);
}

template< class T >
bool TStr<T>::operator!=( const TStr& _str ) const
{
	DBG_CHECK( m_pStringData && m_pStringData->m_pStr );
	DBG_CHECK( _str.m_pStringData && _str.m_pStringData->m_pStr );

	return strcmp( m_pStringData->m_pStr, _str.m_pStringData->m_pStr ) != 0;
}

template< class T >
bool TStr<T>::operator<( const TStr& _str ) const
{
	DBG_CHECK( m_pStringData && m_pStringData->m_pStr );
	DBG_CHECK( _str.m_pStringData && _str.m_pStringData->m_pStr );

	return strcmp( m_pStringData->m_pStr, _str.m_pStringData->m_pStr ) < 0;
}

template< class T >
void TStr<T>::MakeUnique()
{
	if( m_pStringData && (m_pStringData->m_RefCount > 1) )
	{
		--m_pStringData->m_RefCount;

		StringData* pNewStringData = xtmNew StringData;

		pNewStringData->m_RefCount = 1;
		pNewStringData->m_Size = m_pStringData->m_Size;
		pNewStringData->m_Allocated = m_pStringData->m_Size + 1;

		pNewStringData->m_pStr = (T*)malloc( pNewStringData->m_Allocated * sizeof(T) );

		//TODO don't use strcpy ( NOT working if T != char )
		Xmemcpy( pNewStringData->m_pStr, m_pStringData->m_pStr, m_pStringData->m_Allocated * sizeof(T) );

		m_pStringData = pNewStringData;
	}
}


template< class T >
void TStr<T>::MakeUniqueAndResize( size_type _offset, size_type _size )
{
	DBG_CHECK( _size > 0 );

	if( (_offset == 0) && m_pStringData && (m_pStringData->m_RefCount == 1) )
	{
		//fast path (realloc, no copy)

		m_pStringData->m_Size = Min( _size, m_pStringData->m_Size );
		m_pStringData->m_Allocated = _size + 1;

		T* newPtr = (T*)realloc( m_pStringData->m_pStr, (_size + 1) * sizeof(T) );

		if( !newPtr )
		{
			//should come here ??? (is realloc supposed to return NULL except for OOM ?)
			newPtr = (T*)malloc( (_size + 1) * sizeof( T ) );
			DBG_CHECK( newPtr );

			Xmemcpy( newPtr, m_pStringData->m_pStr, m_pStringData->m_Size );

			free( m_pStringData->m_pStr );

			m_pStringData->m_pStr = newPtr;
		}
		else if( newPtr == m_pStringData->m_pStr )
		{
			//FAST PATH realloc successfully expanded/retracted the block
			static int bkpt = 0;
			++bkpt;
		}
		else
		{
			//realloc fallbacked on a classic malloc
			//did realloc do the copy ? Xmemcpy( newPtr, m_pStringData->m_pStr, Min( m_pStringData->m_Size, _size ) * sizeof(T) );
			m_pStringData->m_pStr = newPtr;
		}

		newPtr[ m_pStringData->m_Size ] = (T)'\0';

		return;
	}


	if( m_pStringData )
	{
		DBG_CHECK( _offset < m_pStringData->m_Size );

		u32 c = Min( _size, m_pStringData->m_Size - _offset );

		if( m_pStringData->m_RefCount == 1 )
		{
			T* newPtr = (T*)malloc( (_size + 1) * sizeof(T) );
			DBG_CHECK( newPtr );

			Xmemcpy( newPtr, m_pStringData->m_pStr + _offset, c * sizeof(T) );
			newPtr[ c ] = (T)'\0';

			m_pStringData->m_pStr = newPtr;
			m_pStringData->m_Allocated = _size + 1;
			m_pStringData->m_Size = c;
		}
		else
		{
			--m_pStringData->m_RefCount; //TODO thread safe

			StringData* pNewStringData = xtmNew StringData;

			pNewStringData->m_RefCount = 1;
			pNewStringData->m_Allocated = _size + 1;
			pNewStringData->m_Size = c;
			pNewStringData->m_pStr = (T*)malloc( pNewStringData->m_Allocated * sizeof(T) );

			Xmemcpy( pNewStringData->m_pStr, m_pStringData->m_pStr + _offset, c * sizeof(T) );
			pNewStringData->m_pStr[ c ] = (T)'\0';

			m_pStringData = pNewStringData;
		}
	}
	else
	{
		DBG_CHECK( _size > 0 );

		m_pStringData = xtmNew StringData;
		m_pStringData->m_RefCount = 1;
		m_pStringData->m_Allocated = _size + 1;
		m_pStringData->m_Size = 0;

		m_pStringData->m_pStr = (T*)malloc( m_pStringData->m_Allocated * sizeof(T) );
		m_pStringData->m_pStr[ 0 ] = (T)'\0';
	}

}


