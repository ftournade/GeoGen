//#include "stdafx.h"
#include "Log.h"

#include "StandardLib.h"
#include <Core/Debug.h>
//TODO make it portable

//#include <Windows.h>


    Log g_log;

	Log::Log() :	OutputCallback(NULL),
					DebuggerOutputEnabled(false)
	{
	}

	Log::~Log()
	{
	}


	void Log::SetHTMLColor( int color )
	{
		if( File.IsOpen() && ( color != HtmlColor ) )
		{

			char buffer[128];

			Xsprintf( buffer, "</FONT>\n<FONT color=\"#%.6x\">\n", color );

			File.WriteBytes( (const byte*)buffer, Xstrlen( buffer ) );

			HtmlColor = color;
		}
	}

	void Log::printf( const char* format, ... )
	{
		const s32 bufferSize = 8192;

		char buffer[bufferSize];
		char buffer2[bufferSize];


		va_list args;

		va_start(args, format);
		Xvsnprintf( buffer, bufferSize, format, args );
		va_end(args);

		if( File.IsOpen() )
		{
			//Str str;
			//str.format( "%s<br>\n", buffer );
			Xsnprintf( buffer2, bufferSize, "%s<br>\n", buffer );
			File.WriteBytes( (const byte*)buffer2,  Xstrlen(buffer2) );
			File.Flush();
		}

		if( DebuggerOutputEnabled )
		{
			#ifdef XTM_WIN32
				Xsnprintf( buffer2, bufferSize, "%s\n", buffer );
				OutputDebugStringA( buffer2 );
			#else
				::printf( "%s\n", buffer );
			#endif
		}

		if( OutputCallback )
			OutputCallback(buffer);

	}

	void Log::EnableHTMLOutput( const char* _filename, bool _bAppend )
	{
		File.Close();

		if( !File.Open( _filename, _bAppend ? "a+" : "w" ) )
		{
			DBG_CHECK( false );
			return;
		}

		char buffer[]="<FONT color=\"#000000\">\n";

		File.WriteBytes( (const byte*)buffer, sizeof(buffer)/sizeof(buffer[0]) );

	}

	void Log::EnableDebuggerOutput( bool bEnable )
	{
		DebuggerOutputEnabled = bEnable;
	}

	void Log::SetOutputCallback( OutputCB cb )
	{
		OutputCallback = cb;
	}

