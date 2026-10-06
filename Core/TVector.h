#ifndef XTM_TVECTOR_H
#define XTM_TVECTOR_H

//#define USE_STL_VECTOR

//#define DBG_VECTOR

#include <Core/IByteStream.h>
#include <Core/Debug.h>

#ifdef DBG_VECTOR
	#include <Core/Log.h>
#endif

#ifdef XTM_WIN32
	//( Warning: needs to have dll-interface to be used by clients of class )
	#pragma warning(disable:4251)
#endif

	#ifdef USE_STL_VECTOR

		#define TVector std::vector

	#else

	//Adapted from DirectX Utility library

		#define CALL_CONSTRUCTOR( ptr, val ) ::new (ptr) val
		#define CALL_DESTRUCTOR( ptr, type ) (ptr)->~type()

		#undef INLINE
		#define INLINE inline

		template< class T >
		class TVector
		{
		public:
			typedef T* iterator;
			typedef const T* const_iterator;

			TVector();
			TVector( const TVector<T>& a );
			TVector( const T* a, u32 count );
			TVector( u32 count, const T& value = T() );
			~TVector();

			void DeepCopy( const TVector<T>& a );

			TVector& operator=( const TVector<T>& a );
			void operator+=( const TVector<T>& a );

			bool operator<( const TVector<T>& a ) const;
			bool operator==( const TVector<T>& a ) const;

			INLINE			T&		operator[]( u32 _index );
			INLINE	const	T&		operator[]( u32 _index ) const;

			INLINE			T&		back();
			INLINE	const	T&		back() const;

							bool			create( const T* _pData, u32 _count, bool _shrinkIfTooBig=true );

							bool			assign( u32 _count, const T& _value, bool _shrinkIfTooBig=true );

			INLINE			iterator		begin()							{ return m_pData; }
			INLINE			const_iterator	begin() const					{ return m_pData; }

			INLINE			iterator		end()							{ return m_pData + m_Size; }
			INLINE			const_iterator	end() const						{ return m_pData + m_Size; }

			//add / remove

							u32				findfirst( const T& value ) const;
			
							bool			reserve( u32 _count );
							bool			resize( u32 _newSize, const T& value = T(), bool _shrinkIfTooBig=true );

			INLINE			bool			add( u32 _n, const T& value = T() )		{ return resize( size() + _n, value ); }
			INLINE			bool			remove( u32 _n, const T& value = T() )	{ return resize( (_n >= size()) ? 0 : size() - _n, value ); }
							bool			remove( const T& value );
							void			RemoveFast( u32 _index ); //Doesn't preserve ordering !
							bool			remove_all( const T& value );


							bool			push_back( const T& value );
							void			pop_back();

			INLINE			u32				size() const					{ return m_Size; }

			INLINE			bool			empty() const					{ return m_Size == 0; }

			INLINE			void			clear()							{ resize(0); }

#ifdef DBG_VECTOR
							void			dump() const;
#endif

							iterator		insert( u32 _i, u32 _n = 1, const T& value = T() );
							
							iterator		insert( const_iterator _where, const T& value );

							template <class Iter>
							void			insert( const_iterator _where, Iter _first, Iter _last )
							{
								u32 insertCount = _last - _first;

								if( insertCount == 0 )
									return; //nothing to do

								u32 newSize = size() + insertCount;

								T* newArray = (T*)malloc( newSize * sizeof(T) );
								DBG_CHECK( newArray );
								
								//copy prefix
								T* dst = newArray;
								const T* src = m_pData;
								
								for( ; src != _where ; ++dst, ++src )
								{
									CALL_CONSTRUCTOR( dst, T( *src ) );
								}

								//insert new values								
								for( Iter it=_first ; it != _last ; ++it, ++dst )
								{
									CALL_CONSTRUCTOR( dst, T( *it ) );
								}

								//copy suffix
								for( ; dst != newArray + newSize ; ++dst, ++src )
								{
									CALL_CONSTRUCTOR( dst, T( *src ) );
								}

								//erase old array
								for( u32 i=0 ; i < m_Size ; ++i )
								{
									CALL_DESTRUCTOR( m_pData + i, T );
								}

								free( m_pData );

								//replace by new one
								m_pData = newArray;
								m_Size = newSize;
								m_AllocatedSize = newSize;

							}


							void			Replace( const T& _a, const T& _b ); //replaces every occurence of _a by _b

							void			ReverseArrayElements();

		private:
			u32 m_Size;        // # of elements (upperBound - 1)
			T* m_pData;      // the actual array of data
			u32 m_AllocatedSize;     // max allocated

			bool resizeInternal( u32 _newSize );  // This version doesn't call ctor or dtor.
		};

	#endif

	template < class T >
	inline IByteStream::OffsetType Write(IByteStream& _stream, const TVector<T>& _vector )
	{
		u32 vecSize = (u32)_vector.size();

		IByteStream::OffsetType numWroteBytes = Write( _stream, vecSize );

		for( u32 i=0 ; i < vecSize ; ++i )
			numWroteBytes += Write( _stream, _vector[i] );

		return numWroteBytes;
	}

	template < class T >
	inline IByteStream::OffsetType Read(IByteStream& _stream, TVector<T>& _vector )
	{
		u32 vecSize;

		IByteStream::OffsetType numReadBytes = Read( _stream, vecSize );

		if( vecSize > 0 )
			_vector.resize( vecSize );

		for( u32 i=0 ; i < vecSize ; ++i )
			numReadBytes += Read( _stream, _vector[i] );

		return numReadBytes;
	}


