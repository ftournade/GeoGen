#pragma once
#ifndef XTM_MEM_MGR_H
#define XTM_MEM_MGR_H



#include <Core/Threading.h>

#ifndef SAFE_FREE
	#define SAFE_FREE(ptr)	if(ptr) {free(ptr); ptr=0x0;}
#endif

#ifndef SAFE_DELETE
	#define SAFE_DELETE(ptr)	if(ptr) {delete ptr; ptr=0x0;}
#endif

#ifndef SAFE_DELETE_ARRAY
	#define SAFE_DELETE_ARRAY(ptr)	if(ptr) {delete[] ptr; ptr=0x0;}
#endif

#ifndef SAFE_RELEASE
	#define SAFE_RELEASE(ptr)	if(ptr) {(ptr)->Release(); ptr=0x0;}
#endif


//TAutoReleaser
//
// Typical usage
// void toto()
// {
//		LPDIRECT3DSURFACE9 pSurf;
//
//		g_pDev->CreateRenderTarget( ..., &pSurf );
//
//		TAutoReleaser renderTarget( pSurf );
// ...
//
//		if( FAILED( ... ) )
//		{
//			//No need to call 'pSurf->Release()'
//			return false;
// }
namespace xtm
{

	template <class T>
	class TAutoReleaser
	{
		T m_ptr;
	 public:
		TAutoReleaser( T ptr ):m_ptr(ptr) {}
		~TAutoReleaser() { if(m_ptr) m_ptr->Release(); }
	};

}

//#undef XTM_USE_MEMORY_MANAGER //TEST


#ifdef XTM_USE_MEMORY_MANAGER

	#define MEMMGR_FILENAME_SIZE 104
	#define MEMMGR_FUNCTION_SIZE 64
	#define MEMMGR_COMMENT_SIZE 82

	#if defined(XTM_WIN32) || defined(XTM_LINUX)
		typedef size_t tSize;
	#else
		typedef long unsigned int tSize;
	#endif

	#if defined(XTM_LINUX)
        #include <new>
        #define XTM_THROW_BAD_ALLOC throw (std::bad_alloc)
        #define XTM_THROW           throw ()
    #else
        #define XTM_THROW_BAD_ALLOC
        #define XTM_THROW
    #endif


	class MemoryManager
	{
		DECLARE_SINGLETON( MemoryManager )

		inline static MemoryManager& Get();

	protected:

		struct AllocDesc
		{
			AllocDesc();

			void Set(	tSize _size,
						const char* _comment,
						const char* _function,
						const char* _filename,
						s32 _line);

			char			filename[MEMMGR_FILENAME_SIZE];
			char			function[MEMMGR_FUNCTION_SIZE];
			char			comment[MEMMGR_COMMENT_SIZE];
			s32				line;
			tSize			size; //in bytes
		};

		//ON 32 bit processors sizeof(AllocBlock) ==

		struct	AllocBlock
		{
			u32				magicNumber;

			AllocDesc		desc;
			AllocBlock*		pPrev;
			AllocBlock*		pNext;
		};

		//ON 32 bit processors sizeof(AllocBlock) ==

	private:

		AllocBlock*				m_pBlockList;
		xtm::CriticalSectionObject	m_LockBlockList;


		char				m_comment[MEMMGR_COMMENT_SIZE];

		//Statistics
		tSize				m_memAllocated;
		u32					m_numBlockAllocated;
		u32 				m_reverseDeleteMisses;

		AllocDesc			m_biggestBlockSize;
		AllocDesc			m_peakMemUsage;

	public:

		~MemoryManager();

		tSize GetMemAllocated();

		void* Malloc(tSize size,const char* function,const char* filename,s32 line);
		void* Realloc(void* address,tSize size,const char* function,const char* filename,s32 line);
		void  Free(void* address,const char* function,const char* filename,s32 line);

		void SetComment( const char* comment );

		void DumpMemory();

		bool CheckNoMansLand( byte* _ptr, tSize _size, byte** _overwrittenMemory );

	};

	IMPLEMENT_SINGLETON( MemoryManager )


	inline void* operator new(tSize _size) XTM_THROW_BAD_ALLOC
	{
		return MemoryManager::Get().Malloc(_size,0x0,0x0,0);
	}

	inline void* operator new[](tSize _size) XTM_THROW_BAD_ALLOC
	{
		return MemoryManager::Get().Malloc(_size,0x0,0x0,0);
	}

	inline void* operator new(tSize size,const char* function,const char* filename,s32 line)
	{
		return MemoryManager::Get().Malloc(size,function,filename,line);
	}

	inline void* operator new[](tSize size,const char* function,const char* filename,s32 line)
	{
		return MemoryManager::Get().Malloc(size,function,filename,line);
	}

	inline void operator delete(void* address) XTM_THROW
	{
		MemoryManager::Get().Free(address,0x0,0x0,0);
	}


	inline void operator delete[](void* address) XTM_THROW
	{
		MemoryManager::Get().Free(address,0x0,0x0,0);
	}

	inline void operator delete(void* address,const char* function,const char* filename,s32 line)
	{
		MemoryManager::Get().Free(address,function,filename,line);
	}


	inline void operator delete[](void* address,const char* function,const char* filename,s32 line)
	{
		MemoryManager::Get().Free(address,function,filename,line);
	}

	//Placement new
	/*
	inline void* operator new( tSize _size, void* _address ) XTM_THROW
	{	
		return _address;
	}

	inline void* operator new[]( tSize _size, void* _address ) XTM_THROW
	{	
		return _address;
	}

	//Placement delete

	inline void operator delete(void*, void*) XTM_THROW
	{
	}

	inline void operator delete[](void*, void*) XTM_THROW
	{
	}
	*/

	#ifdef New
		#pragma error("New already defined")
	#endif

	#define xtmNew new(__FUNCTION__,__FILE__,__LINE__)
	//	#define delete delete(__FUNCTION__,__FILE__,__LINE__)
	//	#define delete[] delete[](__FUNCTION__,__FILE__,__LINE__)

	#define malloc( _size ) MemoryManager::Get().Malloc(_size,__FUNCTION__,__FILE__,__LINE__)
	#define realloc( _ptr, _size ) MemoryManager::Get().Realloc(_ptr,_size,__FUNCTION__,__FILE__,__LINE__)
	#define free( _ptr ) MemoryManager::Get().Free( _ptr, __FUNCTION__,__FILE__,__LINE__)


	#define SET_ALLOC_COMMENT( str )		MemoryManager::Get().SetComment( str );
	#define COMMENT_FUNCTION_ALLOCS( str )	CommentFonctionAllocs autoAllocComment( str );

	class CommentFonctionAllocs
	{
	public:
		inline CommentFonctionAllocs( const char* comment )
		{
			SET_ALLOC_COMMENT( comment )
		}
		inline ~CommentFonctionAllocs()
		{
			SET_ALLOC_COMMENT( 0x0 )
		}
	};

#else
	#define xtmNew new

	#define SET_ALLOC_COMMENT( str )
	#define COMMENT_FUNCTION_ALLOCS( str )
#endif

#endif //XTM_MEM_MGR_H

