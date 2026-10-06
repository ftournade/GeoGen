#pragma once
#ifndef XTM_STANDARD_LIB_H
#define XTM_STANDARD_LIB_H



#include <Core/Utility.h>

//#include <stdio.h>
//#include <stdarg.h>
//#include <string.h>//MACOSX only ?

namespace xtm
{
	
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
		#define SAFE_RELEASE(p) if(p) { (p)->Release(); (p)=NULL; }
	#endif
	
	#ifndef XTM_WIN32
		inline void ZeroMemory( void* _ptr, size_t _size ) 
		{
			memset( _ptr, 0, _size );
		}
	#endif

	typedef size_t		XSize;

#if 0
	inline	void		RandomSeed( u32 seed )									{ srand( seed ); }

	template <class T>
	inline	T			Random( T min, T max )									{ float r = (T)rand() / (T)RAND_MAX; return Lerp( min, max, r ); }
#else
	extern s32 xtmRandSeed;

	inline	void		RandomSeed( u32 seed )									{ xtmRandSeed = seed; }

	union unionFloatU32
	{
		float f;
		u32 i;
	};

	float Random();
	/*
	inline	float		Random()
	{
		xtmRandSeed *= 16807;
		u32 a = (xtmRandSeed & 0x007fffff) | 0x40000000;
		unionFloatU32 v;
		v.f = ( *((float*)&a) - 3.0f );
		v.i &= 0x7fffffff;//TODO bof [-1,1] => [0,1] (fabs)
		return v.f;
	}
	 */

	/*
	From inigo quilez

	float sfrand( int *seed )
	{
		float res;

		seed[0] *= 16807;

		*((unsigned int *) &res) = ( ((unsigned int)seed[0])>>9 ) | 0x40000000;

		return( res-3.0f );
	}
	*/

	template <class T>
	inline	T			Random( T min, T max )										{ return (T)(Random() * (T)(max - min) + (T)min); }

	//template <class T>
	inline	int			Random( int min, int max )									{ return (min == max) ? min : rand() % (max - min + 1) + min; }

#endif

	struct RandomFloat
	{
		float m_Average, m_Variance;

		inline float Random()
		{
			return xtm::Random( m_Average - m_Variance, m_Average + m_Variance );
		}
	};

	void Xstrcpy( char* _dest, u32 _destSize, const char* _src );

#ifdef XTM_WIN32
	#define				Xstricmp				_stricmp
	#define				Xstrncmp				strncmp
	#define				Xsprintf				sprintf
	#define				Xsnprintf				_snprintf
	#define				Xvsnprintf				_vsnprintf_s
	#define				Xsscanf					sscanf_s
	//#define			Xstrcpy					strcpy_s
	#define				Xstrtok					strtok_s
#else
	#define				Xstricmp				strcmp //TODO code stricmp
	#define				Xstrncmp				strncmp
	#define				Xsprintf				sprintf
	#define				Xsnprintf				snprintf
	#define				Xvsnprintf				vsnprintf
	#define				Xsscanf					sscanf
	//#define			Xstrcpy( a, b, c )		strcpy( a, c )
	#define				Xstrtok( a, b, c )		strtok( a, b )
#endif

	#define				Xprintf		printf

	#define				Xstrlen		strlen

	#define				Xstrcat		strcat

	#define				Xstrcmp		strcmp

	#define				Xstrstr		strstr

	#define				Xatoi		atoi

	#define				Xatof		(float)atof

	#define				Xatod		atof

	#define				Xmemcpy		memcpy

	#define				Xmemcmp		memcmp

	//#define			Xisdigit	isdigit
	inline		bool	IsDigit( char c )	{ return ( c >= '0' ) && ( c <= '9' ); }

	#define				Xisalpha	isalpha

	#define				Xisalnum	isalnum

	#define				Xtolower	tolower
	
	#define				Xtoupper	toupper

	
	//E.g.: 1.45000 becomes 1.45
	void RemoveTrailingZeros( char* _text );

	//E.g.: 2048 becomes "2.000 Kb"
	void FormatSizeInBytes( size_t _size, char* _result, u32 _bufferSize );

	void Sleep( float _timeInSeconds );
}

#endif
