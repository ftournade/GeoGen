#ifndef XTM_TLIST_H
#define XTM_TLIST_H

//#define USE_STL_LIST


#include <Core/Debug.h>

#ifdef XTM_WIN32
	//( Warning: needs to have dll-interface to be used by clients of class )
	#pragma warning(disable:4251)
#endif

#if USE_STL_LIST
	#include <list>
#endif

	#if USE_STL_LIST
		
		#define TList std::list

	#else

		template < class T >
		class TList
		{
		private:
			struct Node
			{
				T		m_Data;
				Node*	m_pNext;		
			};
			
		public:
			class iterator
			{
			public:
				inline iterator() : m_pNode(NULL) {}
				inline iterator( Node* _pNode ) : m_pNode(_pNode) {}
				
				inline T& operator*()
				{
					DBG_CHECK( m_pNode && m_pNode->m_pNext );
					
					return m_pNode->m_pNext->m_Data;
				}

				inline const T& operator*() const
				{
					DBG_CHECK( m_pNode && m_pNode->m_pNext );
					
					return m_pNode->m_pNext->m_Data;
				}

				inline T* operator->()
				{
					DBG_CHECK( m_pNode && m_pNode->m_pNext );
					
					return &m_pNode->m_pNext->m_Data;
				}

				inline void operator++()
				{
					if( m_pNode )
						m_pNode = m_pNode->m_pNext;
				}
				
				inline bool operator==( const iterator& _it ) const
				{
					return m_pNode == _it.m_pNode;
				}

				inline bool operator!=( const iterator& _it ) const
				{
					return m_pNode != _it.m_pNode;
				}
				
			private:
				Node* m_pNode;

				friend class TList;
				friend class const_iterator;
			};
			
			
			class const_iterator
			{
			public:
				inline const_iterator() : m_pNode(NULL) {}
				inline const_iterator( const Node* _pNode ) : m_pNode(_pNode) {}

				inline const T& operator*() const
				{
					DBG_CHECK( m_pNode && m_pNode->m_pNext );
					
					return m_pNode->m_pNext->m_Data;
				}
				
				inline const T* operator->() const
				{
					DBG_CHECK( m_pNode && m_pNode->m_pNext );
					
					return &m_pNode->m_pNext->m_Data;
				}

				inline void operator++()
				{
					if( m_pNode )
						m_pNode = m_pNode->m_pNext;
				}

				inline const_iterator& operator=( const iterator& _it )
				{
					m_pNode = _it.m_pNode;
					return *this;
				}

				inline bool operator==( const const_iterator& _it ) const
				{
					return m_pNode == _it.m_pNode;
				}

				inline bool operator==( const iterator& _it ) const
				{
					return m_pNode == _it.m_pNode;
				}

				inline bool operator!=( const const_iterator& _it ) const
				{
					return m_pNode != _it.m_pNode;
				}

				inline bool operator!=( const iterator& _it ) const
				{
					return m_pNode != _it.m_pNode;
				}

			private:
				const Node* m_pNode;
			};
			
			
			TList() :
				m_Size(0)
			{
				m_First.m_pNext = NULL;
				m_pLast = &m_First;
			}

			TList( const TList& _o ) :
				m_Size(0)
			{
				m_First.m_pNext = NULL;
				m_pLast = &m_First;
				
				*this = _o;
			}
			
			~TList()
			{
				clear();
			}
			
			//Usage of operator[] is discouraged (linked lists don't like random access, use TVector instead)
					T&				operator[]( u32 _i );
			const	T&				operator[]( u32 _i ) const;
			
			TList& operator=( const TList& _o );
			
			iterator				atindex( u32 _i );
			
			inline u32				size() const		{ return m_Size; }
			
			inline bool				empty() const		{ return m_Size == 0; }
			
			inline iterator			begin()				{ return iterator( &m_First ); }
			inline const_iterator	begin() const		{ return const_iterator( &m_First ); }

			inline iterator			end()				{ return iterator( m_pLast ); }
			inline const_iterator	end() const			{ return const_iterator( m_pLast ); }
			
			void clear();

			void assertvalid();
			
			void push_front( const T& _val );
			void push_back( const T& _val );
			
			void pop_back();
			void pop_front();

			const T& front() const;
			T& front();

			const T& back() const;
			T& back();

			void insert( u32 _pos, const T& _val );
			void insert( iterator _it, const T& _val );
			
			iterator erase( iterator _it );
			
			void remove( const T& _val );

		private:

			Node	m_First;
			Node*	m_pLast;
			u32		m_Size;
		};

		template < class T >
		T& TList<T>::operator[]( u32 _i )
		{
			DBG_CHECK( _i < m_Size );
			
			Node* pNode = m_First.m_pNext;
			
			for( u32 a=0 ; a < _i ; ++a )
				pNode = pNode->m_pNext;
			
			DBG_CHECK( pNode );

			return pNode->m_Data;
		}
	
		template < class T >
		const T& TList<T>::operator[]( u32 _i ) const
		{
			DBG_CHECK( _i < m_Size );
			
			Node* pNode = m_First.m_pNext;
			
			for( u32 a=0 ; a < _i ; ++a )
				pNode = pNode->m_pNext;
			
			return pNode->m_Data;
		}
	
	
		template < class T >
		TList<T>& TList<T>::operator=( const TList<T>& _o )
		{
			m_Size = 0;
			
			Node* pNode = _o.m_First.m_pNext;
			
			while( pNode )
			{
				push_back( pNode->m_Data );
				pNode = pNode->m_pNext;
			}
			
			return *this;
		}

		template < class T >
		typename TList<T>::iterator TList<T>::atindex( u32 _i )
		{
			DBG_CHECK( _i < m_Size );
			
			Node* pNode = &m_First;
			
			for( u32 a=0 ; a < _i ; ++a )
				pNode = pNode->m_pNext;
				
			return iterator( pNode );
		}
	
		template < class T >
		void TList<T>::clear()
		{
			Node* pNode = m_First.m_pNext;
			
			while( pNode )
			{
				Node* pNextNode = pNode->m_pNext;
			
				delete( pNode );
				
				pNode = pNextNode;
			}
			
			m_First.m_pNext = NULL;
			m_pLast = &m_First;
			m_Size = 0;
		}
			
		template < class T >
		void TList<T>::push_front( const T& _val )
		{
			Node* pNewNode = xtmNew Node;
			pNewNode->m_Data = _val;
			pNewNode->m_pNext = m_First.m_pNext;

			if( !m_First.m_pNext )
				m_pLast = pNewNode;
				
			m_First.m_pNext = pNewNode;
			++m_Size;
		}

		template < class T >
		void TList<T>::push_back( const T& _val )
		{
			Node* pNewNode = xtmNew Node;
			pNewNode->m_Data = _val;
			pNewNode->m_pNext = NULL;
			m_pLast->m_pNext = pNewNode;
			m_pLast = pNewNode;
			++m_Size;
		}

		template < class T >
		void TList<T>::pop_back()
		{
			DBG_CHECK( m_Size > 0 );
			
			//TODO TEST this code !!!!

			--m_Size;

			delete m_pLast;

			if( m_Size == 0 )
			{
				m_pLast = &m_First;
				m_First.m_pNext = NULL;
			}
			else
			{
				Node* pNode = &m_First;

				while( pNode->m_pNext != m_pLast )
				{
					pNode = pNode->m_pNext; 
				}

				m_pLast = pNode;
			}
		}

		
		template < class T >
		void TList<T>::pop_front()
		{
			DBG_CHECK( m_Size > 0 );
		
			--m_Size;

			Node* nodeToDelete = m_First.m_pNext;
			m_First.m_pNext = m_First.m_pNext->m_pNext;
			delete nodeToDelete;

			if( m_Size == 0 )
				m_pLast = &m_First;

		}


		
		template < class T >
		T& TList<T>::front()
		{
			DBG_CHECK( m_Size > 0 );

			return m_First.m_pNext->m_Data;
		}


		template < class T >
		const T& TList<T>::front() const
		{
			DBG_CHECK( m_Size > 0 );

			return m_First.m_pNext->m_Data;
		}

		
		template < class T >
		T& TList<T>::back()
		{
			DBG_CHECK( m_Size > 0 );

			return m_pLast->m_Data;
		}
		
		template < class T >
		const T& TList<T>::back() const
		{
			DBG_CHECK( m_Size > 0 );

			return m_pLast->m_Data;
		}


		template < class T >
		typename TList<T>::iterator TList<T>::erase( iterator _it )
		{	
			DBG_CHECK( m_Size > 0 );

			if( _it.m_pNode && _it.m_pNode->m_pNext )
			{
				Node* pNodeToDelete = _it.m_pNode->m_pNext;
				_it.m_pNode->m_pNext = _it.m_pNode->m_pNext->m_pNext;
				delete pNodeToDelete;

				if( _it.m_pNode->m_pNext == NULL )
				{
					m_pLast = _it.m_pNode;
				}

				--m_Size;
			}

			return _it;
		}

		template < class T >
		void TList<T>::remove( const T& _val )
		{
			for( iterator _it = begin() ; _it != end() ; )
			{
				if (*_it == _val)
					_it = erase( _it );
				else
					++_it;
			}
		}
	
		template < class T >
		void TList<T>::insert( u32 _pos, const T& _val )
		{
			iterator it = atindex( _pos );
			insert( it, _val );
		}
	
		template < class T >
		void TList<T>::insert( iterator _it, const T& _val )
		{
			Node* pNewNode = xtmNew Node;
			pNewNode->m_Data = _val;
			pNewNode->m_pNext = _it.m_pNode->m_pNext;
			_it.m_pNode->m_pNext = pNewNode;
			
			++m_Size;
			
			if( _it.m_pNode == &m_First )
				m_First.m_pNext = pNewNode;
			
			if( _it.m_pNode == m_pLast )
				m_pLast = pNewNode;

		}

		template < class T >
		void TList<T>::assertvalid()
		{
			Node* pNode = &m_First;
			
			u32 sz = 0;

			while( pNode->m_pNext )
			{
				pNode = pNode->m_pNext;

				//try to read from Data
				byte b = *((byte*)&(pNode->m_Data));
				
				++sz;
			}

			CHECK( sz == m_Size );
			CHECK( m_pLast == pNode );
			CHECK( pNode->m_pNext == NULL );
		}

	#endif


#endif
