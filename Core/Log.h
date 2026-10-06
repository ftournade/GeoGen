#pragma once

#include "FileStream.h"


#include <stdio.h>

#if defined(XTM_WIN32)
	#include <tchar.h>
//	#include <DxErr.h>
#endif


#define LOG		g_log.SetHTMLColor(0x00000000); g_log.printf
#define LOG_R	g_log.SetHTMLColor(0x00ff0000); g_log.printf
#define LOG_G	g_log.SetHTMLColor(0x00007f00); g_log.printf

#define LOG_D3DERROR(str, hr)	g_log.SetHTMLColor(0x00ff0000);		\
								g_log.printf( str ## " ( %s - %s )",	\
											DXGetErrorString(hr),			\
											DXGetErrorDescription(hr) )	

//#define ENABLE_VERBOSE_LOG //Uncomment to debug in situations where no debugger or symbols is present

//Between 0 and 3 inclusive, the higher the more verbose
//Level 1 : Non critical messages
//Level 2 : REALLY non critical messages
//Level 3 : Very verbose, for debugging when no symbol or debugger is present ( e.g "Entered World::Render", "[D3D] DrawPrimitive(...)" )
#ifdef XTM_DISTRUTION
	#define LOG_VERBOSITY_LEVEL 0
#else
	#define LOG_VERBOSITY_LEVEL 2 
#endif

#if ( LOG_VERBOSITY_LEVEL == 3 )

	#define LOG0_L1( str )					LOG( str )
	#define LOG1_L1( str, a0 )				LOG( str, a0 )
	#define LOG2_L1( str, a0, a1 )			LOG( str, a0, a1 )
	#define LOG3_L1( str, a0, a1, a2 )		LOG( str, a0, a1, a2 )
	#define LOG4_L1( str, a0, a1, a2, a3 )	LOG( str, a0, a1, a2, a3 )

	#define LOG0_L2( str )					LOG( str )
	#define LOG1_L2( str, a0 )				LOG( str, a0 )
	#define LOG2_L2( str, a0, a1 )			LOG( str, a0, a1 )
	#define LOG3_L2( str, a0, a1, a2 )		LOG( str, a0, a1, a2 )
	#define LOG4_L2( str, a0, a1, a2, a3 )	LOG( str, a0, a1, a2, a3 )

	#define LOG0_L3( str )					LOG( str )
	#define LOG1_L3( str, a0 )				LOG( str, a0 )
	#define LOG2_L3( str, a0, a1 )			LOG( str, a0, a1 )
	#define LOG3_L3( str, a0, a1, a2 )		LOG( str, a0, a1, a2 )
	#define LOG4_L3( str, a0, a1, a2, a3 )	LOG( str, a0, a1, a2, a3 )

#elif ( LOG_VERBOSITY_LEVEL == 2 )

	#define LOG0_L1( str )					LOG( str )
	#define LOG1_L1( str, a0 )				LOG( str, a0 )
	#define LOG2_L1( str, a0, a1 )			LOG( str, a0, a1 )
	#define LOG3_L1( str, a0, a1, a2 )		LOG( str, a0, a1, a2 )
	#define LOG4_L1( str, a0, a1, a2, a3 )	LOG( str, a0, a1, a2, a3 )

	#define LOG0_L2( str )					LOG( str )
	#define LOG1_L2( str, a0 )				LOG( str, a0 )
	#define LOG2_L2( str, a0, a1 )			LOG( str, a0, a1 )
	#define LOG3_L2( str, a0, a1, a2 )		LOG( str, a0, a1, a2 )
	#define LOG4_L2( str, a0, a1, a2, a3 )	LOG( str, a0, a1, a2, a3 )

	#define LOG0_L3( str )					
	#define LOG1_L3( str, a0 )				
	#define LOG2_L3( str, a0, a1 )			
	#define LOG3_L3( str, a0, a1, a2 )		
	#define LOG4_L3( str, a0, a1, a2, a3 )	

#elif ( LOG_VERBOSITY_LEVEL == 1 )

	#define LOG0_L1( str )					LOG( str )
	#define LOG1_L1( str, a0 )				LOG( str, a0 )
	#define LOG2_L1( str, a0, a1 )			LOG( str, a0, a1 )
	#define LOG3_L1( str, a0, a1, a2 )		LOG( str, a0, a1, a2 )
	#define LOG4_L1( str, a0, a1, a2, a3 )	LOG( str, a0, a1, a2, a3 )

	#define LOG0_L2( str )					
	#define LOG1_L2( str, a0 )				
	#define LOG2_L2( str, a0, a1 )			
	#define LOG3_L2( str, a0, a1, a2 )	
	#define LOG4_L2( str, a0, a1, a2, a3 )

	#define LOG0_L3( str )					
	#define LOG1_L3( str, a0 )				
	#define LOG2_L3( str, a0, a1 )			
	#define LOG3_L3( str, a0, a1, a2 )		
	#define LOG4_L3( str, a0, a1, a2, a3 )	

#else

	#define LOG0_L1( str )					
	#define LOG1_L1( str, a0 )				
	#define LOG2_L1( str, a0, a1 )			
	#define LOG3_L1( str, a0, a1, a2 )		
	#define LOG4_L1( str, a0, a1, a2, a3 )	

	#define LOG0_L2( str )					
	#define LOG1_L2( str, a0 )				
	#define LOG2_L2( str, a0, a1 )			
	#define LOG3_L2( str, a0, a1, a2 )	
	#define LOG4_L2( str, a0, a1, a2, a3 )

	#define LOG0_L3( str )					
	#define LOG1_L3( str, a0 )				
	#define LOG2_L3( str, a0, a1 )			
	#define LOG3_L3( str, a0, a1, a2 )		
	#define LOG4_L3( str, a0, a1, a2, a3 )

#endif


	class Log
	{
	 public:
		typedef void (*OutputCB)( const char* );
				
	 private:
		FileStream	File;
		
		int			HtmlColor;
		
		OutputCB	OutputCallback;

		bool		DebuggerOutputEnabled;
		
	 public:
		Log();
		~Log();
		
		void EnableHTMLOutput( const char* _filename, bool _bAppend = false );
		
		void SetHTMLColor( int color );
		
		void EnableDebuggerOutput( bool bEnable );
		
		void SetOutputCallback( OutputCB cb );

		void printf( const char* format, ... );
	};

	extern Log g_log;

