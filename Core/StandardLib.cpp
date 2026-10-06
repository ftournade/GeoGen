//#include "stdafx.h"
#include "StandardLib.h"

#if defined(XTM_IPHONE) || defined(XTM_LINUX)
	#include <unistd.h>
#elif defined(XTM_MACOSX)
	#include <CoreServices/CoreServices.h>
#endif

#include <Core/Debug.h>

	//u32 xtmRandSeed = 69;
	s32 xtmRandSeed = 69;

	float Random()
	{
		#if 0
			xtmRandSeed *= 16807;
			u32 a = (xtmRandSeed & 0x007fffff) | 0x40000000;
			unionFloatU32 v;
			v.f = ( *((float*)&a) - 3.0f );
			v.i &= 0x7fffffff;//TODO bof [-1,1] => [0,1] (fabs)
			return v.f;
		#else
			xtmRandSeed = (214013 * xtmRandSeed + 2531011);
			s32 r = (xtmRandSeed >> 16) & 0x7FFF;
		//printf( "RAND %d\n", r );
			float f = (float)r * (1.0f / 32767.0f);
			return f;
		#endif
	}

	void Xstrcpy( char* _dest, u32 _destSize, const char* _src )
	{
		DBG_CHECK( _src && _dest );
		DBG_CHECK( ( _src >= _dest + _destSize ) || ( _src < _dest ) );

		u32 available = _destSize;
		
		char* dst = _dest;

		while( ((*dst++ = *_src++) != '\0') && (--available > 0) )
		{
			DBG_CHECK( ( _src >= _dest + _destSize ) || ( _src < _dest ) );
		}

		_dest[ _destSize - 1 ] = '\0';
	}

	void FormatSizeInBytes( size_t _size, char* _result, u32 _bufferSize )
	{
		const u32 unitCount = 4; //5

		static const char unit[unitCount][6]=
		{
			"bytes",
			"Kb",
			"Mb",
			"Gb",
			//"Tb"
		};

		static const size_t unitSize[]=
		{	1024,
			1024*1024,
			1024*1024*1024,
			//1024*1024*1024*1024,
			//1024*1024*1024*1024*1024
		};

		if( _size < 1024 )
		{
			Xsprintf( _result, "%zd %s", _size ,unit[0] );
			return;
		}

		for( u32 i=1 ; i < unitCount - 1 ; ++i )
		{
			if( _size < unitSize[i] )
			{
				Xsprintf( _result, "%.3f %s", (float)_size / (float)unitSize[i-1] ,unit[i] );
				return;
			}
		}

		Xsprintf( _result, "%.3f %s", (float)_size / (float)unitSize[unitCount - 2] ,unit[unitCount - 1] );
	}

	void RemoveTrailingZeros( char* _text )
	{
		s32 i=0,
			cutPos = -1,
			periodPos = -1;

		while( true )
		{
			char c = _text[i];

			if( c == '\0' )
				return;
			else if( c == '.' )
			{
				periodPos = i++;
				break;
			}

			++i;
		}


		while( true )
		{
			char c = _text[i];

			if( c == '\0' )
				break;
			else if( c != '0' )
				cutPos = i + 1;

			++i;
		}

		if( periodPos != -1 )
		{
			if( cutPos != -1 )
				_text[ cutPos ] = '\0';
			else if( periodPos + 2 < i )
				_text[ periodPos + 2 ] = '\0';

		}

	}

	void SleepSeconds( float _timeInSeconds )
	{
		#if defined(XTM_IPHONE) || defined(XTM_LINUX) || defined(XTM_MACOSX)
			//TODO on iPhone use [NSThread sleepForTimeInterval:(NSTimeInterval)_timeInSeconds ] ??
		
			u32 usec = (u32)(_timeInSeconds * 1000000.0f);
			int res = usleep( usec );
		//#elif defined(XTM_MACOSX)
		//	UInt32 finalCount;
		//	Delay( (UInt32)(_timeInSeconds * 60.0f), &finalCount );
		#elif defined(XTM_WIN32)
			::Sleep( (DWORD)(_timeInSeconds * 1000.0f) );
		#else
			#error "Implement Sleep"
		#endif
	}