#ifndef USE_STL_VECTOR


	template< class T >
	TVector<T>::TVector()
	:	m_pData( NULL ),
		m_Size( 0 ),
		m_AllocatedSize( 0 )
	{
	}

	template< class T >
	TVector<T>::TVector( const TVector<T>& a )
	:	m_pData( NULL ),
		m_Size( 0 ),
		m_AllocatedSize( 0 )
	{
		resize( a.size() );

		for( u32 i=0 ; i < a.m_Size; i++ )
			m_pData[i] = a.m_pData[i];
	}

	template< class T >
	TVector<T>::TVector( const T* a, u32 count )
	{
		m_pData = NULL;
		m_Size = 0;
		m_AllocatedSize = 0;

		resize( count );

		for( u32 i=0 ; i < count; i++ )
			m_pData[i] = a[i];
	}
	
	template< class T >
	TVector<T>::TVector( u32 count, const T& value )
	{
		m_pData = NULL;
		m_Size = 0;
		m_AllocatedSize = 0;
		
		resize( count, value );
	}

	template< class T >
	TVector<T>::~TVector()
	{
		free( m_pData );
	}

	template< class T >
	void TVector<T>::DeepCopy( const TVector<T>& a )
	{
		u32 n = a.size();

		resize( n );

		for( u32 i = 0 ; i < n ; ++i )
		{
			m_pData[ i ] = a[ i ] ? (T)a[ i ]->Clone() : nullptr;
		}
	}

	template< class T >
	TVector<T>& TVector<T>::operator=( const TVector<T>& a )
	{
		if( this == &a )
			return *this;

		resize( a.size() );

		for( u32 i=0 ; i < a.m_Size; i++ )
			m_pData[i] = a.m_pData[i];

		return *this;
	}

	template< class T >
	void TVector<T>::operator+=( const TVector<T>& a )
	{
		u32 n1 = size();
		u32 n2 = a.size();
		
		resize( n1 + n2 );
		
		for( u32 i=0 ; i < n2 ; ++i )
			m_pData[ n1 + i ] = a.m_pData[ i ];
	}

	template< class T >
	INLINE T& TVector<T>::operator[]( u32 _index )
	{
		DBG_CHECK( _index < m_Size );
		return m_pData[_index];
	}

	template< class T >
	INLINE const T&	TVector<T>::operator[]( u32 _index ) const
	{
		DBG_CHECK( _index < m_Size );
		return m_pData[_index];
	}

	template< class T >
	const T& TVector<T>::back() const
	{
		DBG_CHECK( m_Size > 0 );
		return m_pData[ m_Size -1 ];
	}
	
	template< class T >
	T& TVector<T>::back()
	{
		DBG_CHECK( m_Size > 0 );
		return m_pData[ m_Size -1 ];
	}

	template< class T >
	bool TVector<T>::create( const T* _pData, u32 _count, bool _shrinkIfTooBig )
	{
		if( _count > m_Size )
		{
			//Need to grow
			resize( _count );
		}
		else if( _count < m_Size )
		{
			//We are bigger than necessary

			if( _shrinkIfTooBig )
			{
				resize( _count );
			}
		}

		//TODO use memcopy ?

		for( u32 i=0 ; i < _count ; ++i )
			m_pData[i] = _pData[i];

		return true;
	}


	template< class T >
	bool TVector<T>::resizeInternal( u32 _newSize )
	{
/*
		if( _newSize == 0 )
		{
			// Shrink to 0 size & cleanup
			if( m_pData )
			{
				free( m_pData );
				m_pData = NULL;
			}

			m_AllocatedSize = 0;
			m_Size = 0;
		}
		else */
		if( (_newSize > 0) && ( (m_pData == NULL) || (_newSize > m_AllocatedSize)) )
		{
			// Grow array

			u32 nGrowBy = ( m_AllocatedSize == 0 ) ? 16 : m_AllocatedSize;

			_newSize = Max( _newSize, m_AllocatedSize + nGrowBy );

			//TODO use memory manager
			T* pDataNew = (T*) realloc( m_pData, _newSize * sizeof(T) );

			if( !pDataNew )
			{
				DBG_CHECK_MSG( false, "TVector failed to realloc" );
				return false;
			}

			m_pData = pDataNew;
			m_AllocatedSize = _newSize;
		}

		return true;
	}

	template< class T >
	bool TVector<T>::reserve( u32 _count )
	{
		if( m_AllocatedSize >= _count )
			return true;

		return resizeInternal( _count );
	}

	//--------------------------------------------------------------------------------------
	template< class T >
	bool TVector<T>::resize( u32 _newSize, const T& _value, bool _shrinkIfTooBig )
	{
		u32 oldSize = m_Size;

		if( oldSize > _newSize )
		{
			// Removing elements. Call dtor.

			for( u32 i = _newSize; i < oldSize; ++i )
				CALL_DESTRUCTOR( m_pData + i, T );
		}

		// Adjust buffer.  Note that there's no need to check for error
		// since if it happens, oldSize == _newSize will be true.)
		bool res = resizeInternal( _newSize );

		if( !res )
			return false;

		m_Size = _newSize;

		if( oldSize < _newSize )
		{
			for( u32 i = oldSize; i < _newSize; ++i )
				CALL_CONSTRUCTOR( m_pData + i, T( _value ) );
		}

		return true;
	}

	template< class T >
	bool TVector<T>::assign( u32 _count, const T& _value, bool _shrinkIfTooBig )
	{
		if( !resize( _count, T(), _shrinkIfTooBig ) )
			return false;

		for( u32 i=0 ; i < _count ; ++i )
			m_pData[i] = _value;

		return true;
	}

	//--------------------------------------------------------------------------------------
	template< class T >
	bool TVector<T>::push_back( const T& _value )
	{
		if( !resizeInternal( m_Size + 1 ) )
			return false;

		CALL_CONSTRUCTOR( &m_pData[m_Size], T( _value ) );

		// Assign
		++m_Size;

		return true;
	}


	template< class T >
	void TVector<T>::pop_back()
	{
		DBG_CHECK( m_Size > 0 );	
		
		//Call the destructor
		CALL_DESTRUCTOR( &m_pData[ m_Size - 1 ], T );

		if( !resizeInternal( m_Size - 1 ) )
		{
			DBG_CHECK( false );
		}

		--m_Size;
	}

	template< class T >
	bool TVector<T>::remove( const T& value )
	{
		//TODO call constructors and destructors ?

		bool found = false;

		for( u32 i=0 ; i < m_Size ; ++i )
		{
			if( !found )
			{
				if( m_pData[i] == value )
				{
					found = true;
					m_pData[i] = m_pData[i+1];
					--m_Size;
				}
			}
			else
			{
				m_pData[i] = m_pData[i+1];
			}

		}

		return found;
	}

	template <class T>
	void TVector< T >::RemoveFast( u32 _index )
	{
		DBG_CHECK( _index < m_Size );

		if( _index != m_Size - 1 )
			m_pData[ _index ] = m_pData[ m_Size - 1 ]; //Swap with the last element

		resize( m_Size - 1 );
	}

	/*
	template< class T >
	bool TVector<T>::remove_all( const T& value )
	{
		//TODO
		
		return false;
	}
	*/


	template< class T >
	bool TVector<T>::operator==( const TVector<T>& a ) const
	{
		if( m_Size != a.m_Size )
			return false;

		for( u32 i=0 ; i < m_Size ; ++i )
		{
			if( m_pData[i] != a.m_pData[i] )
				return false;
		}

		return true;
	}


	template< class T >
	bool TVector<T>::operator<( const TVector<T>& a ) const
	{
		u32 n = Min( m_Size, a.m_Size );

		const T* pData1 = m_pData;
		const T* pData2 = a.m_pData;

		u32 i;

		for( i=0 ; i < n ; ++i, ++pData1, ++pData2 )
		{
			if( *pData1 < *pData2 )
				return true;
			else if( *pData2 < *pData1 )
				return false;
		}

		return ( (i == m_Size) && (i != a.m_Size) );

/*
		if( m_Size < a.m_Size )
			return true;

		if( m_Size > a.m_Size )
			return false;

		for( u32 i=0 ; i < m_Size ; ++i )
		{
			if( m_pData[i] < a.m_pData[i] )
			{
				DBG_CHECK( !( a.operator<(*this) ) );
				return true;
			}
			else if( m_pData[i] < )
		}

		return false;	//identical*/
	}
	
	template< class T >
	u32 TVector<T>::findfirst( const T& value ) const
	{
		for( u32 i=0 ; i < m_Size ; ++i )
		{
			if( m_pData[i] == value )
				return i;
		}
		
		return (u32)-1;
	}

	template< class T >
	typename TVector<T>::iterator TVector<T>::insert( typename TVector<T>::const_iterator _where, const T& _value )
	{
		u32 newSize = size() + 1;

		T* newArray = (T*)malloc( newSize * sizeof(T) );
		DBG_CHECK( newArray );
								
		//copy prefix
		T* dst = newArray;
		const T* src = m_pData;
										
		for( ; src != _where ; ++dst, ++src )
		{
			CALL_CONSTRUCTOR( dst, T( *src ) );
		}

		//insert new values
		iterator insertedElement( dst );
		CALL_CONSTRUCTOR( dst, T( _value ) );
		++dst;

		//copy suffix
		for( ; dst != newArray + newSize ; ++dst, ++src )
		{
			CALL_CONSTRUCTOR( dst, T( *src ) );
		}

		//erase old array
		for( u32 i=0 ; i < m_Size ; ++i )
		{
			CALL_DESTRUCTOR( m_pData + i, T );
		}

		free( m_pData );

		//replace by new one
		m_pData = newArray;
		m_Size = newSize;
		m_AllocatedSize = newSize;

		return insertedElement;
	}

	template <class T>
	void TVector<T>::Replace( const T& _a, const T& _b )
	{
		for( u32 i = 0; i < m_Size; ++i )
		{
			if( m_pData[ i ] == _a )
				m_pData[ i ] = _b;
		}
	}

	template <class T>
	void TVector<T>::ReverseArrayElements()
	{
		int sz = m_Size / 2;

		for( int i=0 ; i < sz ; ++i )
		{
			TSwap( m_pData[i], m_pData[m_Size - 1 - i] );
		}
	}

