#pragma once


#include "BasicTypes.h"

#include <stdio.h>

namespace xtm
{

	bool DbgOpenCheckDialogBox(	const char* condition,
											const char* msg,
											bool& bDontBreak,
											const char* filename,
											s32 line,
											const char* function );

}

#undef BREAK

#if defined(XTM_WIN32)
	#define BREAK		DebugBreak();
	//#define BREAK		__asm { int 3 };
#elif defined(XTM_IPHONE)
	#define BREAK		pthread_kill( pthread_self(), SIGINT );
#elif defined(XTM_MACOSX)
	//#define BREAK		asm {trap}            //PPC32 or PPC64.
	//#define BREAK		__asm {int 3}         //IA-32.
	#define BREAK		Debugger();
#else
	#define BREAK		while(true) {}
#endif

#undef NOT_YET_IMPLEMENTED
#define NOT_YET_IMPLEMENTED BREAK

#if !defined(_GCC)
	#define __func__ ""
#endif

#define CHECK_MSG( condition, m ) \
{ \
	static bool bDontBreak = false; \
	if( !(condition) && !bDontBreak && xtm::DbgOpenCheckDialogBox( #condition, m, bDontBreak, __FILE__, __LINE__, __func__ ) )	\
	{ BREAK } \
}

#define CHECK_MSG1( condition, m, val1 ) \
{ \
	static bool bDontBreak = false; \
	if( !(condition) && !bDontBreak ) \
	{ \
		char _dbgBuffer[1024]; \
		Xsnprintf( _dbgBuffer, 1024, m, val1 ); \
		if( xtm::DbgOpenCheckDialogBox( #condition, _dbgBuffer, bDontBreak, __FILE__, __LINE__, __func__ ) ) \
		{ BREAK } \
	} \
}

#define CHECK( condition ) CHECK_MSG( condition, 0 )


#ifdef _DEBUG
	#define DBG_BREAK						BREAK
	#define DBG_CHECK_MSG					CHECK_MSG
	#define DBG_CHECK_MSG1					CHECK_MSG1
	#define DBG_CHECK						CHECK
#else
	#define DBG_BREAK						((void)0)
	#define DBG_CHECK_MSG( condition, m )	((void)0)
	#define DBG_CHECK_MSG1( condition, m, v1 )	((void)0)
	#define DBG_CHECK( condition )			((void)0)
#endif

#ifdef _DEBUG
	#define MONITOR_FLOAT_BOUNDS( val, name, skip ) \
		static float name##MinBound = 10000000.0f; \
		static float name##MaxBound = -10000000.0f; \
		static u32 name##Skip = 0; \
		name##MinBound = Min( val, name##MinBound ); \
		name##MaxBound = Max( val, name##MaxBound ); \
		if( ++name##Skip > skip ) \
		{ name##Skip = 0; printf( #name " bounds are [%f %f]\r\n", name##MinBound, name##MaxBound ); }
#else
	#define MONITOR_FLOAT_BOUNDS( val, name, skip)
#endif
