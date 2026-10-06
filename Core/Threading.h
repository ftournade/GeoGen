#pragma once



#if defined(XTM_WIN32)

#elif defined(XTM_MACOSX) && !defined(XTM_IPHONE)
	#include <CoreServices/CoreServices.h>
	#include <pthread.h>
	#include <unistd.h>
#else
	#include <pthread.h>
	#include <unistd.h>
#endif

	struct ThreadParameters
	{
		//...
		void*	UserPtr;
	};

	typedef u32 (*ThreadEntryPoint)( ThreadParameters* );


	class Thread
	{
	public:
		Thread();
		~Thread();

		bool Start( ThreadEntryPoint _entryPoint, void* _param );
		bool IsStarted() const { return m_hThread != NULL;  }

		bool Pause();
		bool Resume();
		inline bool IsPaused()  { return m_bPaused; }

		bool WaitForTermination();
		bool TerminateImmediatly();

	private:
		#if defined(XTM_WIN32)
			HANDLE		m_hThread;
		#else
			pthread_t	m_hThread;
		#endif

			bool 		m_bPaused;
	};


	class CriticalSectionObject
	{
	#if defined(XTM_WIN32)
		CRITICAL_SECTION m_CS;
	#elif defined(XTM_MACOSX) && !defined(XTM_IPHONE)
		MPCriticalRegionID m_CS;
	#else
		pthread_mutex_t m_CS;
	#endif

	public:
		CriticalSectionObject();
		~CriticalSectionObject();

		void WaitForFinish();

		inline bool TryEnter();
		inline void Enter();
		inline void Leave();

	};


	class CriticalSection
	{
		CriticalSectionObject& m_cso;
	public:
		inline CriticalSection( CriticalSectionObject& cso )
			: m_cso( cso )
		{
			m_cso.Enter();
		}

		inline ~CriticalSection()
		{
			m_cso.Leave();
		}
	};


    inline bool CriticalSectionObject::TryEnter()
    {
        #if defined(XTM_WIN32)
            //TODO TryEnterCriticalSection( &m_CS );
            return true;
        #elif defined(XTM_MACOSX) && !defined(XTM_IPHONE)
            //TODO ...
            return true;
        #else
            return pthread_mutex_trylock( &m_CS ) == 0;
        #endif
    }


    inline void CriticalSectionObject::Enter()
    {
        //WaitForFinish(); //TODO verifier si c cool de faire ca

        #if defined(XTM_WIN32)
            EnterCriticalSection( &m_CS );
        #elif defined(XTM_MACOSX) && !defined(XTM_IPHONE)
            MPEnterCriticalRegion( m_CS, kDurationForever );
        #else
            pthread_mutex_lock( &m_CS );
        #endif
    }

    inline void CriticalSectionObject::Leave()
    {
        #if defined(XTM_WIN32)
            LeaveCriticalSection( &m_CS );
        #elif defined(XTM_MACOSX) && !defined(XTM_IPHONE)
            MPExitCriticalRegion( m_CS );
        #else
            pthread_mutex_unlock( &m_CS );
        #endif
    }