#ifdef DBG_VECTOR
	template< class T >
	void TVector<T>::dump() const
	{
		LOG( "TVector (0x%x): 0x%x size %d allocated %d", this, m_pData, m_Size, m_AllocatedSize );
	}
#endif

	/*
	template <class T, class Iter>
	bool TVector<T>::insert( const_iterator _where, Iter _first, Iter _last )
	{
		return true;
	}

	//--------------------------------------------------------------------------------------
	template< class T >
	bool TVector<T>::Insert( int nIndex, const T& value )
	{
		bool hr;

		// Validate index
		if( nIndex < 0 || nIndex > m_Size )
		{
			assert( false );
			return E_INVALIDARG;
		}

		// Prepare the buffer
		if( FAILED( hr = resizeInternal( m_Size + 1 ) ) )
			return hr;

		// Shift the array
		MoveMemory( &m_pData[nIndex+1], &m_pData[nIndex], sizeof(T) * (m_Size - nIndex) );

		// Construct the new element
		::new (&m_pData[nIndex]) T;

		// Set the value and increase the size
		m_pData[nIndex] = value;
		++m_Size;

		return S_OK;
	}


	//--------------------------------------------------------------------------------------
	template< class T >
	bool TVector<T>::Remove( int nIndex )
	{
		if( nIndex < 0 || nIndex >= m_Size )
		{
		assert( false );
		return E_INVALIDARG;
		}

		// Destruct the element to be removed
		m_pData[nIndex].~T();

		// Compact the array and decrease the size
		MoveMemory( &m_pData[nIndex], &m_pData[nIndex+1], sizeof(T) * (m_Size - (nIndex+1)) );
		--m_Size;

		return S_OK;
	}


#endif


*/

#endif

#endif
