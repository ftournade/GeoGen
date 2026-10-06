//#include "stdafx.h"
#include "Debug.h"

#include "StandardLib.h"
#include <Core/Log.h>

#include <signal.h>

#include "UIHelper.h"


namespace xtm
{

	bool DbgOpenCheckDialogBox(	const char* cond,
											const char* txt,
											bool& bDontBreakAnymore,
											const char* file,
											s32 line,
											const char* function )
	{
		const s32 textSize = 4096;

		char text[textSize];

	#ifdef XTM_WIN32
		const s32 winErrorSize = 256;

		TCHAR winError[winErrorSize];

		winError[0] = '\0';

		FormatMessage(	FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
						GetUserDefaultLangID(), winError, winErrorSize, NULL );


		Xsprintf(	text,	"%s(%d):\nAssertion failed in %s\n\n%s\n\n%s\n\nLast Windows error : %s",
					file, line, function, cond, txt, winError );
	#else
		Xsprintf(	text,	"%s(%d):\nAssertion failed in %s\n%s\n%s",
					file, line, function, cond, txt );

	#endif

	#ifdef XTM_WIN32
		CREATESIMPLEMESSAGEBOX csmb;

		csmb.Title = "Assertion failed";
		csmb.Text = text;
		csmb.Buttons.push_back( "Break" );
		csmb.Buttons.push_back( "Ignore" );
		csmb.Buttons.push_back( "Ignore always" );
		csmb.Buttons.push_back( "Quit" );

		if( !CreateSimpleMessageBox( csmb, 500, 400 ) )
			return true;

		switch( csmb.ButtonClicked )
		{

			case 0: //Break
				return true;

			case 1: //Ignore
				return false;

			case 2: //Ignore always
				bDontBreakAnymore = true;
				return false;

			case 3: //Quit
				raise( SIGABRT );
				exit(3);
				return true;
		}
	#endif
		return false;
	}

}

